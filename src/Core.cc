#include "Poco/StreamCopier.h"
#include "Poco/DeflatingStream.h"
#include "Poco/InflatingStream.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Exception.h"

#include "Application.h"
#include "Store.h"
#include "Core.h"
#include "Version.h"
#include "Utils.h"

using Poco::Mutex;
using Poco::DeflatingOutputStream;
using Poco::DeflatingStreamBuf;
using Poco::StreamCopier;
using Poco::URI;
using Poco::Exception;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::InflatingInputStream;
using Poco::InflatingStreamBuf;

Poco::Mutex Core::connect_mutex;
Poco::Mutex Core::post_to_core_mutex;
Poco::Timestamp Core::connect_timestamp;

bool Core::stalled() {
    connect_mutex.lock();
    Poco::Timestamp ts;
    connect_mutex.unlock();

    if ((ts - connect_timestamp) > 299999999) {
        return true;
    }

    return false;
}


void Core::CoreForm::init() {
    Store::AgentInfo ai;
    Store::getAgentInfo(ai);

    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());
    poco_debug(app.logger(), "CoreForm::init");

    if (ai.passphrase.size() > 0) {
        set("id", ai.agent_id);
        payload_key = ai.passphrase;
    } else {
        LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());
        std::string s = app.getDaemonKey();
        set("api_key", s);
        payload_key = s;
    }
}

Core::CoreForm::CoreForm(Json::Value payload) {
    init();

    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    if (!payload.empty()) {


        try {
            Json::StreamWriterBuilder builder;
            builder.settings_["indentation"] = "";
            const std::string s = Json::writeString(builder, payload);
    

            if (s.length() <= 2000) {
                set("compressed", "false");
                set("payload", s);
            } else {
                std::istringstream is(s);
                std::ostringstream os;
                DeflatingOutputStream deflater(os, DeflatingStreamBuf::STREAM_GZIP);
                StreamCopier::copyStream(is, deflater, 2000);
                deflater.close();
    
                set("payload", os.str());
                set("compressed", "true");
            }
        } catch (Exception &ex) {
            poco_error_f1(app.logger(), "CoreForm failed: %s", ex.displayText());
        }
    }
}


Core::CoreForm::CoreForm(void) {
    init();
}


bool Core::postToCore(std::string& action, Core::CoreForm& form, std::string &resp_string) {
    post_to_core_mutex.lock();
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    poco_information_f1(app.logger(), "Calling core with action: %s", action);

    bool result = false;

    try {
        std::string grid;
        if (app.config().has("grid")) {
            grid = app.config().getString("grid");
        } else {
            grid = "http://localhost/grid";
        }
    
        Poco::URI uri(grid);
    
        if (!(uri.getScheme() == "http")) {
            // TODO TLS
        }
    
        Store::AgentInfo ai;
        Store::getAgentInfo(ai);
    
    
        // TODO DNS cache
    
        if (app.config().has("no-grid-ssl-verification")) {
            // TODO
        }
    
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_1);
        req.set("UserAgent", "LTE/" VERSION_STRING);
        req.set("X-LTE-AGENT-ID", ai.agent_id);

        form.prepareSubmit(req);
        std::ostream& ostr = session.sendRequest(req);
        form.write(ostr);
    
        Poco::Timestamp ts;
        connect_timestamp = ts;
    
        Poco::Net::HTTPResponse res;
        std::istream& rs = session.receiveResponse(res);
    
        Poco::Timestamp ts2;
        connect_timestamp = ts2;
    
        if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
            if (res.get("Content-Encoding", "") == "gzip") {
                InflatingInputStream inflater(rs, InflatingStreamBuf::STREAM_GZIP);
                Poco::StreamCopier::copyToString(inflater, resp_string);
            }
            result = true;
        } else {
            poco_error_f1(app.logger(), "Calling core with response: %d", (int)res.getStatus());
            result = false;
        }
    } catch (Exception &ex) {
        poco_error_f1(app.logger(), "Calling core failed: %s", ex.displayText());
        result = false;
    }

    post_to_core_mutex.unlock();

    return result;
}

bool Core::postToCore(std::string& action, std::string& payload) {
    std::string response;
    CoreForm form(payload);
    return postToCore(action, form, response);
}

bool Core::postToCore(std::string& action, const Json::Value& payload, Json::Value& response) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    CoreForm form(payload);

    std::string rawresp;

    bool r = postToCore(action, form, rawresp);
    if (r) {
        if (rawresp.compare("") != 0) {
            std::string errmsg;
            Json::CharReaderBuilder builder;
            const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            r = reader->parse(rawresp.c_str(), rawresp.c_str() + rawresp.length(), &response, &errmsg);
            if (!r) {
                poco_error_f2(app.logger(), "JSON parse of payload '%s' failed: %s", rawresp, errmsg);
            }
        }
    }
    return r;
}


void Core::call(std::string& action, Json::Value payload) {
    Json::Value response;
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    payload["created_at"] = Utils::utcNowStr();

    if (!postToCore(action, payload, response)) {
        poco_information_f1(app.logger(), "save message for action: '%s'", action);
        Store::saveMessage(action, payload);
    }
}


void Core::event(const std::string& type, const std::string& message, const std::string& detail) {
    Json::Value root;

    root["type"] = type;
    root["message"] = message;
    root["detail"] = detail;

    std::string action = "event";
    call(action, root);
}

void Core::warn(const std::string& message, const std::string& detail) {
    std::string type = "warn";

    event(type, message, detail);
}

void Core::error(const std::string& message, const std::string& detail) {
    std::string type = "error";

    event(type, message, detail);
}

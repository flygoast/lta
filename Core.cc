#include "Poco/StreamCopier.h"
#include "Poco/DeflatingStream.h"
#include "Poco/InflatingStream.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

#include "Application.h"
#include "Store.h"
#include "Core.h"
#include "Version.h"

using Poco::Mutex;
using Poco::DeflatingOutputStream;
using Poco::DeflatingStreamBuf;
using Poco::StreamCopier;
using Poco::URI;
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

    if (payload.empty()) {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "";
        const std::string s = Json::writeString(builder, payload);

        if (s.length() <= 2000) {
            set("compressed", "false");
            set("payload", payload.asString());
        } else {
            std::istringstream is(payload.asString());
            std::ostringstream os;
            DeflatingOutputStream deflater(os, DeflatingStreamBuf::STREAM_GZIP);
            StreamCopier::copyStream(is, deflater, 2000);
            deflater.close();

            set("payload", os.str());
            set("compressed", "true");
        }
    }
}


Core::CoreForm::CoreForm(void) {
    init();
}


bool Core::postToCore(std::string& action, Core::CoreForm& form, std::string *resp_string) {
    post_to_core_mutex.lock();
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    poco_information_f1(app.logger(), "Calling core with action: %s", action);

    std::string grid;
    if (app.config().has("grid")) {
        grid = app.config().getString("grid");
    } else {
        grid = "http://localhost/grid";
    }

    Poco::URI uri(grid);

    if (!(uri.getScheme() == "http")) {
        // TODO
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
            Poco::StreamCopier::copyToString(inflater, *resp_string);
        }
    }

    post_to_core_mutex.unlock();

    return true;
}


std::string Core::postToCore(std::string& action) {
    CoreForm form;
    return postToCore(action, form);
}

std::string Core::postToCore(std::string& action, std::string& payload) {
    CoreForm form(payload);
    return postToCore(action, form);
}



std::string Core::call(std::string& action, Json::Value& payload) {
    Json::Value response;

    if (postToCore(action, payload, &response)) {

    }


}

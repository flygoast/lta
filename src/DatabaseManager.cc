#include "Poco/File.h"
#include "Poco/Data/SQLite/Connector.h"

#include "Application.h"
#include "DatabaseManager.h"

using namespace Poco::Data::Keywords;

using Poco::Util::Application;
using Poco::File;
using Poco::Data::Session;
using Poco::Data::Statement;

DatabaseManager::DatabaseManager(void) {
    pSession = NULL;
}

DatabaseManager::~DatabaseManager(void) {
    // TODO
}


bool DatabaseManager::initialized(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Application::instance());

    Poco::File file(app.store_file);

    if (!file.exists() || !file.isFile()) {
        return false;
    }

    return true;
}


std::string DatabaseManager::dbPath(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Application::instance());

    return app.store_file;
}


std::string DatabaseManager::dbVectorPath(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Application::instance());

    std::string s = app.store_file;
    s.append(".vector");

    return s;
}


bool DatabaseManager::encrypted(void) {
    Poco::File file(dbVectorPath());

    if (!file.exists() || !file.isFile()) {
        return false;
    }

    return true;
}


Session *DatabaseManager::session(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Application::instance());
    
    bool inited = false;

    if (!pSession) {

        inited = initialized();

        Poco::Data::SQLite::Connector::registerConnector();
        
        pSession = new Session("SQLite", dbPath());

        if (pSession == NULL) {
            app.logger().error("Session failed");
        }

        *pSession << "create table if not exists messages(id interger primary key, target text, payload text, created_at text)", now;

        *pSession << "create table if not exists agent_info(id integer primary key, agent_id text, passphrase text, grid text default null, tag text default null, "
            "proxy_host text, proxy_port integer, proxy_user text, proxy_password text, server_label text, dns text, read_only integer, enable_log_rotation integer, max_log_size integer, max_log_files integer)"
            , now;

        *pSession << "create table if not exists key_value_pairs(key text primary key not null, value text, expires text default null)", now;
        *pSession << "create index if not exists expires_idx on key_value_pairs(expires)", now;
    }

    return pSession;
}

void DatabaseManager::closeSession(void) {
    if (pSession) {
        delete pSession;
        pSession = NULL;
    }
}


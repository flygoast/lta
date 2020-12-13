#include "Store.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Statement;
using Poco::Data::Session;

std::string empty;
DatabaseManager     Store::db;


Store::AgentInfo::AgentInfo(void) {
    agent_id = empty;
    passphrase = empty;
    grid = empty;
    tag = empty;
    proxy_host = empty;
    proxy_port = 0;
    proxy_user = empty;
    proxy_password = empty;
    server_label = empty;
    dns = empty;
    read_only = false;
    enable_log_rotation = false;
    max_log_size = 1;
    max_log_files = 10;
}

Store::AgentInfo::~AgentInfo(void) {
}


void Store::getAgentInfo(Store::AgentInfo& ai) {
    db.mutex.lock();

    Session *pSession = db.session();
    *pSession << "select agent_id, passphrase, grid, tag, proxy_host, proxy_port, proxy_user, proxy_password, server_label, dns, read_only, enable_log_rotation, max_log_size, max_log_files from agent_info limit 1",
        into(ai.agent_id),
        into(ai.passphrase),
        into(ai.grid),
        into(ai.tag),
        into(ai.proxy_host),
        into(ai.proxy_port),
        into(ai.proxy_user),
        into(ai.proxy_password),
        into(ai.server_label),
        into(ai.dns),
        into(ai.read_only),
        into(ai.enable_log_rotation),
        into(ai.max_log_size),
        into(ai.max_log_files),
        now;

    db.mutex.unlock();
}


void Store::setAgentInfo(Store::AgentInfo& ai) {
    db.mutex.lock();

    Session *pSession = db.session();

    *pSession << "delete from agent_info", now;

    Statement statement(*pSession);

    statement << "insert into agent_info(agent_id, passphrase, grid, tag, proxy_host, proxy_port, proxy_user, proxy_password, server_label, dns, read_only, enable_log_rotation, max_log_size, max_log_files) values "
        "(:agent_id, :passphrase, :grid, :tag, :proxy_host, :proxy_port, :proxy_user, :proxy_password, :server_label, :dns, :read_only, :enable_log_rotation, :max_log_size, :max_log_files)",
        use(ai.agent_id),
        use(ai.passphrase),
        use(ai.grid),
        use(ai.tag),
        use(ai.proxy_host),
        use(ai.proxy_port),
        use(ai.proxy_user),
        use(ai.proxy_password),
        use(ai.server_label),
        use(ai.dns),
        use(ai.read_only),
        use(ai.enable_log_rotation),
        use(ai.max_log_size),
        use(ai.max_log_files),
        now;

    db.mutex.unlock();
}


void Store::closeSession(void) {
    db.mutex.lock();
    db.closeSession();
    db.mutex.unlock();
}


void Store::deleteExpireRestartKeys(void) {
    db.mutex.lock();
    Session *pSession = db.session();
    *pSession << "delete from key_value_pairs where expires = 'restart'", now;
    db.mutex.unlock();

    deleteExpireKeys();
}


void Store::deleteExpireKeys(void) {
    db.mutex.lock();
    Session *pSession = db.session();
    *pSession << "delete from key_value_pairs where datetime(expires) <= datetime('now')", now;

    db.mutex.unlock();
}

void Store::saveMessage(const std::string& action, const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    const std::string jsonstr = Json::writeString(builder, value);

    db.mutex.lock();

    Session *pSession = db.session();

    Statement statement(*pSession);

    std::string a = action;
    std::string j = jsonstr;
    statement << "insert into message(target, payload) values(:target, :payload)",
        use(a),
        use(j),
        now;

    db.mutex.unlock();
}

#include "Store.h"

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
        Poco::Data::into(ai.agent_id),
        Poco::Data::into(ai.passphrase),
        Poco::Data::into(ai.grid),
        Poco::Data::into(ai.tag),
        Poco::Data::into(ai.proxy_host),
        Poco::Data::into(ai.proxy_port),
        Poco::Data::into(ai.proxy_user),
        Poco::Data::into(ai.proxy_password),
        Poco::Data::into(ai.server_label),
        Poco::Data::into(ai.dns),
        Poco::Data::into(ai.read_only),
        Poco::Data::into(ai.enable_log_rotation),
        Poco::Data::into(ai.max_log_size),
        Poco::Data::into(ai.max_log_files),
        Poco::Data::now;

    db.mutex.unlock();
}


void Store::setAgentInfo(Store::AgentInfo& ai) {
    db.mutex.lock();

    Session *pSession = db.session();

    *pSession << "delete from agent_info", Poco::Data::now;

    Statement statement(*pSession);

    statement << "insert into agent_info(agent_id, passphrase, grid, tag, proxy_host, proxy_port, proxy_user, proxy_password, server_label, dns, read_only, enable_log_rotation, max_log_size, max_log_files) values "
        "(:agent_id, :passphrase, :grid, :tag, :proxy_host, :proxy_port, :proxy_user, :proxy_password, :server_label, :dns, :read_only, :enable_log_rotation, :max_log_size, :max_log_files)",
        Poco::Data::use(ai.agent_id),
        Poco::Data::use(ai.passphrase),
        Poco::Data::use(ai.grid),
        Poco::Data::use(ai.tag),
        Poco::Data::use(ai.proxy_host),
        Poco::Data::use(ai.proxy_port),
        Poco::Data::use(ai.proxy_user),
        Poco::Data::use(ai.proxy_password),
        Poco::Data::use(ai.server_label),
        Poco::Data::use(ai.dns),
        Poco::Data::use(ai.read_only),
        Poco::Data::use(ai.enable_log_rotation),
        Poco::Data::use(ai.max_log_size),
        Poco::Data::use(ai.max_log_files),
        Poco::Data::now;

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
    *pSession << "delete from key_value_pairs where expires = 'restart'", Poco::Data::now;
    db.mutex.unlock();

    deleteExpireKeys();
}


void Store::deleteExpireKeys(void) {
    db.mutex.lock();
    Session *pSession = db.session();
    *pSession << "delete from key_value_pairs where datetime(expires) <= datetime('now')", Poco::Data::now;

    db.mutex.unlock();
}

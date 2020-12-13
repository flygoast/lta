#ifndef __STORE_H__
#define __STORE_H__

#include "Poco/Mutex.h"

#include "json/json.h"

#include "DatabaseManager.h"


//class Store;
//class Store::AgentInfo;

class Store {
public:
    class AgentInfo {
    public:
        AgentInfo(void);
        ~AgentInfo(void);

        std::string     agent_id;
        std::string     passphrase;
        std::string     grid;
        std::string     tag;
        std::string     proxy_host;
        unsigned        proxy_port;
        std::string     proxy_user;
        std::string     proxy_password;
        std::string     server_label;
        std::string     dns;
        bool            read_only;
        bool            enable_log_rotation;
        long            max_log_size;
        long            max_log_files;
    };

    static void closeSession();
    static void getAgentInfo(Store::AgentInfo& ai);
    static void setAgentInfo(Store::AgentInfo& ai);
    static void deleteExpireRestartKeys();
    static void deleteExpireKeys();
    static void saveMessage(const std::string& action, const Json::Value& value);

private:
    static DatabaseManager  db;
};


#endif /* __STORE_H__ */

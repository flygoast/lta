#ifndef __CORE_H__
#define __CORE_H__

#include "Poco/Net/HTMLForm.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"

#include "json/json.h"

using Poco::Net::HTMLForm;

class Core {
public:
    Core(void);
    ~Core(void) {};

    class CoreForm: public HTMLForm {
    public:
        CoreForm(void);
        CoreForm(Json::Value payload);
        ~CoreForm(void) {}
        void init(void);

        std::string     payload_key;
    };

    static bool stalled(); 
    static void call(std::string& action, const Json::Value& payload);
    static bool postToCore(std::string& action, Core::CoreForm& form, std::string &resp_string);
    static bool postToCore(std::string& action, const Json::Value& payload, Json::Value& response);
    static bool postToCore(std::string& action, std::string& payload);

    static Poco::Mutex connect_mutex;
    static Poco::Mutex post_to_core_mutex;
    static Poco::Timestamp connect_timestamp;
};


#endif /* __CORE_H__ */

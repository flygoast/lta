#ifndef __UTILS_H__
#define __UTILS_H__

#include "json/json.h"
#include "Poco/Path.h"

using Poco::Path;

class Utils {
public:
    static std::string daemonFile(const std::string& dir, const std::string& file);
    static Poco::Path installDir(void);

    static Json::Value networkInterfaces(void);
    static std::string fqdn(void);
    static std::string nodeName(void);
    static bool firewallPresent(void);
    static std::string osDisplayName(void);
    static std::string machine(void);
    static std::string utcNowStr(void);
};

#endif /*  __UTILS_H__ */

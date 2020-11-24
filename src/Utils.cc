#include <iostream>

#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Environment.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/DNS.h"
#include "Poco/Path.h"
#include "Poco/File.h"

#include "Utils.h"

using Poco::Path;
using Poco::Net::NetworkInterface;
using Poco::Net::IPAddress;
using Poco::Net::DNS;
using Poco::DateTime;
using Poco::DateTimeFormatter;

Poco::Path Utils::installDir(void) {
    Poco::Path path("/opt/lte");
    return path;
}

std::string Utils::daemonFile(const std::string& dir, const std::string& file) {
    Path base = installDir();
    if (file.size() > 0) {
        Path pDir(dir);
        base.append(pDir);
    }

    Path pFile(file);
    base.append(pFile);
    return base.toString();
}


Json::Value Utils::networkInterfaces(void) {
    Json::Value root;
    NetworkInterface::NetworkInterfaceList ifs = NetworkInterface::list();

    for (NetworkInterface::NetworkInterfaceList::const_iterator it = ifs.begin(); it != ifs.end(); ++it) {
        Json::Value data;

        data["name"] = it->name();
        
        IPAddress address = it->address();

        data["address"] = address.toString();
        data["mask"] = it->subnetMask().toString();
        data["display_name"] = it->displayName();

        root.append(data);
    }

    return root;
}


std::string Utils::fqdn(void) {
    return DNS::thisHost().name();
}


std::string Utils::nodeName(void) {
    std::string s =  Poco::Environment::nodeName();
    long pos = s.find('.');

    return s.substr(0, pos);
}


bool Utils::firewallPresent(void) {
    Poco::File file("/sbin/iptables");
    return file.exists();
}


std::string Utils::osDisplayName(void) {
    return Poco::Environment::osName();
}


std::string Utils::machine(void) {
    return Poco::Environment::osArchitecture();
}


std::string Utils::utcNowStr(void) {
    std::string s;
    s.reserve(0x40);

    Poco::DateTime dt;
    DateTimeFormatter::append(s, dt, "%Y-%m-%d %H:%M:%S.%F");
    return s;
}

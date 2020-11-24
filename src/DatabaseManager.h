#ifndef __DATABASEMANAGER_H__
#define __DATABASEMANAGER_H__

#include "Poco/Mutex.h"
#include "Poco/Data/Session.h"

using Poco::Mutex;
using Poco::Data::Session;

class DatabaseManager {
public:
    DatabaseManager(void);
    ~DatabaseManager(void);

    std::string dbPath(void);
    std::string dbVectorPath(void);

    bool initialized(void);
    bool encrypted(void);

    void closeSession(void);
    Session *session(void);

    Poco::Mutex     mutex;

private:    
    Session        *pSession;
    
};


#endif /* __DATABASEMANAGER_H__ */

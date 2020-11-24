#ifndef __GENERICTASK_H__
#define __GENERICTASK_H__

#include <iostream>

class GenericTask {
public:
    static void processScript(std::string& command);
    static void processExecutableScript(std::string& command);
    static void processLibraryScript(std::string& command);

    static bool initialize(void);

    static void executeCommand(std::string& command);
    static void reportMessages();
    static void heartbeat();
    static void shutdown();
};

#endif /* __GENERICTASK_H__ */

#ifndef __MAINLOOP_H__
#define __MAINLOOP_H__

#include "Poco/Util/Application.h"
#include "Poco/Task.h"

using Poco::Task;

class MainLoop: public Task {
public:
    MainLoop(): Task("MainLoop"){}
    ~MainLoop(){}

    void runTask(void);

    bool initialize(void);
    void startScheduledTasks(void);
};

#endif /* __MAINLOOP_H__ */

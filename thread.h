#ifndef THREAD_H
#define THREAD_H

#include <stdlib.h>
#include <time.h>
#include <list>
#include <iostream>
#include <string>
#include <thread>
#include "Blockable.h"
void ThreadFunction(void * me);
class Thread
{
    friend void ThreadFunction(void * me);      // Allows ThreadFunction() to access private and protected members of this class
private:
	std::thread theThread;
    int exitTimeout;

protected:
    Sync::Event terminationEvent;

public:
    Thread(int exitTimeout = 1000);     // Constructor
    virtual ~Thread();                  // Destructor (can be overridden in subclasses)

    // Override this function in your threads
    virtual long ThreadMain(void) = 0;
};

#endif // THREAD_H

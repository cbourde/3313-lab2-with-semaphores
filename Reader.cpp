#include <iostream>
#include "SharedObject.h"
#include "Semaphore.h"

struct MyShared{
	int threadID;
	int reportNum;
	time_t timeSinceLastReport;
};

int main(void)
{
	// Declare shared memory object so the program can access it
	Shared<MyShared> sharedMemory("sharedMemory");

	// Get a pointer to the shared memory object
	MyShared* shared = sharedMemory.get();

	// Declare semaphores so the reader can access them
	Semaphore semSafeToWrite("SafeToWrite");	// Tells the WriterThreads when it is safe to write to the shared memory object
	Semaphore semAvailable("Available");		// Tells the Reader when a new report is available to read

	std::cout << "I am a reader" << std::endl;

	// Loops continuously until externally terminated (^C or other means)
	while (true)
	{
		semAvailable.Wait();		// Wait for a new report

		// Read report from shared memory object and output contents
		std::cout << "Thread: " << (shared -> threadID) << "; Report:" << (shared -> reportNum) << "; TimeDiff: " << (shared -> timeSinceLastReport) << std::endl;

		semSafeToWrite.Signal();	// Tell the WriterThreads that is is okay to write to the shared memory object

	}
}

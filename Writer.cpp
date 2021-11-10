#include <iostream>
#include <string>
#include <list>
#include "thread.h"
#include "SharedObject.h"
#include "Semaphore.h"

// Data structure for the shared memory object
struct MyShared{
	int threadID;
	int reportNum;
	time_t timeSinceLastReport;
};

class WriterThread : public Thread{
	public:
		int threadNum;		// Thread number: Assigned by the main function
		int reportNum;		// Report number: Starts at 0, increments each time this thread writes to the shared memory object
		int delaySeconds;	// Delay between writes to the shared memory object: Assigned by main()
		bool flag;			// Flag to terminate thread: Starts false, set to true once user terminates main program
		time_t prevReportTime;	// Timestamp of this thread's previous report

		// Declare semaphores so they can be used in ThreadMain()
		Semaphore semSafeToWrite = Semaphore("SafeToWrite");
		Semaphore semAvailable = Semaphore("Available");
		
		// Constructor
		WriterThread(int threadNum, int delay):Thread(8*1000){
			this->threadNum = threadNum;
			this -> reportNum = 0;
			this -> delaySeconds = delay;
			this -> prevReportTime = time(NULL);
		}

		virtual long ThreadMain(void) override{
			
			//declare shared memory var so this thread can access it
			Shared<MyShared> sharedMemory ("sharedMemory");

			while(true)
			{
				//write to shared memory
				MyShared* shared = sharedMemory.get();	// Get a pointer to the shared memory object

				semSafeToWrite.Wait();					// Wait until the Reader signals that it is safe to write to the shared memory object

				shared -> threadID = threadNum;			// Overwrite the thread ID in the shared object with this thread's ID
				shared -> reportNum = ++reportNum;		// Overwrite the report number in the shared object with this thread's report number

				time_t currentReportTime = time(NULL);	// The timestamp for this report
				shared -> timeSinceLastReport = currentReportTime - prevReportTime;	// Overwrite the time since last report (TSLR) with this thread's TSLR
				prevReportTime = currentReportTime;		// Set previous report time to the current report time.

				semAvailable.Signal();					// Signal to the Reader that a new report is available
				
				// End the loop (which will terminate the thread) if the flag is set
				if(flag){
					break;
				}

				// Wait the specified number of seconds before writing again
				sleep(delaySeconds);
			}
		}
};

int main(void)
{
	std::cout << "I am a Writer" << std::endl;
	Shared<MyShared> shared("sharedMemory", true); //Create shared memory object with this thread as the owner

	// Create semaphores with this thread as the owner of both
	Semaphore semSafeToWrite("SafeToWrite", 1, true);		// Tells a WriterThread when it is safe to write to the shared memory object
	Semaphore semAvailable("Available", 0, true);	// Tells the Reader when a new report is available to read

	int threadNum = 0;	// Stores current thread number
	int delay = 0;		// Stores user input for write delay

	// Create a list for pointers to all user-created threads (allows for unlimited threads to be created)
	std::list<WriterThread*> threadPtrs;
	
	// Loops continuously until user does not enter "y"
	while(true){

		// Ask if user wants to create another thread
		std::cout << "Create a new writer thread? [y/n]" << std::endl;
		std::string input = "";
		std::cin >> input;	

		// If user says yes, then ask how long the thread's write delay should be				
		if (input == "y" || input == "Y")
		{
			std::cout << "Enter write delay in seconds:" << std::endl;
			std::cin >> delay;

			// Create a new thread with the user-specified delay and the current thread number in the sequence,
			// then increment the thread number for the next thread
			WriterThread* thread = new WriterThread(threadNum++, delay);

			// Add the new thread to the list and skip to the beginning of the loop
			threadPtrs.push_front(thread);
			continue;
		}

		// If the execution reaches this point, it means the user did not enter "y" to create a new thread.
		// Therefore, the user input loop can be ended.
		break;		
	}
	//Terminate all threads
	while (threadPtrs.size() > 0)
	{
		WriterThread* thread = threadPtrs.front();	// Get first element of list
		thread -> flag = true;						// Set its flag to true so it ends its loop
		delete thread;								// Delete the thread from memory
		threadPtrs.pop_front();						// Remove the pointer from the list
	}
}
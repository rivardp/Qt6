#pragma once

#ifndef PTHREAD_H
#define PTHREAD_H

#include <windows.h>
#include <process.h>

class PThread{
	 
private:
	unsigned int threadID;		// Thread ID
	HANDLE hThread;				// Handle to the thread

public:
	// Variables
	bool threadCreated;	// Set to true if thread has been created
	bool end;			// Set this variable to true to force thread to end execution
	int  join_time;		// Number of milliseconds to wait for thread to join/start

	// Methods
	void initiateThread(void * pparams);

	// Constructor and Destructor
	PThread();
	~PThread();

protected:
	void *pparams;		// Pointer to param structure containing execution instructions

	// Methods
	DWORD run(void *pparams);
	static unsigned int WINAPI entryPoint(void*);	// Creates the thread
	virtual void setup();							// To be defined by derived class - initial setup
	virtual void execute(void *pparams);			// To be defined by derived class
	void* getParams() const; 						// Returns the pointer to the params structure
	void  setParams(void* pparams);					// Sets the pointer to the params structure
	void  stop();									// Hard stop to the thread
	void  waitForExecutionToEnd();					// Allow for main program to pause while waiting for thread to finish
	void  handleClose();							// Closes the handle

};

PThread::PThread() : threadCreated(false), end(false), join_time(100), threadID(0), hThread(0), pparams(0)
{}

PThread::~PThread()
{
}

void PThread::initiateThread(void *pparams)
{
	setParams(pparams);																	// store the pointer for the param structure
	hThread = (HANDLE)_beginthreadex(NULL, 0, entryPoint, (void *)this, 0, &threadID);	// sets threadID if successful - 4th parameter is entryPoint parameter
	if (threadID)
		threadCreated = true;
	else
	{
		wprintf(L"Thread creation failed\n");
		_exit(0);
	}
}

/* static */
unsigned int WINAPI PThread::entryPoint(void* pthis)		// pthis is really "this" passed from CreateThread as the 4th parameter
{
	PThread *pthread = (PThread*)pthis;
	return pthread->run(pthread->getParams());
}

DWORD PThread::run(void *pparams)
{
	execute(pparams);
	return DWORD(0);
}

void* PThread::getParams() const
{
	return pparams;
}

void PThread::setParams(void* params)
{
	pparams = params;
}

/* virtual - gets overriden */
void PThread::setup()
{
	// Insert any required setup code
}

/* virtual - gets overriden */
void PThread::execute(void* params)
{
	do{
		/// instructions
		end = true;
	} while (!end);	// "end" can be tripped by "stop" or completion of instructions
}

void PThread::stop()
{
	end = true;
}

void PThread::waitForExecutionToEnd()
{
	do{
		Sleep(join_time);		//Delay.
	} while (!end);        //Loops until Running is set to false.
}

void PThread::handleClose()
{
	if (hThread)
		CloseHandle(hThread);
}
#endif

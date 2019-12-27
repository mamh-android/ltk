/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFRWSem.h"
#include "STAFEventSem.h"
#include "STAFThreadManager.h"
#include "STAF_iostream.h"

const unsigned int NUM_READ_THREADS = 3;
const unsigned int NUM_WRITE_LOOPS = 3;

unsigned int readThread(void *);
unsigned int writeThread(void *);
STAFRWSem gRWSem;
STAFEventSem gEndSem;

int main(void)
{
    STAFThreadManager tm(2);

    for (int i = 0; i < NUM_READ_THREADS; ++i)
    {
        tm.dispatch(readThread, 0);
    }

    tm.dispatch(writeThread, 0);

    gEndSem.wait();

    return 0;
}

unsigned int readThread(void *)
{
    for (;;)
    {
        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " waiting for read lock" << endl;

        STAFRWSemRLock theLock(gRWSem);
//        gRWSem.readLock();

        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " acquired read lock and sleeping for 2 seconds" << endl;

        STAFThreadManager::sleepCurrentThread(2000);

        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " releasing read lock" << endl;

//        gRWSem.readUnlock();
    }

    return 0;
}


unsigned int writeThread(void *)
{
    for (int i = 0; i < NUM_WRITE_LOOPS; ++i)
    {
        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " waiting for write lock" << endl;

        STAFRWSemWLock lock(gRWSem);
//        gRWSem.writeLock();

        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " acquired write lock and sleeping for 5 seconds" << endl;

        STAFThreadManager::sleepCurrentThread(5000);

        cout << "Thread " << STAFThreadManager::getCurrentThreadID()
             << " releasing write lock" << endl;

//        gRWSem.writeUnlock();
    }

    gEndSem.post();

    return 0;
}

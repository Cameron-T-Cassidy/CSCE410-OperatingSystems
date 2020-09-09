/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "thread.H"
#include "scheduler.H" 

extern Scheduler * SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
void BlockingDisk::yieldCall() {
	// This function is called by the scheduler to poll

	if (!is_ready() && (queue.seek(Thread::CurrentThread()))) {
		//The current thread is IO and we're not ready
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread()); 
        SYSTEM_SCHEDULER->yield();
	}
	if (is_ready() == false)
		return;
	// Loop through threads until we find the head of queue
	Queue toGo = queue.dequeue();
	while (Thread::CurrentThread() != toGo.thread) {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread()); 
        SYSTEM_SCHEDULER->yield();		
	} 
	if (toGo.readOrWrite == 0) {
		//Complete the read
		int i;
		unsigned short tmpw;
		for (i = 0; i < 256; i++) {
			tmpw = Machine::inportw(0x1F0);
			toGo.buf[i*2]   = (unsigned char)tmpw;
			toGo.buf[i*2+1] = (unsigned char)(tmpw >> 8);
		}
	} 
	else {
		//Complete the write
		int i; 
		unsigned short tmpw;
		for (i = 0; i < 256; i++) {
			tmpw = toGo.buf[2*i] | (toGo.buf[2*i+1] << 8);
			Machine::outportw(0x1F0, tmpw);
		}
	}
	Queue nextQueue = queue.peek();
	if (nextQueue.thread == NULL)
		return;
	if (nextQueue.readOrWrite == 0) {
		// SimpleDisk functions only call the private member issue_operation
		// it has been edited and does NOT busy wait.
		SimpleDisk::read(nextQueue.block_no, nextQueue.buf);
	}
	else {
		// SimpleDisk functions only call the private member issue_operation
		// it has been edited and does NOT busy wait.
		SimpleDisk::write(nextQueue.block_no, nextQueue.buf);
	}	
}
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	// This function initiates the read or (resumes the thread, yields, adds it to queue)
	if (queue.thread == NULL) {
		// Nobody is waiting for IO, init the read
		
		// SimpleDisk functions only call the private member issue_operation
		// it has been edited and does NOT busy wait.
		SimpleDisk::read(_block_no, _buf);
	}
	Thread * toGo = Thread::CurrentThread();
	queue.enqueue(toGo, _block_no, _buf, 0);
	SYSTEM_SCHEDULER->resume(toGo); 
	SYSTEM_SCHEDULER->yield();		
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
	// This function initiates the write or (resumes the thread, yields, adds it to queue)
	
	if (queue.thread == NULL) {
		// Nobody is waiting for IO, init the write
		
		// SimpleDisk functions only call the private member issue_operation
		// it has been edited and does NOT busy wait.
		SimpleDisk::write(_block_no, _buf);
	}
	Thread * toGo = Thread::CurrentThread();
	queue.enqueue(toGo, _block_no, _buf, 1);
	SYSTEM_SCHEDULER->resume(toGo); 
	SYSTEM_SCHEDULER->yield();	
}

/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
//#include "simple_disk.C"
//# include "blocking_disk.C"
class BlockingDisk {
	public:
	void yieldCall();
};
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
	Node * head = NULL;
	Node * tail = NULL;
	Node * curr = NULL;
	extern Scheduler * SYSTEM_SCHEDULER;
	extern BlockingDisk * SYSTEM_DISK;
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/
void yielder () {
	while (1) {
		SYSTEM_SCHEDULER -> yield();
	}
}
Scheduler::Scheduler() {
 	Node * head = NULL;
	Node * tail = NULL;
	Node * curr = NULL;
	yielderFlag = false;
	
	char * a = new char[1024];
	threadY= new Thread(yielder, a, 1024);
  Console::puts("Constructed Scheduler.\n");
}


void Scheduler::yield() {
	SYSTEM_DISK -> yieldCall();
	if (yielderFlag &&(head -> next == NULL) ) {
	}
	if (head -> thread != Thread::CurrentThread()) {
		Thread::dispatch_to(head -> thread);
		return;
	}
	if ((head -> next == NULL)) {
	// yield with only 1 thread, should cause a yielder
		delete head;
		head = NULL;
	}
	else {
	// Next thread to go is not head and (|ready Queue| > 1)
		Node * i = head;
		if (i == head) {
			//This should happen the vast majority of the time, curr == head
			head = i -> next;
			head -> prev = NULL;
		}
		//add the yielded thread to end and delete i
		delete i;
	}
	if (head == NULL ) {
		//Thread::dispatch_to(threadY);
	}
	else {
	Thread::dispatch_to(head -> thread);
	}
} 

void Scheduler::resume(Thread * _thread) {
	Node * newNode = new Node();
	newNode -> thread = _thread;
	if (head == NULL) {
		//There are no threads
		head = newNode;
		return;
	}
	else if (tail == NULL) {
		//There is one thread
		tail = newNode;
		tail -> prev = head;
		head -> next = tail;
		return;
	} 
	else {
		//There are many threads
		tail -> next = newNode;
		newNode -> prev = tail;
		tail = newNode;
	}
}

void Scheduler::add(Thread * _thread) {
	resume( _thread);
}

void Scheduler::terminate(Thread * _thread) {
	if (head -> next == NULL) {
		delete head; 
		head = NULL;
		tail = NULL;
		curr = NULL;
		Console::puts("We are now yielding until a new thread is started\n");
		yielderFlag = true;
		SYSTEM_SCHEDULER->resume(threadY);
		Thread::dispatch_to(threadY);
		SYSTEM_SCHEDULER->yield();
		return;
	}
	Node * toDelete = NULL;
	if (tail -> thread == _thread) {
		toDelete = tail;
		(tail -> prev) -> next = NULL;
		// If tail -> prev is head then we don't need to set tail to prev
		tail = (tail -> prev == head) ? NULL : (tail -> prev);
		tail -> next = NULL;
		
		curr = head;
		delete toDelete;
	}
	else {
		Node * i = head;
		do {
			if (i -> thread == _thread)
				break;
			i = i -> next;
		} while (i != tail);
		curr = i -> next;
		toDelete = i;
		//             dummy        skip toDelete from prev side
		(toDelete == head) ? (toDelete) : ((toDelete -> prev) -> next = toDelete -> next);
		(toDelete -> next) -> prev = toDelete -> prev; 
		delete toDelete;
	}
	assert(toDelete != NULL);
	Thread::dispatch_to(head -> thread); //changed from curr -> threadd
}

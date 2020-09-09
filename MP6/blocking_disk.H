/*
     File        : blocking_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "thread.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
struct Queue{
	public:
		Thread* thread;
		unsigned long block_no;
		unsigned char * buf;
		bool readOrWrite; // 0 = read 1 = write
		Queue* next;
		Queue(){
			thread=NULL;
			next=NULL;
		}
		Queue(Thread* _thread, unsigned long _block_no, unsigned char * _buf, bool _readOrWrite){
			thread = _thread;
			buf = _buf;
			block_no = _block_no;
			next = NULL;
			readOrWrite = _readOrWrite;
		}
		Queue peek () {
			return *this;
		}
		bool seek (Thread* _thread) {
			if (thread == NULL)
				return false;
			else {
				Queue * i = this;
				do {
					if (i -> thread = _thread)
						return true;
				i = i -> next;
				} while (i != NULL);
			} 
			return false;			
		}
		void enqueue(Thread* _thread, unsigned long _block_no, unsigned char * _buf, bool _readOrWrite){
			if (thread == NULL) {
				thread= _thread;
				buf = _buf;
				block_no = _block_no;
				readOrWrite = _readOrWrite;
				return;
			}
			else {
				if (next == NULL){
					next = new Queue(_thread, _block_no, _buf, _readOrWrite);
					return;
				}
				else {
					Queue * i = next;
					do {
						if (i -> next == NULL) {
							i -> next = new Queue(_thread, _block_no, _buf, _readOrWrite);
							return;
						}
						i = i -> next;
					} while (i != NULL);
				}
			}
			assert(false);
		}
		Queue dequeue(){
			Queue returns = *this;
			if (thread == NULL) {
				returns.thread = NULL;
				return returns;
			}
			if (next != NULL){
				thread = next -> thread;
				block_no = next -> block_no;
				buf = next -> buf;
				readOrWrite = next -> readOrWrite;
				Queue* kill= next;
				next = next -> next;
				delete kill;
				return returns; 
			}
			returns = *this;
			thread=NULL;
			block_no = NULL;
			buf = NULL;
			return returns;
		}
};
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class BlockingDisk : public SimpleDisk {
	Queue queue;
public:
   BlockingDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */

   /* DISK OPERATIONS */
	virtual void yieldCall();
	//The polling function called by yield in the scheduler
	
   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif

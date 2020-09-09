/*
     File        : simple_disk.c

     Author      : Riccardo Bettati
     Modified    : 10/04/01

     Description : Block-level READ/WRITE operations on a simple LBA28 disk 
                   using Programmed I/O.
                   
                   The disk must be MASTER or SLAVE on the PRIMARY IDE controller.

                   The code is derived from the "LBA HDD Access via PIO" 
                   tutorial by Dragoniz3r. (google it for details.)
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
#include "simple_disk.H"
#include "machine.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

SimpleDisk::SimpleDisk(DISK_ID _disk_id, unsigned int _size) {
   disk_id   = _disk_id;
   disk_size = _size;
}

/*--------------------------------------------------------------------------*/
/* DISK CONFIGURATION */
/*--------------------------------------------------------------------------*/

unsigned int SimpleDisk::size() {
  return disk_size;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void SimpleDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no) {

  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
                         /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_id << 4));
                         /* send drive indicator, some bits, 
                            highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == READ) ? 0x20 : 0x30);

}

bool SimpleDisk::is_ready() {
   return ((Machine::inportb(0x1F7) & 0x08) != 0);
}

void SimpleDisk::read(unsigned long _block_no, unsigned char * _buf) {
// Calls the private function issue_operation

  issue_operation(READ, _block_no);

}

void SimpleDisk::write(unsigned long _block_no, unsigned char * _buf) {
// Calls the private function issue_operation

  issue_operation(WRITE, _block_no);

}

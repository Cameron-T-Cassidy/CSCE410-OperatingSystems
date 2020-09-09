/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"
#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool; 
    page_table = _page_table;
    page_table->register_pool(this);
    
    metaData = (data*) ((Machine::PAGE_SIZE) * (frame_pool -> get_frames(1)));
    used = 0;
    metaDataSize = 0;
}

unsigned long VMPool::allocate(unsigned long _size) {
	//round up to mutlipe of page size (adds internal fragmentation)
	unsigned long modSize = _size % Machine::PAGE_SIZE == 0 ? _size : ((_size/Machine::PAGE_SIZE)*Machine::PAGE_SIZE) + Machine::PAGE_SIZE;
    unsigned long returns = 0; 
    if ((metaDataSize == 511) || (used == size)) {
		return returns; 
		Console::puts("ERROR: cannot allocate region of memory.\n");
	}
	else if (metaDataSize == 0) {
		//First region
		metaData[metaDataSize].basePage = base_address;
		metaData[metaDataSize].length = modSize;
		returns = base_address;
		metaDataSize++;
		Console::puts("Allocated region of memory.\n");
	}
	else {
		//Additional regions
		metaData[metaDataSize].basePage = metaData[metaDataSize - 1].basePage + metaData[metaDataSize - 1].length;
		metaData[metaDataSize].length = modSize;
		metaDataSize++;
		returns = metaData[metaDataSize].basePage;
		Console::puts("Allocated region of memory.\n");
	}
	used += modSize;
    return returns;
}

void VMPool::release(unsigned long _start_address) {
	//find the location of address in meta data
	unsigned int index=0;
	for(int i = 0; i < metaDataSize; i++) {
		if(metaData[i].basePage == _start_address){
			index=i;
			break;	
		}
	}
	//remove the pages
	unsigned long killThis = _start_address;
	for (int i = 0; i < metaData[index].length/Machine::PAGE_SIZE; i++) {
		page_table -> free_page(killThis); 
		killThis += Machine::PAGE_SIZE;
		used -= Machine::PAGE_SIZE; //Used may be decreased, but not usable due to ext. fragmentation
	}
	//get rid of entry in metadata
	// This will lead to external fragmentation, since new regions are allocated from the last index in the metadata
	// But it could be fixed by keeping track of these freed regions (but we were instructed to keep it simple)
	for (int i = index; i < metaDataSize-1; i++) {
		metaData[i] = metaData[i+1];
	}
	metaDataSize--; 
	Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
	return true;
    for (int i = 0; i < metaDataSize; i++) {
		if ((_address >= metaData[i].basePage) && (_address < (metaData[i].basePage + metaData[i].length)))
			return true;
	}
	return false;
		
}


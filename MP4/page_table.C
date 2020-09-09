#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"
#include "utils.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
	process_mem_pool = _process_mem_pool;
	kernel_mem_pool = _kernel_mem_pool;
	shared_size = _shared_size; //In our case, this is 4MB
	Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
	// ******Should these be * by 4096?******
	page_directory = (unsigned long *) (PAGE_SIZE * process_mem_pool -> get_frames(1)); 
	unsigned long * page_table = (unsigned long *) (PAGE_SIZE * process_mem_pool -> get_frames(1)); // should be right after directory in frame pool
	
	unsigned long address=0; // holds the physical address of where a page is
	// map first 4MB of memory
	for (int i = 0; i < 1024; i++) {
		page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
		address = address + 4096; // 4096 = 4kb
	} 
	
	// map to first page and initalize directory
	page_directory[0] = (unsigned long) page_table;
	page_directory[0] = page_directory[0] | 3;
	for( int i=1; i<1023; i++) {
		page_directory[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
	}
	page_directory[1023] = (unsigned long)(page_directory)|3;
	Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
	write_cr3( (unsigned long) (current_page_table-> page_directory));
	write_cr0(read_cr0() | 0x80000000); 
	Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
	VMPool * curr = current_page_table -> head; 
	bool flag = false;
	do {
		if ( curr -> is_legitimate(read_cr2())) {
			flag = true;
			break;
		}
		curr = curr -> next;
	} while (curr != NULL);
	
	// protection fault check
	if ((_r -> err_code % 2 == 1) || !flag)
		return;
		
	// 10 bit page table nr. : 10 bit page nr. : 12 bit offset
	unsigned long errDir = read_cr2() >> 22;
	unsigned long errPage = (read_cr2() >> 12) & 0x3FF; //More like an index than a page, MP3 variable name
	
	unsigned long* directory = (unsigned long*) 0xFFFFF000; // directory
	unsigned long* pageTable = (unsigned long*)((0x3FF<<22)+(errDir << 12));   //logic address page table
	if (directory[errDir] % 2 == 1) { // Directory entry exists, create page in page table
		pageTable[errPage] = (unsigned long) (process_mem_pool-> get_frames(1) * 4096);
		pageTable[errPage] = pageTable[errPage] | 3;
	}
	else { //Directory entry doesn't exist, must create one & page table
		unsigned long * newPageTable = (unsigned long*) (process_mem_pool->get_frames(1) * 4096);
		directory[errDir] = (unsigned long) (newPageTable);
		directory[errDir] = directory[errDir] | 3;
		for (int i = 0; i < 1024; i++) {
			newPageTable[i] = 0 | 2;
		}
		newPageTable[errPage] = (unsigned long) (process_mem_pool-> get_frames(1) * 4096);
		newPageTable[errPage] = newPageTable[errPage] | 3;
	}
    
  Console::puts("handled page fault\n");
}
void PageTable::register_pool(VMPool * pool) {
	if (head == NULL) {
		head = pool;
		tail = head;
		return;
	}
	tail -> next = pool; 
	tail = pool;
}
void PageTable::free_page(unsigned long _page_no) {
	unsigned long errDir = _page_no >> 22; 
	unsigned long errPage = (_page_no >> 12) & 0x3FF;
	unsigned long * pageTable = (unsigned long*)((0x3FF<<22)+(errDir << 12));
	process_mem_pool -> release_frame(pageTable[errPage]); 
	
	// Clear TLB
	unsigned long temp = read_cr3();
	write_cr3(temp);
	return; 
}





















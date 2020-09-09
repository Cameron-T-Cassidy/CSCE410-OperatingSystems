/*
 File: ContFramePool.C
 
 Author: Cameron Cassidy
 Date  : 06/15/2020
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

ContFramePool* ContFramePool:: head;
ContFramePool* ContFramePool:: tail;

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    assert(_n_frames <= FRAME_SIZE * 8);
    
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info 
    if(_info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

	//add to linked list
	if (ContFramePool::head == NULL) {
   		ContFramePool::head = this; 
		ContFramePool::tail = this;
	}
	else {
		prev = ContFramePool::tail; 
		(ContFramePool::tail) -> next = this;
		ContFramePool::tail = this; 
	}
    // Everything ok. Proceed to mark all bits in the bitmap
    for(int i=0; i < _n_frames; i++) {
        bitmap[i] = 0xFF;
    }
    
    // Mark the frames as being used if it is an info frame
    for (int i = 0; i < _n_info_frames; i++) {
			bitmap[_info_frame_no + i] = 0xFE;
			nFreeFrames--; 
	}
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    assert(nFreeFrames > 0);
    // used to iterate through potential frames for the sequence
    int seqPos = 0;
    // the potential start of the sequence
    int seqStart = 0;
    
    while (seqStart < (nframes - _n_frames)) {
			if (bitmap[seqStart] == 0xFF) {
				seqPos = 0;
				while (seqPos < _n_frames) {
					//Found a free frame 
					if (bitmap[seqStart + seqPos] == 0xFF) {
						seqPos++;
					}
					else {
						//We know it won't fit in the space between SeqStart and the next seq, so don't check
						seqStart += seqPos;
						break;
					}
				}
				//We found a place for the sequence
				if (seqPos == _n_frames)
					break;
			} else {
					seqStart++; 
			}
	}
	if (seqStart == (nframes - _n_frames))
		return 0; 
	bitmap[seqStart] = 0x01; 
	for (int i = 1; i < seqPos; i++)
		bitmap[seqStart + i] = 0x00;
	return seqStart; 
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
	if (_base_frame_no < base_frame_no || (_base_frame_no + _n_frames > base_frame_no + nframes)) {
		Console::puts("Mark inaccessible: range error\n");
		assert(false); 
	}
	for (int i = _base_frame_no; i < _n_frames; i++) {
		assert(bitmap[i] != 0xFF);
		bitmap[i] = 0x01;
	}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	
	ContFramePool* curr = head;
	unsigned long bitmapNo = (_first_frame_no - (curr -> base_frame_no));
	//There's only one pool:
	if (curr == tail) {
		//Check that the _First_frame_no is in fact a part of the pool
		if ((_first_frame_no >= curr -> base_frame_no) && ( _first_frame_no < curr -> base_frame_no + (curr -> nframes))) {
			if (curr -> bitmap[bitmapNo] == 0x01) {
				//It's head of sequence
				curr -> bitmap[bitmapNo] = 0xFF;
				for (int i = bitmapNo+1; i < (curr -> nframes); i++) {
					if ( curr -> bitmap[i] != 0x00) {
						return;
					}
					curr -> bitmap[i] = 0xFF;
				}
			}
			else {
			//It's not head of a sequence
				for(int i = bitmapNo; i > 0; i--) {
					if (curr -> bitmap[i] == 0x00) {
						curr -> bitmap[i] = 0xFF;
					}
					if (curr -> bitmap[i] == 0x01) {
						curr -> bitmap[i] = 0xFF;
						return;
					} else {
						//we've hit an info frame
						return;
					}
				}
			}
			return;
		}
		return;
	}
	while (curr != tail) {
		//There's more than one pool
		//Check that the _First_frame_no is in fact a part of the pool
		if ((_first_frame_no >= curr -> base_frame_no) && ( _first_frame_no < curr -> base_frame_no + (curr -> nframes))) {
			if (curr -> bitmap[bitmapNo] == 0x01) {
				//It's head of sequence
				curr -> bitmap[bitmapNo] = 0xFF;
				for (int i = bitmapNo+1; i < (curr -> nframes); i++) {
					if ( curr -> bitmap[i] != 0x00) {
						return;
					}
					curr -> bitmap[i] = 0xFF;
				}
			}
			else {
			//It's not head of a sequence
				for(int i = bitmapNo; i > 0; i--) {
					if (curr -> bitmap[i] == 0x00) {
						curr -> bitmap[i] = 0xFF;
					}
					if (curr -> bitmap[i] == 0x01) {
						curr -> bitmap[i] = 0xFF;
						return;
					} else {
						//we've hit an info frame
						return;
					}
				}
			}
			return;
		}
		curr = curr -> next;
	    bitmapNo = (_first_frame_no - (curr -> base_frame_no));
	}
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	unsigned long neededBits = _n_frames * 8;
	unsigned long neededFrames = neededBits / (1024 * 4);
	unsigned long extFrames = (_n_frames * 8) % (1024 * 8) ? 0 : 1;
	return neededFrames + extFrames;
}

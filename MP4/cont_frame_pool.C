#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                                 unsigned long _nframes,
                                 unsigned long _info_frame_no,
                                 unsigned long dummy)
{
    // Bitmap must fit in a single frame!
    assert(_nframes <= FRAME_SIZE * 8);
    
    base_frame_no = _base_frame_no;
    nframes = _nframes;
    nFreeFrames = _nframes;
    info_frame_no = _info_frame_no;
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }
    
    // Number of frames must be "fill" the bitmap!
    assert ((nframes % 8 ) == 0);
    
    
    // Everything ok. Proceed to mark all bits in the bitmap
    for(int i=0; i*8 < _nframes; i++) {
        bitmap[i] = 0xFF;
    }
    
    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        bitmap[0] = 0x7F;
        nFreeFrames--;
    }
    
    Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int dummy)
{
    
    // Any frames left to allocate?
    assert(nFreeFrames > 0);
    
    // Find a frame that is not being used and return its frame index.
    // Mark that frame as being used in the bitmap.
    unsigned int frame_no = base_frame_no;
    
    unsigned int i = 0;
    while (bitmap[i] == 0x0) {
        i++;
    }
    
    frame_no += i * 8;
    
    unsigned char mask = 0x80;
    while ((mask & bitmap[i]) == 0) {
        mask = mask >> 1;
        frame_no++;
    }
    nFreeFrames--;
    
    // Update bitmap
    bitmap[i] = bitmap[i] ^ mask;
    
    return (frame_no);
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                        unsigned long _nframes)
{
    // Mark all frames in the range as being used.
    int i ;
    for(i = _base_frame_no; i < _base_frame_no + _nframes; i++){
        mark_inaccessible_cb(i);
    }
}

void ContFramePool::mark_inaccessible_cb(unsigned long _frame_no)
{
    // Let's first do a range check.
    assert ((_frame_no >= base_frame_no) && (_frame_no < base_frame_no + nframes));
    
    unsigned int bitmap_index = (_frame_no - base_frame_no) / 8;
    unsigned char mask = 0x80 >> ((_frame_no - base_frame_no) % 8);
    
    // Is the frame being used already?
    assert((bitmap[bitmap_index] & mask) != 0);
    
    // Update bitmap
    bitmap[bitmap_index] ^= mask;
    nFreeFrames--;
}

void ContFramePool::release_frame(unsigned long _frame_no)
{
}
unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames){
	return (_n_frames/4096) + 1;
}

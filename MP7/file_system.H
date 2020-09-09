/* 
    File: file_system.H
    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05
    Description: Simple File System.
    
*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"
#include "utils.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

struct Node{
	// 4 each
	unsigned int useState;
	unsigned int size;
	int file_id;
	// 125 * 4
	unsigned int data[125];
	//dataNode size is 512
};
  
/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/
static unsigned char buf[512]; // Temp storage to/from disk
static Node* dataNode=(Node*) buf;
class File;

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
     SimpleDisk * disk;
     unsigned int size;
     unsigned int diskBlocks;
	File*  files;
	unsigned int numFiles; // |files|
	unsigned int curDataNode;// Where to look for more nodes
     
public:
	void setdisk(SimpleDisk* _disk){
		disk = _disk;
	};
    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */
    
    bool CreateFile(int _file_id);
	void AddFile(File* file); //helper function
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk dataNode occupied by the file. */


	unsigned int getNode();
};
#endif

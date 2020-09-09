/*
     File        : file_system.C
     Author      : Riccardo Bettati
     Modified    : 2017/05/01
     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define FREE    0x0000
#define USED    0xFFFF

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
	Console::puts("In file system constructor.\n");
	curDataNode=0;
	numFiles=0;
	files=NULL;
	memset(buf,0, 512); 
// I compulsivly clear the buffer throughout this file
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/
bool FileSystem::Mount(SimpleDisk * _disk) {
	Console::puts("mounting file system form disk\n");
	disk=_disk;
	return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
	Console::puts("formatting disk. This will take a while. . .\n");
	FILE_SYSTEM->setdisk(_disk);
	memset(buf,0,512);

	size = _size;
	diskBlocks = size / 512;
	memset(buf,0, 512);
	for (int i = 0;i < diskBlocks; i++)
		_disk->write(i,buf);
	return true;
}

File * FileSystem::LookupFile(int _file_id) {
	for (int i = 0;i < numFiles + 1; i++){
		if (files[i].file_id == _file_id)
			return &files[i];
	}
	return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
	//Console::puts("creating file\n");
	File* file=(File*) new File();
	memset(buf, 0, 512);
	file->file_id=_file_id;
	
	// Add file to files
	AddFile(file);
	return true;
}
void FileSystem::AddFile(File* newFile){
	File* newFiles= new File[numFiles+1];
	if (files==NULL) {
		files = newFiles;
		files[0] = *newFile;
		numFiles++;
	}
	else {
		unsigned int i=0;
		for (i=0;i<numFiles;++i)//copy old list
			newFiles[i]=files[i];
		newFiles[numFiles++]=*newFile;
		
		//Update |files| and files
		delete files; 
		files=newFiles;
	}
}

bool FileSystem::DeleteFile(int _file_id) {
	File* newFiles= new File[numFiles];
	int p = 0;
	bool flag = false; 
	for (int i = 0; i < numFiles; i++) {
		if (files[i].file_id == _file_id){
			files[i].Rewrite();
			flag = true;
			p = i + 1;
		}
		p = (flag ? i + 1 : i);
		newFiles[i] = files[p];
	}
	// update numFiles and update files
	numFiles = (p >= numFiles) ? numFiles-1 : numFiles; 
	delete files;
	files=newFiles;
	return flag;
}

unsigned int FileSystem::getNode(){
	memset(buf, 0, 512);

	disk->read(curDataNode,buf);
	for ( int i = 0; i < (diskBlocks - 1); i++) {
		if(dataNode->useState == FREE)
			break;
		curDataNode++;
		disk -> read(curDataNode, buf);
	}
	disk->read(curDataNode,buf);
	curDataNode++;
	dataNode->useState=USED;
	disk->write(curDataNode,buf);
	return curDataNode;
}


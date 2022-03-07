# Assign1_Storage_Manager
Assign1_Storage_Manager is a C program which could read blocks from a file on disk and write blocks from memory to the disk. This program help us learn how the storage manager control the data in the memeory and how the page file works on disk.

## Team Member:
Chen Muhao	  mchen69@hawk.iit.edu \
Hao	Tianyi	  thao3@hawk.iit.edu \
Cui	Rongbin 	rcui7@hawk.iit.edu 

## Test and Memory Leak:
Go through the test and there is no memory leaks.

## Folder structure:
assign1
-     README.txt 
      dberror.c 
      dberror.h 
      storage_mgr.c 
      storage_mgr.h 
      test_assign1_1.c 
      test_helper.h 
      Makefile 
      
## Run Program:
### 1. environment
   We recommend using Linux and gcc environment. \ 
   
### 2. start  
   ```bash
    > $make
    > $./test_assign1
    > $make clean
   ```
   
## Core Code Function (storage_mgr.c):
### initStorageManager
- Keep in mind, no parameter is returned.
### createPageFile
- Allocate the storage space under page_size and name the page file with file_name.
- Use allocated space to put into the file, and close(significant).
### openPageFile
- If exists, open the file and initialize the file's SM FileHandle.
- Remember, if you want to write the number of pages, you should divide the length by size of page.
### closePageFile,destroyPageFile
- Close and destroy the page file with regular code.
- It's import to the memory leak.
### readBlock
- Read the content on the specific position.
### readFirstBlock , readLastBlock
- Find the first and last position.
### readPreviousBlock , readCurrentBlock , readNextBlock
- Change(plus,subtract) to control the position.
### writeBlock , writeCurrentBlock
- Get the position and write the content to the block.
### Ensurecapacity
- Use while loop to add one by one till finish the requirement.

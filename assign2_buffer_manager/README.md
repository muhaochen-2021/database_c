# Assign2_BufferPool_Manager
Assign2_Buffer_Manager is a C program. The aim of this program is to implement a buffer manager. The buffer manager manages the pages in memory. These pages are the pages in the page file managed by the storage manager implemented in assignment 1. The memory pages managed by the buffer manager are called page frames.We refer to these collectively as buffer pools.Realize in the program to read the pages in the buffer pool and write their data to the disk. And can realize the query of the key data in the buffer pool and destroy the buffer manager. The buffer manager should be able to handle multiple open buffer pools at the same time. However, each page file can only appear in one buffer pool. Each buffer pool uses a page replacement strategy (FIFO and LRU), which is determined when the buffer pool is initialized. 

## Team Member:
Hao	Tianyi	  thao3@hawk.iit.edu \
Cui	Rongbin 	rcui7@hawk.iit.edu  \
Chen Muhao	  mchen69@hawk.iit.edu 

## Test and Memory Leak:
Go through the test_assign2_1 and test_assign2_2.

## Folder structure:
assign2
-     README.txt 
      buffer_mgr_stat.c
      buffer_mgr_stat.h
      buffer_mgr.c
      buffer_mgr.h
      dberror.c 
      dberror.h 
      dt.h
      storage_mgr.c 
      storage_mgr.h 
      test_assign2_1.c 
      test_assign2_2.c 
      test_helper.h 
      Makefile 
      
## Run Program:
### 1. environment
   We recommend using Linux and gcc environment. 
   
### 2. start  
   ```bash
    > $make
    > $./test_assign2_1
    > $./test_assign2_2
    > $make clean
   ```

## include
In this program, we quoted the following header files to implement the corresponding functions:<stdio.h>,<stdlib.h>, <string.h>,"storage_mgr.h","dberror.h", "buffer_mgr.h","buffer_mgr_stat.h".Among them, <stdio.h>,<stdlib.h>, <string.h> these three are to implement linked list related functions. The rest are to realize the related functions required in the "buffer_mgr.c".

## Sturcture 
We are using  a linked list data sturcture to implement the frames. 
Each node of frames contains the following attributes below and we named this frame_node.

**pageNumber:** 
page number of page
**frameNumber:**
the number of the frame in the list
**pin_list::**
the number of threads in pinned page
**dirtyFlag:**
check whether the page is modified or not, dirtyFlag=1 means dirty, dirtyFlag=0 means not.
**rfBit:**
reference bit of node, mostly used in clock replacement stratedy
**data**
actual pointer of data 
**pre_node**
pointer to the previous node of frames
**next_node**
pointer to the next node of frames


We formed a sturcture of frame list of NodeInfo called bf_linked_list

**head**
pointer to head of the list
**tail**
pointer to the tail of the list
**length**
length of the list 

**create_start_node**
this is designed for creating a new node to frame list
**Initial_linked_list**
this is designed for initializing the linked list

We have implement the attribute that need for buffer pool in a sturcture named bf_head_data

**bf_page_sum**
number of frames that have been filled in frame list
**read_times**
number of read operation in the buffer pool 
**write_times**
the number of write operation in the buffer pool 
**pin_sums**
number of pins in the buffer pool 
**pages_list**
mapping the page number that in the page table to the frameNumber.
**dirty_list**
mapping the page number that in the page table to the frame number
**pin_list**
recording all the page situation about the threads access that in the buffer pool
**ref_list**
the record about the reference bit of every pages in pool 

## function of linked list
We used a doubly linked list to implement the function of frameset.The following are the functions we added in order to implement the framelist and buffer_mgr.c.
### Init_add_linked_list 
We use this function to imlement adding the elements to the linked list. We put the new element to the tail of the linked list, then update the the tail of the list to the new element. And update the pre and next of the new tail, the ncrease the value of length by one.
### Change_head 
This function makes the variable declared in the function be added to the linked list and make it the head of the linked list. In order to achieve this function, we first judge the element to determine whether it is already the head of the linked list or whether the element/the head is null. Then add the element and update the list's length and head's pre and tail.
### update_info
This function implements to put the elements of the variable nodeinfo declared in the function into the framelist and update the information of the framelist. Before adding elements, we must first determine whether the declared nodeinfo is a dirty page. If it is, we will write his data to the page. Then update the relevant information of the nodeinfo element (including dirtylag and pincount, etc.). Finally, close the file to release the memory.
## repalcement
### FIFO_STRA
This function implements the first-in first-out strategy in the page replacement strategy. When we want to add a new page to the bufferpool and the bufferpool is full, the strategy will know the first page added to the bufferpool.In this strategy, we first determine whether the bufferpool is full. If it is not full, add the newly added page directly to the head of the linked list. In the case of full, we find the first unmarked page and remove the page, and then add the newly added page to the head of the linked list. After completing the adding operation, use the NewFrameUpdating function to update the relevant information of the framlist.
### LRU_STRA
It checks wheather the page is in memory first then it will call the PageSearching function. If it's not null, it returns  RC_OK for result. when a frame is uesd, it will be  moved to the head of the frame list. so the head will always be the latest used frame, at the same time, the tail of the list be the least used frame.
If the page is not in memory (Result of PageSerching equals null), it wil returen the error message.
**iniBufferPool**
This function creates a new buffer pool with bf_head_data using the page replacement strategy . 
it will initiate the info of buffer pool, buffer pool and linked list,by order then it will add the data to the linked list then 
If any attribute is invalid, it will returns error message.
**unpinPage:**
It will locate the page to be unpinned first. 
If the page is not avalible it will returns "RC_ERROR_No_FIND_NODE". If the page is found, "pin_list" will minus by 1 and returns RC_OK.
**markDirty:**
This function marks a page as dirty. If the page is found then it willpage is marked as dirty and set node as 1 then return RC_OK.
**forcePage:**
it's a funtion that write the current content of the page back to the page file on disk.
It will check the possible error at first if there's any, it will returns the following error message.
Then it will open the file then write it to the disk. if the frame node is null it will return "return RC_ERROR_No_FIND_NODE" otherwise, after the operation the write_times will puls by 1 then return RC_OK
### LRUk_STRA
In this strategy, when the number of accesses reaches K times, the data index is moved from the history queue to the cache queue; the cache data queue is reordered after being accessed; when data needs to be eliminated, the data at the end of the cache queue is eliminated. We first determine whether the bufferpool is full. If it is not full, add the newly added page directly to the head of the linked list. In the case of full, remove the pages that have been used k times recently and have not been visited. Then add the newly added page to the head of the linked list. After completing the adding operation, use the NewFrameUpdating function to update the relevant information of the framlist.
## bufferpool
### shutdownBufferPool
This function is designed to destroy a bufferpool. Before destroying, all dirty pages in the bufferpool must be written to disk. We use the forceFlushPool function to find all dirty pages in the bufferpool and write all their data to disk. After completing this operation, we have to destroy the framelist. The free function is used to release the memory occupied by each node. Finally, all the memory occupied by the bufferpool is released.
### forceFlushPool
This function causes all dirty pages in the bufferpool to be written to the hard disk.In this function, in order to implement related functions, we first judge whether the variable in the function declaration is null, and if it is, throw an error declaration. If it is not, search for all dirty pages in the bufferpool (ie dirtyflag=1). Then write all the data of the found pages to the disk. Finally, close the file and release the memory.
## Statistics Interface
### getFrameContents
The function returns framearray, where the i-th element is the page number stored in the i-th page frame.
### getDirtyFlags
function returns an array of bools where the ith element is TRUE if the page stored in the ith page frame is dirty. Empty page frames are considered as clean.
### getFixCounts
This function loops to find the page stored in the frame, finds the marked page, and returns the position of the page.
### getNumReadIO
Return the readCount in mgmtData of the bufferpool to display the current number of pages read.
### getNumWriteIO
Return the writeCount in mgmtData of the bufferpool to display the current number of pages write written.


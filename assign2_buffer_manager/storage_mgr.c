#include "storage_mgr.h"
#include "dberror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void initStorageManager(void)
{
}

extern RC createPageFile(char *fileName)
{
    FILE *pages_file = fopen(fileName,"w+");

    /*check file*/
    if (pages_file == NULL){
        return RC_FILE_NOT_FOUND;
    }

    /*allocate the memory of first_page*/
    if (PAGE_SIZE == PAGE_SIZE*sizeof(char)){
        SM_PageHandle firstpage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
        int size = sizeof(firstpage);
        printf("The size of firstpage:%d bytes/",size);

        /*write the first_page into the file*/
        fwrite(firstpage, sizeof(char), PAGE_SIZE, pages_file);

        /*Release*/
        free(firstpage);
        printf("Successful to create./");
    }else{
        printf("The computer's char size is false./");
    }
    
    fclose(pages_file);

    return RC_OK;
}

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
    FILE *pages_file = fopen(fileName, "r+");
    if (pages_file == NULL){
        return RC_FILE_NOT_FOUND;
    }

    /*Initialize the info for fHandle*/
    if (pages_file != NULL)
    {
        fHandle->fileName = fileName;
        fHandle->mgmtInfo = pages_file;
        fHandle->curPagePos = 0;

        /*Compute the number of pages*/
        fseek(pages_file, 0L, SEEK_END); 
        int f_size = ftello(pages_file);
        //printf("%d",f_size);
        int f_pagenum = (f_size / PAGE_SIZE);
        fHandle->totalNumPages = f_pagenum;
        return RC_OK;

    }else{
        return RC_FILE_NOT_FOUND;
    }
}

extern RC closePageFile(SM_FileHandle *fHandle)
{
    /*Judge whether file exists*/
    if (fHandle->mgmtInfo == NULL){
        return RC_FILE_NOT_FOUND;
    }
    FILE *mgmtInfo = fHandle->mgmtInfo;
    fclose(mgmtInfo);

    //printf("Close the file./");
    return RC_OK;
    
}

extern RC destroyPageFile(char *fileName)
{
    /*Remove the file*/
    int judge = remove(fileName);
    if (judge == 0) 
    {
        return RC_OK;
    }else{
        return RC_FILE_NOT_FOUND;
    }
}

/* reading blocks from disc */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    /*Judge whether file available*/
    if (fHandle->mgmtInfo == NULL)
    { 
        return RC_FILE_NOT_FOUND;
    }

   if (pageNum > fHandle->totalNumPages || pageNum < 0)
    { //check the page is regular.
        RC_message = "the page does not exist";
        return RC_READ_NON_EXISTING_PAGE;
    }

    //pages_file = (FILE *)fHandle->mgmtInfo;

    /*Read block and change the current position*/
    int one_block = PAGE_SIZE * sizeof(char);
    fseek(fHandle->mgmtInfo, (pageNum+1) * one_block, SEEK_SET); 
    int f_size = ftello(fHandle->mgmtInfo);
    fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);

    fHandle->curPagePos = pageNum;
    return RC_OK;
}

extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = 0;
    printf("current_pos:%d/",cur_pos);
    return readBlock(cur_pos, fHandle, memPage);
}
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = fHandle->curPagePos - 1;
    printf("current_pos:%d/",cur_pos);
    return readBlock(cur_pos, fHandle, memPage);
}
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = fHandle->curPagePos;
    printf("current_pos:%d/",cur_pos);
    return readBlock(cur_pos, fHandle, memPage);
}
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = fHandle->curPagePos + 1;
    printf("current_pos:%d/",cur_pos);
    return readBlock(cur_pos, fHandle, memPage);
}
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = fHandle->curPagePos - 1;
    printf("current_pos:%d/",cur_pos);
    return readBlock(cur_pos, fHandle, memPage);
}

/* writing blocks to a page file */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    /*Judge whether file available*/
    if (fHandle->mgmtInfo == NULL)
    { 
        return RC_FILE_NOT_FOUND;
    }

    int pos = (pageNum+1) * PAGE_SIZE; //store the starting position

    //pages_file = (FILE *)fHandle->mgmtInfo;

    /*Trun to the point and write, update the current position*/
    int one_block = PAGE_SIZE * sizeof(char);
    fseek(fHandle->mgmtInfo, (pageNum+1) * one_block, SEEK_SET); 
    fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); 
    
    /*print current point position*/
    int f_size = ftello(fHandle->mgmtInfo);
    
    fHandle->curPagePos = pageNum;

    return RC_OK;
    
}
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int cur_pos = fHandle->curPagePos;
    return writeBlock(cur_pos, fHandle, memPage);
}

extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
    /*Allocate the space for page appended*/
    SM_PageHandle Page_Append = (SM_PageHandle)calloc(PAGE_SIZE , sizeof(char));

    /*Trun to the end point and write data*/
    fseek(fHandle->mgmtInfo, 0L, SEEK_END);
    fwrite(Page_Append, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);

    /*The number of page plus 1*/
    fHandle->totalNumPages++;

    /*Change the current position*/
    fHandle->curPagePos = fHandle->totalNumPages;

    free(Page_Append);

    return RC_OK;
}
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
    /*Judge whether necessary to add empty blocks*/
	if (fHandle->totalNumPages < numberOfPages){
		int numPages = numberOfPages - fHandle->totalNumPages;
        int i = 1;
        while(i < numPages+1){
            appendEmptyBlock(fHandle);
            i++;
        }
    }
    return RC_OK;
}

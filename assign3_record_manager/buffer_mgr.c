#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"

/* ############################# Data Structure #############################*/


void *fenpei (void *a,int b,size_t count)
 {
    const unsigned char c1 = b;
    unsigned char *temp;
    /*for(su = s;0 < n;++su,--n)
        *su = uc;*/
    temp=a;
    while (count>0)
    {
       *temp =c1;
       temp++;
       count--;
    }
    
    return a;
 }


/*frame page holder*/
typedef struct NodeInfo
{
    int pageNumber; 
    int frameNumber; 
    unsigned int pinCount;    
    unsigned int dirtyFlag;  
    int rfBit;       
    char *data;      
    struct NodeInfo *previous;
    struct NodeInfo *next; 
} NodeInfo;

/*linked list*/
typedef struct FM_list
{
    NodeInfo *head;
    NodeInfo *tail;
    int length;  
} FM_list;

/*the information in buffer pool*/
typedef struct BM_MetaData
{
    int frameCount; int readCount; int writeCount; int numPin;    
    void *sData; int pagesArray[15000]; int framesArray[100];                 
    bool dirtySet[100]; int pinCount[100]; int RefArray[15000][10]; 
    FM_list *flist;
} BM_MetaData;

/* ############################# function of linked list #############################*/


/*change the head node*/
void AddInHead(FM_list **framelist,NodeInfo *node) {
    if ((*framelist)->head==NULL){
        return ;
    }
    if (node==NULL){
        return ;
    }
    if (node==(*framelist)->head){
        return ;
    }

    int case_judge;
    if (node==(*framelist)->tail){
        case_judge = 0;
    }
    if (node!=(*framelist)->tail){
        case_judge = 1;
    }

    NodeInfo *t_node_i=(*framelist)->tail->previous;
    switch ((case_judge))
    {
    case 0:
        t_node_i->next=NULL;
        (*framelist)->tail=t_node_i;
        break;
    
    case 1:
        node->previous->next=node->next;
        node->next->previous=node->previous;
        break;

    }
    node->next=(*framelist)->head; 
    (*framelist)->head->previous=node;
    if (node != NULL){
    node->previous=NULL;
    (*framelist)->head=node;
    }
}

/*create the new node in frame*/
NodeInfo *InitNewNode(){
    NodeInfo *newnode = calloc(1,sizeof(NodeInfo));
    newnode->pageNumber = NO_PAGE;
    newnode->data = malloc(PAGE_SIZE * sizeof(SM_PageHandle));
    newnode->pinCount=0;
    newnode->frameNumber=0;
    newnode->next = NULL;
    newnode->previous = NULL;
    newnode->dirtyFlag=0;
    newnode->rfBit=0;
    return newnode;
}


/*the function of updating the next new frame*/
RC NewFrameUpdating(BM_BufferPool *const bm,BM_PageHandle *const page,NodeInfo *curNode,const PageNumber pageNum){
    SM_FileHandle fh;
    BM_MetaData *metaData=(BM_MetaData *)bm->mgmtData;
  
    openPageFile(bm->pageFile,&fh);
   
    while(curNode->dirtyFlag==1){
        ensureCapacity(pageNum,&fh);
        writeBlock(curNode->pageNumber,&fh,curNode->data);
       (metaData->writeCount)++; 
        break;
    }

    (metaData->pagesArray)[curNode->pageNumber]=NO_PAGE; 
  
    ensureCapacity(pageNum,&fh);
    readBlock(pageNum,&fh,curNode->data);
   
    curNode->dirtyFlag=0;
    curNode->pinCount=1;
    curNode->pageNumber=pageNum;

    page->pageNum=pageNum;
    page->data=curNode->data;
    (metaData->readCount)++;

    (metaData->pagesArray)[curNode->pageNumber]=curNode->frameNumber;
    (metaData->framesArray)[curNode->frameNumber]=curNode->pageNumber;
    
    closePageFile(&fh);
    return RC_OK;
}

/*get the specific info/data of the page*/
NodeInfo *NodeSearching(FM_list *flist, const PageNumber pageNum)
{
    NodeInfo *curNode = flist->head; 
    int a = 0;
    while ((curNode != NULL)&&(a == 0))
    {
        if (curNode->pageNumber == pageNum) 
        {
            return curNode;
        }
        curNode = curNode->next;
    }
    return NULL;
}

NodeInfo *PageSearching(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    BM_MetaData *metaData=(BM_MetaData *)bm->mgmtData;
    NodeInfo *curNode=NodeSearching(metaData->flist,pageNum);
    if((metaData->pagesArray)[pageNum]!=NO_PAGE){ 
        if(curNode!=NULL){
            page->pageNum=pageNum;
            page->data=curNode->data;
            (curNode->pinCount)++;
            curNode->rfBit=1;
            return curNode;
        }else{
            return NULL;
        }
    }else{
        return NULL;
    }
}

RC FIFO_ReplacementStrategy(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    NodeInfo *curNode = PageSearching(bm, page, pageNum);
    BM_MetaData *metaData = (BM_MetaData *)bm -> mgmtData;
    if(curNode!=NULL){
        return RC_OK;
    }
    
    int case_judge;
    if ((metaData->frameCount)<bm->numPages){
        case_judge = 0;
    }
    if (((metaData->frameCount)>bm->numPages)||((metaData->frameCount)==bm->numPages)){
        case_judge = 1;
    }

    switch ((case_judge)){
        case 0:
            curNode=metaData->flist->head;
            int a = 0;
            while (a<metaData->frameCount){
                a++;
                curNode=curNode->next;
            }
            (metaData->frameCount)++; 
            AddInHead(&(metaData->flist),curNode);
        
        case 1:
            curNode=metaData->flist->tail;
            for (int i; i < 20000; i++){
                if ((curNode!=NULL&&curNode->pinCount!=0)){
                    curNode=curNode->previous;
                }else{
                    break;
                }
            }
            if(curNode==NULL){
                return RC_ERROR_Frame_Number;
            }
            AddInHead(&(metaData->flist),curNode);

    }

    if(NewFrameUpdating(bm,page,curNode,pageNum)!=RC_OK){
        return NewFrameUpdating(bm,page,curNode,pageNum);
    }else{
        return RC_OK;
    }
    
    
}

RC LRU_ReplacementStrategy(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    NodeInfo *curNode = PageSearching(bm, page, pageNum);
    BM_MetaData *metaData = (BM_MetaData *)bm -> mgmtData;
    if(curNode!=NULL){ 
        AddInHead(&(metaData->flist),curNode);
        return RC_OK;
    }

    int case_judge;
    if ((metaData->frameCount)<bm->numPages){
        case_judge = 0;
    }
    if (((metaData->frameCount)>bm->numPages)||((metaData->frameCount)==bm->numPages)){
        case_judge = 1;
    }

    switch ((case_judge)){
        case 0:
            curNode=metaData->flist->head;

            int a = 0;
            while (a<metaData->frameCount){
                a++;
                curNode=curNode->next;
            }
            (metaData->frameCount)++; 
            AddInHead(&(metaData->flist),curNode);
        
        case 1:
            curNode=metaData->flist->tail;
            for (int i; i < 20000; i++){
                if ((curNode!=NULL&&curNode->pinCount!=0)){
                    curNode=curNode->previous;
                }else{
                    break;
                }
            }
            if(curNode==NULL){
                return RC_ERROR_Frame_Number;
            }

    }

        AddInHead(&(metaData->flist),curNode);
        if(NewFrameUpdating(bm,page,curNode,pageNum)!=RC_OK){
            return NewFrameUpdating(bm,page,curNode,pageNum);
        }else{
            return RC_OK;
        }
    
}

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
    SM_FileHandle fh;
    if (numPages <= 0)
    {
        return RC_ERROR_ALL_NODE_IN_PIN;
    }

    BM_MetaData *metaData = malloc(sizeof(BM_MetaData));
    metaData->frameCount = 0;
    metaData->readCount = 0;
    metaData->writeCount = 0;
    metaData->sData = stratData;
    metaData->numPin = 0;
    fenpei(metaData->framesArray, NO_PAGE, 100 * (sizeof(int)));
    fenpei(metaData->pagesArray, NO_PAGE, 15000 * (sizeof(int)));
    fenpei(metaData->dirtySet, NO_PAGE, 100 * (sizeof(bool)));
    fenpei(metaData->pinCount, NO_PAGE, 100 * (sizeof(int)));
    fenpei(metaData->RefArray, -1, sizeof((metaData->RefArray)));
    metaData->flist = calloc(1, sizeof(FM_list));
    metaData->flist->tail = InitNewNode();
    metaData->flist->head = metaData->flist->tail;
    int a = 0;
    while (a < numPages){
        metaData->flist->tail->next = InitNewNode();
        metaData->flist->tail->next->previous = metaData->flist->tail;
        metaData->flist->tail = metaData->flist->tail->next;
        metaData->flist->tail->frameNumber = a;
        a++;
    }

    bm->mgmtData = metaData;
    bm->numPages = numPages;
    bm->pageFile = (char *)pageFileName;
    bm->strategy = strategy;

    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{
    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }

    forceFlushPool(bm);

    BM_MetaData *head_data_info = (BM_MetaData *)bm->mgmtData;
    NodeInfo *test_node = head_data_info->flist->head;

    for (int i=0;i<200;i++) 
    {
        test_node = test_node->next;
        free(head_data_info->flist->head->data);
        free(head_data_info->flist->head);
        head_data_info->flist->head = test_node;
        
        if (test_node == NULL){
            i=200;
        }
    }

    free(head_data_info->flist);
    free(head_data_info);

    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm)
{
    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }

    SM_FileHandle fhandel;
    openPageFile(bm->pageFile, &fhandel);

    BM_MetaData *frame_data = (BM_MetaData *)bm->mgmtData;
    NodeInfo *shut_node = frame_data->flist->head;
 
    for (int i=0;i<200;i++) 
    {
        if (shut_node->dirtyFlag == 1)
        {
            shut_node->dirtyFlag = 0;
            (frame_data->writeCount) = (frame_data->writeCount)+1;
            writeBlock(shut_node->pageNumber, &fhandel, shut_node->data);
        }
        shut_node = shut_node->next;

        if (shut_node == NULL){
            i=200;
        }
    }
 
    closePageFile(&fhandel);
    return RC_OK;
}

RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (bm->numPages <= 0 || bm == NULL)
    {
        return RC_ERROR_No_BF;
    }
    BM_MetaData *metadata = (BM_MetaData *)bm->mgmtData;
    NodeInfo *searchNode = NodeSearching(metadata->flist, page->pageNum);
    if (searchNode == NULL)
    {
        return RC_ERROR_ALL_NODE_IN_PIN;
    }
    else
    {
        searchNode->dirtyFlag = 1;
        return RC_OK;
    }
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (NULL != bm && bm->numPages > 0)
    {
        BM_MetaData *metaData = (BM_MetaData *)bm->mgmtData;
        NodeInfo *curNode = NodeSearching(metaData->flist, page->pageNum);
        if (curNode == NULL)
        {
            return RC_ERROR_No_BF;
        }
        if (curNode->pinCount > 0)
        {
            (curNode->pinCount)--;
        }
        else
        {
            return RC_ERROR_No_BF;
        }
        return RC_OK;
    }
    return RC_ERROR_No_BF;
}
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (NULL != bm && bm->numPages > 0)
    {
        SM_FileHandle fh;
        BM_MetaData *metaData = (BM_MetaData *)bm->mgmtData;
        if (openPageFile((char *)(bm->pageFile), &fh) != RC_OK)
        {
            return RC_FILE_NOT_FOUND;
        }

        NodeInfo *curNode = NodeSearching(metaData->flist, page->pageNum);
        if (curNode == NULL)
        {
            closePageFile(&fh);
            return RC_ERROR_No_PG;
        }

        if (writeBlock(curNode->pageNumber, &fh, curNode->data) != RC_OK)
        {
            closePageFile(&fh);
            return RC_WRITE_FAILED;
        }
        closePageFile(&fh);
        (metaData->writeCount)++;
        return RC_OK;
    }
    return -2;
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum)
{
    if (pageNum < 0)
    {
        return RC_READ_NON_EXISTING_PAGE;
    }

    if (NULL != bm && bm->numPages >= 0)
    {
        int a = bm->strategy;
        if (a == 0){
            return FIFO_ReplacementStrategy(bm,page,pageNum);
        }
        else if (a == 1){
            return LRU_ReplacementStrategy(bm,page,pageNum);
        }
        else{
            return RC_ERROR_Frame_Number;
        }
    }
    return RC_ERROR_No_BF;
}

/* Statistics Interface */
PageNumber *getFrameContents(BM_BufferPool *const bm) {
     if (bm == NULL){
        return NULL;
    }
    PageNumber *num=((BM_MetaData *)bm -> mgmtData)->framesArray;
    return num;
}
bool *getDirtyFlags(BM_BufferPool *const bm) {
    BM_MetaData *metaData = (BM_MetaData *)bm -> mgmtData;
    NodeInfo *curNode = metaData -> flist -> head;
    
    while (curNode != NULL){
        (metaData ->dirtySet)[curNode->frameNumber] = curNode->dirtyFlag;
        curNode = curNode -> next;
    }
    
    return metaData->dirtySet;
}
int *getFixCounts(BM_BufferPool *const bm) {
     BM_MetaData *metaData = (BM_MetaData *)bm->mgmtData;
    NodeInfo *curNode = metaData->flist->head;

    while (curNode != NULL){
        (metaData->pinCount)[curNode->frameNumber] =curNode->pinCount;
        curNode=curNode->next;
    }

    return metaData->pinCount;
}
int getNumReadIO(BM_BufferPool *const bm) {
     if (bm == NULL){
        return 0;
    }
    int num=((BM_MetaData *)bm->mgmtData)->readCount;
    return num;
}
int getNumWriteIO(BM_BufferPool *const bm) {
     if (bm == NULL){
        return 0;
    }
    int num=((BM_MetaData *)bm->mgmtData)->writeCount;
    return num;
}

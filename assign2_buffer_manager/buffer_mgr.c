#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"

/* ############################# Data Structure #############################*/

/*frame page holder*/
typedef struct frame_node
{
    /*k*/
    int ts[10];
    /*the number/id of disk*/
    int pageNumber;  
    /*the number/id of bf memory*/
    int frameNumber; 
    /*the tag of pin*/
    unsigned int pin_list;    
    /*the tag of dirty*/
    unsigned int dirtyFlag;   
    /*the content*/
    char *data;      
    /*linked list*/
    struct frame_node *pre_node; 
    struct frame_node *next_node; 
} frame_node;

/*linked list*/
typedef struct bf_linked_list
{
    frame_node *head;
    frame_node *tail;
    int length;  
} bf_linked_list;

/*create the new node in linked list*/
frame_node *create_start_node()
{ 
    frame_node *newnode = calloc(1,sizeof(frame_node));
    /*initial the stat*/
    int init_data[100];
    for(int i=0;i<4;i++){
        init_data[i] = 0;
    }
    newnode->dirtyFlag=init_data[2];
    newnode->pin_list=init_data[0];
    newnode->frameNumber=init_data[1];

    newnode->data = malloc(PAGE_SIZE*sizeof(SM_PageHandle));
    newnode->pageNumber = -1;

    newnode->next_node = NULL;
    newnode->pre_node = NULL;
   for ( int i = 0; i < 10; i++ )
   {
      newnode->ts[ i ] = 0; 
   }
    return newnode;
}

/*initial the linked list*/
void Initial_linked_list(bf_linked_list *framelist){
    (framelist)->head = create_start_node();
    frame_node *tmp_node = create_start_node();
    tmp_node = (framelist)->head;
    (framelist)->tail = tmp_node;
    (framelist)->length = 1;
}

/*the information in buffer pool*/
typedef struct bf_head_data
{
    int ts_record;
    unsigned int bf_page_sum; 
    unsigned int read_times;  
    unsigned int write_times; 
    unsigned int pin_sums;     
    void *strat_Data;
    unsigned int pages_list[1000];                
    unsigned int frames_list[1000];                 
    bool dirty_list[1000];                   
    unsigned int pin_list[1000];                    
    unsigned int ref_list[1000][1000]; 
    bf_linked_list *l_list;
} bf_head_data;

/*initial the buffer pool*/
void Initial_buffer_pool_info(bf_head_data *buffer_pool_data,void *stratData){
    /*initial the stat*/
    int init_data[100];
    for(int i=0;i<4;i++){
        init_data[i] = 0;
    }
    buffer_pool_data->bf_page_sum = init_data[0];
    buffer_pool_data->read_times = init_data[1];
    buffer_pool_data->write_times = init_data[2];
    buffer_pool_data->pin_sums = init_data[3];
    buffer_pool_data->strat_Data = stratData;

    /*initial the stat*/
    memset(buffer_pool_data->pages_list, -1, 1000);
    memset(buffer_pool_data->frames_list, -1, 1000);
    memset(buffer_pool_data->pin_list, -1, 1000);
    memset(buffer_pool_data->dirty_list, -1, 1000);
    memset(buffer_pool_data->ref_list, -1, 1000);
}


/* ############################# function of linked list #############################*/

/*add the elements to the linked list*/
void Init_add_linked_list(bf_linked_list *framelist,int numPages){
    int ini_num = 0;
    while (ini_num < numPages -1 ){
        if ((framelist)->tail != NULL){
        (framelist)->tail->next_node = create_start_node();
        (framelist)->tail->next_node->pre_node = (framelist)->tail;
        (framelist)->length = (framelist)->length + 1;
        (framelist)->tail = (framelist)->tail->next_node;
        (framelist)->tail->frameNumber = ini_num+1;
        ini_num ++;
        }
    }
}

/*change the head node*/
void Change_head(bf_linked_list **framelist,frame_node *node) {
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

    frame_node *t_node_i=(*framelist)->tail->pre_node;
    switch ((case_judge))
    {
    case 0:
        t_node_i->next_node=NULL;
        (*framelist)->tail=t_node_i;
        break;
    
    case 1:
        node->pre_node->next_node=node->next_node;
        node->next_node->pre_node=node->pre_node;
        break;

    }
    node->next_node=(*framelist)->head; 
    (*framelist)->head->pre_node=node;
    if (node != NULL){
    node->pre_node=NULL;
    (*framelist)->head=node;
    }
}

/*the function of updating the next new frame*/
RC update_info(BM_BufferPool *const bm,BM_PageHandle *const page,frame_node *newnode,const PageNumber pageNum){
    SM_FileHandle fh;
    bf_head_data *head_data_info=(bf_head_data *)bm->mgmtData;

    /*firstly open the page file*/
    openPageFile(bm->pageFile,&fh);

    /*test the node whether is dirty*/
    ensureCapacity(pageNum,&fh);
    int i = 0;
    while ((newnode->dirtyFlag==1)&&(i==0)){
        writeBlock(newnode->pageNumber,&fh,newnode->data);
        (head_data_info->write_times) = (head_data_info->write_times)+1; 
        i = 1;
    }

    ensureCapacity(pageNum,&fh);
    readBlock(pageNum,&fh,newnode->data);
    (head_data_info->pages_list)[newnode->pageNumber]=-1;

    /*set the new tags*/
    newnode->dirtyFlag=0;
    newnode->pin_list=1;
    newnode->pageNumber=pageNum;

    /*get new info for page*/
    page->data=newnode->data;
    page->pageNum=pageNum;
    (head_data_info->read_times) = (head_data_info->read_times)+1;

    (head_data_info->frames_list)[newnode->frameNumber]=newnode->pageNumber;
    (head_data_info->pages_list)[newnode->pageNumber]=newnode->frameNumber;

    closePageFile(&fh);
    return RC_OK;
}

/* ############################# searching function #############################*/

/*get the specific info/data of the page*/
frame_node *PageSearching(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    bf_head_data *head_data_info=(bf_head_data *)bm->mgmtData;
    frame_node *now_node = head_data_info->l_list->head; 
    
    int a = 0;
    while ((now_node != NULL)&&(a == 0))
    {
        if (now_node->pageNumber == pageNum) 
        {
            a = 1;
        }
        if (a == 0){
        now_node = now_node->next_node;
        }
    }

    if(now_node == NULL){
        return now_node;
    }

    (now_node->pin_list) = (now_node->pin_list) + 1;
    page->data=now_node->data;
    page->pageNum=pageNum;

    return now_node;
}

/* ############################# repalcement #############################*/

RC FIFO_STRA(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    
    bf_head_data *head_data_info = (bf_head_data *)bm -> mgmtData;
    frame_node *pos_node = PageSearching(bm, page, pageNum);

    /*replace node is not null*/
    if(pos_node!=NULL){
        return RC_OK;
    }
    
    /*no node means no or full*/
    /*not full, we can add directly*/
    int case_judge;
    if ((head_data_info->bf_page_sum)<bm->numPages){
        case_judge = 0;
    }
    if (((head_data_info->bf_page_sum)>bm->numPages)||((head_data_info->bf_page_sum)==bm->numPages)){
        case_judge = 1;
    }

    switch ((case_judge))
    {
    case 0:
        pos_node=head_data_info->l_list->head;
        int a=0;
        while(a<head_data_info->bf_page_sum){
            pos_node=pos_node->next_node;
            a = a + 1;
        }
        Change_head(&(head_data_info->l_list),pos_node);
        head_data_info->bf_page_sum = head_data_info->bf_page_sum + 1;
        
        break;
    
    case 1:
        pos_node=head_data_info->l_list->tail;
        for(int i=0;i<200 ;i++){
            if (pos_node==NULL){
                i = 200;
            }
            else if (pos_node->pin_list!=0){
                pos_node=pos_node->pre_node;    
            }else{
                i = 200;
            }
        }

        if(pos_node==NULL){
            return RC_ERROR_ALL_NODE_IN_PIN;
        }
        Change_head(&(head_data_info->l_list),pos_node);
        break;
    }

    /*Update the framelist.*/
    update_info(bm,page,pos_node,pageNum);
    
}

RC LRU_STRA(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    /*find the specific node for the page required*/
    frame_node *pos_node = PageSearching(bm, page, pageNum);
    /*get the linked_list*/
    bf_head_data *head_data_info = (bf_head_data *)bm -> mgmtData;

    /*if find the page, then put this node to the head, and return ok */
    if(pos_node!=NULL){ 
        Change_head(&(head_data_info->l_list),pos_node);
        return RC_OK;
    }

    /*no node means: 1. not full, add directly 2. full, we should replace.*/
    /*not full, we can add directly*/
    int case_judge;
    /*the page existing is smaller than the numPages*/
    if ((head_data_info->bf_page_sum)<bm->numPages){
        case_judge = 0;
    }
    /*the page existing is equal to or larger than the numPages*/
    if (((head_data_info->bf_page_sum)>bm->numPages)||((head_data_info->bf_page_sum)==bm->numPages)){
        case_judge = 1;
    }

    switch ((case_judge))
    {
    case 0:
        /*firstly, we should get the head node*/
        pos_node=head_data_info->l_list->head;
        
        int a=0;
        /*put the pointer to the tail of the linked list*/
        while(a<head_data_info->bf_page_sum){
            pos_node=pos_node->next_node;
            a = a + 1;
        }
        /*add one page and bf_page_sum plus 1*/
        (head_data_info->bf_page_sum) = (head_data_info->bf_page_sum)+1; 
        /*put the pos_node to the head of the linked list*/
        Change_head(&(head_data_info->l_list),pos_node);
        
        break;
    
    case 1:
        /*we replace from the tail*/
        pos_node=head_data_info->l_list->tail;
        /*if there is no pos_node, return error*/
        for(int i=0;i<200 ;i++){
            if (pos_node==NULL){
                i = 200;
            }
            /*if pos_node is not used currently*/
            else if (pos_node->pin_list!=0){
                /*throw the tail node*/
                pos_node=pos_node->pre_node;    
            }else{
                i = 200;
            }
        }
        /*if there is no pos_node, return error*/
        if(pos_node==NULL){
            return RC_ERROR_OTHER;
        }
        break;
    }
    /*put the pos_node to the head of the linked list*/
    Change_head(&(head_data_info->l_list),pos_node);

    update_info(bm,page,pos_node,pageNum);
    
}
RC LRUk_STRA(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
  
    /*set the initial k*/
    int k = 2;
    /*find the specific node for the page required*/
    frame_node *pos_node = PageSearching(bm, page, pageNum);
    /*get the linked_list*/
    bf_head_data *head_data_info = (bf_head_data *)bm -> mgmtData;

    /*ts_record add one*/
    head_data_info->ts_record++;

    /*if find the page, then k is 1, other nodes' k plus and return ok */
    if(pos_node!=NULL){ 
        /*add the time stamp to the node*/
        for (int j = 0; j < 10; j++ )
        {
            if (pos_node->ts[j] == 0){
                pos_node->ts[j] = head_data_info->ts_record;
            }
        }

        pos_node=head_data_info->l_list->tail;
        /*print the pagenumber*/
        while (pos_node!=NULL)
        {
            printf("/%d",pos_node->pageNumber);
            /*if the node is pinned before, and plus one because new node is 1th*/
            pos_node = pos_node->pre_node;
        }
        return RC_OK;
    }

    /*no node means: 1. not full, add directly 2. full, we should replace.*/
    /*not full, we can add directly*/
    int case_judge;
    /*the page existing is smaller than the numPages*/
    if ((head_data_info->bf_page_sum)<bm->numPages){
        case_judge = 0;
    }
    /*the page existing is equal to or larger than the numPages*/
    if (((head_data_info->bf_page_sum)>bm->numPages)||((head_data_info->bf_page_sum)==bm->numPages)){
        case_judge = 1;
    }
    
    frame_node *test_node =head_data_info->l_list->head;
    int judge = 0;
    int mints = 500;
    switch ((case_judge))
    {
    case 0:

        /*firstly, we should get the head node*/
        pos_node=head_data_info->l_list->head;
        
        int a=0;
        /*put the pointer to the tail of the linked list*/
        while(a<head_data_info->bf_page_sum){
            pos_node=pos_node->next_node;
            a = a + 1;
        }
        /*add one page and bf_page_sum plus 1*/
        (head_data_info->bf_page_sum) = (head_data_info->bf_page_sum)+1; 
        /*record timestamp it's accessed*/
        /*add the time stamp to the node*/
        for (int j = 0; j < 10; j++ )
        {
            if (pos_node->ts[j] == 0){
                pos_node->ts[j] = head_data_info->ts_record;
            }
        }

        
        break;
    
    case 1:
        pos_node=head_data_info->l_list->head;
        while (pos_node!=NULL)
        {
            /*every node k plus one except the new node*/
            if (pos_node->ts[k-1] == 0){
                judge = 1;
                break;
            }
            pos_node = pos_node->next_node;
        }

        if (judge == 1){
        /*if there is no pos_node, return error*/
        if(pos_node==NULL){
            return RC_ERROR_OTHER;
        }
        /*judge whether this node is in pin*/
        if (pos_node==NULL){}
            else if (pos_node->pin_list!=0){
                /*throw the tail node*/
                pos_node=pos_node->pre_node;    
            }else{}
            break;
        }

        /*find the element with the min ts*/

        pos_node=head_data_info->l_list->head;
        while (pos_node != NULL){
            if (mints > pos_node->ts[2-k]){
                mints = pos_node->ts[2-k];
            }
            pos_node = pos_node->next_node;
        }
        
        pos_node=head_data_info->l_list->head;
        while (pos_node != NULL){
            if (mints == pos_node->ts[2-k]){
                pos_node->ts[2-k] = head_data_info->ts_record;
                pos_node->ts[1] = 0;
                break;
            }
            pos_node = pos_node->next_node;
        }

        /*if there is no pos_node, return error*/
        if(pos_node==NULL){
            return RC_ERROR_OTHER;
        }
        /*judge whether this node is in pin*/
        if (pos_node==NULL){}
            else if (pos_node->pin_list!=0){
                /*throw the tail node*/
                pos_node=pos_node->pre_node;    
            }else{}
        
        break;
    }

    /*print the pagenumber to see the process*/
    test_node =head_data_info->l_list->head;
        while (test_node!=NULL)
        {
            printf("**%d",test_node->pageNumber);
            test_node = test_node->next_node;
        }
    /*print the k of the process*/
    test_node =head_data_info->l_list->head;
        while (test_node!=NULL)
        {
            printf("//%d",test_node->ts[1]);
            test_node = test_node->next_node;
        }

    update_info(bm,page,pos_node,pageNum);

}

/* ############################# bufferpool #############################*/

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{    
    SM_FileHandle file_han;
    if (numPages <= 0)
    {
        return RC_ERROR_PAGE;
    }
    RC judge = openPageFile((char *)pageFileName, &file_han);
    if (judge != RC_OK)
    {
        closePageFile(&file_han);
        bm->pageFile = "fail";
        return RC_ERROR_ALL_INVALID_BIN;
    }

    /*Initialize Buffer Pool*/
    int meta_data_size;
    meta_data_size = sizeof(bf_head_data);
    bf_head_data *head_data_info = calloc(1,meta_data_size);

    /*Initialize the info of buffer pool data*/
    Initial_buffer_pool_info((head_data_info),stratData);
    
    /*Initial Buffer Pool*/
    head_data_info->ts_record = 0;
    bm->mgmtData = head_data_info;
    bm->numPages = numPages;
    bm->strategy = strategy;
    bm->pageFile = (char *)pageFileName;

    /*initial the linked list*/
    head_data_info->l_list = malloc(sizeof(bf_linked_list));
    Initial_linked_list((head_data_info->l_list));

    /*add the elements to the linked list*/
    Init_add_linked_list((head_data_info->l_list),numPages);

    return RC_OK;
    
}
RC shutdownBufferPool(BM_BufferPool *const bm)
{
    /*test error*/
    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }

    /*clean all dirty pages*/
    forceFlushPool(bm);

    bf_head_data *head_data_info = (bf_head_data *)bm->mgmtData;
    frame_node *test_node = head_data_info->l_list->head;

    /*delete*/
    for (int i=0;i<200;i++) 
    {
        test_node = test_node->next_node;
        free(head_data_info->l_list->head->data);
        free(head_data_info->l_list->head);
        head_data_info->l_list->head = test_node;
        
        if (test_node == NULL){
            i=200;
        }
    }

    free(head_data_info->l_list);
    free(head_data_info);

    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm)
{
    /*test error*/
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

    bf_head_data *frame_data = (bf_head_data *)bm->mgmtData;
    frame_node *shut_node = frame_data->l_list->head;
 
    /*find dirty page*/
    for (int i=0;i<200;i++) 
    {
        if (shut_node->dirtyFlag == 1)
        {
            shut_node->dirtyFlag = 0;
            (frame_data->write_times) = (frame_data->write_times)+1;
            writeBlock(shut_node->pageNumber, &fhandel, shut_node->data);
        }
        shut_node = shut_node->next_node;

        if (shut_node == NULL){
            i=200;
        }
    }
 
    closePageFile(&fhandel);
    return RC_OK;
}

/* Access the Pages in the bf */
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    bf_head_data *iniData = (bf_head_data *)bm->mgmtData;

    /*find the dirty page*/
    frame_node *s_node = iniData->l_list->head; 
    int a = 0;
    while ((s_node != NULL)&&(a == 0))
    {
        if (s_node->pageNumber == page->pageNum) 
        {
            a = 1;
        }
        if (a == 0){
        s_node = s_node->next_node;
        }
    }

   if (s_node == NULL){
        return RC_ERROR_No_FIND_NODE;
    }

    s_node->dirtyFlag = 1;
    return RC_OK;
    
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    
    bf_head_data *iniData = (bf_head_data *)bm->mgmtData;

    /*find the unpin page*/
    frame_node *s_node = iniData->l_list->head; 
    int a = 0;
    while ((s_node != NULL)&&(a == 0))
    {
        if (s_node->pageNumber == page->pageNum) 
        {
            a = 1;
        }
        if (a == 0){
        s_node = s_node->next_node;
        }
    }
    if (s_node == NULL){
        return RC_ERROR_No_FIND_NODE;
    }

    if (s_node->pin_list == 1)
    {
        (s_node->pin_list)--;
    }
    return RC_OK;
}

RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    /*test error*/
    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (page < 0){
        return RC_ERROR_Frame_Number;
    }

    SM_FileHandle fh;
    bf_head_data *head_data_info = (bf_head_data *)bm->mgmtData;
    /*open the file*/
    openPageFile((char *)(bm->pageFile), &fh);

    /*write to the disk*/
    frame_node *s_node = head_data_info->l_list->head; 
    int a = 0;
    while ((s_node != NULL)&&(a == 0))
    {
        if (s_node->pageNumber == page->pageNum) 
        {
            a = 1;
        }
        if (a == 0){
        s_node = s_node->next_node;
        }
    }

   if (s_node == NULL){
        return RC_ERROR_No_FIND_NODE;
    }

    writeBlock(s_node->pageNumber, &fh, s_node->data);
    closePageFile(&fh);
    (head_data_info->write_times)++;
    return RC_OK;
    
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum)
{
    /*test error*/
    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (page < 0){
        return RC_ERROR_Frame_Number;
    }
   if (pageNum < 0){
        return RC_ERROR_Frame_Number;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }

    /*judge whether the strategy is*/
    int sg = bm->strategy;
    if (sg == 0){
        return FIFO_STRA(bm,page,pageNum);
    }
    if (sg == 1){
        return LRU_STRA(bm,page,pageNum);
    }
    if (sg == 4){
        return LRUk_STRA(bm,page,pageNum);
    }

}

/* Statistics Interface */
PageNumber *getFrameContents(BM_BufferPool *const bm) {
    if (bm == NULL){
        return NULL;
    }
    PageNumber *num = ((bf_head_data *)bm -> mgmtData)->frames_list;
    return num;
}

bool *getDirtyFlags(BM_BufferPool *const bm) {
    bf_head_data *head_data_info = (bf_head_data *)bm -> mgmtData;
    frame_node *dir_node = head_data_info -> l_list -> head;

    for (int i=0;i<(head_data_info -> l_list->length);i++) 
    {
        (head_data_info ->dirty_list)[dir_node->frameNumber] = dir_node->dirtyFlag;
        dir_node = dir_node -> next_node;
    }
    return head_data_info->dirty_list;
}

int *getFixCounts(BM_BufferPool *const bm) {
     bf_head_data *head_data_info = (bf_head_data *)bm->mgmtData;
    frame_node *fix_node = head_data_info->l_list->head;

    for (int i=0;i<(head_data_info -> l_list->length);i++) 
    {
        (head_data_info ->pin_list)[fix_node->frameNumber] = fix_node->pin_list;
        fix_node = fix_node -> next_node;
    }
    return head_data_info->pin_list;
}

int getNumReadIO(BM_BufferPool *const bm) {
    if (bm == NULL){
        return 0;
    }
    int num = ((bf_head_data *)bm -> mgmtData)->read_times;
    return num;
}

int getNumWriteIO(BM_BufferPool *const bm) {
    if (bm == NULL){
        return 0;
    }
    int num = ((bf_head_data *)bm -> mgmtData)->write_times;
    return num;
}


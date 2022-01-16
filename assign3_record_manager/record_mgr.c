#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"


void *compare2 (void *char_1, const void *char_2, size_t i)
{
	char *c1 = char_1;
	const char *c2 = char_2;
	for ( i; i>0; i--)
	{
		*c1++ = *c2++;
	}

	return char_1;
}


extern RC initRecordManager (void *mgmtData)
{   
	if (mgmtData == NULL){
    	initStorageManager();
	}else{
		return RC_init_eeror;
	}
	printf("initialize the record manager");
	return RC_OK;
}


typedef struct ritem_struc
{
	BM_BufferPool buffer;	  
	BM_PageHandle pHandler;     
	RID rID;					
	Expr *expr;					
	int tup_sum;			
	int p_find;					
	int scn_sum;				
} ritem_struc;

ritem_struc *re_item_s;

extern RC shutdownRecordManager ()
{
	if (re_item_s == NULL){
		free(re_item_s);
		return RC_OK;
	} 
	else{ 
		re_item_s = NULL;
		free(re_item_s);
		return RC_OK;
	}
}


char *compare(char *dest,const char *source, size_t count){
		int a = count;
		char *start = dest;
		for (int k=0; k < 20000; k++){
			if ((a)&&(*dest++ = *source++)){
				break;
			}a--;
		}
		/* pad out with zeroes */
		if (a){
			while (--a){
				*dest++ ='\0';
			}
		}
		return start;
}

extern RC createTable (char *name, Schema *schema)
{   
	re_item_s = (ritem_struc*)calloc(1,sizeof(ritem_struc));
	initBufferPool(&re_item_s->buffer, name, 100, RS_LRU, NULL);

	char data[PAGE_SIZE];
	char *pHandler = data;

	/*the page handle plus int size*/
	*(int*)pHandler = 0; 
	int tmp_offset = (*&*pHandler + 1)*4; 
	pHandler = pHandler + tmp_offset;
	int tmp_offset_2 = (double)(tmp_offset/4);
	*(int*)pHandler =  tmp_offset_2;
	pHandler = pHandler + tmp_offset;

    /*give the number of attrs space*/
	*(int*)pHandler = schema->numAttr; 
	pHandler = pHandler + tmp_offset;
	*(int*)pHandler = schema->keySize; 
	pHandler = pHandler + tmp_offset_2*4;

	/*initialize the atrributes.*/
	int number_attribute = schema->numAttr;
	int count = 0;

	while (count != number_attribute){

		compare(pHandler, schema->attrNames[count], 20);
		pHandler = pHandler + 20;
		int data_nums = (int)schema->dataTypes[count];
		*(int*)pHandler = data_nums;
		pHandler = pHandler + tmp_offset;
		*(int*)pHandler = data_nums;
		pHandler = pHandler + tmp_offset_2*4;

	count++;
	}

	int resultCode;
	SM_FileHandle fHandler;

	int step = 1;
		switch(step)
		{ 
			case 1:
				if((resultCode = createPageFile(name)) != RC_OK){
					return resultCode;
				}
			case 2:
				if((resultCode = openPageFile(name, &fHandler)) != RC_OK)
					return resultCode;
			case 3:
				if((resultCode = writeBlock(0, &fHandler, data)) != RC_OK)
					return resultCode;
			case 4:
				if((resultCode = closePageFile(&fHandler)) != RC_OK)
					return resultCode;
				break;
			default:
				return RC_file_fail;
				break;
		}
	
	return RC_OK;
}

extern RC openTable (RM_TableData *rel, char *name)
{
	int attributeCount, i;
	rel->name = name;
	rel->mgmtData = re_item_s;
	
	pinPage(&re_item_s->buffer, &re_item_s->pHandler, 0);
	SM_PageHandle pHandler = (char*) re_item_s->pHandler.data;
	
	/*get the information from page files*/
	re_item_s->tup_sum= *(int*)pHandler;
	pHandler = pHandler + 4;

	/*get the free pages*/
	re_item_s->p_find= *(int*) pHandler;
	pHandler = pHandler + 4;
	attributeCount = *(int*)pHandler;
	pHandler = pHandler + 4;
 	
	/*fill the attrs info to the schema*/
	Schema *schema;
	schema = (Schema*) calloc(1,sizeof(Schema));
	char* att_spac = (char*) calloc(1,500);
	schema->numAttr = attributeCount;
	schema->attrNames = (char**) calloc(1,sizeof(char*) *attributeCount);
	char *result = calloc(1,PAGE_SIZE);
	strcat(result,",");
	schema->typeLength = (int*) calloc(1,sizeof(int) *attributeCount);
	schema->dataTypes = (DataType*) calloc(1,sizeof(DataType) *attributeCount);

	int att_num = attributeCount;
	int count = 0;
	while (count != att_num){
		schema->attrNames[count]= (char*) calloc(1,300);
		count ++;
	}
    free(att_spac);
	free(result);
	att_num = schema->numAttr;
	count = 0;
	while (count != att_num){
		compare(schema->attrNames[count], pHandler, 20);
		pHandler = pHandler + 20;

		schema->dataTypes[count]= *(int*) pHandler;
		pHandler = pHandler + sizeof(int);

		schema->typeLength[count]= *(int*)pHandler;
		pHandler = pHandler + sizeof(int);

		count ++;
	}

	rel->schema = schema; 
	unpinPage(&re_item_s->buffer, &re_item_s->pHandler); 
	forcePage(&re_item_s->buffer, &re_item_s->pHandler); 
	return RC_OK;
}   

extern RC closeTable (RM_TableData *rel)
{
	if (rel==NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	else 
	{
		ritem_struc *mgm_store = rel->mgmtData; 
		BM_PageHandle handle_info = mgm_store->pHandler;
		&handle_info == NULL;
		return RC_OK;
	}
	
	return RC_OK;

}


/*delet the table*/
extern RC deleteTable (char *name)
{
	int judge_ok;
	if((judge_ok = destroyPageFile(name)) != RC_OK){
		return RC_FILE_NOT_FOUND;
	}
	else{
		return RC_OK;
	}
}

int num_tuple(int number_tuples){
	if (number_tuples !=0){
		return number_tuples;
	}
	return RC_ERROR_OTHER;
}

/*return the nubmer of tuples*/
extern int getNumTuples (RM_TableData *rel)
{
	ritem_struc *other_data = rel->mgmtData;
	int number_tuples = other_data->tup_sum;
	num_tuple(number_tuples);
}

void *pin_unpin_process (BM_BufferPool *a1, BM_PageHandle *a2, bool judge_s)
{	
	if (judge_s){
	unpinPage(a1,a2);}
	else{
	pinPage(a1,a2,0);
	}
}

void *mark_unpin_process (BM_BufferPool *a1, BM_PageHandle *a2)
{	
	markDirty(a1,a2);

	unpinPage(a1,a2);
	
}


extern RC insertRecord (RM_TableData *rel, Record *record)
{
	/*get the head data info*/
	ritem_struc *re_item_s = rel->mgmtData;

	/*choose the pinned page*/
	BM_BufferPool *buffer_attr = &re_item_s->buffer;
	RID *rID = &record->id;
	char *result = calloc(1,PAGE_SIZE);
	rID->page = re_item_s->p_find;
	BM_PageHandle *hand_attr = &re_item_s->pHandler;
	pinPage(buffer_attr, hand_attr, rID->page);

	/*find the avalialbe slots*/
	float size_item = (float)getRecordSize(rel->schema);
	char *Handler_value;
	strcat(result,",");
	Handler_value = (*hand_attr).data;
	int size_item_ava= (int)size_item;
	float slot_div = 4096 / size_item_ava;
	int slot_ava = (int)slot_div;
	int k;
	int count = 0;
	while (count != slot_ava){
		char judge_str = Handler_value[count * size_item_ava];
		if (judge_str != '+'){
			k = count;
			break;
		}
		count ++;
	}
	if (k == count) {}
	else {
		k = -1;
	}
	rID->slot = k;	
	/*find the free slots, jump while there is slot*/
	bool judge_for = false;
	if (rID->slot == -1){
		judge_for = true;
	}

	bool judge_s = true;
	
	if (judge_for){
		for (int i; i < 20000000; i ++ ){
			BM_BufferPool *buffer_attr_unpin = &re_item_s->buffer;
			BM_PageHandle *hand_attr_unpin = &re_item_s->pHandler;
			/*unpin the page which is full*/
			judge_s = true;
			pin_unpin_process(buffer_attr_unpin,hand_attr_unpin,judge_s);
			rID->page++;
			/*pin the page which is not full*/
			pinPage(buffer_attr_unpin,hand_attr_unpin, rID->page);

			Handler_value = re_item_s->pHandler.data;

				float slot_div = 4096 / size_item_ava;
				int slot_ava = (int)slot_div;
				int k;
				int count = 0;
				while (count != slot_ava){
					char judge_str = Handler_value[count * size_item_ava];
					if (judge_str != '+'){
						k = count;
						break;
					}
					count ++;
				}
				if (k == count) {}
				else {
					k = -1;
				}
				rID->slot = k;	
			if (rID->slot != -1){
				break;
			}
		}
	}

	/*mark dirty if there is modification*/
	char *P_slot;
	BM_BufferPool *buffer_attr_dirty = &re_item_s->buffer;
	BM_PageHandle *hand_attr_dirty = &re_item_s->pHandler;
	markDirty(buffer_attr_dirty, hand_attr_dirty);
	P_slot = Handler_value;
	P_slot = P_slot + (rID->slot * size_item_ava);
	*P_slot = '+'; 
	P_slot++; 
	compare2(P_slot, record->data + 1, size_item_ava - 1);
	pin_unpin_process(buffer_attr_dirty, hand_attr_dirty,judge_s);
	re_item_s->tup_sum++;
	judge_s = false;
	pin_unpin_process(buffer_attr_dirty, hand_attr_dirty,judge_s);

	return RC_OK;
}

extern RC deleteRecord (RM_TableData *rel, RID id)
{
	ritem_struc *re_item_s = rel->mgmtData;

	BM_BufferPool *buffer_att = &re_item_s->buffer;
	BM_PageHandle *hand_att = &re_item_s->pHandler;

	pinPage(buffer_att, hand_att, id.page);
	re_item_s->p_find = id.page;

	char *h_value = re_item_s->pHandler.data;
	int recordsNum = getRecordSize(rel->schema);
	h_value = h_value + (id.slot * recordsNum);
	*h_value = '-';
	mark_unpin_process(buffer_att, hand_att);
	return RC_OK;
}

extern RC updateRecord (RM_TableData *rel, Record *record)
{
	ritem_struc *re_item_s = rel->mgmtData; 
	BM_BufferPool *buffer_att = &re_item_s->buffer;
	BM_PageHandle *hand_att = &re_item_s->pHandler;
	pinPage(buffer_att, hand_att, record->id.page);
	
	char *val;
	int recordsNum = getRecordSize(rel->schema);
	RID id = record->id;
	val = re_item_s->pHandler.data; 
	val = val + (id.slot * recordsNum);
	*val = '+';
	val++;  
	compare2 (val, record->data + 1, recordsNum - 1 ); 
	mark_unpin_process(buffer_att, hand_att);
	return RC_OK;	
}

extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
	ritem_struc *re_item_s = rel->mgmtData;
	pinPage(&re_item_s->buffer, &re_item_s->pHandler, id.page);
	int recordsNum = getRecordSize(rel->schema);
	char *recordPointer = re_item_s->pHandler.data;
	recordPointer = recordPointer + (id.slot * recordsNum);
	if(*recordPointer == '+') // check the record has space and search the specific one.
	{
        record->id = id;
        char *data = record->data;
        compare2(++data, recordPointer + 1, recordsNum - 1);
        unpinPage(&re_item_s->buffer, &re_item_s->pHandler);
        return RC_OK;
	}
    return RC_ERROR_OTHER;
}


extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	openTable(rel, "ScanTable");
	ritem_struc *search_mg;
	search_mg = (ritem_struc*) calloc(1,sizeof(ritem_struc));
	scan->mgmtData = search_mg;

	/*initial slot is 0*/
	search_mg->rID.slot = 0;
	/*initial page is 1*/
	search_mg->rID.page = 1; 
	search_mg->scn_sum = 0; 
	search_mg->expr = cond;

	ritem_struc *t_mg;
	t_mg = rel->mgmtData;
	t_mg->tup_sum = 20;
	scan->rel= rel;

	return RC_OK;

}

extern RC next (RM_ScanHandle *scan, Record *record)
{
	Schema *schema = scan->rel->schema;
	Value *value = (Value *) calloc(1,sizeof(Value));
	int recordsNum = getRecordSize(schema);
	ritem_struc *scanMgt = scan->mgmtData;
	int scn_sum = scanMgt->scn_sum;
	ritem_struc *tableMgt = scan->rel->mgmtData;
	if (tableMgt->tup_sum == 0)
		return RC_ERROR_OTHER;

	if (scanMgt->expr != NULL)
	{
        Value *value = (Value *) calloc(1,sizeof(Value));
        char *recordData;
        int totalSlots = 4096 / recordsNum;
        int tup_sum = tableMgt->tup_sum;
        if (tup_sum == 0)
            return RC_ERROR_No_BF;
		int ju_d = tup_sum +1;
		for (int i=0; i <ju_d;i++){
			int expression = 0;
			if (scn_sum <= 0){
				expression = 1; 
			}else{
				expression = 2;
			}
			
			switch (expression)
			{
			case 1:
				scanMgt->rID.page = 1;
                scanMgt->rID.slot = 0;
				break;
			case 2:
                scanMgt->rID.slot++;
				while (scanMgt->rID.slot >= totalSlots){
					scanMgt->rID.slot = 0;
                    scanMgt->rID.page++;
					break;
				}
			default:
				break;
			}
            pinPage(&tableMgt->buffer, &scanMgt->pHandler, scanMgt->rID.page);

            recordData = scanMgt->pHandler.data;
            recordData = recordData + (scanMgt->rID.slot * recordsNum);
            record->id.slot = scanMgt->rID.slot;
            record->id.page = scanMgt->rID.page;
            char *recordPointer = record->data;
            *recordPointer = '-';

            memcpy(++recordPointer, recordData + 1, recordsNum - 1);
            scanMgt->scn_sum++;
            scn_sum++;
            evalExpr(record, schema, scanMgt->expr, &value);
			bool judge = value->v.boolV;
			while (judge){
				unpinPage(&tableMgt->buffer, &scanMgt->pHandler);
                return RC_OK;
			}
		}

        unpinPage(&tableMgt->buffer, &scanMgt->pHandler);
		//reset the value of scan function
        scanMgt->rID.page = 1;
        scanMgt->rID.slot = 0;
        scanMgt->scn_sum = 0;

        return RC_RM_NO_MORE_TUPLES;
	}
    return RC_ERROR_No_PG;

}

extern RC closeScan (RM_ScanHandle *scan)
{
	ritem_struc *scan_item = scan->mgmtData;
	ritem_struc *item_cont = scan->rel->mgmtData;
	BM_BufferPool *buffer_scan = &item_cont->buffer;
	BM_PageHandle *hand_scan = &item_cont->pHandler;

	unpinPage(buffer_scan, hand_scan);
	scan_item->scn_sum = 0;
	scan_item->rID.page = 1;
	scan_item->rID.slot = 0;
	
    scan->mgmtData = NULL;
    free(scan->mgmtData);

	return RC_OK;
}


extern int getRecordSize (Schema *schema)
{
	int offSet = 0;

	int number_datatype = schema->numAttr;
	int *s_len = schema->typeLength;
	int count = 0;
	while (count != number_datatype){
		if (schema->dataTypes[count] == 0){
			offSet = offSet + sizeof(int);
		}
		else if (schema->dataTypes[count] == 1){
			offSet = offSet + s_len[count];
		}
		else if (schema->dataTypes[count] == 2){
			offSet = offSet + sizeof(float);
		}
		else if (schema->dataTypes[count] == 3){
			offSet = offSet + sizeof(bool);
		}
		else{
			return RC_NOT_Datatype;
		}
		count++;
		if (count == number_datatype){
			offSet = offSet + 1;
		}
	}
	
	return offSet;
}

/* create schema */
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema = (Schema *) calloc(1,sizeof(Schema));
	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keySize = keySize;
	schema->keyAttrs = keys;
	return schema; 
}

extern RC freeSchema (Schema *schema)
{
	free(schema);
	return RC_OK;
}



extern RC createRecord (Record **record, Schema *schema)
{
    int size_of_record = sizeof(record);
    *record = (Record*)calloc(1,size_of_record);
    int recordSize = getRecordSize(schema);
    (*record) -> data = (char*) calloc(1,recordSize);
	(*record) ->id.page=(*record) ->id.slot-1;
	char *PointerToData=(*record)->data;
	*PointerToData='-';
	*(++PointerToData)='\0'; 
    return RC_OK;
}

extern RC freeRecord (Record *record)
{
	free(record);
	return RC_OK;
}
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	int dataType = 1;
	int number_datatype = attrNum;
	int *s_len = schema->typeLength;
	int count = 0;
	while (count != number_datatype){
		if (schema->dataTypes[count] == 0){
			dataType = dataType + sizeof(int);
		}
		else if (schema->dataTypes[count] == 1){
			dataType = dataType + s_len[count];
		}
		else if (schema->dataTypes[count] == 2){
			dataType = dataType + sizeof(float);
		}
		else if (schema->dataTypes[count] == 3){
			dataType = dataType + sizeof(bool);
		}
		else{
			return RC_NOT_Datatype;
		}
		count++;
	}

	Value *attr_need = (Value*) calloc(1,sizeof(Value));

	if (attrNum == 1){
		schema->dataTypes[attrNum] = 1;
	}else{
	}

	int number_datatype_2 = schema->dataTypes[attrNum];

	if (number_datatype_2 == 0){
			char *recordPointer = record->data;
			recordPointer = recordPointer + dataType;
			attr_need->dt = 0;
			int value = 0;
			compare2(&value, recordPointer, sizeof(int));
			attr_need->v.intV = value;
	}

	else if (number_datatype_2 == 1){
			char *recordPointer = record->data;
			recordPointer = recordPointer + dataType;
			attr_need->dt = 1;
			int length = schema->typeLength[attrNum];
			attr_need->v.stringV = (char *) calloc(1,length + 1);
			strncpy(attr_need->v.stringV, recordPointer, length);
			/*becuse the end of the string is 0*/
			attr_need->v.stringV[length] = '\0';
	}
	
	else if (number_datatype_2 == 2){
			char *recordPointer = record->data;
			recordPointer = recordPointer + dataType;
			attr_need->dt = 2;
	  		float value;
	  		compare2(&value, recordPointer, sizeof(float));
	  		attr_need->v.floatV = value;
	}

	else if (number_datatype_2 == 3){
			char *recordPointer = record->data;
			recordPointer = recordPointer + dataType;
			attr_need->dt = 3;
			bool value;
			compare2(&value,recordPointer, sizeof(bool));
			attr_need->v.boolV = value;
	}

	else{
		return RC_NOT_Datatype;
	}

	*value = attr_need;
	return RC_OK;
}

/*set the attribute value*/
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{

	int dataType = 1;
	int number_datatype = attrNum;
	int *s_len = schema->typeLength;
	int count = 0;
	while (count != number_datatype){
		if (schema->dataTypes[count] == 0){
			dataType = dataType + sizeof(int);
		}
		else if (schema->dataTypes[count] == 1){
			dataType = dataType + s_len[count];
		}
		else if (schema->dataTypes[count] == 2){
			dataType = dataType + sizeof(float);
		}
		else if (schema->dataTypes[count] == 3){
			dataType = dataType + sizeof(bool);
		}
		else{
			return RC_NOT_Datatype;
		}
		count++;
	}

	char *recordPointer = record->data;
	recordPointer = recordPointer + dataType;
		
	int number_datatype_2 = schema->dataTypes[attrNum];

	if (number_datatype_2 == 0){
		*(int *) recordPointer = value->v.intV;	  
		recordPointer = recordPointer + sizeof(int);
	}

	else if (number_datatype_2 == 1){
		int length = schema->typeLength[attrNum];
		strncpy(recordPointer, value->v.stringV, length);
		recordPointer = recordPointer + schema->typeLength[attrNum];
	}
	
	else if (number_datatype_2 == 2){
		*(float *) recordPointer = value->v.floatV;
		recordPointer = recordPointer + sizeof(float);
	}

	else if (number_datatype_2 == 3){
		*(bool *) recordPointer = value->v.boolV;
		recordPointer = recordPointer + sizeof(bool);
	}

	else{
		return RC_NOT_Datatype;
	}
			
	return RC_OK;
}

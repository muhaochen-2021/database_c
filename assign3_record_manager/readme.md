# Assign3_Record_Manager

## Team Member:
Hao	Tianyi	  thao3@hawk.iit.edu \
Cui	Rongbin 	rcui7@hawk.iit.edu  \
Chen Muhao	  mchen69@hawk.iit.edu 

## Test and Memory Leak:
Go through the test and there is no memory leaks.

## Folder structure:
assign3
-     README.md
      buffer_mgr_stat.c
      buffer_mgr_stat.h
      buffer_mgr.c
      buffer_mgr.h
      dberror.c 
      dberror.h 
      dt.h
      expr.c
      expr.h
      record_mgr.c
      record_mgr.h
      storage_mgr.c 
      storage_mgr.h
      tables.h
      test_assign3_1_V2.c
      test_expr.c 
      rm_serializer.c
      test_helper.h 
      Makefile 
      
## Run Program:
### 1. environment
   We recommend using Linux and gcc environment. \ 
### 2. start  
   ```bash
    > $make
    > $./assign_3_test1
    > $./assign_3_test2
    > $make clean
   ```
## Source code
### compare
Used to copy the first n characters of a string.
### compare2
The continuous n bytes of data with the address pointed to by source as the starting address are copied to the space with the address pointed to by dest as the starting address.
### initRecordManager 
This function is used to initialize the record manager.
### shutdownRecordManager 
This function is used to turn off the record manager.
### createTable 
Create a table in the record manager to implement related functions.
### openTable 
Open the table created in the creatorable.
### closeTable 
Close the table created in the creatorable.
### deleteTable
Delete the table created in the creatorable. 
### getNumTuples 
Get the number of tuples in the record manager.
### insertRecord 
Add record to the created table.
### deleteRecord 
DElete record to the created table.
### updateRecord 
Update the information of record in the created table.
### getRecord 
Get the information of the record declared in the function.


### startScan 
Starting a scan initializes the RM_ScanHandle data structure passed as an argument to 
### next 
Return the next tuple that satisfied the scan condition. If NULL is passed as a scan condition, then all tuples of the table should be returned.
### closeScan 
clean all connected resources.

### getRecordSize 
It will count the size of the record and return it.
### Schema 
It will create a schema that contains attribute that should be built in.
### freeSchema 
Free the memory of the schema.

### createRecord 
It will allocate the memory for the record. 
### freeRecord 
It will free the memort space of record
### getAttr 
It will fetch the attributes of records
### setAttr 
It will set the attributes of records 
   

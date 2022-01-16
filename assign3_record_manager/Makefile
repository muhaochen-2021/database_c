CC=gcc

all: assign_3_test1 assign_3_test2 


assign_3_test2: test_expr.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o -lm buffer_mgr_stat.o
	$(CC) -o assign_3_test2 test_expr.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o

assign_3_test1: test_assign3_1_V2.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o -lm buffer_mgr_stat.o
	$(CC) -o assign_3_test1 test_assign3_1_V2.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o

storage_mgr.o:storage_mgr.c
	$(CC) -c storage_mgr.c
	
dberror.o:dberror.c
	$(CC) -c dberror.c
	
buffer_mgr.o:buffer_mgr.c
	$(CC) -c buffer_mgr.c
	
buffer_mgr_stat.o:buffer_mgr_stat.c
	$(CC) -c buffer_mgr_stat.c
	
expr.o: expr.c
	$(CC) -c expr.c

rm_serializer.o:rm_serializer.c
	$(CC) -c rm_serializer.c

record_mgr.o:record_mgr.c
	$(CC) -c record_mgr.c


run2: assign_3_test1
	./assign_3_test1

run3: assign_3_test2
	./assign_3_test2

.PHONY:clean
clean:
		rm *.o assign_3_test1 assign_3_test2

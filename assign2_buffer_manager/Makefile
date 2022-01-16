CC=gcc


all: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.c storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o
	$(CC) -o test_assign2_1 test_assign2_1.c storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o

test_assign2_2: test_assign2_2.c storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o
	$(CC) -o test_assign2_2 test_assign2_2.c storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o

storage_mgr.o:storage_mgr.c
	$(CC) -c storage_mgr.c
dberror.o:dberror.c
	$(CC) -c dberror.c
buffer_mgr.o:buffer_mgr.c
	$(CC) -c buffer_mgr.c
buffer_mgr_stat.o:buffer_mgr_stat.c
	$(CC) -c buffer_mgr_stat.c


run1: test_assign2_1
	./test_assign2_1

run2:test_assign2_2
	./test_assign2_2

.PHONY:clean
clean:
		rm *.o test_assign2_1 test_assign2_2
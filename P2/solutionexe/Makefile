CC=gcc
CFLAGS=-g

SRCDIR=src
INCLDIR=include
LIBDIR=lib

mapreduce: $(SRCDIR)/mapreduce.c $(LIBDIR)/utils.o mapper reducer
	$(CC) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/mapreduce.c -o mapreduce

mapper: $(SRCDIR)/mapper.c $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/mapper.c -o mapper

reducer: $(SRCDIR)/reducer.c $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -I$(INCLDIR)  $(LIBDIR)/utils.o $(SRCDIR)/reducer.c -o reducer

$(LIBDIR)/utils.o: $(LIBDIR)/utils.c
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $(LIBDIR)/utils.c -o $(LIBDIR)/utils.o	

.PHONY: run clean t1 t2 t3

#500KB
t1:
# 	make -i clean
# 	make
	./mapreduce 5 2 test/T1/F1.txt

#44KB
t2:
# 	make -i clean
# 	make
	./mapreduce 5 2 test/T2/F2.txt

#0KB
t3:
# 	make -i clean
# 	make
	./mapreduce 5 2 test/T3/F3.txt

# m >= r

# m=1 r=1 (min case)
t4:
# 	make -i clean
# 	make
	./mapreduce 1 1 test/T1/F1.txt

# m=32 r=26 (max case)
t5:
# 	make -i clean
# 	make
	./mapreduce 32 26 test/T1/F1.txt


clean:
	rm lib/utils.o mapreduce mapper reducer
	rm -rf output
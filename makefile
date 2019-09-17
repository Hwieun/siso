TestCase_exe : Thread.c Init.c MsgQueue.c Plus.c Scheduler.c TestMain.c TestCase1.c TestCase2.c TestCase3.c TestCase4.c Plus.h
	gcc -o TestCase_exe Thread.c Init.c MsgQueue.c Plus.c Scheduler.c TestMain.c TestCase1.c TestCase2.c TestCase3.c TestCase4.c Plus.h -lpthread
Scheduler.o : Scheduler.c Thread.o
	gcc -c Scheduler.c Thread.o -lpthread
Thread.o : Thread.c Plus.h Init.h
	gcc -c Thread.c Plus.h Init.h -lpthread
Init.o : Init.c Thread.c Plus.h Init.h MsgQueue.h
	gcc -c Init.c Thread.c Plus.h Plus.c -lpthread
MsgQueue.o : MsgQueue.c MsgQueue.h Thread.c Thread.h Plus.h Plus.c
	gcc -c MsgQueue.c MsgQueue.h Thread.c Thread.h Plus.h Plus.c -lpthread
clean : 
	rm -rf *.o

# 多目标
all : test_logger.out test_noncopyable.out test_time.out
.PHONY : all

######################################################
obj1 = Logger_test.o Timestamp.o Logger.o
test_logger.out : $(obj1)
	g++ -o test_logger.out $(obj1)


#第二个可执行文件
obj2 = noncopyable_test.o
test_noncopyable.out : $(obj2)
	g++ -o test_noncopyable.out $(obj2)
	
# Timestamp_test

obj3 = Timestamp_test.o Timestamp.o
test_time.out : $(obj3)
	g++ -o test_time.out $(obj3)

########################################################
Logger_test.o : Logger.h

noncopyable_test.o : noncopyable.h

Timestamp_test.o : Timestamp.h

Logger.o : noncopyable.h Timestamp.h

Timestamp.o : copyable.h


.PHONY : clean

clean :
	rm -f *.out *.o
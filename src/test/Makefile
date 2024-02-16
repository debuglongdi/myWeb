
# 设置编译选项

CC := g++
CFLAGS := -g3 -O0 -Wall 
LIBS := -lmymuduo_utils -lmymuduo_net -lpthread


myserver :
	$(CC) -o myserver testServer.cc $(CFLAGS) $(LIBS)

clean :
	rm -f myserver *.o
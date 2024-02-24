
# 设置编译选项

CC := g++
CFLAGS := -g3 -O0 -Wall 
LIBS := -lmymuduo_utils -lmymuduo_net -lpthread

all: server client

server :
	$(CC) -o server testServer.cc $(CFLAGS) $(LIBS)

client :
	$(CC) -o client testClient.cc $(CFLAGS) $(LIBS)

clean :
	rm -f *.o server client
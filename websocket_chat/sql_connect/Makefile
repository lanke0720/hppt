ROOTPATH=$(shell pwd)
MYSQL_INCLUDE=$(ROOTPATH)/lib/include
MYSQL_LIB=$(ROOTPATH)/lib/lib

.PHONY:all
all:insert_cgi select_cgi

insert_cgi:insert_cgi.o sql_api.o
	g++ -o $@ $^ -L$(MYSQL_LIB) -lmysqlclient
select_cgi:select_cgi.o sql_api.o
	g++ -o $@ $^ -L$(MYSQL_LIB) -lmysqlclient

%.o:%.cpp
	g++ -c $< -I$(MYSQL_INCLUDE)

.PHONY:clean
clean:
	rm -f insert_cgi select_cgi *.o

.PHONY:debug
debug:
	echo $(MYSQL_INCLUDE)

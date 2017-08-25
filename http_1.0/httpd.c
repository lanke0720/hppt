#include "httpd.h"

void print_log(char *msg, int level)
{
#ifdef _STDOUT_
	const char * const level_msg[]={
		"SUCCESS",
		"NOTICE",
		"WARNING",
		"ERROR",
		"FATAL",
	};
	printf("[%s][%s]\n", msg, level_msg[level%5]);
#endif
}

int startup(const char *ip, int  port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		print_log(strerror(errno), FATAL);
		exit(2);
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		print_log(strerror(errno), FATAL);
		exit(3);
	}

	if(listen(sock, 10) < 0){
		print_log(strerror(errno), FATAL);
		exit(4);
	}
	return sock;
}

//ret > 1, line != '\0', ret=1&line='\n',  ret<=0&&line=='\0'
static int get_line(int sock, char line[], int size)
{
	// read 1 char , one by one
	char c = '\0';
	int len = 0;
	while( c != '\n' && len < size-1){
		int r = recv(sock, &c, 1, 0);
		if(r > 0){
		   if(c == '\r'){
			   //窥探
			   int ret = recv(sock, &c, 1, MSG_PEEK);
			   if(ret > 0){
				   if(c == '\n'){
					   recv(sock, &c, 1, 0);
				   }else{
					   c = '\n';
				   }
			   }
		   }// \r->\n \r\n -> \n
		   line[len++] = c;
		}else{
			c = '\n';
		}
	}
	line[len]='\0';
	return len;
}

static void echo_string(int sock)
{}

static int echo_www(int sock, char *path, int size)
{
	int fd = open(path, O_RDONLY);
	if(fd < 0){
		echo_string(sock);
		print_log(strerror(errno), FATAL);
		return 8;
	}

	const char *echo_line="HTTP/1.0 200 OK\r\n";
	send(sock, echo_line, strlen(echo_line), 0);
	const char *null_line="\r\n";
	send(sock, null_line, strlen(null_line), 0);

	if(sendfile(sock, fd, NULL, size) < 0){
		echo_string(sock);
		print_log(strerror(errno), FATAL);
		return 9;
	}

	close(fd);
	return 0;
}

static void drop_header(int sock)
{
	char line[1024];
	int ret = -1;
	do{
		ret = get_line(sock, line, sizeof(line));
	}while(ret>0 && strcmp(line, "\n"));
}

static int exe_cgi(int sock, char *method, \
		char *path, char *query_string)
{
	int content_len = -1;
	char method_env[SIZE/10];
	char query_string_env[SIZE];
	char content_len_env[SIZE/10];

	if( strcasecmp(method, "GET") == 0 ){
		drop_header(sock);
	}else{//POST
    	char line[1024];
    	int ret = -1;
    	do{
    		ret = get_line(sock, line, sizeof(line));
			if(ret > 0 &&\
			   strncasecmp(line,"Content-Length: ", 16)== 0){
				content_len = atoi(&line[16]);
			}
    	}while(ret>0 && strcmp(line, "\n"));
		if(content_len == -1){
			echo_string(sock);
			return 10;
		}
	}
	const char *echo_line="HTTP/1.0 200 OK\r\n";
	send(sock, echo_line, strlen(echo_line), 0);
	const char *type="Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock, type, strlen(type), 0);
	const char *null_line="\r\n";
	send(sock, null_line, strlen(null_line), 0);

	printf("query_string: %s\n", query_string);
	//path-> exe
	int input[2];
	int output[2];
	if(pipe(input) < 0 || pipe(output) < 0){
		echo_string(sock);
		return 11;
	}
	pid_t id = fork();
	if(id < 0){
		echo_string(sock);
		return 12;
	}else if(id == 0){//child
		close(input[1]);
		close(output[0]);
		sprintf(method_env, "METHOD=%s", method);
		putenv(method_env);

		if(strcasecmp(method, "GET") == 0){
			sprintf(query_string_env, "QUERY_STRING=%s", query_string);
			putenv(query_string_env);
		}else{ // POST
			sprintf(content_len_env, "CONTENT_LENGTH=%d", content_len);
			putenv(content_len_env);
		}
		dup2(input[0], 0);
		dup2(output[1], 1);
		execl(path, path, NULL);
		printf("execl error!\n");
		exit(1);
	}else{
		close(input[0]);
		close(output[1]);

		int i = 0;
		char c = '\0';
		if(strcasecmp(method, "POST") == 0){
		    for( ; i < content_len; i++ ){
		    	recv(sock, &c, 1, 0);
		    	write(input[1], &c, 1);
		    }
		}

		c='\0';
		while(read(output[0], &c, 1) > 0){
			send(sock, &c, 1, 0);
		}

		waitpid(id, NULL, 0);
		close(input[1]);
		close(output[0]);
	}
}

//thread 
void *handler_request(void *arg)
{
	int sock = (int)arg;
#ifdef _DEBUG_
	char line[1024];
	do{
		int ret = get_line(sock, line, sizeof(line));
		if(ret > 0){
			printf("%s", line);
		}else{
			printf("request ...... done!\n");
			break;
		}
	}while(1);
#else
	int ret = 0;
	char buf[SIZE];
	char method[SIZE/10];
	char url[SIZE];
	char path[SIZE];
	int i, j;
	int cgi = 0;
	char *query_string = NULL;
	if(get_line(sock, buf, sizeof(buf)) <= 0){
		echo_string(sock);
		ret = 5;
		goto end;
	}
	i=0;//method ->index
	j=0;//buf -> index

	while( !isspace(buf[j]) &&\
			j < sizeof(buf) &&\
			i < sizeof(method)-1){
		method[i]=buf[j];
		i++, j++;
	}
	method[i] = 0;
	if(strcasecmp(method, "GET") &&\
			strcasecmp(method, "POST") ){
		echo_string(sock);
		ret = 6;
		goto end;
	}
	if(strcasecmp(method, "POST") == 0){
		cgi = 1;
	}
	//buf -> "GET          /      http/1.0"
	while(isspace(buf[j]) && j < sizeof(buf)){
		j++;
	}
	i=0;
	while(!isspace(buf[j]) && j < sizeof(buf) && i < sizeof(url)-1){
		url[i] = buf[j];
		i++, j++;
	}
	url[i] = 0;
	printf("method: %s, url: %s\n", method, url);
	query_string = url;
	while(*query_string != '\0'){
		if(*query_string == '?'){
			*query_string = '\0';
			query_string++;
			cgi = 1;
			break;
		}
		query_string++;
	}
	sprintf(path, "wwwroot%s", url);
	//method, url, query_string, cgi
	if(path[strlen(path)-1] == '/'){ // '/'
		strcat(path, "index.html");
	}

	struct stat st;
	if(stat(path, &st) != 0){
		echo_string(sock);
		ret = 7;
		goto end;
	}else{
		if(S_ISDIR(st.st_mode)){
			strcat(path, "/index.html");
		}else if( (st.st_mode & S_IXUSR) || \
				  (st.st_mode & S_IXGRP) || \
				  (st.st_mode & S_IXOTH) ){
			cgi=1;
		}else{
		}
		//ok->cgi=?, path, query_string, method
		if(cgi){
			printf("enter CGI\n");
			exe_cgi(sock, method, path, query_string);
		}else{
			printf("method: %s, url: %s, path: %s, cgi: %d, query_string: %s\n", method, url, path, cgi, query_string);
			drop_header(sock); //!!!!!!!!!!!!!!
			echo_www(sock, path, st.st_size);
		}
	}

end:
	printf("quit client...\n");
	close(sock);
	return (void*)ret;
#endif
}




















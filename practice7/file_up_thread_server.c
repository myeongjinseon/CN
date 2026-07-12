#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define FILE_LEN 32
#define BUF_SIZE 1024
void error_handling(char *message);
void *handle_client(void *arg);

// int clnt_cnt = 0;
// int clnt_socks[BUF_SIZE];
// pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	pthread_t thread; 
	
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	int i;
	
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if (bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sd, 5) == -1)
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);    
		clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);	

		// TODO: pthread_create & detach 
		// pthread_mutex_lock(&mutx);
		// clnt_socks[clnt_cnt++] = clnt_sd;
		// pthread_mutex_unlock(&mutx);

		pthread_create(&thread, NULL, handle_client, (void*)&clnt_sd);
		pthread_detach(thread);
		printf("Connected client IP(sock=%d): %s \n", clnt_sd, inet_ntoa(clnt_adr.sin_addr));
	}
	
	close(serv_sd);
	return 0;
}

void *handle_client(void *arg)
{
	// TODO: file receiving 
	int clnt_sd = *((int*)arg);
	int read_len;
	int recv_size =0;
	char buf[BUF_SIZE];
	char file_name[FILE_LEN];
	FILE *fp;

	// TODO: Receive file name 
	memset(file_name, 0, sizeof(file_name));
	read_len = read(clnt_sd, file_name, sizeof(file_name));
	if (read_len <= 0) {
		error_handling("read() error!");
	}
	printf("Received file name: %s\n", file_name);

	fp = fopen(file_name, "wb");
	
	while ((read_len=read(clnt_sd, buf, BUF_SIZE))>0){
		fwrite((void*)buf, 1, read_len, fp);
		recv_size += read_len;
	}

	printf("Complete! file: %s(%dbytes)\n", file_name, recv_size);
	
	memset(buf, 0, BUF_SIZE);
	recv_size = 0;

	fclose(fp);

	// pthread_mutex_lock(&mutx);
	// for(int i=0;i<clnt_cnt; i++){
	// 	if(clnt_sd == clnt_socks[i]){
	// 		while(i<clnt_cnt-1){
	// 			clnt_socks[i]=clnt_socks[i+1];
	// 		}
	// 		break;
	// 	}
	// }
	// clnt_cnt--;
	// pthread_mutex_unlock(&mutx);
	close(clnt_sd);
	return NULL;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>

#define FILE_LEN 32
#define BUF_SIZE 1024
void error_handling(char *message);
void *handle_client(void *arg);
int is_active(char *cid);
void add_active(char *cid);
void remove_active(char *cid);

int clnt_cnt = 0;
int clnt_socks[BUF_SIZE];
pthread_mutex_t mutx;

#define CLIENTS 100
pthread_mutex_t client_mutx = PTHREAD_MUTEX_INITIALIZER;
char active_clients[CLIENTS][FILE_LEN];
int active_count = 0;
char *msg;

int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	pthread_t thread; 
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;

	int i;
	
	if(argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sd = socket(PF_INET, SOCK_STREAM, 0);   

	int optval = 1;
	setsockopt(serv_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sd, 5) == -1)
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);    
		clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);	
	
		// TODO: pthread_create & detach 
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sd;
		pthread_mutex_unlock(&mutx);

		pthread_create(&thread, NULL, handle_client, (void*)&clnt_sd);
		pthread_detach(thread);
		// printf("Connection established with client %d\n", clnt_sd);
	}
	
	close(serv_sd);
	return 0;
}

void *handle_client(void *arg)
{
	int clnt_sd = *((int*)arg);
	int read_len = 0;
	int recv_size =0;
	char buf[BUF_SIZE];
	char file_name[FILE_LEN];
	char client_id[FILE_LEN];
	FILE *fp;
	// new
    char filepath[256];
	char dirpath[128];
    struct stat st;
    long saved_bytes = 0; 
	long ack_bytes = 0;
	long final_bytes = 0;

	// Receiving cleint id, file name from client
	memset(client_id, 0, sizeof(client_id));
	read_len = read(clnt_sd, client_id, sizeof(client_id));
	if(read_len <= 0){
        perror("read client_id");
        close(clnt_sd);
        return NULL;
	}

	memset(file_name, 0, sizeof(file_name));
	read_len = read(clnt_sd, file_name, sizeof(file_name));
	if(read_len <= 0){
        perror("read file_name");
        close(clnt_sd);
        return NULL;
	}

	//no duplicate Client IDs will connect simultaneously
    pthread_mutex_lock(&client_mutx);
    if (is_active(client_id)){
        msg = "ERROR_DUPLICATE";
        write(clnt_sd, msg, strlen(msg));
        pthread_mutex_unlock(&client_mutx);

        printf("Rejected duplicate client %s\n", client_id);
        close(clnt_sd);
        return NULL;
    }

    // adding a current client_id in activ list
    add_active(client_id);
    pthread_mutex_unlock(&client_mutx);

	printf("Connection established with client %s %s\n", client_id, file_name);
	
	// make a floder for storing a upload file
	snprintf(dirpath, sizeof(dirpath), "./%s", client_id);
	mkdir(dirpath, 0777);
	snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, file_name);

    if(stat(filepath, &st) == 0) {
        saved_bytes = st.st_size;
    } else {
        saved_bytes = 0;
    }

	//send a saved_bytes to client
	write(clnt_sd, &saved_bytes, sizeof(saved_bytes));
	printf("Sending %ld bytes in %s/%s to client\n", saved_bytes, client_id, file_name);

	// if saved_bytes is 0 then, new write. 
	// if saved_bytes is 1 then, rewrite from offset 
	if(saved_bytes==0){
		fp = fopen(filepath, "wb");
	}
	else{
		fp = fopen(filepath, "ab"); 
	}

	// Stroring a file that sends from client
	while((read_len=read(clnt_sd, buf, BUF_SIZE))>0){
		fwrite((void*)buf, 1, read_len, fp);
		recv_size += read_len;

	    ack_bytes = saved_bytes + recv_size;
        write(clnt_sd, &ack_bytes, sizeof(ack_bytes));
		printf("[%s] Confirming saved bytes to client: %ld\n", client_id, ack_bytes);
	}

	// when a complete uploading, then server send a FIN msg to client
    final_bytes = saved_bytes + recv_size;
    write(clnt_sd, &final_bytes, sizeof(final_bytes));
	printf("Sending FIN %ld bytes to client\n", final_bytes);

	fclose(fp);

	// remove a client from active_list
    pthread_mutex_lock(&client_mutx);
    remove_active(client_id);
    pthread_mutex_unlock(&client_mutx);

	pthread_mutex_lock(&mutx);
	for(int i=0;i<clnt_cnt; i++){
		if(clnt_sd == clnt_socks[i]){
			while(i<clnt_cnt-1){
				clnt_socks[i]=clnt_socks[i+1];
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sd);
	return NULL;

}

// error handling
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

// managing for duplicate clients with same client_id
int is_active(char *cid){
    for (int i = 0; i < active_count; i++){
        if (strcmp(active_clients[i], cid) == 0)
            return 1;
    }
    return 0;
}

void add_active(char *cid){
    strcpy(active_clients[active_count], cid);
    active_count++;
}

void remove_active(char *cid){
    for (int i = 0; i < active_count; i++){
        if (strcmp(active_clients[i], cid) == 0){
            for (int j = i; j < active_count - 1; j++){
                strcpy(active_clients[j], active_clients[j + 1]);
            }
            active_count--;
            return;
        }
    }
}

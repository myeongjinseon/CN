#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define FILE_LEN 32
#define BUF_SIZE 1024
void error_handling(char *message);
int reconnect_to_server(int *sd, struct sockaddr_in *serv_adr);

int main(int argc, char *argv[])
{
	int sd;
	FILE *fp;
	
	char file_name[FILE_LEN];
	char client_id[FILE_LEN];
	char buf[BUF_SIZE];
	int read_cnt = 0;
	struct sockaddr_in serv_adr;
    long saved_bytes = 0;
    long ack_bytes = 0;
	long file_size = 0;
	long final_bytes = 0;

	if(argc != 5){
		printf("Usage: %s <IP> <port> <client id> <file name>\n", argv[0]);
		exit(1);
	}
	
	fp = fopen(argv[4], "rb");
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);  

	strcpy(client_id, argv[3]);
	strcpy(file_name, argv[4]);

	sd = socket(PF_INET, SOCK_STREAM, 0);   

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));



	// repeating a code when finishing a file uploads to server 
	while(saved_bytes<file_size){
		//connect to server
        reconnect_to_server(&sd, &serv_adr);

		write(sd, client_id, FILE_LEN);
		write(sd, file_name, FILE_LEN);

		printf("Connection start %s %s\n", client_id, file_name);

		char peek_buf[32];
		int peek_n = recv(sd, peek_buf, sizeof(peek_buf)-1, MSG_PEEK);

		if(peek_n > 0){
			peek_buf[peek_n] = '\0';

			if(strncmp(peek_buf, "ERROR_DUPLICATE", 15) == 0){

				recv(sd, peek_buf, strlen("ERROR_DUPLICATE"), 0);
				printf("Server rejected duplicate client ID.\n");
				close(sd);
				return 0;
			}
		}

		// Receive saved_bytes from server and read error handling by reconnection
		if(read(sd, &saved_bytes, sizeof(saved_bytes))<=0){
            printf("Failed to receive saved_bytes. Reconnecting\n");
            continue;
		}
		printf("Received %ld from server\n", saved_bytes);

		// Check the saved_bytes
		if(saved_bytes==0) printf("starting a fresh upload\n");
		else printf("resuming from saved_bytes offset\n");

		// If upload is already done. then, stop
		if(saved_bytes == file_size){
			printf("%s/%s is already saved in server\n", client_id, file_name);
			shutdown(sd, SHUT_WR);
			fclose(fp);
			close(sd);
			return 0;
		}

		// mv to saved_bytes offset
		fseek(fp, saved_bytes, SEEK_SET);

		// Send file data (saved_bytes 이후부터)
		while(1)
		{
			read_cnt=fread(buf, 1, BUF_SIZE, fp);
			if(read_cnt<=0){
				break;
			}
			
			if(write(sd, buf, read_cnt)==-1){
				perror("Disconnected during write(). Reconnecting\n");
				break;
			}; 

			if(read(sd, &ack_bytes, sizeof(ack_bytes))<=0){
				perror("Discconected waithing server confirming. Recoonectiong\n");
				break;
			}; 
			printf("Server confirms %ld saved\n", ack_bytes);

			saved_bytes = ack_bytes;

			if(read_cnt<BUF_SIZE) break;
		}
	}
	shutdown(sd, SHUT_WR);	

	// Receiving a finish msg from server
    if(read(sd, &final_bytes, sizeof(final_bytes))>0){
		printf("Received FIN %ld from server\n", final_bytes);
	};

	fclose(fp);
	close(sd);
	return 0;
}

// error handling
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

// reconnet to server
int reconnect_to_server(int *sd, struct sockaddr_in *serv_adr){
    close(*sd);
    *sd = socket(PF_INET, SOCK_STREAM, 0);

    while(connect(*sd, (struct sockaddr *)serv_adr, sizeof(*serv_adr))==-1){
        printf("Reconnect failed. Retrying\n");
        sleep(1);
    }
    return 1;
}

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define PORT 8080
 
struct sockaddr_in address;
int sock = 0, valread;
struct sockaddr_in serv_addr;
char buffer[1024] = {0};

int buffersize = 1024;

char sendbuffer[1024] = {0};
char receivebuffer[1024] = {0};

void send_message(char *message){
    send(sock , message , buffersize , 0 );
}

void receive_message(){	
    valread = read(sock , receivebuffer, buffersize);
}

int prepare_socket(){	
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    return 0;
}

// Client
int main(int argc, char const *argv[]) {
	int error = prepare_socket();

	if (error != 0){
		return error;
	}

	uid_t euid = geteuid();
	
	if (euid != 0){
		puts("REGULAR");
	} else {
		puts("ROOT");
	}

	printf("Test Connect : \n");

	while (true){
		scanf(" %[^\n]", sendbuffer);
		send_message(sendbuffer);
		receive_message();
		printf("%s", receivebuffer);
	}

	return 0;
}

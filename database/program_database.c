#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 8080

int amain(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    return 0;
}

char *databases_path = "databases";
int buffersize = 1024;

bool make_directory(char *path){
	struct stat st = {0};

	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
		return true;
	}

	return false;
}

bool make_file(char *path){
	FILE *file = fopen(path, "w");
	fclose(file);
}

void write_database_path(char *arr, char *name){
	sprintf(arr, "%s/%s", databases_path, name);
}


void write_table_path(char *arr, char *database_name, char *name){
	sprintf(arr, "%s/%s/%s", databases_path, database_name, name);
}

bool make_database(char *name){
	char temp[buffersize];

	write_database_path(temp, name);

	bool success = make_directory(temp);
	return success;
}

bool make_table(char *database_name, char *name){
	char temp[buffersize];

	write_table_path(temp, database_name, name);

	bool success = make_file(temp);
	return success;
}

int main(int argc, const char *argv[]) {
	make_directory(databases_path);
	make_database("__ROOT__");
	make_table("__ROOT__", "__USER__");
}

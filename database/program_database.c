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
char filebuffer[1024];

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

bool write_file(char *path, char *content){
	FILE *file = fopen(path, "w");
	fwrite(content, 1, buffersize, file);
	fclose(file);
}

bool remove_directory(char *path){
	int res = rmdir(path);
	if(res == -1){
		return false;
	}
	return true;
}

bool remove_file(char *path){
	int res = unlink(path);
	if (res == -1){
		return false;
	}
	return true;
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

bool remove_database(char *name){
	char temp[buffersize];

	write_database_path(temp, name);

	bool success = remove_directory(temp);
}

bool remove_table(char *database_name, char *name){
	char temp[buffersize];

	write_table_path(temp, database_name, name);

	bool success = remove_file(temp);
}

char columns_list[128][1024];
char columns_list_type[128][32];
int column_list_size = 0;

char rows_list[128][1024];
char row_list_size = 0;

void get_table_columns(char *database_name, char *table_name) {
	char temp[buffersize];

	write_table_path(temp, database_name, table_name);

	printf("Table Path : %s\n", temp);

	FILE *file = fopen(temp , "r");

	column_list_size = 0;

	char *token;
	bool on_structure = false;

	while ((token = fgets(filebuffer, buffersize, file)) != NULL){
		if (on_structure && *token == '\n'){
			break;
		}

		if (on_structure){
			//printf("%s", token);
			char *spl = strtok(token, ":");
			strcpy(columns_list[column_list_size], spl);
			spl = strtok(NULL, ":");
			strncpy(columns_list_type[column_list_size], spl, strlen(spl) - 1);
			column_list_size++;
		} else if (strstr(token, "#Structure")){
			on_structure = true;
		}
	}

	fclose(file);
}

char *get_table_column_type(char *database_name, char *table_name, char *column_name){
	get_table_columns(database_name, table_name);

	int column_index = 0;
	for (;column_index < column_list_size;column_index++){
		if (strcmp(columns_list[column_index], column_name) == 0){
			char *o = malloc(32);
			strcpy(o, columns_list_type[column_index]);
			return o;
		}
	}
	return NULL;
}

void get_table_rows(char *database_name, char *table_name) {
	char temp[buffersize];

	write_table_path(temp, database_name, table_name);

	printf("Table Path : %s\n", temp);

	FILE *file = fopen(temp , "r");

	row_list_size = 0;

	char *token;
	bool on_data = false;

	while ((token = fgets(filebuffer, buffersize, file)) != NULL){
		if (on_data && *token == '\n'){
			break;
		}

		if (on_data){
			strncpy(rows_list[row_list_size], token, strlen(token) - 1);
			row_list_size++;
		} else if (strstr(token, "#Data")){
			on_data = true;
		}
	}
}

void dump_local_table(char *database_name, char *table_name){
	filebuffer[0] = 0;
	strcat(filebuffer, "#Structure\n");

	int i = 0;

	for (i = 0;i < column_list_size;i++){
		strcat(filebuffer, columns_list[i]);
		strcat(filebuffer, ":");
		strcat(filebuffer, columns_list_type[i]);
		strcat(filebuffer, "\n");
	}

	strcat(filebuffer, "\n");
	strcat(filebuffer, "#Data\n");

	for (i = 0;i < row_list_size;i++){
		strcat(filebuffer, rows_list[i]);
		strcat(filebuffer, "\n");
	}

	char temp[buffersize];
	
	write_table_path(temp, database_name, table_name);

	write_file(temp, filebuffer);
}

bool remove_column(char *database_name, char *table_name, char *column_name){
	get_table_columns(database_name, table_name);
	get_table_rows(database_name, table_name);

	int deleted_column_index = -1;
	int i = 0;
	for (;i < column_list_size;i++){
		if (strcmp(columns_list[i], column_name) == 0){
			deleted_column_index = i;
			column_list_size--;
			//row_list_size--;
		}
		if (deleted_column_index != -1){//if succes ngehapus
			strcpy(columns_list[i], columns_list[i + 1]);
			strcpy(columns_list_type[i], columns_list_type[i + 1]);
		}
	}

	if (deleted_column_index == -1){
		return false;
	}

	i = 0;
	for (;i < row_list_size;i++){
		char temprow[buffersize];
		strcpy(temprow, rows_list[i]);
		
		rows_list[i][0] = 0;

		char *token = strtok(temprow, "|");
		int j = 0;
	       
		while (token != NULL){
			if (j == deleted_column_index){
				token = strtok(NULL, "|");
				j++;
				continue;
			}

			strcat(rows_list[i], token);

			token = strtok(NULL, "|");

			if (token != NULL){
				strcat(rows_list[i], "|");
			}
			j++;
		}

		if (rows_list[i][strlen(rows_list[i]) - 1] == '|'){
			rows_list[i][strlen(rows_list[i]) - 1] = '\0';
		}
	}

	dump_local_table(database_name, table_name);

	return true;
}

char current_database[1024];
char current_client[1024];

bool is_logined = false;

void client_logout(){
	is_logined = false;
	current_database[0] = 0;
	current_client[0] = 0;
}

bool client_login(char *username, char *password){
	client_logout();

	get_table_columns("__ROOT__", "__USER__");
	get_table_rows("__ROOT__", "__USER__");

	char *tempusername;
	char *temppassword;

	int i;
	for (i = 0;i < row_list_size;i++){
		tempusername = strtok(rows_list[i], "|");
		temppassword = strtok(NULL, "|");

		if (strcmp(tempusername, username) == 0 && strcmp(temppassword, password) == 0){
			strcpy(current_client, username);

			is_logined = true;

			return true;
		}
	}

	return false;
}

bool use_database(char *username, char *database_name) {
	strcpy(current_database, database_name);
	
	get_table_columns("__ROOT__", "__DATABASE__");
	get_table_rows("__ROOT__", "__DATABASE__");

	char *tempusername;
	char *tempdatabase;

	int i;
	for (i = 0;i < row_list_size;i++){
		tempusername = strtok(rows_list[i], "|");
		tempdatabase = strtok(NULL, "|");

		if (strcmp(tempusername, current_client) == 0 && strcmp(tempdatabase, database_name) == 0){
			strcpy(current_database, database_name);
			return true;
		}
	}

	return false;
}

bool create_database(char *username, char *database_name){
	make_database(database_name);
}

bool create_table(char *username, char *database_name, char *table_name){
	make_table(database_name, table_name);
}

bool drop_database(char *username, char *database_name){
	remove_database(database_name);
}

bool drop_table(char *username, char *database_name, char *table_name){
	remove_table(database_name, table_name);
}

bool drop_column(char *username, char *database_name, char *table_name, char *column_name){
	remove_column(database_name, table_name, column_name);
}

int main(int argc, const char *argv[]) {
	make_directory(databases_path);
	make_database("__ROOT__");
	make_table("__ROOT__", "__USER__"); //user login password relation
	make_table("__ROOT__", "__DATABASE__"); //user and database relation

	get_table_columns("__ROOT__", "__USER__");
	get_table_rows("__ROOT__", "__USER__");
}

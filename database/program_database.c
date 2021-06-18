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

char *databases_path = "databases";
int buffersize = 1024;
char filebuffer[1024];

int server_fd, new_socket, valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char sendbuffer[1024] = {0};
char receivebuffer[1024] = {0};

void receive_message(){	
    	valread = read( new_socket , receivebuffer, buffersize);
}

void send_message(char *message){
	send(new_socket , message , buffersize, 0 );
}

bool make_directory(char *path){
	struct stat st = {0};

	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
		return true;
	}

	return false;
}

bool make_file(char *path){
	if (access(path, F_OK) != 0) {
		FILE *file = fopen(path, "w");
		fclose(file);
		return true;
	}

	return false;
}


bool write_file(char *path, char *content){
	if (access(path, F_OK) != 0) {	
		FILE *file = fopen(path, "w");
		fwrite(content, 1, buffersize, file);
		fclose(file);
		return true;
	}

	return false;
}

bool write_file_force(char *path, char *content){
//	if (access(path, F_OK) != 0) {	
		FILE *file = fopen(path, "w");
		fwrite(content, 1, buffersize, file);
		fclose(file);
		return true;
//	}

//	return false;
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

void log_output(char *message){
	printf("%s\n", message);
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

	puts("gajah");
	bool success = make_file(temp);
	return success;
}

bool make_table_columns(char *database_name, char *table_name, char columns[128][256], int column_length){
	printf("Make table columns\n");
	if (column_length <= 0){
		return false;
	}

	printf("Lanjut\n");

	char temp[buffersize];
	
	write_table_path(temp, database_name, table_name);

	FILE *file = fopen(temp ,"w");

	char content[buffersize];

	strcpy(content, "#Structure\n");

	int i = 0;
	for (;i < column_length;i++){
		char * token = columns[i];
		while (*token == ' ' || *token == '\0') token++;

		char *first = strtok(token," ");
		char *second = strtok(NULL, " ");

		strcat(content, first);
		strcat(content, ":");
		strcat(content, second);
		strcat(content, "\n");
	}

	strcat(content, "\n#Data\n");

	printf("CONTENT :\n%s\n", content);

	fwrite(content, 1, strlen(content), file);

	fclose(file);

	return true;
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
			memset(columns_list[column_list_size], 0, 1024);
			strcpy(columns_list[column_list_size], spl);
			spl = strtok(NULL, ":");
			memset(columns_list_type[column_list_size], 0, 32);
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
			memset(columns_list_type[column_index], 0, 32);
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

	printf("Rowsing\n");

	while ((token = fgets(filebuffer, buffersize, file)) != NULL){
		if (on_data && (*token == '\n' || *token == 0)){
			break;
		}

		if (on_data){
			memset(rows_list[row_list_size], 0, 1024);
			strncpy(rows_list[row_list_size], token, strlen(token) - 1);
			row_list_size++;
		} else if (strstr(token, "#Data")){
			on_data = true;
		}
	}

	printf("Selesai rowsing\n");
}

void dump_local_table(char *database_name, char *table_name){
	filebuffer[0] = 0;
	
	memset(filebuffer, 0, 1024);
	
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

	write_file_force(temp, filebuffer);
}

bool remove_column(char *database_name, char *table_name, char *column_name){
	get_table_columns(database_name, table_name);
	get_table_rows(database_name, table_name);

	printf("Removing column..\n");

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
		memset(rows_list[i], 0, 1024);

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

	printf("dumping table..\n");

	dump_local_table(database_name, table_name);

	return true;
}

char current_database[1024] = {0};
char current_client[1024] = {0};

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

bool create_user(char *username, char *password){
	printf("%s -> %s\n", username, password);
	return false;
}

bool grant_permission(char *username, char *granted_user, char *database_name){
	return false;
}

bool create_database(char *username, char *database_name){
	return make_database(database_name);
}

bool create_table(char *username, char *database_name, char *table_name, char columns[128][256], int column_length){
	printf("making table..\n");
	bool flag = make_table(database_name, table_name);

	printf("check flag\n");

	if (flag == false){
		return false;
	}

	printf("Try make table columns\n");

	return make_table_columns(database_name, table_name, columns, column_length);
}

bool drop_database(char *username, char *database_name){
	return remove_database(database_name);
}

bool drop_table(char *username, char *database_name, char *table_name){
	return remove_table(database_name, table_name);
}

bool drop_column(char *username, char *database_name, char *table_name, char *column_name){
	return remove_column(database_name, table_name, column_name);
}

bool insert_into(char *username, char *database_name, char *table_name, char values[128][256], int column_length){
	get_table_columns(database_name, table_name);
	
	if (column_length != column_list_size){
		return false;
	}

	char temp[buffersize];
	
	write_table_path(temp, database_name, table_name);

	FILE *file = fopen(temp ,"a");

	char content[buffersize];
	content[0] = 0;

	int i = 0;
	for (;i < column_length;i++){
		char * token = values[i];

		printf("%s -> %s\n", columns_list_type[i], token);

		if (strcmp(columns_list_type[i], "string") == 0){
			if (*token == '\'' && strstr(token+1, "\'")){
				token++;
				*strstr(token, "\'") = 0;
				//biarin, bener
			} else {
				printf("string gak cocok int\n");
				return false;
			}
		} else if (strcmp(columns_list_type[i], "int") == 0){
			if (*token >= '0' && *token <= '9'){
				//biarin, bener
			} else {
				printf("int gak cocok string\n");
				return false;
			}
		}

		strcat(content, token);

		if (i < column_length - 1){
			strcat(content, "|");
		}
	}
	strcat(content, "\n");

	fwrite(content, 1, strlen(content), file);

	fclose(file);

	return true;
}

bool update_table(char *username, char *database_name, char *table_name, char *key, char *value, char *where, char *where_key, char *where_value){
	printf("%s -> %s\n", database_name, table_name);

	get_table_columns(database_name, table_name);
	get_table_rows(database_name, table_name);

	printf("Prepare update\n");

	int i = 0;

	int column_to_be_changed = -1;
	int column_on_where = -1;

	for (i = 0;i < column_list_size;i++){
		printf("%s -> %s\n", key, columns_list[i]);
		if (strcmp(key, columns_list[i]) == 0){
			column_to_be_changed = i;
		}
		if (where != NULL && strcmp(where_key, columns_list[i]) == 0){
			column_on_where = i;
		}
	}

	printf("Updating\n");

	if (column_to_be_changed == -1){
		printf("No Column exist\n");
		return false;
	}

	if (where != NULL && column_on_where == -1){
		printf("No Where Column exist\n");
		return false;
	}


	for (i = 0;i < row_list_size;i++){
		char temprow[buffersize];
		char temprow2[buffersize];
		
		strcpy(temprow, rows_list[i]);
		strcpy(temprow2, rows_list[i]);
		
		rows_list[i][0] = 0;
		memset(rows_list[i], 0, 1024);

		char *token = strtok(temprow, "|");
		int j = 0;

		bool changed = false;
	       
		while (token != NULL){
			//printf("TOKEN : %s -> %s\n", token, where_value);
			if (j == column_on_where && strcmp(token, where_value) == 0){
				changed = true;
			}

			if (j == column_to_be_changed){
				strcat(rows_list[i], value);
			} else {
				strcat(rows_list[i], token);
			}

			token = strtok(NULL, "|");

			if (token != NULL){
				strcat(rows_list[i], "|");
			}
			j++;
		}

		if (where == NULL){
			//biarin, bakal diganti
		} else if (where != NULL && changed){
			//biarin, bakal diganti
		} else if (where != NULL && !changed){
			memset(rows_list[i], 0, 1024);
			strcpy(rows_list[i], temprow2);
		}

		if (rows_list[i][strlen(rows_list[i]) - 1] == '|'){
			rows_list[i][strlen(rows_list[i]) - 1] = '\0';
		}
	}

	printf("dumping update..\n");

	dump_local_table(database_name, table_name);

	return true;
}

bool create_user_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);
	char * fourth = strtok(NULL, s);
	char * fifth = strtok(NULL, s);
	char * sixth = strtok(NULL, s);

	if(!strcmp(first, "CREATE")){
		if(!strcmp(second, "USER")){
			if(!strcmp(fourth, "IDENTIFIED")){
				if(!strcmp(fifth, "BY")){
					if (!strcmp(current_client, "__ROOT__")){
						create_user(third, sixth);
						log_output("User Added");
					} else {
						log_output("Failed : Must ROOT");
					}
					return true;
				}
			}
		}
	}
	return false;
}

bool use_database_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);


	if(!strcmp(first, "USE")){
		//function use database(second)
		use_database(current_client, second);
		return true;
	}
	return false;
}

bool grant_permission_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);
	char * fourth = strtok(NULL, s);
	char * fifth = strtok(NULL, s);

	if(!strcmp(first, "GRANT")){
		if(!strcmp(second, "PERMISSION")){
			if (!strcmp(fourth, "INTO")){
				grant_permission(current_client, third, fifth);
			}
		}
		return true;
	}
	return false;
}

bool create_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);
	char * fourth = strtok(NULL, s);

	printf("LKJASLKDJASLDJ\n");

	if(!strcmp(first, "CREATE")){
		if(!strcmp(second, "DATABASE")){
			create_database(current_client, third);
		}
		else if(!strcmp(second, "TABLE")){
			if (current_database[0] == 0){
				log_output("Use table first");
				return true;
			}

			log_output("Creating TABLE");

			char *com = strstr(full_command, "(");

			if (com && *com == '('){
				com++;

				char command[1024];

				strcpy(command, com);

				char columns[128][256];
				int column_length = 0;

				char * token = strtok(command, ",");

				printf("command ->\n");
				while (token != NULL){
					while(*token == ' ') token++;
					strcpy(columns[column_length], token);
	
					if (columns[column_length][strlen(columns[column_length])-1] == ')'){	
						columns[column_length][strlen(columns[column_length])-1] = 0;
					}

					puts(columns[column_length]);

					column_length++;

					token = strtok(NULL, ",");
				}

				printf("Before create_table()\n");

				create_table(current_client, current_database, third, columns, column_length);
				return true;
			}
		}
		return true;
	}
	return false;
}

bool drop_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s); //nama
	char * fourth = strtok(NULL, s); //FROM
	char * fifth = strtok(NULL, s); //nama table

	if(!strcmp(first, "DROP")){
		if(!strcmp(second, "DATABASE")){
			//function drop database (third)
			drop_database(current_client, third);
		}
		else if(!strcmp(second, "TABLE")){
			//function drop table (third)
			drop_table(current_client, current_database, third);
		}
		else if(!strcmp(second, "COLUMN")){
			//function drop column (third)
			if (!strcmp(fourth, "FROM")){
				printf("dropping column..\n");
				drop_column(current_client, current_database, fifth, third);
			}
		}
		return true;
	}
	return false;
}

bool insert_table_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s); //nama table
	char * fourth = strtok(NULL, s); //deret kolom

	if(!strcmp(first, "INSERT")){
		if(!strcmp(second, "INTO")){
			char *com = strstr(full_command, "(");

			if (com && *com == '('){
				com++;

				char command[1024];

				strcpy(command, com);

				char values[128][256];
				int column_length = 0;

				printf("insert tbbbb\n");

				char * token = strtok(command, ",");

				printf("insert lagi\n");

				while (token != NULL){
					while (*token == ' ') token++;

					strcpy(values[column_length], token);

					if (values[column_length][strlen(values[column_length])-1] == ')'){	
						values[column_length][strlen(values[column_length])-1] = 0;
					}

					puts(values[column_length]);

					column_length++;

					token = strtok(NULL, ",");
				}

				printf("sudah\n");

				insert_into(current_client, current_database, third, values, column_length);

				return true;
			}
			return true;
		}
	}
	return false;
}

bool update_table_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);
	char * fourtheq = strtok(NULL, s);
	char tempp[1024];
	strcpy(tempp, fourtheq);
	char * fourth = strtok(tempp, "=");
	char * fifth = strtok(NULL, "=");
	//where
	sprintf(str, "%s", full_command);
	char * sixth = strtok(str, " ");
	sixth = strtok(NULL, " ");
	sixth = strtok(NULL, " ");
	sixth = strtok(NULL, " ");
	sixth = strtok(NULL, " ");
	char * seventheq = strtok(NULL, " ");
	char * seventh = strtok(seventheq, "="); //key
	char * eight = strtok(NULL, "="); //value

	printf("Mau ngupdate\n");

	printf("%s -> %s -> %s\n", fourtheq, fourth, fifth);
	printf("%s -> %s -> %s\n", sixth, seventh, eight);

	if(!strcmp(first, "UPDATE")){
		if(!strcmp(third, "SET")){
			//second = nama tabel
			//fourth = isi update an nya
			//function update tabel (third)
			if (strstr(full_command, "=")) {
				printf("Update strstr\n");
				update_table(current_client, current_database, second, fourth, fifth, sixth, seventh, eight);
			} else {
				printf("Syntax error : No =(equal sign)\n");
			}
		}
		return true;
	}
	return false;
}

bool delete_table_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);

	if(!strcmp(first, "DELETE")){
		if(!strcmp(second, "FROM")){
			//function delete (third)
		}
		return true;
	}
	return false;
}

bool select_table_input(char* full_command){
	char str [1024];
	sprintf(str, "%s", full_command);
	const char s[2] = " ";
	char * first = strtok(str, s);
	char * second = strtok(NULL, s);
	char * third = strtok(NULL, s);
	char * fourth = strtok(NULL, s);

	if(!strcmp(first, "SELECT")){
		if(!strcmp(third, "FROM")){
			//second = nama kolom
			//fourth = nama tabel
			//function select (second, fourth(?))
			//bebas parameternya apa
		}
		return true;
	}
	return false;
}

//kurang yang where, probably

int check_input (char *inp){
	char input[1024];

	strcpy(input, inp);

	int len = strlen(input);

	if (input[len - 1] != ';'){
		puts("syntax error no ;");
		return 0;
	}

	input[len - 1] = 0;

	if (create_user_input(input)){
		puts("create user");
	}
	else if(use_database_input(input)){
		puts("use database");
	}
	else if(grant_permission_input(input)){
		puts("grant permission");
	}
	else if(create_input(input)){
		puts("create input");
	}
	else if(drop_input(input)){
		puts("drop input");
	}
	else if(insert_table_input(input)){
		puts("insert table");
	}
	else if(update_table_input(input)){
		puts("update table");
	}
	else if(delete_table_input(input)){
		puts("delete table");
	}
	else if(select_table_input(input)){
		puts("select table");
	}

	return 0;
}

int initial() {
	make_directory(databases_path);
	make_database("__ROOT__");
	//make_table("__ROOT__", "__USER__"); //user login password relation
	//make_table("__ROOT__", "__DATABASE__"); //user and database relation

	//get_table_columns("__ROOT__", "__USER__");
	//get_table_rows("__ROOT__", "__USER__");

	
	//check_input("USE DATABASE __ROOT__;");

	strcpy(current_client, "__ROOT__");
	strcpy(current_database, "__ROOT__");

	printf("CREATE __USER__\n");

	check_input("CREATE TABLE __USER__ (username string, password string);");
	check_input("INSERT INTO __USER__ ('__ROOT__', '__ROOT__');");
	
	printf("CREATE __DATABASE__\n");

	check_input("CREATE TABLE __DATABASE__ (username string, database string);");
	check_input("INSERT INTO __DATABASE__ ('__ROOT__', '__ROOT__');");
}

void prepare_socket(){	
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
}

// Server
int main(int argc, char const *argv[]) {
	//prepare_socket();

	initial();

	uid_t uid = getuid();
	uid_t euid = geteuid();

	if (euid != 0){
		puts("REGULAR");
	} else {
		puts("ROOT");
	}	

	check_input("CREATE DATABASE BINATANG;");
	check_input("USE BINATANG;");

	printf("JJJJ : %s\n", current_database);

	check_input("CREATE TABLE KUCING (id int, nama string, usia int);");
	check_input("INSERT INTO KUCING (1, 'Un', 4);");
	check_input("INSERT INTO KUCING (2, 'Any', 3);");
	check_input("INSERT INTO KUCING (3, 'Liza', 2);");

	//check_input("DROP COLUMN nama FROM KUCING;");
	//check_input("DROP TABLE KUCING;");
	//check_input("DROP DATABASE BINATANG;");
	
	check_input("UPDATE KUCING SET usia=10;");
	check_input("UPDATE KUCING SET usia=20 WHERE id=2;");

	return 0;
}


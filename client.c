#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int ADMIN = 1;
int FACULTY = 2;
int STUDENT = 3;

int output_size(char *buff, int size) {
	int res = write(1, buff, size);
	return res;
}

int output(char *buff) {
	for(int i = 0; buff[i] != '\0'; i++) {
		int res = write(1, &buff[i], sizeof(buff[i]));
		if(res == -1) {
			return res;
		}
	}
	return 0;
}

int input_size(char *buff, int size) {
	memset(&buff, '\0', size);
	int res = read(0, buff, size);
	return res;
}

int input(char *buff) {
	int size = 0;
	char c;
	while(read(0, &c, sizeof(c))) {
		buff[size++] = c;
		if(c == '\n') {
			return 0;
		}
	}
	return -1;
}

int parse_int(int size, char str[100]) {
	int num = 0;
	for(int i = 0; i < size; i++) {
		num = (num * 10) + (int)(str[i] - '0');
	}
	return num;
}

int login(int sfd, int type) {
	char email[100];
	char password[100];
	int logged_in = 0;
	int res = send(sfd, &type, sizeof(type), 0);
	if(res == -1) {
		perror("Error sending login type: ");
		return -1;
	}
	while(!logged_in) {
		output("Enter email: ");
		input_size(email, sizeof(email));
		output("Enter password: ");
		input_size(password, sizeof(password));
		send(sfd, &email, sizeof(email), 0);
		send(sfd, &password, sizeof(password), 0);
		int response;
		output("Logging in...\n");
		recv(sfd, &response, sizeof(response), 0);
		if(response == 0) {
			output("Incorrect credentials, try again\n");
		}
		else if(response == -1) {
			output("User not found\n");
		}
		else if(response == -2) {
			perror("Server oof'd\n");
			return -1;
		}
		else {
			logged_in = response;
		}
	}
	output("Logged in successfully!\n");
	return 0;
}

int add_faculty(int sfd) {
	char name[100];
	char email[100];
	char password[100];
	output("Enter name: ");
	int res = input_size(name, sizeof(name));
	output("Enter email: ");
	res = input_size(email, sizeof(email));
	output("Enter password: ");
	res = input_size(password, sizeof(password));
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);
	if(res != 0) {
		output("Server oof'd in adding faculty\n");
	}
	else {
		output("Faculty created\n");
	}
	return res;
}

int edit_faculty(int sfd) {
	char id_str[100];
	output("Enter faculty id: ");
	int size = input_size(id, sizeof(id));
	int id = parse_id(size, id_str);
	
	// Send data to server
	char name[100];
	char email[100];
	char password[100];
	output("Enter new name: ");
	int res = input_size(name, sizeof(name));
	output("Enter new email: ");
	res = input_size(email, sizeof(email));
	output("Enter new password: ");
	res = input_size(password, sizeof(password));
	send(sfd, &id, sizeof(id), 0);
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);
	if(res != 0) {
		output("Server oof'd in editing faculty\n");
	}
	else {
		output("Faculty edited\n");
	}
	return 0;
}

int status_faculty(int sfd) {
	char id_str[100];
	output("Enter faculty id: ");
	int size = input_size(id, sizeof(id));
	
	int id = parse_id(size, id_str);
	char option_char;
	output("Enter 1 to activate or 0 to deactivate: ");
	input_size(&option, 1);
	int option = (int)(option - '0');
	send(sfd, &id, sizeof(id), 0);
	send(sfd, &option, sizeof(option), 0);
	
	int res;
	recv(sfd, &res, sizeof(res), 0);
	
	if(res == 1) {
		output("Status updated\n");
	}
	else {
		output("Server oof'd while changing status\n");
	}	
	
	return 0;
}

int add_student(int socket_fd) {
	char name[100];
	char email[100];
	char password[100];
	output("Enter name: ");
	int res = input_size(name, sizeof(name));
	output("Enter email: ");
	res = input_size(email, sizeof(email));
	output("Enter password: ");
	res = input_size(password, sizeof(password));
	recv(sfd, &res, sizeof(res), 0);
	if(res != 0) {
		output("Server oof'd in adding student\n");
		return res;
	}
	return 0;
}

int edit_student(int socket_fd) {
	char id_str[100];
	output("Enter student id: ");
	int size = input_size(id, sizeof(id));
	int id = parse_id(size, id_str);
	
	// Send data to server
	char name[100];
	char email[100];
	char password[100];
	output("Enter new name: ");
	int res = input_size(name, sizeof(name));
	output("Enter new email: ");
	res = input_size(email, sizeof(email));
	output("Enter new password: ");
	res = input_size(password, sizeof(password));
	send(sfd, &id, sizeof(id), 0);
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);
	if(res != 0) {
		output("Server oof'd in editing student\n");
	}
	else {
		output("Student edited\n");
	}
	return 0;
}

int status_student(int socket_fd) {
	char id_str[100];
	output("Enter student id: ");
	int size = input_size(id, sizeof(id));
	
	int id = parse_id(size, id_str);
	char option_char;
	output("Enter 1 to activate or 0 to deactivate: ");
	input_size(&option, 1);
	int option = (int)(option - '0');
	send(sfd, &id, sizeof(id), 0);
	send(sfd, &option, sizeof(option), 0);
	
	int res;
	recv(sfd, &res, sizeof(res), 0);
	
	if(res == 1) {
		output("Status updated\n");
	}
	else {
		output("Server oof'd while changing status\n");
	}	
	
	return 0;
}

int admin(int socket_fd) {
	int id = login(socket_fd, ADMIN);
	if(id == -1) {
		return -1;
	}
	output("Welcome to admin\n");
	int run = 1;
	int res = -1;
	while(run) {
		output("Enter 1 to add faculty\nEnter 2 to edit faculty\n Enter 3 to activate/deactivate faculty\nEnter 4 to add student\nEnter 5 to edit student\nEnter 6 to activate/deactivate student\nEnter 7 to sign out\n");
		char option;
		input_size(&option, 1);
		if(option == '1') {
			res = add_faculty(socket_fd);
		}
		else if(option == '2') {
			res = edit_faculty(socket_fd);
		}
		else if(option == '3') {
			res = status_faculty(socket_fd);
		}
		else if(option == '4') {
			res = add_student(socket_fd);
		}
		else if(option == '5') {
			res = edit_student(socket_fd);
		}
		else if(option == '6') {
			res = status_student(socket_fd);
		}
		else if(option == '7') {
			res = 0;
			run = 0;
		}
		else {
			output("Invalid option, try again\n");
		}
	}
	return res;
}

int add_course(int socket_fd, int fid) {
	char name[100];
	char semesters[100];
	char credits[100];
	output("Enter course name: ");
	input_size(name, sizeof(name));
	output("Enter semesters: ");
	input_size(semesters, sizeof(semesters));
	output("Enter credits: ");
	input_size(credits, sizeof(credits));
	return 0;
}

int remove_course(int socket_fd, int fid) {
	char id[100];
	output("Enter course id: ");
	input_size(id, sizeof(id));
	return 0;
}

int view_enrollments(int socket_fd, int fid) {
	output("Enter course id: ");
	char id[100];
	input_size(id, sizeof(id));
	return 0;
}

int change_password(int socket_fd, int id, int user) {
	char password[100];
	output("Enter new password: ");
	input_size(password, sizeof(password));
	return 0;
}

int faculty(int socket_fd) {
	int id = login(socket_fd, FACULTY);
    if(id == -1) {
    	return -1;
    }
    output("Welcome to faculty\n");
   	int run = 1;
   	int res = -1;
   	while(run) {
   		output("Enter 1 to add a new course\nEnter 2 to Remove a course\nEnter 3 to view enrollments for a course\nEnter 4 to change password\nEnter 0 to sign out\n");
   		char option;
   		input_size(&option, 1);
   		if(option == '1') {
   			res = add_course(socket_fd, id);
   		}
   		else if(option == '2') {
   			res = remove_course(socket_fd, id);
   		}
   		else if(option == '3') {
   			res = view_enrollments(socket_fd, id);
   		}
   		else if(option == '4') {
   			res = change_password(socket_fd, id, FACULTY);
   		}
   		else if(option == '0') {
   			res = 0;
   			run = 0;
   		}
   		else {
   			output("Invalid option, try again\n");
   		}
   	}
   	return res;
}

int enroll_course(int socket_fd, int sid) {
	char id[100];
	output("Enter course id: ");
	input_size(id, sizeof(id));
	return 0;
}

int unenroll_course(int socket_fd, int sid) {
	char id[100];
    output("Enter course id: ");
    input_size(id, sizeof(id));
	return 0;
}

int student(int socket_fd) {
	int id = login(socket_fd, STUDENT);
	if(id == -1) {
		output("Error in logging in\n");
		return -1;
	}
	output("Welcome to student\n");
	int run = 1;
	int res = -1;
	while(run) {
		output("Enter 1 to enroll to new course\nEnter 2 to unenroll from a course\nEnter 3 to change password\nEnter 0 to exit\n");
		char option;
		input_size(&option, 1);
		if(option == '1') {
			res = enroll_course(socket_fd, id);
		}
		else if(option == '2') {
			res = unenroll_course(socket_fd, id);
		}
		else if(option == '3') {
			res = change_password(socket_fd, id, STUDENT);
		}
		else if(option == '0') {
			res = 0;
			run = 0;
		}
		else {
			output("Invalid option, try again\n");
			res = -1;
		}
	}
	return res;
}

int main() {
	/*int cfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
   	server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
	int connect_res = connect(cfd, (struct sockaddr*)&server, sizeof(server));
	if(connect_res == -1) {
		char connect_error[] = "Error connecting\n";
        output(connect_error, sizeof(connect_error));
       	return -1;
   	}*/
	int run = 1;
	output("Welcome to Academia Portal\n");
	while(run) {
		output("Enter 1 to sign in as admin\nEnter 2 to sign in as faculty\nEnter 3 to sign in as student\nEnter 0 to exit\n");
		char option;
		input(&option);
		if(option == '1') {
			admin(cfd);
		}
		else if(option == '2') {
			faculty(cfd);
		}
		else if(option == '3') {
			student(cfd);
		}
		else if(option == '0') {
			run = 0;
		}
		else {
			output("Invalid option, try again\n");
		}
	}
	return 0;
}

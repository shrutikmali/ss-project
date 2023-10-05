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

int add_faculty(int socket_fd) {
	return 0;
}

int edit_faculty(int socket_fd) {
	return 0;
}

int status_faculty(int socket_fd) {
	return 0;
}

int add_student(int socket_fd) {
	return 0;
}

int edit_student(int socket_fd) {
	return 0;
}

int status_student(int socket_fd) {
	return 0;
}

int admin(int socket_fd) {
	int logged_in = login(socket_fd, ADMIN);
	if(logged_in == -1) {
		return;
	}
	output("Welcome to admin\n");
	int run = 1;
	while(run) {
		output("Enter 1 to add faculty\nEnter 2 to edit faculty\n Enter 3 to activate/deactivate faculty\nEnter 4 to add student\nEnter 5 to edit student\nEnter 6 to activate/deactivate student\nEnter 7 to sign out\n");
		char option;
		input_size(&option, 1);
		int res = -1;
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
	return 0;
}

int remove_course(int socket_fd, int fid) {
	return 0;
}

int view_enrollments(int socket_fd, int fid) {
	return 0;
}

int change_password(int socket_fd, int id, int user) {
	return 0;
}

int faculty(int socket_fd) {
	int id = login(socket_fd, FACULTY);
    if(logged_in == -1) {
    	return;
    }
    output("Welcome to admin\n");
   	int run = 1;
   	while(run) {
   		output("Enter 1 to add a new course\nEnter 2 to Remove a course\nEnter 3 to view enrollments for a course\nEnter 4 to change password\nEnter 0 to sign out\n";
   		char option;
   		input_size(&option, 1);
		int res = -1;
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
	return 0;
}

int unenroll_course(int socket_fd, int sid) {
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
	while(run) {
		output("Enter 1 to enroll to new course\nEnter 2 to unenroll from a course\nEnter 3 to change password\nEnter 0 to exit\n");
		char option;
		read_size(&option, 1);
		int res;
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
	output("Welcome to Course Registration Portal\n");
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

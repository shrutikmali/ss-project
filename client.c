#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define MAX_LEN 100

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

void flush_input() {
    char c;
    while (read(0, &c, 1) > 0 && c != '\n') {
		
    }
}

int input_size(char *buff, int size) {
	memset(buff, '\0', size);
	int res = read(0, buff, size);
	flush_input();
	return res;
}

int input(char *buff) {
	int size = 0;
	char c;
	while(read(0, &c, sizeof(c)) > 0) {
		if(c == '\n') {
			buff[size++] = '\0';
			break;
		}
		else {
			buff[size++] = c;
		}
	}
	// flush_input();
	return size;
}

int parse_int(int size, char str[100]) {
	int num = 0;
	for(int i = 0; i < size && '0' <= str[i] && str[i] <= '9'; i++) {
		num = (num * 10) + (int)(str[i] - '0');
	}
	return num;
}

int login(int sfd, int type) {
	int opcode = 0;
	int res = send(sfd, &opcode, sizeof(opcode), 0);
	if(res == -1) {
		perror("Error sending opcode");
	}

	char email[100];
	char password[100];

	int logged_in = 0;
	res = send(sfd, &type, sizeof(type), 0);
	if(res == -1) {
		perror("Error sending login type");
		return -1;
	}

	while(!logged_in) {

		// Reset fields
		memset(email, '\0', 100);
		memset(password, '\0', 100);

		output("Enter email: ");
		input(email);
		output("Enter password: ");
		input(password);

		send(sfd, &email, sizeof(email), 0);
		send(sfd, &password, sizeof(password), 0);

		int response;
		output("Logging in...\n");
		recv(sfd, &response, sizeof(response), 0);

		if(response > 0) {
			logged_in = response;
		}
		else if(response == 0) {
			output("User not found\n");
		}
		else if(response < -1) {
			output("Server oof'd while logging in\n");
			return -1;
		}
		else {
			output("Incorrect credentials, try again\n");
		}
	}
	
	output("Logged in successfully!\n");
	return logged_in;
}

int add_faculty(int sfd) {

	int opcode = 1;
	send(sfd, &opcode, sizeof(opcode), 0);

	char name[100];
	char email[100];
	char password[100];
	
	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	output("Enter name: ");
	int res = input(name);
	output("Enter email: ");
	res = input(email);
	output("Enter password: ");
	res = input(password);

	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);

	if(res == -2) {
		output("Server oof'd in adding faculty\n");
	}
	else if(res == -1) {
		output("Email already exists\n");
	}
	else {
		output("Faculty created\n");
	}

	return res;
}

int edit_faculty(int sfd) {

	int opcode = 2;
	send(sfd, &opcode, sizeof(opcode), 0);

	char id_str[100];
	output("Enter faculty id: ");
	int size = input(id_str);
	int id = parse_int(size, id_str);
	// printf("Parsed id: %d\n", id);

	// Send data to server
	char name[100];
	char email[100];
	char password[100];
	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	output("Enter new name: ");
	int res = input(name);

	output("Enter new email: ");
	res = input(email);

	output("Enter new password: ");
	res = input(password);

	send(sfd, &id, sizeof(id), 0);
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);

	if(res > 0) {
		output("Faculty edited\n");
	}
	else if(res == 0) {
		output("Faculty not found\n");
	}
	else if(res == -1) {
		output("Email already exists\n");
	}
	else {
		output("Server oof'd in editing faculty\n");
	}
	return res;
}

int status_faculty(int sfd) {
	int opcode = 3;
	send(sfd, &opcode, sizeof(opcode), 0);
	
	char id_str[100];
	memset(id_str, '\0', 100);

	output("Enter faculty id: ");
	int size = input(id_str);
	int id = parse_int(size, id_str);
	
	char option_char;
	output("Enter 1 to activate or 0 to deactivate: ");
	input_size(&option_char, 1);
	int option = (int)(option_char - '0');

	send(sfd, &id, sizeof(id), 0);
	send(sfd, &option, sizeof(option), 0);
	
	int res;
	recv(sfd, &res, sizeof(res), 0);
	
	if(res > 0) {
		output("Status updated\n");
	}
	else if(res == 0) {
		output("Faculty not found\n");
	}
	else {
		output("Server oof'd while changing status\n");
	}	
	
	return res;
}

int get_faculty_details(int sfd) {
	int opcode = 15;
	send(sfd, &opcode, sizeof(opcode), 0);
	
	char fid_str[100];
	memset(fid_str, '\0', 100);
	
	output("Enter faculty id: ");
	int size  = input(fid_str);
	int fid = parse_int(size, fid_str);

	send(sfd, &fid, sizeof(fid), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);
	if(res == 1) {
		int status;
		char name[100];
		char email[100];
		
		memset(name, '\0', 100);
		memset(email, '\0', 100);
		
		recv(sfd, &name, sizeof(name), 0);
		recv(sfd, &email, sizeof(email), 0);
		recv(sfd, &status, sizeof(status), 0);

		output("Name: ");
		output(name);
		output("\n");
		
		output("Email: ");
		output(email);
		output("\n");

		output("Status: ");
		output(status == 1 ? "Active" : "Inactive");
		output("\n");

	}
	else if(res == -1) {
		output("Faculty not found\n");
	}
	else if(res < -1) {
		output("Server oof'd in getting faculty details\n");
	}
	return res;
}

int add_student(int sfd) {
	int opcode = 4;
	send(sfd, &opcode, sizeof(opcode), 0);

	char name[MAX_LEN];
	char email[MAX_LEN];
	char password[MAX_LEN];

	memset(name, '\0', sizeof(name));
	memset(email, '\0', sizeof(email));
	memset(password, '\0', sizeof(password));

	output("Enter name: ");
	int res = input(name);

	output("Enter email: ");
	res = input(email);

	output("Enter password: ");
	res = input(password);

	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);

	recv(sfd, &res, sizeof(res), 0);
	if(res > 0) {
		output("New student added sucessfully\n");
	}
	if(res == -1) {
		output("Email already exists\n");
	}
	else {
		output("Server oof'd in adding student\n");
	}

	return res;
}

int edit_student(int sfd) {
	int opcode = 5;
	send(sfd, &opcode, sizeof(opcode), 0);

	char id_str[100];
	memset(id_str, '\0', 100);

	output("Enter student id: ");
	int size = input(id_str);
	int id = parse_int(size, id_str);
	
	char name[100];
	char email[100];
	char password[100];

	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	output("Enter new name: ");
	int res = input(name);

	output("Enter new email: ");
	res = input(email);

	output("Enter new password: ");
	res = input(password);

	// Send data to server
	send(sfd, &id, sizeof(id), 0);
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &email, sizeof(email), 0);
	send(sfd, &password, sizeof(password), 0);
	
	recv(sfd, &res, sizeof(res), 0);
	if(res > 0) {
		output("Student edited\n");
	}
	else if(res == 0) {
		output("Student not found\n");
	}
	else if(res == -1) {
		output("Email already exists\n");
	}
	else {
		output("Server oof'd in editing student\n");
	}

	return res;
}

int status_student(int sfd) {
	int opcode = 6;
	send(sfd, &opcode, sizeof(opcode), 0);

	char id_str[100];
	memset(id_str, '\0', sizeof(id_str));
	output("Enter student id: ");
	int size = input(id_str);
	int id = parse_int(size, id_str);
	
	char option_char;
	output("Enter 1 to activate or 0 to deactivate: ");
	input_size(&option_char, 1);
	int option = (int)(option_char - '0');

	send(sfd, &id, sizeof(id), 0);
	send(sfd, &option, sizeof(option), 0);
	
	int res;
	recv(sfd, &res, sizeof(res), 0);
	
	if(res == 1) {
		output("Status updated\n");
	}
	else if(res == -1) {
		output("Student not found\n");
	}
	else {
		output("Server oof'd while changing status\n");
	}
	
	return res;
}

int get_student_details(int sfd) {
	int opcode = 14;
	send(sfd, &opcode, sizeof(opcode), 0);
	
	char sid_str[100];
	memset(sid_str, '\0', sizeof(sid_str));

	output("Enter student id: ");
	int size  = input(sid_str);
	int sid = parse_int(size, sid_str);

	send(sfd, &sid, sizeof(sid), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);
	if(res == 1) {
		char name[100];
		char email[100];
		int status;
		recv(sfd, &name, sizeof(name), 0);
		recv(sfd, &email, sizeof(email), 0);
		recv(sfd, &status, sizeof(status), 0);

		output("Name: ");
		output(name);
		output("\n");
		
		output("Email: ");
		output(email);
		output("\n");

		output("Status: ");
		output(status == 1 ? "Active" : "Inactive");
		output("\n");

	}
	else if(res == -1) {
		output("Student not found\n");
	}
	else if(res < -1) {
		output("Server oof'd in getting student details\n");
	}
	return res;
}

int admin(int socket_fd) {
	int id = login(socket_fd, ADMIN);
	if(id == -1) {
		return -1;
	}
	output("\n");
	output("Welcome to admin\n");
	int run = 1;
	int res = -1;
	while(run) {
		output("Enter 1 to add faculty\nEnter 2 to edit faculty\nEnter 3 to activate/deactivate faculty\nEnter 4 to get faculty details\nEnter 5 to add student\nEnter 6 to edit student\nEnter 7 to activate/deactivate student\nEnter 8 to get student details\nEnter 0 to sign out\nEnter your option: ");
		char option;
		input_size(&option, 1);
		output("\n");
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
			res = get_faculty_details(socket_fd);
		}
		else if(option == '5') {
			res = add_student(socket_fd);
		}
		else if(option == '6') {
			res = edit_student(socket_fd);
		}
		else if(option == '7') {
			res = status_student(socket_fd);
		}
		else if(option == '8') {
			res = get_student_details(socket_fd);
		}
		else if(option == '0') {
			res = 0;
			run = 0;
		}
		else {
			output("Invalid option, try again\n");
		}
		output("------------------------------------------------\n");
	}
	return res;
}

int add_course(int sfd, int fid) {
	int opcode = 7;
	send(sfd, &opcode, sizeof(opcode), 0);

	char name[100];
	char credits_str[100];
	int credits;
	char maxStrength_str[100];
	int maxStrength;

	memset(name, '\0', sizeof(name));
	memset(credits_str, '\0', sizeof(credits_str));
	memset(maxStrength_str, '\0', sizeof(maxStrength_str));

	output("Enter course name: ");
	int len = input(name);
	
	output("Enter credits: ");
	len = input(credits_str);
	credits = parse_int(len, credits_str);
	
	output("Enter maximum class size: ");
	len = input(maxStrength_str);
	maxStrength = parse_int(len, maxStrength_str);

	printf("Max strength is: %d\n", maxStrength);

	send(sfd, &fid, sizeof(fid), 0);
	send(sfd, &name, sizeof(name), 0);
	send(sfd, &credits, sizeof(credits), 0);
	send(sfd, &maxStrength, sizeof(maxStrength), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);

	if(res == 1) {
		output("Course added successfully\n");
	}
	else {
		output("Server oof'd in adding course\n");
	}

	return res;
}

int remove_course(int sfd, int fid) {
	int opcode = 8;
	send(sfd, &opcode, sizeof(opcode), 0);
	
	char id_str[100];

	memset(id_str, '\0', sizeof(id_str));

	output("Enter course id: ");
	input(id_str);
	int id = parse_int(100, id_str);
	printf("Id is: %d\n", id);
	send(sfd, &id, sizeof(id), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);

	if(res == 1) {
		output("Course removed successfully\n");
	}
	else if(res == 0) {
		output("Course not found\n");
	}
	else {
		output("Server oof'd in removing course\n");
	}

	return res;
}

int view_enrollments_faculty(int sfd, int fid) {
	int opcode = 9;
	send(sfd, &opcode, sizeof(opcode), 0);
	
	char id_str[100];

	memset(id_str, '\0', sizeof(id_str));

	output("Enter course id: ");
	int size = input(id_str);
	int id = parse_int(size, id_str);

	send(sfd, &id, sizeof(id), 0);

	output("Enrolled students are:\n");
	while(1) {
		int res;
		recv(sfd, &res, sizeof(res), 0);
		if(res != 0) {
			int id;
			char name[100];

			memset(name, '\0', sizeof(res));

			recv(sfd, &id, sizeof(id), 0);
			recv(sfd, &name, sizeof(name), 0);

			output(name);
			output("\n");
		}
		else {
			break;
		}
	}

	return 0;
}

int change_password(int sfd, int id, int user) {
	char password[100];
	memset(password, '\0', sizeof(password));

	output("Enter new password: ");
	input(password);

	int opcode;
	if(user == STUDENT) {
		opcode = 11;
	}
	else {
		opcode = 10;
	}
	
	send(sfd, &opcode, sizeof(opcode), 0);

	send(sfd, &id, sizeof(id), 0);
	send(sfd, &password, sizeof(password), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);
	if(res == 1) {
		output("Password changed successfully\n");
	}
	else if(res == 0) {
		output("User not found\n");
	}
	else {
		output("Server oof'd in changing password\n");
	}
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
   		output("Enter 1 to add a new course\nEnter 2 to Remove a course\nEnter 3 to view enrollments for a course\nEnter 4 to change password\nEnter 0 to sign out\nEnter your option: ");
   		char option;
   		input_size(&option, 1);
   		if(option == '1') {
   			res = add_course(socket_fd, id);
   		}
   		else if(option == '2') {
   			res = remove_course(socket_fd, id);
   		}
   		else if(option == '3') {
   			res = view_enrollments_faculty(socket_fd, id);
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
		output("------------------------------------------------\n");
   	}
   	return res;
}

int enroll_course(int sfd, int sid) {
	int opcode = 12;
	send(sfd, &opcode, sizeof(opcode), 0);
	printf("Student id is: %d\n", sid);
	char cid_str[100];
	memset(cid_str, '\0', sizeof(cid_str));
	
	output("Enter course id: ");
	int cid_size = input(cid_str);
	int cid = parse_int(cid_size, cid_str);
	

	send(sfd, &sid, sizeof(sid), 0);
	send(sfd, &cid, sizeof(cid), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);

	if(res > 0) {
		output("Enrolled successfully\n");
	}
	else if(res == 0) {
		output("Reached course enrollment limit\n");
	}
	else {
		output("Server oof'd in enrolling course\n");
	}

	return res;
}

int unenroll_course(int sfd, int sid) {
	int opcode = 13;
	send(sfd, &opcode, sizeof(opcode), 0);

	char cid_str[100];
	memset(cid_str, '\0', sizeof(cid_str));

	output("Enter course id: ");
	int cid_size = input(cid_str);
	int cid = parse_int(cid_size, cid_str);

	send(sfd, &sid, sizeof(sid), 0);
	send(sfd, &cid, sizeof(cid), 0);

	int res;
	recv(sfd, &res, sizeof(res), 0);

	if(res > 0) {
		output("Course unenrolled successfully\n");
	}
	else if(res == 0) {
		output("Course/Student not found\n");
	}
	else {
		output("Server oof'd in unenrolling course\n");
	}
}

int view_enrollments_student(int sfd, int id) {
	int opcode = 17;
	send(sfd, &opcode, sizeof(opcode), 0);

	send(sfd, &id, sizeof(id), 0);
	int res = 0;

	char name[100];

	while(1) {
		memset(name, '\0', sizeof(name));

		recv(sfd, &res, sizeof(res), 0);
		if(res > 0) {
			recv(sfd, &name, sizeof(name), 0);

			output(name);
			output("\n");
		}
		else {
			break;
		}
	}

	return res;
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
		output("Enter 1 to enroll to new course\nEnter 2 to unenroll from a course\nEnter 3 to view enrollments\nEnter 4 to change password\nEnter 0 to exit\nEnter your option: ");
		char option;
		input_size(&option, 1);
		if(option == '1') {
			res = enroll_course(socket_fd, id);
		}
		else if(option == '2') {
			res = unenroll_course(socket_fd, id);
		}
		else if(option == '3') {
			res = view_enrollments_student(socket_fd, id);
		}
		else if(option == '4') {
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
		output("------------------------------------------------\n");
	}
	return res;
}

int main() {
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_res = connect(cfd, (struct sockaddr*)&server, sizeof(server));
    if(connect_res == -1) {
        output("Error connecting\n");
        return -1;
    }
    output("Connected to server\n");
	output("Getting next port\n");
	sleep(3);
    int newPort;

	recv(cfd, &newPort, sizeof(newPort), 0);
    printf("New port: %d\n", newPort);
   	cfd = socket(AF_INET, SOCK_STREAM, 0);
   	server.sin_port = htons(newPort);
   	connect_res = connect(cfd, (struct sockaddr*)&server, sizeof(server));
   	if(connect_res == -1) {
   		printf("Error connecting\n");
   		return -1;
   	}

	int run = 1;
	output("Welcome to Academia Portal\n");
	while(run) {
		output("Enter 1 to sign in as admin\nEnter 2 to sign in as faculty\nEnter 3 to sign in as student\nEnter 0 to exit\n");
		output("Enter your choice: ");
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
			int quit = -1;
			send(cfd, &quit, sizeof(quit), 0);
			close(cfd);
			run = 0;
		}
		else {
			output("Invalid option, try again\n");
		}
	}

	close(cfd);
	return 0;
}

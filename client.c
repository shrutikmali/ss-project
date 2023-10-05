#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int output(char *buff, int size) {
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

int input(char *buff, int size) {
	int res = read(0, buff, size);
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
		output("Enter 1 to sign in as admin\nEnter 2 to sign in as faculty\nEnter 3 to sign in as student\nEnter 0 to exit");
		char option;
		input(&option, 1);
		output(option, 1);
	}
	return 0;
}

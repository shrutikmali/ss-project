#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LEN 100

struct id_count {
	int admin;
	int student;
	int faculty;
	int course;
};

struct Admin {
	int id;
	char email[MAX_LEN];
	char password[MAX_LEN];
};

struct Student {
	int id;
	char name[MAX_LEN];
	char email[MAX_LEN];
	char password[MAX_LEN];
	int courseIdx;
	int courses[MAX_LEN];
	char status;
};

struct Faculty {
	int id;
	char name[MAX_LEN];
	char email[MAX_LEN];
	char password[MAX_LEN];
	int courseIdx;
	int courses[MAX_LEN];
	int status;
};

struct Course {
	int id;
	char name[MAX_LEN];
	int fid;
	int credits;
	int maxStrength;
	int status;
	int students[MAX_LEN];
	int studentIdx;
};

int main() {
	int fd_id = open("./data/id", O_CREAT | O_RDWR);
	int fd_student = open("./data/student", O_CREAT | O_RDWR);
	int fd_faculty = open("./data/faculty", O_CREAT | O_RDWR);
	int fd_course = open("./data/course", O_CREAT | O_RDWR);
	int fd_admin = open("./data/admin", O_CREAT | O_RDWR);
	ftruncate(fd_id, 0);
	ftruncate(fd_student, 0);
	ftruncate(fd_faculty, 0);
	ftruncate(fd_course, 0);
	ftruncate(fd_admin, 0);

	struct Admin admin;
	admin.id = 1;
	strcpy(admin.email, "abc@abc.com");
	strcpy(admin.password, "123");
	write(fd_admin, &admin, sizeof(admin));

	struct id_count ids;
	ids.admin = 1;
	ids.student = 1;
	ids.faculty = 1;
	ids.course = 1;
	write(fd_id, &ids, sizeof(ids));

	printf("Reset complete\n");

	return 0;
}

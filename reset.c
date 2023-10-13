#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct id_count {
	int admin;
	int student;
	int faculty;
	int course;
};

struct Admin {
	int id;
	char email[100];
	char password[100];
};

struct Student {
	int id;
	char name[100];
	char email[100];
	char password[100];
	int courseIdx;
	int courses[100];
	char status;
};

struct Faculty {
	int id;
	char name[100];
	char email[100];
	char password[100];
	int courseIdx;
	int courses[100];
	int status;
};

struct Course {
	int id;
	char name[100];
	int fid;
	int credits;
	int maxStrength;
	int status;
	int students[100];
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
	admin.id = 0;
	strcpy(admin.email, "abc@abc.com");
	strcpy(admin.email, "123");
	write(fd_admin, &admin, sizeof(admin));

	return 0;
}

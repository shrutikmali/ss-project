#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	int fd_id = open("./data/id", O_CREAT | O_RDWR);
	int fd_student = open("./data/student", O_CREAT | O_RDWR);
	int fd_faculty = open("./data/faculty", O_CREAT | O_RDWR);
	int fd_course = open("./data/course", O_CREAT | O_RDWR);
	ftruncate(fd_id, 0);
	ftruncate(fd_student, 0);
	ftruncate(fd_faculty, 0);
	ftruncate(fd_course, 0);
	return 0;
}

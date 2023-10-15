#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int errno;

int ADMIN = 1;
int FACULTY = 2;
int STUDENT = 3;
int COURSE = 4;
int UNLOCK = F_UNLCK;
int R_LOCK = F_RDLCK;
int W_LOCK = F_WRLCK;

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
	int status;
};


void student_constructor(struct Student *student) {
	student->id = 0;
	memset(student->name, '\0', sizeof(student->name));
	memset(student->email, '\0', sizeof(student->email));
	memset(student->password, '\0', sizeof(student->password));
	memset(student->courses, 0, sizeof(student->courses));
	student->courseIdx = 0;
	student->status = 0;
}

struct Faculty {
	int id;
	char name[MAX_LEN];
	char email[MAX_LEN];
	char password[MAX_LEN];
	int courseIdx;
	int courses[MAX_LEN];
	int status;
};

void faculty_constructor(struct Faculty *faculty) {
	faculty->id = 0;
	memset(faculty->name, '\0', sizeof(faculty->name));
	memset(faculty->email, '\0', sizeof(faculty->email));
	memset(faculty->password, '\0', sizeof(faculty->password));
	memset(faculty->courses, 0, sizeof(faculty->courses));
	faculty->courseIdx = 0;
	faculty->status = 0;
}

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

void course_constructor(struct Course *course) {
	course->id = 0;
	memset(course->name, '\0', sizeof(course->name));
	course->fid = 0;
	course->credits = 0;
	course->maxStrength = 0;
	course->status = 0;
	memset(course->students, 0, sizeof(course->students));
	course->studentIdx = 0;
}

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

int set_lock(int fd, struct flock *lock, int type, int whence, int start, int len) {
	/*
		return:
		1: success
		-1: fail
	*/
	int res;
	if(type == F_UNLCK) {
		lock->l_type = type;
		lock->l_whence = whence;
		lock->l_start = start;
		lock->l_len = len;
		lock->l_pid = getpid();
		// printf("Boolean check: %d\n", lock->l_type == F_UNLCK);
		res = fcntl(fd, F_SETLK, lock);
	}
	else {
		lock->l_type = type;
		lock->l_whence = whence;
		lock->l_start = start;
		lock->l_len = len;
		lock->l_pid = getpid();
		res = fcntl(fd, F_SETLKW, lock);
	}

	if(res == -1) {
		perror("Error in locking");
	}
	else {
		res = 1;
	}
	return res;
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

int string_equal(char *s, char *t) {
	while(*s != '\0' && *t != '\0') {
		if(*s != *t) {
			return 0;
		}
		s++;
		t++;
	}
	if((*s != '\n' && *t != '\n') && (*s == '\0' && *t != '\0' || *s != '\0' && *t == '\0')) {
		return 0;
	}
	return 1;
}

void string_copy(char src[MAX_LEN], char dest[MAX_LEN]) {
    for(int i = 0; i < MAX_LEN; i++) {
        dest[i] = src[i];
    }
    return;
}

int get_id(int type) {
	int fd = open("./data/id", O_RDWR);

	if(fd < 0) {
		perror("Error in opening ids");
		return -1;
	}
	struct flock lock;
	int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_SET, 0, 0);
	if(lock_res == -1) {
		perror("Error in locking ids file");
		return -1;
	}
	struct id_count ids;
	int read_res = read(fd, &ids, sizeof(ids));
	if(read_res < 0) {
		perror("Error in reading ids");
		return -1;
	}
	int returnID = -1;
	if(type == ADMIN) {
		returnID = ids.admin;
		ids.admin++; 
	}
	else if(type == FACULTY) {
		returnID = ids.faculty;
		ids.faculty++;
	}
	else if(type == STUDENT) {
		returnID = ids.student;
		ids.student++;
	}
	else if(type == COURSE) {
		returnID = ids.course;
		ids.course++;
	}
	lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
	if(lock_res == -1) {
		perror("Error in unlocking ids");
		returnID = -1;
	}

	return returnID;
}

int search_admin_email(char email[MAX_LEN], int fd) {
	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Admin admin;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(admin));
		if(res < -1) {
			perror("Error acquiring lock:search_admin_email");
			break;
		}
		memset(admin.email, '\0', MAX_LEN);
		int size = read(fd, &admin, sizeof(admin));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(string_equal(admin.email, email)) {
			found = 1;
			lseek(fd, -1 * sizeof(admin), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int search_student_id(int id, int fd) {

	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Student student;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(student));
		if(res < -1) {
			perror("Error acquiring lock:search_student_id");
			break;
		}
		int size = read(fd, &student, sizeof(student));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(student.id == id) {
			found = 1;
			lseek(fd, -1 * sizeof(student), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int search_student_email(char email[MAX_LEN], int fd) {
	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Student student;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(student));
		if(res < -1) {
			perror("Error acquiring lock:search_student_email");
			break;
		}
		memset(student.email, '\0', MAX_LEN);
		int size = read(fd, &student, sizeof(student));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(string_equal(email, student.email)) {
			found = 1;
			lseek(fd, -1 * sizeof(student), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int search_faculty_id(int id, int fd) {
	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Faculty faculty;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(faculty));
		if(res < -1) {
			perror("Error acquiring lock:search_faculty_id");
			break;
		}
		int size = read(fd, &faculty, sizeof(faculty));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(faculty.id == id) {
			found = 1;
			lseek(fd, -1 * sizeof(faculty), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int search_faculty_email(char email[MAX_LEN], int fd) {
	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Faculty faculty;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(faculty));
		if(res < -1) {
			perror("Error acquiring lock:search_faculty_email");
			break;
		}
		memset(faculty.email, '\0', 100);
		int size = read(fd, &faculty, sizeof(faculty));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(string_equal(faculty.email, email)) {
			found = 1;
			lseek(fd, -1 * sizeof(faculty), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int search_course_id(int id, int fd) {
	/*
		return:
		1: if found
		0: not found
		-1: error
	*/
	lseek(fd, 0, SEEK_SET);
	int res = 0;
	struct Course course;
	int found = 0, eof = 0;

	while(!found && !eof) {
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(course));
		if(res < -1) {
			perror("Error acquiring lock:search_course_id");
			break;
		}
		int size = read(fd, &course, sizeof(course));
		if(size == 0) {
			res = 0;
			eof = 1;
		}
		else if(course.id == id) {
			found = 1;
			lseek(fd, -1 * sizeof(course), SEEK_CUR);
			set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}
	
	return res;
}

int student_login(int cfd) {
	/*
		return:
		id if correct
		0 if not found
		-1 if incorrect
		< -1 if error
	*/
	int fd = open("./data/student", O_RDONLY);

	char email[100];
	char password[100];

	int logged_in = 0;
	int res = 0;
	struct Student student;

	while(!logged_in) {
		memset(email, '\0', 100);
		memset(password, '\0', 100);
		lseek(fd, 0, SEEK_SET);

		recv(cfd, &email, sizeof(email), 0);
		recv(cfd, &password, sizeof(password), 0);
		
		int search_res = search_student_email(email, fd);
		if(search_res == 1) {
			read(fd, &student, sizeof(student));
			if(string_equal(password, student.password)) {
				logged_in = 1;
				res = student.id;
			}
			else {
				res = -1;
			}
		}
		else if(search_res == 0) {
			res = 0;
			logged_in = 0;
		}
		else {
			res = -2;
			logged_in = 1;
		}

		send(cfd, &res, sizeof(res), 0);
	}
	
	close(fd);
	return res;
}

int faculty_login(int cfd) {
	/*
		return:
		id if correct
		0 if not found
		-1 if incorrect
		< -1 if error
	*/
	int fd = open("./data/faculty", O_RDONLY);

	char email[100];
	char password[100];

	int logged_in = 0;
	int res = 0;
	struct Faculty faculty;

	while(!logged_in) {
		memset(email, '\0', 100);
		memset(password, '\0', 100);
		lseek(fd, 0, SEEK_SET);

		recv(cfd, &email, sizeof(email), 0);
		recv(cfd, &password, sizeof(password), 0);
		
		int search_res = search_faculty_email(email, fd);
		if(search_res == 1) {
			read(fd, &faculty, sizeof(faculty));
			if(string_equal(password, faculty.password)) {
				logged_in = 1;
				res = faculty.id;
			}
			else {
				res = -1;
			}
		}
		else if(search_res == 0) {
			res = 0;
			logged_in = 0;
		}
		else {
			res = -2;
			logged_in = 1;
		}

		send(cfd, &res, sizeof(res), 0);
	}
	
	close(fd);
	return res;
}

int admin_login(int cfd) {
	/*
		return:
		id if correct
		0 if not found
		-1 if incorrect
		< -1 if error
	*/
	int fd = open("./data/admin", O_RDONLY);

	char email[100];
	char password[100];

	int logged_in = 0;
	int res = 0;
	struct Admin admin;

	while(!logged_in) {
		memset(email, '\0', 100);
		memset(password, '\0', 100);
		lseek(fd, 0, SEEK_SET);

		recv(cfd, &email, sizeof(email), 0);
		recv(cfd, &password, sizeof(password), 0);
		
		int search_res = search_admin_email(email, fd);
		if(search_res == 1) {
			read(fd, &admin, sizeof(admin));
			if(string_equal(password, admin.password)) {
				logged_in = 1;
				res = admin.id;
			}
			else {
				res = -1;
			}
		}
		else if(search_res == 0) {
			res = 0;
			logged_in = 0;
		}
		else {
			res = -2;
			logged_in = 1;
		}

		send(cfd, &res, sizeof(res), 0);
	}
	
	close(fd);
	return res;
}

int login(int cfd) {
	int type;
	recv(cfd, &type, sizeof(type), 0);

	if(type == ADMIN) {
		return admin_login(cfd);
	}
	else if(type == FACULTY) {
		return faculty_login(cfd);
	}
	else if(type == STUDENT) {
		return student_login(cfd);
	}
	return 0;
}

int get_student_details(int cfd) {

	/*
		return:
		1: if found
		0: not found
		-1: error
	*/

	int uid;
	int res = 0;
	int fd = open("./data/student", O_RDONLY);

	recv(cfd, &uid, sizeof(uid), 0);
	res = search_student_id(uid, fd);

	char name[100];
	char email[100];
	int status;
	
	if(res > 0) {
		struct Student student;

		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(student));

		if(res < 0) {
			perror("Error locking file:get_student_details");
			res = -1;
		}
		else {
			res = read(fd, &student, sizeof(student));
			if(res <= 0) {
				perror("Error reading record:get_student_details");
				res = -1;
			}
			else {
				memset(name, '\0', 100);
				memset(email, '\0', 100);
				
				string_copy(student.name, name);
				string_copy(student.email, email);
				status = student.status;

				res = 1;
			}
		}
	}

	close(fd);
	send(cfd, &res, sizeof(res), 0);
	if(res == 1) {
		send(cfd, &res, sizeof(res), 0);
		send(cfd, &name, sizeof(name), 0);
		send(cfd, &email, sizeof(email), 0);
		send(cfd, &status, sizeof(status), 0);
	}

	return res;
}

int get_faculty_details(int cfd) {
	int uid;
	int res = -1;
	int fd = open("./data/faculty", O_RDONLY);

	recv(cfd, &uid, sizeof(uid), 0);
	res = search_faculty_id(uid, fd);

	char name[100];
	char email[100];
	int status;

	if(res > 0) {
		struct Faculty faculty;

		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(faculty));

		if(res < 0) {
			perror("Error locking file:get_faculty_details");
			res = -1;
		}
		else {
			res = read(fd, &faculty, sizeof(faculty));
			if(res <= 0) {
				perror("Error reading record:get_faculty_details");
				res = -1;
			}
			else {
				memset(name, '\0', 100);
				memset(email, '\0', 100);

				string_copy(faculty.name, name);
				string_copy(faculty.email, email);
				status = faculty.status;
				res = 1;
			}
		}
	}

	close(fd);
	send(cfd, &res, sizeof(res), 0);
	if(res == 1) {
		send(cfd, &name, sizeof(name), 0);
		send(cfd, &email, sizeof(email), 0);
		send(cfd, &status, sizeof(status), 0);
	}

	return res;
}

int get_course_details(int cfd) {
	int uid;
	int res = -1;
	int fd = open("./data/course", O_RDONLY);

	char name[100];
	int fid;
	int credits;
	int currStrength;
	int maxStrength;
	int status;

	recv(cfd, &uid, sizeof(uid), 0);
	res = search_course_id(uid, fd);

	// Not found (-1) or error
	if(res > 0) {
		struct Course course;
		struct flock lock;

		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(course));
		if(res < 0) {
			perror("Error locking file:get_faculty_details");
			res = -1;
		}
		else {
			res = read(fd, &course, sizeof(course));

			if(res <= 0) {
				perror("Error reading record:get_faculty_details");
				res = -1;
			}
			else {
				memset(name, '\0', 100);

				string_copy(course.name, name);
				fid = course.fid;
				credits = course.credits;
				currStrength = course.studentIdx;
				maxStrength = course.maxStrength;
				status = course.status;
				res = 1;	
			}
		}
	}
	close(fd);
	send(cfd, &res, sizeof(res), 0);

	if(res > 0) {
		send(cfd, &res, sizeof(res), 0);
		send(cfd, &name, sizeof(name), 0);
		send(cfd, &fid, sizeof(fid), 0);
		send(cfd, &credits, sizeof(credits), 0);
		send(cfd, &currStrength, sizeof(currStrength), 0);
		send(cfd, &maxStrength, sizeof(maxStrength), 0);
		send(cfd, &status, sizeof(status), 0);
	}

	return res;
}

int add_faculty(int cfd) {
	char name[100];
	char email[100];
	char password[100];
	struct Faculty newFaculty;

	faculty_constructor(&newFaculty);
	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	int res = 0;
	int fd = open("./data/faculty", O_RDWR);
	int email_collision = search_faculty_email(email, fd);

	if(email_collision == -1) {
		res = -1;
	}
	else if(email_collision == 1) {
		res = 0;
	}
	else {
		int newId = get_id(FACULTY);
		int res = 0;
		// printf("Received id: %d\n", newId);
		if(newId == -1) {
			output("Error getting id:add_faculty\n");
			res = -2;
		}
		else {
			newFaculty.id = newId;
			string_copy(name, newFaculty.name);
			string_copy(email, newFaculty.email);
			string_copy(password, newFaculty.password);
			newFaculty.courseIdx = 0;
			newFaculty.status = 1;
			
			// printf("%s\n%s\n%s\n", newFaculty.name, newFaculty.email, newFaculty.password);
			
			fd = open("./data/faculty", O_WRONLY);
			if(fd < 0) {
				perror("Error in opening faculty");
				res = -2;
			}
			else {
				int seek_res = lseek(fd, 0, SEEK_END);
				if(seek_res < 0) {
					perror("Error in seeking faculty");
					res = -2;
				}
				else {
					struct flock lock;
					int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(newFaculty));
					if(lock_res == -1) {
						perror("Error in locking faculty:add_faculty");
						res = -2;
					}
					else {
						res = write(fd, &newFaculty, sizeof(newFaculty));
						int unlock_res = set_lock(fd, &lock, F_UNLCK, SEEK_CUR, 0, sizeof(newFaculty));
						if(unlock_res == -1) {
							perror("Error in unlocking faculty:add_faculty");
							res = -2;
						}
						else {
							res = newId;
						}
					}
				}
			}
		}
	}

	close(fd);
	send(cfd, &res, sizeof(res), 0);

	return res;
}

int edit_faculty(int cfd) {
	int id;
	char name[100];
	char email[100];
	char password[100];


	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	int fd = open("./data/faculty", O_RDWR);
	int res = 0;

	if(fd < 0) {
		perror("Error opening faculty:edit_faculty");
		res = -2;
	}
	else {
		int email_collision = search_faculty_email(email, fd);

		if(email_collision == -1) {
			res = -2;
		}
		else if(email_collision == 1) {
			res = 0;
		}
		else {
			struct Faculty faculty;
			res = search_faculty_id(id, fd);

			if(res > 0) {
				struct flock lock;
				
				// First get lock
				int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
				if(lock_res == -1) {
					perror("Error getting write lock:edit_faculty");
					res = -2;
				}
				else {
					// Read into struct
					read(fd, &faculty, sizeof(faculty));

					string_copy(name, faculty.name);
					string_copy(email, faculty.email);
					string_copy(password, faculty.password);
					
					// Write struct back to file
					res = lseek(fd, -1 * sizeof(faculty), SEEK_CUR);
					if(res < 0) {
						perror("Error in seeking while WB:edit_faculty");
					}
					else {
						write(fd, &faculty, sizeof(faculty));

						lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
						if(lock_res == -1) {
							perror("Error releasing write lock:edit_faculty");
							res = -2;
						}
						else {
							res = 1;
						}
					}
				}
			}
		}
		close(fd);
	}
	
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int status_faculty(int cfd) {
	/*
		return:
		1: done
		0: not found
		-1: error
	*/
	int id, option;
	int res = -1;

	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &option, sizeof(option), 0);

	// Open the file
	int fd = open("./data/faculty", O_RDWR);
	if(fd < 0) {
		perror("Error opening file:status_faculty");
		res = -1;
	}
	else {
		struct Faculty faculty;
		res = search_faculty_id(id, fd);
		if(res > 0) {
			// Acquire a lock
			struct flock lock;
			int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
			if(lock_res < 0) {
				perror("Error acquiring lock:status_faculty");
				res = -1;
			}
			else {
				read(fd, &faculty, sizeof(faculty));
				faculty.status = option;
				lseek(fd, -1 * (sizeof(faculty)), SEEK_CUR);
				write(fd, &faculty, sizeof(faculty));
				lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
				if(lock_res < 0) {
					perror("Error releasing lock:status_faculty");
				}
				else {
					res = 1;
				}
			}
		}
	}
	
	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int add_student(int cfd) {
	char name[100];
	char email[100];
	char password[100];
	struct Student newStudent;
	
	student_constructor(&newStudent);
	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	int res = 0;
	int fd = open("./data/student", O_RDWR);
	res = search_student_email(email, fd);
	
	if(res == 0) {
		int newId = get_id(STUDENT);
		if(newId == -1) {
			perror("Error getting id:add_student");
			res = -1;
		}
		else {
			newStudent.id = newId;
			string_copy(name, newStudent.name);
			string_copy(email, newStudent.email);
			string_copy(password, newStudent.password);
			newStudent.courseIdx = 0;
			newStudent.status = 1;
			
			int seek_res = lseek(fd, 0, SEEK_END);

			struct flock lock;
			int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(newStudent));
			if(lock_res == -1) {
				perror("Error in locking faculty:add_student\n");
				res = -1;
			}
			else {
				res = write(fd, &newStudent, sizeof(newStudent));
				int unlock_res = set_lock(fd, &lock, F_UNLCK, SEEK_SET, 0, 0);
				if(unlock_res == -1) {
					output("Error in unlocking faculty:add_student\n");
					res = -1;
				}
				else {
					res = newStudent.id;
				}
			}
		}
	}
	else if(res == 1) {
		res = -1;
	}
	else {
		res = -2;
	}

	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int edit_student(int cfd) {
	int id;
	char name[100];
	char email[100];
	char password[100];

	memset(name, '\0', 100);
	memset(email, '\0', 100);
	memset(password, '\0', 100);

	// Receive data
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	// Open file
	int fd = open("./data/student", O_RDWR);
	int res = search_student_id(id, fd);


	if(res > 0) {
		int email_collision = search_student_email(email, fd);
		if(email_collision == 0) {
			// First get lock
			struct Student student;
			struct flock lock;
			int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(student));
			if(lock_res == -1) {
				perror("Error getting read lock:edit_student");
				res = -1;
			}
			else {
				// Read into struct
				read(fd, &student, sizeof(student));

				// Update fields
				string_copy(name, student.name);
				string_copy(email, student.email);
				string_copy(password, student.password);
				
				// Write struct back to file
				lseek(fd, sizeof(student) * -1, SEEK_CUR);
				write(fd, &student, sizeof(student));
				lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
				res = 1;
			}
		}
		else if(email_collision == 1) {
			res = -1;
		}
		else {
			res = -2;
		}
	}
	
	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int status_student(int cfd) {
	int id, option;
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &option, sizeof(option), 0);

	int res = -2;

	// Open the file
	int fd = open("./data/student", O_RDWR);
	res = search_student_id(id, fd);

	if(res > 0) {
		struct Student student;

		// Acquire a lock
		struct flock lock;
		int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(student));

		read(fd, &student, sizeof(student));
		student.status = option;
		
		lseek(fd,  -1 * (sizeof(student)), SEEK_CUR);
		write(fd, &student, sizeof(student));
		lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
		res = 1;
	}
	else if(res == 0) {
		res = -1;
	}
	else {
		res = -2;
	}
	
	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int add_course(int cfd) {
	char name[100];
	int credits;
	int maxStrength;
	int fid;
	int res = 0;
	
	memset(name, '\0', 100);
	
	recv(cfd, &fid, sizeof(fid), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &credits, sizeof(credits), 0);
	recv(cfd, &maxStrength, sizeof(maxStrength), 0);
	
	struct Course new_course;
	course_constructor(&new_course);
	
	int cid = get_id(COURSE);
	new_course.id = cid;
	new_course.fid = fid;
	new_course.maxStrength = maxStrength;
	new_course.credits = credits;
	string_copy(name, new_course.name);

	int fd = open("./data/course", O_RDWR);
	lseek(fd, SEEK_END, 0);

	struct flock course_lock;
	res = set_lock(fd, &course_lock, W_LOCK, SEEK_SET, 0, sizeof(new_course));
	if(res >= 0) {
		write(fd, &new_course, sizeof(new_course));
		res = set_lock(fd, &course_lock, UNLOCK, SEEK_SET, 0, 0);
		close(fd);

		if(res >= 0) {	
			fd = open("./data/faculty", O_RDWR);
			struct Faculty faculty;
			res = search_faculty_id(fid, fd);			
			if(res > 0) {
				struct flock faculty_lock;
				res = set_lock(fd, &faculty_lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));

				read(fd, &faculty, sizeof(faculty));
				faculty.courseIdx++;
				faculty.courses[faculty.courseIdx] = cid;
				lseek(fd, -1 * sizeof(faculty), SEEK_CUR);
				write(fd, &faculty, sizeof(faculty));

				set_lock(fd, &faculty_lock, UNLOCK, SEEK_SET, 0, 0);
				int res = 1;
			}
			close(fd);
		}
	}


	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int remove_course(int cfd) {
	int cid;
	recv(cfd, &cid, sizeof(cid), 0);

	int res = 0;
	int course_fd = open("./data/course", O_RDWR);

	res = search_course_id(cid, course_fd);

	if(res > 0) {
		struct Course course;
		struct flock course_lock;

		res = set_lock(course_fd, &course_lock, R_LOCK, SEEK_CUR, 0, sizeof(course));
		read(course_fd, &course, sizeof(course));
		set_lock(course_fd, &course_lock, UNLOCK, SEEK_SET, 0, 0);

		// Handle students
		int student_fd = open("./data/student", O_RDWR);
		for(int i = 0; i <= course.studentIdx; i++) {
			res = search_student_id(course.students[i], student_fd);
			if(res > 0) {
				struct Student student;
				struct flock student_lock;
				set_lock(student_fd, &student_lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
				read(student_fd, &student, sizeof(student));
				int j = 0;
				for(j = 0; j <= student.courseIdx; j++) {
					if(student.courses[j] == cid) {
						break;
					}
				}

				if(j <= student.courseIdx) {
					student.courses[j] = student.courses[student.courseIdx];
					student.courses[student.courseIdx] = 0;
					student.courseIdx--;
				}

				lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
				write(student_fd, &student, sizeof(student));
				set_lock(student_fd, &student_lock, UNLOCK, SEEK_SET, 0, 0);
			}
		}
		
		close(student_fd);

		// Handle faculty
		int fid = course.fid;
		int faculty_fd = open("./data/faculty", O_RDWR);
		res = search_faculty_id(fid, faculty_fd);
		
		if(res > 0) {
			struct Faculty faculty;
			struct flock faculty_lock;
			set_lock(faculty_fd, &faculty_lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
			read(faculty_fd, &faculty, sizeof(faculty));
			
			int j = 0;
			for(j = 0; j <= faculty.courseIdx; j++) {
				if(faculty.courses[j] == cid) {
					break;
				}
			}

			if(j <= faculty.courseIdx) {
				faculty.courses[j] = faculty.courses[faculty.courseIdx];
				faculty.courses[faculty.courseIdx] = 0;
				faculty.courseIdx--;
			}

			lseek(faculty_fd, -1 * sizeof(faculty), SEEK_CUR);
			write(faculty_fd, &faculty, sizeof(faculty));
			set_lock(faculty_fd, &faculty_lock, UNLOCK, SEEK_SET, 0, 0);
		}

		close(faculty_fd);

		// Handle course
		lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
		course_constructor(&course);
		write(course_fd, &course, sizeof(course));
		close(course_fd);

		res = 1;
	}

	send(cfd, &res, sizeof(res), 0);
	return res;
}

int view_enrollments(int cfd) {
	int course_id;
	recv(cfd, &course_id, sizeof(course_id), 0);

	int course_fd = open("./data/course", O_RDONLY);
	int res = 0;
	res = search_course_id(course_id, course_fd);

	if(res > 0) {
		struct flock course_lock;
		struct Course course;
		
		set_lock(course_fd, &course_lock, R_LOCK, SEEK_CUR, 0, sizeof(course));
		read(course_fd, &course, sizeof(course));
		set_lock(course_fd, &course_lock, UNLOCK, SEEK_SET, 0, 0);
		
		int idx = 0;

		int student_fd = open("./data/student", O_RDONLY);
		for(int i = 0; i <= course.studentIdx; i++) {
			res = search_student_id(course.students[i], student_fd);
			if(res > 0) {
				struct flock student_lock;
				struct Student student;

				set_lock(student_fd, &student_lock, R_LOCK, SEEK_CUR, 0, sizeof(student));
				read(student_fd, &student, sizeof(student));
				set_lock(student_fd, &student_lock, UNLOCK, SEEK_SET, 0, sizeof(student));
				
				res = 1;
				send(cfd, &res, sizeof(res), 0);
				send(cfd, &student.id, sizeof(student.id), 0);
				send(cfd, &student.name, sizeof(student.name), 0);
			}
		}
		close(student_fd);
	}
	
	close(course_fd);

	res = 0;
	send(cfd, &res, sizeof(res), 0);

	return res;
}

int change_student_password(int cfd) {
	int res = 0;
	int student_id;
	char new_password[100];
	memset(new_password, '\0', 100);

	recv(cfd, &student_id, sizeof(student_id), 0);
	recv(cfd, &new_password, sizeof(new_password), 0);

	int student_fd = open("./data/student", O_RDWR);
	res = search_student_id(student_id, student_fd);

	if(res > 0) {
		struct Student student;
		struct flock student_lock;
		res = set_lock(student_fd, &student_lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
		if(res > 0) {
			read(student_fd, &student, sizeof(student));

			string_copy(new_password, student.password);
			lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
			write(student_fd, &student, sizeof(student));

			set_lock(student_fd, &student_lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}

	close(student_fd);
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int change_faculty_password(int cfd) {
	int res = 0;
	int faculty_id;
	char new_password[100];
	memset(new_password, '\0', 100);

	recv(cfd, &faculty_id, sizeof(faculty_id), 0);
	recv(cfd, &new_password, sizeof(new_password), 0);

	int faculty_fd = open("./data/faculty", O_RDWR);
	res = search_faculty_id(faculty_id, faculty_fd);

	if(res > 0) {
		struct Faculty faculty;
		struct flock faculty_lock;
		res = set_lock(faculty_fd, &faculty_lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
		if(res > 0) {
			read(faculty_fd, &faculty, sizeof(faculty));

			string_copy(new_password, faculty.password);
			lseek(faculty_fd, -1 * sizeof(faculty), SEEK_CUR);
			write(faculty_fd, &faculty, sizeof(faculty));

			set_lock(faculty_fd, &faculty_lock, UNLOCK, SEEK_SET, 0, 0);
			res = 1;
		}
	}

	close(faculty_fd);
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int enroll_course(int cfd) {
	int sid, cid;
	
	recv(cfd, &sid, sizeof(sid), 0);
	recv(cfd, &cid, sizeof(cid), 0);

	int res = 0;

	int course_fd = open("./data/course", O_RDWR);
	res = search_course_id(cid, course_fd);

	if(res > 0) {
		struct Course course;
		struct flock course_lock;
		
		set_lock(course_fd, &course_lock, W_LOCK, SEEK_CUR, 0, sizeof(course));
		read(course_fd, &course, sizeof(course));

		int already_enrolled = 0;
		for(int i = 0; i <= course.studentIdx; i++) {
			if(course.students[i] == sid) {
				already_enrolled = 1;
				break;
			}
		}
		if(already_enrolled == 0) {
			if(1 + course.studentIdx <= course.maxStrength) {
				course.studentIdx++;
				course.students[course.studentIdx] = sid;

				lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
				write(course_fd, &course, sizeof(course));
				res = 1;
			}
			else {
				res = 0;
			}

			set_lock(course_fd, &course_lock, UNLOCK, SEEK_SET, 0, 0);
			close(course_fd);

			if(res > 0) {
				int student_fd = open("./data/student", O_RDWR);
				res = search_student_id(student_fd, sid);
				if(res > 0) {
					struct Student student;
					struct flock student_lock;
					
					set_lock(student_fd, &student_lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
					read(student_fd, &student, sizeof(student));

					if(student.courseIdx + 1 < 100) {
						student.courseIdx++;
						student.courses[student.courseIdx] = cid;
						
						lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
						write(student_fd, &student, sizeof(student));
						set_lock(student_fd, &student_lock, UNLOCK, SEEK_SET, 0, 0);
						res = 1;
					}
					else {
						res = 0;
					}
				}
				close(student_fd);
			}
		}
		else {
			res = 1;
		}
	}
	
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int unenroll_course(int cfd) {
	int res = 0;
	int sid, cid;
	recv(cfd, &sid, sizeof(sid), 0);
	recv(cfd, &cid, sizeof(cid), 0);

	int student_fd = open("./data/student", O_RDWR);
	struct Student student;
	res = search_student_id(sid, student_fd);

	if(res > 0) {
		struct flock student_lock;
		int lock_res = set_lock(student_fd, &student_lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
		read(student_fd, &student, sizeof(student));

		int enrolled_idx = -1;
		for(int i = 0; i < student.courseIdx; i++) {
			if(student.courses[i] == cid) {
				enrolled_idx = i;
				break;
			}
		}

		if(enrolled_idx != -1) {
			student.courses[enrolled_idx] = student.courses[student.courseIdx - 1];
			student.courses[student.courseIdx - 1] = -1;
			student.courseIdx--;

			lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
			write(student_fd, &student, sizeof(student));

			res = 1;

			int course_fd = open("./data/course", O_RDWR);
			struct Course course;
			res = search_course_id(cid, course_fd);
			if(res > 0) {
				struct flock course_lock;
				lock_res = set_lock(course_fd, &course_lock, W_LOCK, SEEK_CUR, 0, sizeof(course));
				read(course_fd, &course, sizeof(course));
				
				int student_idx = -1;
				for(int i = 0; i < course.studentIdx; i++) {
					if(course.students[i] == sid) {
						student_idx = i;
						break;
					}
				}
				if(student_idx != -1) {
					course.students[student_idx] = course.students[course.studentIdx - 1];
					course.students[course.studentIdx - 1] = -1;
					course.studentIdx--;
					lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
					write(course_fd, &course, sizeof(course));

					res = 1;
				}
				else {
					res = 0;
				}

				lock_res = set_lock(course_fd, &course_lock, UNLOCK, SEEK_CUR, 0, 0);
			}

			close(course_fd);		
		}
		else {
			res = 0;
		}

		lock_res = set_lock(student_fd, &student_lock, UNLOCK, SEEK_CUR, 0, 0);
		close(student_fd);
	}
	
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int main() {
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5002);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    int bind_res = bind(sfd, (struct sockaddr*)&server, sizeof(server));

	if(bind_res == -1) {
		output("Error binding\n");
		return -1;
	}
	output("Listening\n");
	int listen_res = listen(sfd, 5);

	if(listen_res != 0) {
		output("Error listening\n");
	}

	struct sockaddr_in client;
    int client_size = sizeof(client);
    int cfd = accept(sfd, (struct sockaddr*)&client, &client_size);
	if(cfd == -1) {
		output("Error accepting client\n");
	}
	output("Client connected\n");
	int run = 1;
	while(run) {
		int opcode;
		recv(cfd, &opcode, sizeof(opcode), 0);
		printf("%d\n", opcode);
		switch(opcode) {
			case 0:
				login(cfd);
				break;
			case 1:
				add_faculty(cfd);
				break;
			case 2:
				edit_faculty(cfd);
				break;
			case 3:
				status_faculty(cfd);
				break;
			case 4:
				add_student(cfd);
				break;
			case 5:
				edit_student(cfd);
				break;
			case 6:
				status_student(cfd);
				break;
			case 7:
				add_course(cfd);
				break;
			case 8:
				remove_course(cfd);
				break;
			case 9:
				view_enrollments(cfd);
				break;
			case 10:
				change_faculty_password(cfd);
				break;
			case 11:
				change_student_password(cfd);
				break;
			case 12:
				enroll_course(cfd);
				break;
			case 13:
				unenroll_course(cfd);
				break;
			case 14:
				get_student_details(cfd);
				break;
			case 15:
				get_faculty_details(cfd);
				break;
			case 16:
				get_course_details(cfd);
				break;
			default:
				break;
		}
	}
	close(cfd);
	return 0;
}
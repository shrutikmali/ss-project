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

extern int errno;

int ADMIN = 1;
int FACULTY = 2;
int STUDENT = 3;
int COURSE = 4;
int UNLOCK = F_UNLCK;
int R_LOCK = F_RDLCK;
int W_LOCK = F_WRLCK;

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
	int status;
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
	int res;
	// printf("Lock args:\n%d\n%d\n%d\n%d\n%d\n", fd, type, whence, start, len);
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
		printf("Error no: %d\n", errno);
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
		// printf("%c %c\n", *s, *t);
		if(*s != *t) {
			// printf("Not equal inside loop %c %c\n", *s, *t);
			return 0;
		}
		s++;
		t++;
	}
	if((*s != '\n' && *t != '\n') && (*s == '\0' && *t != '\0' || *s != '\0' && *t == '\0')) {
		// printf("Not equal outside loop %c %c\n", *s, *t);
		return 0;
	}
	return 1;
}

void string_copy(char src[100], char dest[100]) {
    for(int i = 0; i < 100; i++) {
        dest[i] = src[i];
    }
    return;
}

int get_id(int type) {
	// printf("Getting id\n");
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
	// printf("Getting id complete\n");
	return returnID;
}

int search(int id, int type, int fd) {
	int res = -1;
	if(type == FACULTY) {
		struct Faculty user;
		int found = 0, eof = 0;
		while(!found && !eof) {
			struct flock lock;
			int lock_res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(user));
			if(lock_res < 0) {
				perror("Error acquiring lock:search");
				return -2;
			}
			else {
				int read_res = read(fd, &user, sizeof(user));
				if(read_res < 0) {
					eof = 1;
				}
				if(user.id == id) {
					found = 1;
					res = 1;
					lseek(fd, -1 * sizeof(user), SEEK_CUR);
					lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
				}
			}
		}
	}
	else if(type == STUDENT) {
		struct Student user;
		int found = 0, eof = 0;
		while(!found && !eof) {
			struct flock lock;
			int lock_res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(user));
			if(lock_res < 0) {
				perror("Error acquiring lock:search");
				return -2;
			}
			else {
				int read_res = read(fd, &user, sizeof(user));
				if(read_res < 0) {
					eof = 1;
					return -1;
				}
				if(user.id == id) {
					found = 1;
					res = 1;
					lseek(fd, -1 * sizeof(user), SEEK_CUR);
					lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
				}
			}
		}	
	}
	else if(type == COURSE) {
		struct Course user;
		int found = 0, eof = 0;
		while(!found && !eof) {
			struct flock lock;
			int lock_res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(user));
			if(lock_res < 0) {
				perror("Error acquiring lock:search");
				return -2;
			}
			else {
				int read_res = read(fd, &user, sizeof(user));
				if(read_res < 0) {
					eof = 1;
					return -1;
				}
				if(user.id == id) {
					found = 1;
					res = 1;
					lseek(fd, -1 * sizeof(user), SEEK_CUR);
					lock_res = set_lock(fd, &lock, UNLOCK, SEEK_SET, 0, 0);
				}
			}
		}
	}
	else {
		return -1;
	}

	return res;
}

int login(int cfd) {
	int type;
	recv(cfd, &type, sizeof(type), 0);
	int logged_in = 0;
	char email[100];
	char password[100];
	if(type == ADMIN) {
		struct Admin admin;
		// Open admin file
		int fd = open("./data/admin", O_RDONLY);
		if(fd == -1) {
			perror("Error opening file");
		}
		// Set read lock
		struct flock lock;
		int lock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
		if(lock_res == -1) {
			perror("Error locking login:admin");
			int error = -2;
			send(cfd, &error, sizeof(error), 0);
			return -1;
		}
		
		
		while(!logged_in) {
			// Get credentials
			recv(cfd, &email, sizeof(email), 0);
			recv(cfd, &password, sizeof(password), 0);
			// Check if user exists
			int user_found = 0;
			lseek(fd, SEEK_SET, 0);
			while(read(fd, &admin, sizeof(admin)) > 0) {
				// printf("Credentials: %s %s\n", admin.email, admin.password);
				int email_match = string_equal(email, admin.email); 
				if(email_match) {
					user_found = 1;
					break;
				}
			}
			if(!user_found) {
				// printf("User found\n");
				int error = -1;
				send(cfd, &error, sizeof(error), 0);
			}
			else {
				// Check if password matches
				int password_match = string_equal(password, admin.password);
				// printf("Password test: %d\n", password_match);
				if(password_match) {
					send(cfd, &admin.id, sizeof(int), 0);
					logged_in = 1;
				}
				else {
					int error = 0;
					send(cfd, &error, sizeof(error), 0);
					int unlock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
					if(unlock_res == -1) {
						output("Error unlocking login:admin\n");
						return -1;
					}
				}
			}
		}
	}
	else if(type == FACULTY) {
		struct Faculty user;
		// Open faculty file
		int fd = open("./data/faculty", O_RDONLY);
		
		// Set read lock
		struct flock lock;
		int lock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
		if(lock_res == -1) {
			output("Error locking in login\n");
			int error = -2;
			send(cfd, &error, sizeof(error), 0);
			return -1;
		}
		
		
		while(!logged_in) {
			// Get credentials
			recv(cfd, &email, sizeof(email), 0);
			recv(cfd, &password, sizeof(password), 0);
			
			// Check if user exists
			int user_found = 0;
			while(!read(fd, &user, sizeof(user))) {
				if(string_equal(email, user.email)) {
					user_found = 1;
					break;
				}
			}
			if(!user_found) {
				int error = -1;
				send(cfd, &error, sizeof(error), 0);
			}
			else {
				// Check if password matches
				if(string_equal(password, user.password)) {
					send(cfd, &user.id, sizeof(int), 0);
					logged_in = 1;
				}
				else {
					int error = 0;
					send(cfd, &error, sizeof(error), 0);
					int unlock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
					if(unlock_res == -1) {
						output("Error unlocking login:faculty\n");
						return -1;
					}
				}
			}
		}
	}
	else if(type == STUDENT) {
		struct Student user;
		// Open faculty file
		int fd = open("./data/student", O_RDONLY);
		
		// Set read lock
		struct flock lock;
		int lock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
		if(lock_res == -1) {
			output("Error locking in login\n");
			int error = -2;
			send(cfd, &error, sizeof(error), 0);
			return -1;
		}
		
		
		while(!logged_in) {
			// Get credentials
			recv(cfd, &email, sizeof(email), 0);
			recv(cfd, &password, sizeof(password), 0);
			
			// Check if user exists
			int user_found = 0;
			while(!read(fd, &user, sizeof(user))) {
				if(string_equal(email, user.email)) {
					user_found = 1;
					break;
				}
			}
			if(!user_found) {
				int error = -1;
				send(cfd, &error, sizeof(error), 0);
			}
			else {
				// Check if password matches
				if(string_equal(password, user.password)) {
					send(cfd, &user.id, sizeof(int), 0);
					logged_in = 1;
				}
				else {
					int error = 0;
					send(cfd, &error, sizeof(error), 0);
					int unlock_res = set_lock(fd, &lock, F_RDLCK, SEEK_SET, 0, 0);
					if(unlock_res == -1) {
						output("Error unlocking login:student\n");
						return -1;
					}
				}
			}
		}
	}
	else {
		int error = -2;
		send(cfd, &error, sizeof(error), 0);
		return -1;
	}
	return 0;
}

int get_student_details(int cfd) {
	int uid;
	int res = -1;
	res = recv(cfd, &uid, sizeof(uid), 0);
	int fd = open("./data/student", O_RDONLY);
	res = search(uid, STUDENT, fd);

	// Not found (-1) or error
	if(res < 0) {
		send(cfd, &res, sizeof(res), 0);
	}
	else {
		struct Student student;
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(student));
		if(res < -1) {
			perror("Error locking file:get_student_details");
			res = -2;
			send(cfd, &res, sizeof(res), 0);
		}
		else {
			res = read(fd, &student, sizeof(student));
			if(res <= 0) {
				perror("Error reading record:get_student_details");
				res = -2;
				send(cfd, &res, sizeof(res), 0);
			}
			else {
				char name[100];
				char email[100];
				int status;
				int courses[100];
				string_copy(student.name, name);
				string_copy(student.email, email);
				for(int i = 0; i < student.courseIdx; i++) {
					courses[i] = student.courses[i];
				}
				status = student.status;
				res = 1;
				send(cfd, &res, sizeof(res), 0);
				send(cfd, &name, sizeof(name), 0);
				send(cfd, &email, sizeof(email), 0);
				send(cfd, &courses, sizeof(courses), 0);
				send(cfd, &status, sizeof(status), 0);
			}
		}
	}
	return res;
}

int get_faculty_details(int cfd) {
	int uid;
	int res = -1;
	res = recv(cfd, &uid, sizeof(uid), 0);
	int fd = open("./data/faculty", O_RDONLY);
	res = search(uid, FACULTY, fd);

	// Not found (-1) or error
	if(res < 0) {
		send(cfd, &res, sizeof(res), 0);
	}
	else {
		struct Faculty faculty;
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(faculty));
		if(res == -1) {
			perror("Error locking file:get_faculty_details");
			res = -2;
			send(cfd, &res, sizeof(res), 0);
		}
		else {
			res = read(fd, &faculty, sizeof(faculty));
			if(res <= 0) {
				perror("Error reading record:get_faculty_details");
			}
			else {
				char name[100];
				char email[100];
				int status;
				int courses[100];
				string_copy(faculty.name, name);
				string_copy(faculty.email, email);
				for(int i = 0; i < faculty.courseIdx; i++) {
					courses[i] = faculty.courses[i];
				}
				status = faculty.status;
				res = 1;
				send(cfd, &res, sizeof(res), 0);
				send(cfd, &name, sizeof(name), 0);
				send(cfd, &email, sizeof(email), 0);
				send(cfd, &courses, sizeof(courses), 0);
				send(cfd, &status, sizeof(status), 0);
			}
		}
	}
	return res;
}

int get_course_details(int cfd) {
	int uid;
	int res = -1;
	res = recv(cfd, &uid, sizeof(uid), 0);
	int fd = open("./data/course", O_RDONLY);
	res = search(uid, COURSE, fd);

	// Not found (-1) or error
	if(res < 0) {
		send(cfd, &res, sizeof(res), 0);
	}
	else {
		struct Course course;
		struct flock lock;
		res = set_lock(fd, &lock, R_LOCK, SEEK_CUR, 0, sizeof(course));
		if(res < -1) {
			perror("Error locking file:get_faculty_details");
			res = -1;
		}
		else {
			res = read(fd, &course, sizeof(course));
			if(res <= 0) {
				perror("Error reading record:get_faculty_details");
			}
			else {
				char name[100];
				int fid;
				int credits;
				int currStrength;
				int maxStrength;
				int status;
				string_copy(course.name, name);
				status = course.status;
				res = 1;
				send(cfd, &res, sizeof(res), 0);
				send(cfd, &name, sizeof(name), 0);
				send(cfd, &fid, sizeof(fid), 0);
				send(cfd, &credits, sizeof(credits), 0);
				send(cfd, &currStrength, sizeof(currStrength), 0);
				send(cfd, &maxStrength, sizeof(maxStrength), 0);
				send(cfd, &status, sizeof(status), 0);
				return 0;
			}
		}
	}
	return res;
}

int add_faculty(int cfd) {
	// TODO: add check to see if email already exists
	printf("CFD is: %d\n", cfd);
	printf("Adding faculty\n");
	char name[100];
	char email[100];
	char password[100];
	struct Faculty newFaculty;
	
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	// printf("Received data:\n%s\n%s\n%s\n", name, email, password);
	
	int newId = get_id(FACULTY);
	int res = -1;
	// printf("Received id: %d\n", newId);
	if(newId == -1) {
		output("Error getting id:add_faculty\n");
		res = -1;
	}
	else {
		newFaculty.id = newId;
		string_copy(name, newFaculty.name);
		string_copy(email, newFaculty.email);
		string_copy(password, newFaculty.password);
		newFaculty.courseIdx = 0;
		newFaculty.status = 1;
		
		// printf("%s\n%s\n%s\n", newFaculty.name, newFaculty.email, newFaculty.password);
		
		int fd = open("./data/faculty", O_WRONLY);
		if(fd < 0) {
			perror("Error in opening faculty");
			res = -1;
		}
		else {
			int seek_res = lseek(fd, 0, SEEK_END);
			if(seek_res < 0) {
				perror("Error in seeking faculty");
				res = -1;
			}
			else {
				struct flock lock;
				int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(newFaculty));
				if(lock_res == -1) {
					perror("Error in locking faculty:add_faculty");
					res = -1;
				}
				else {
					res = write(fd, &newFaculty, sizeof(newFaculty));
					int unlock_res = set_lock(fd, &lock, F_UNLCK, SEEK_CUR, 0, sizeof(newFaculty));
					if(unlock_res == -1) {
						perror("Error in unlocking faculty:add_faculty");
					}
					else {
						res = newId;
					}
				}
			}
		}
	}
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int edit_faculty(int cfd) {
	int id;
	char name[100];
	char email[100];
	char password[100];
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	int res = -1;
	int fd = open("./data/faculty", O_RDWR);
	if(fd < 0) {
		perror("Error opening faculty:edit_faculty");
		res = -1;
	}
	else {
		// printf("FD is: %d\n", fd);
		struct Faculty faculty;
		res = lseek(fd, sizeof(faculty) * (id - 1), SEEK_SET);
		if(res == -1) {
			perror("Error seeking:edit_faculty");
		}
		else {
			struct flock lock;
			
			// First get lock
			int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
			if(lock_res == -1) {
				// printf("Lock args:\n%d\n%d\n%d\n%d\n%lu\n", fd, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
				perror("Error getting write lock:edit_faculty");
				res = -1;
			}
			else {
				// Read into struct
				read(fd, &faculty, sizeof(faculty));
				string_copy(name, faculty.name);
				string_copy(email, faculty.email);
				string_copy(password, faculty.password);
				
				// Write struct back to file
				res = lseek(fd, sizeof(faculty) * -1, SEEK_CUR);
				if(res < 0) {
					perror("Error in seeking while WB:edit_faculty");
				}
				else {
					write(fd, &faculty, sizeof(faculty));
					res = lseek(fd, sizeof(faculty) * -1, SEEK_CUR);
					struct flock lock;
					// printf("Lock args:\n%d\n%d\n%d\n%d\n%lu\n", fd, UNLOCK, SEEK_CUR, 0, sizeof(faculty));
					lock_res = set_lock(fd, &lock, UNLOCK, SEEK_CUR, 0, sizeof(faculty));
					if(lock_res == -1) {
						perror("Error releasing write lock:edit_faculty");
						res = -1;
					}
					else {
						res = 0;
					}
				}
			}
		}
	}
	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int status_faculty(int cfd) {
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
		lseek(fd, (id - 1) * sizeof(faculty), SEEK_SET);

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
				res = faculty.id;
			}
		}
	}
	close(fd);
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int add_student(int cfd) {
	// TODO: add check to see if email already exists
	char name[100];
	char email[100];
	char password[100];
	struct Student newStudent;
	
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);
	
	int newId = get_id(STUDENT);
	int res = -1;
	if(newId == -1) {
		output("Error getting id:add_student\n");
		res = -1;
	}
	else {
		newStudent.id = newId;
		string_copy(name, newStudent.name);
		string_copy(email, newStudent.email);
		string_copy(password, newStudent.password);
		newStudent.courseIdx = 0;
		newStudent.status = 1;
		
		
		int fd = open("./data/student", O_WRONLY);
		int seek_res = lseek(fd, 0, SEEK_END);
		struct flock lock;
		int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(newStudent));
		if(lock_res == -1) {
			output("Error in locking faculty:add_student\n");
			res = -1;
		}
		else {
			res = write(fd, &newStudent, sizeof(newStudent));
			int unlock_res = set_lock(fd, &lock, F_UNLCK, SEEK_CUR, 0, sizeof(newStudent));
			if(unlock_res == -1) {
				output("Error in unlocking faculty:add_student\n");
			}
		}
	}
	send(cfd, &res, sizeof(res), 0);
	return res;
}

int edit_student(int cfd) {
	int id;
	char name[100];
	char email[100];
	char password[100];
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &email, sizeof(email), 0);
	recv(cfd, &password, sizeof(password), 0);

	int fd = open("./data/student", O_RDWR);
	struct Student student;
	lseek(fd, sizeof(student) * (id - 1), SEEK_SET);
	struct flock lock;
	
	// First get lock
	int lock_res = set_lock(fd, &lock, F_WRLCK, SEEK_CUR, 0, sizeof(student));
	int res = -1;
	if(lock_res == -1) {
		output("Error getting read lock:edit_faculty\n");
		res = -1;
		send(cfd, &res, sizeof(res), 0);
	}
	else {
		// Read into struct
		read(fd, &student, sizeof(student));
		string_copy(name, student.name);
		string_copy(email, student.email);
		string_copy(password, student.password);
		
		// Write struct back to file
		lseek(fd, sizeof(student) * -1, SEEK_CUR);
		write(fd, &student, sizeof(student));
		lseek(fd, sizeof(student) * -1, SEEK_CUR);
		lock_res = set_lock(fd, &lock, UNLOCK, SEEK_CUR, 0, sizeof(student));
		res = 0;
		send(cfd, &res, sizeof(res), 0);
	}

	return 0;
}

int status_student(int cfd) {
	int id, option;
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &option, sizeof(option), 0);
	// Open the file
	int fd = open("./data/student", O_RDWR);
	struct Student student;
	lseek(fd, (id - 1) * sizeof(student), SEEK_SET);
	// Acquire a lock
	struct flock lock;
	int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
	read(fd, &student, sizeof(student));
	student.status = option;
	lseek(fd,  -1 * (sizeof(student)), SEEK_CUR);
	write(fd, &student, sizeof(student));
	close(fd);
	int res = 1;
	send(cfd, &res, sizeof(res), 0);
	return 0;
}

int add_course(int cfd) {
	struct Course course;
	struct Faculty faculty;
	char name[100];
	int credits;
	int maxStrength;
	int fid;
	recv(cfd, &fid, sizeof(fid), 0);
	recv(cfd, &name, sizeof(name), 0);
	recv(cfd, &credits, sizeof(credits), 0);
	recv(cfd, &maxStrength, sizeof(maxStrength), 0);
	course.id = get_id(COURSE);
	course.fid = fid;
	course.maxStrength = maxStrength;
	course.credits = credits;
	string_copy(name, course.name);
	course.studentIdx = 0;

	int res = 0;
	send(cfd, &res, sizeof(res), 0);

	return 0;
}

int remove_course(int cfd) {
	int cid, fid;
	recv(cfd, &fid, sizeof(fid), 0);
	recv(cfd, &cid, sizeof(cid), 0);
	/*
		int id;
		char name[100];
		int courseStatus
		int studentIdx;
		int students[100];
		int credits;
		int maxStrength;
		int fid;

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

		Initial steps:
			Read course into struct from file
		IDs are 1-indexed
		Student steps:
			1. For each student in students array of course struct:
				2. Acquire lock
				3. Move fd to to student_idx
				4. Re-order student_courses array to remove course
				5. Update courseIdx
				6. Release lock
			
		Faculty steps:
			1. Go to faculty in file using fid of course
			2. Obtain record lock
			3. Update courses array
			4. Update coursesIdx
			5. Release lock

		Course steps:
			1. Iterate over every record
				2. Get record lock
				3. If record found:
					4. Set status to inactive
					5. Set studentIdx = 0;
					6. Clear student array
					7. Set credits = 0
					8. Set maxStrength = 0
					9. Set fid = 0;
					10. Write course back to file
				11. Release write lock
		
	*/

	struct Course course;
	int course_fd = open("./data/course", O_RDWR);
	lseek(course_fd, (cid - 1) * sizeof(course), SEEK_SET);
	struct flock courseLock;
	int lock_res = set_lock(course_fd, &courseLock, W_LOCK, SEEK_CUR, 0, sizeof(course));
	read(course_fd, &course, sizeof(course));

	// Student
	for(int i = 0; i < course.studentIdx; i++) {
		int sid = course.students[i];
		struct Student student;
		struct flock studentLock;
		int student_fd = open("./data/student", O_RDWR);
		lseek(student_fd, (sid - 1) * sizeof(student_fd), SEEK_SET);
		int lock_res = set_lock(student_fd, &studentLock, W_LOCK, SEEK_CUR, 0, sizeof(student));
		read(student_fd, &student, sizeof(student));
		int j = 0;
		while(j < student.courseIdx) {
			if(student.courses[j] == cid) {
				break;
			}
			j++;
		}
		int temp = student.courses[j];
		student.courses[j] = student.courses[student.courseIdx - 1];
		lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
		write(student_fd, &student, sizeof(student));
		lock_res = set_lock(student_fd, &studentLock, UNLOCK, SEEK_CUR, 0, sizeof(course));
	}

	// Faculty
	struct Faculty faculty;
	int faculty_fd = open("./data/faculty", O_RDWR);
	lseek(faculty_fd, (fid - 1) * sizeof(faculty), SEEK_SET);
	
	struct flock faculty_lock;
	lock_res = set_lock(faculty_fd, &faculty_lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
	read(faculty_fd, &faculty, sizeof(faculty));
	for(int i = 0; i < faculty.courseIdx; i++) {
		if(faculty.courses[i] == cid) {
			int temp = faculty.courses[i];
			faculty.courses[i] = faculty.courses[faculty.courseIdx - 1];
			break;
		}
	}
	faculty.courseIdx--;

	course.status = 0;
	course.credits = 0;
	course.maxStrength = 0;
	course.fid = 0;
	for(int i = 0; i < course.studentIdx; i++) {
		course.students[i] = 0;
	}
	course.studentIdx = 0;
	lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
	write(course_fd, &course, sizeof(course));
	lock_res = set_lock(course_fd, &courseLock, UNLOCK, SEEK_CUR, 0, 0);

	int res = 0;
	send(cfd, &res, sizeof(res), 0);

	return 0;
}

int view_enrollments(int cfd) {
	/*
		Get courseId
		Read course from file into course struct
		Iterate over students array from courseStruct
		If student is active then details will be sent	
	*/
	int course_id = -1;
	recv(cfd, &course_id, sizeof(course_id), 0);
	// Handle course_id error
	int course_fd = open("./data/course", O_RDWR);
	struct Course course;
	lseek(course_fd, (course_id - 1) * sizeof(course), SEEK_SET);
	// Handle lseek error
	struct flock course_lock;
	int lock_res = set_lock(course_fd, &course_lock, W_LOCK, SEEK_CUR, 0, sizeof(course));
	// Handle lock error
	read(course_fd, &course, sizeof(course));
	// Handle read error
	
	// Send the number of students
	send(cfd, &course.studentIdx, sizeof(course.studentIdx), 0);

	// Iterate over every student
	int student_fd = open("./data/student", O_RDONLY);
	for(int i = 0; i < course.studentIdx; i++) {
		int sid = course.students[i];
		struct Student student;
		// Move fd
		lseek(student_fd, (sid - 1) * sizeof(student), SEEK_SET);
		struct flock student_lock;
		lock_res = set_lock(student_fd, &student_lock, R_LOCK, SEEK_CUR, 0, sizeof(student));
		read(student_fd, &student, sizeof(student));

		// Send id and name
		char name[100];
		string_copy(student.name, name);
		send(cfd, &sid, sizeof(sid), 0);
		send(cfd, &name, sizeof(name), 0);
	}

	return 0;
}

int change_password(int cfd) {
	/*
		Get the type, id, and new password
		Accordingly open the file
		Move fd to required position
		Get write lock
		Read into struct
		Update password
		Write struct back to file
		Release lock
		Close file
	*/
	int type, id;
	char new_password[100];
	recv(cfd, &type, sizeof(type), 0);
	recv(cfd, &id, sizeof(id), 0);
	recv(cfd, &new_password, sizeof(new_password), 0);
	
	if(type == FACULTY) {
		int fd = open("./data/faculty", O_RDWR);
		struct Faculty faculty;
		lseek(fd, (id - 1) * sizeof(faculty), SEEK_SET);

		struct flock lock;
		int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(faculty));
		read(fd, &faculty, sizeof(faculty));
		string_copy(new_password, faculty.password);
		lseek(fd, -1 * sizeof(faculty), SEEK_CUR);
		write(fd, &faculty, sizeof(faculty));
		set_lock(fd, &lock, UNLOCK, SEEK_CUR, 0, 0);
		
		int res = 0;
		send(cfd, &res, sizeof(res), 0);
	}
	else if(type == STUDENT) {
		int fd = open("./data/student", O_RDWR);
		struct Student student;
		lseek(fd, (id - 1) * sizeof(student), SEEK_SET);

		struct flock lock;
		int lock_res = set_lock(fd, &lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
		read(fd, &student, sizeof(student));
		string_copy(new_password, student.password);
		lseek(fd, -1 * sizeof(student), SEEK_CUR);
		write(fd, &student, sizeof(student));
		set_lock(fd, &lock, UNLOCK, SEEK_CUR, 0, 0);

		int res = 0;
		send(cfd, &res, sizeof(res), 0);
	}
	else {
		int res = -1;
		send(cfd, &res, sizeof(res), 0);
	}

	return 0;
}

int enroll_course(int cfd) {
	/*
		Get student id and course id
		Get student struct
		Check if student already in course
		If not, add student to course
		Close student

		Get course struct
		Add student into students array
		Close course

	*/
	int sid, cid;
	recv(cfd, &sid, sizeof(sid), 0);
	recv(cfd, &cid, sizeof(cid), 0);

	struct Student student;
	// Open student data
	int student_fd = open("./data/student", O_RDWR);
	// Move fd to position
	lseek(student_fd, (sid - 1) * sizeof(student), SEEK_SET);

	// Lock the record
	struct flock student_lock;
	int lock_res = set_lock(student_fd, &student_lock, W_LOCK, SEEK_CUR, 0, sizeof(student));
	read(student_fd, &student, sizeof(student));
	int already_enrolled = 0;
	for(int i = 0; i < student.courseIdx; i++) {
		if(student.courses[i] == cid) {
			already_enrolled = 1;
			break;
		}
	}

	if(!already_enrolled) {
		student.courseIdx++;
		student.courses[student.courseIdx] = cid;
		lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
		write(student_fd, &student, sizeof(student));
		lock_res = set_lock(student_fd, &student_lock, UNLOCK, SEEK_CUR, 0, 0);

		// Update course
		int course_fd = open("./data/course", O_RDWR);
		struct Course course;
		lseek(course_fd, (cid - 1) * sizeof(course), SEEK_SET);
		struct flock course_lock;
		lock_res = set_lock(course_fd, &course_lock, W_LOCK, SEEK_CUR, 0, sizeof(course));
		read(course_fd, &course, sizeof(course));
		course.studentIdx++;
		course.students[course.studentIdx] = sid;
		lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
		write(course_fd, &course, sizeof(course));
		lock_res = set_lock(course_fd, &course_lock, UNLOCK, SEEK_CUR, 0, 0);
	}
	else {
		lock_res = set_lock(student_fd, &student_lock, UNLOCK, SEEK_CUR, 0, 0);
	}
	
	int res = 0;
	send(cfd, &res, sizeof(res), 0); 

	return 0;
}

int unenroll_course(int cfd) {
	/*
		Receive cid and sid
		Get student struct
		Check if student is enrolled in course
		Remove course from courses array
		Write student struct

		Get course struct
		Remove student from students array
		Write course struct
	*/
	int sid, cid;
	recv(cfd, &sid, sizeof(sid), 0);
	recv(cfd, &cid, sizeof(cid), 0);

	int student_fd = open("./data/student", O_RDWR);
	struct Student student;
	lseek(student_fd, (sid - 1) * sizeof(student), SEEK_SET);
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
	if(enrolled_idx == -1) {
		lock_res = set_lock(student_fd, &student_lock, UNLOCK, SEEK_CUR, 0, 0);
		close(student_fd);
	}
	else {
		student.courses[enrolled_idx] = student.courses[student.courseIdx - 1];
		student.courses[student.courseIdx - 1] = -1;
		student.courseIdx--;
		lseek(student_fd, -1 * sizeof(student), SEEK_CUR);
		write(student_fd, &student, sizeof(student));
		lock_res = set_lock(student_fd, &student_lock, UNLOCK, SEEK_CUR, 0, 0);
		close(student_fd);

		// Update course
		int course_fd = open("./data/course", O_RDWR);
		struct Course course;
		lseek(course_fd, (cid - 1) * sizeof(course), SEEK_SET);
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

		course.students[student_idx] = course.students[course.studentIdx - 1];
		course.students[course.studentIdx - 1] = -1;
		course.studentIdx--;
		lseek(course_fd, -1 * sizeof(course), SEEK_CUR);
		write(course_fd, &course, sizeof(course));
		lock_res = set_lock(course_fd, &course_lock, UNLOCK, SEEK_CUR, 0, 0);
	}

	int res = 0;
	send(cfd, &res, sizeof(res), 0);
	return 0;
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
				change_password(cfd);
				break;
			case 11:
				enroll_course(cfd);
				break;
			case 12:
				unenroll_course(cfd);
				break;
			case 13:
				get_student_details(cfd);
				break;
			case 14:
				get_faculty_details(cfd);
				break;
			case 15:
				get_course_details(cfd);
				break;
			default:
				break;
		}
	}
	close(cfd);
	return 0;
}
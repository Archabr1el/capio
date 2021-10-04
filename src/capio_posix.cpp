#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include<unordered_map>
#include<string>
#include<iostream>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stddef.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdarg.h>

static int (*real_open)(const char* pathname, int flags, ...) = NULL;
static ssize_t (*real_read)(int fd, void* buffer, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void* buffer, size_t count) = NULL;
static int (*real_close)(int fd) = NULL;

struct circular_buffer {
	void* buf;
	int* i;
};

std::unordered_map<int, std::pair<void*, int>> files;
circular_buffer buf_requests; 
int* buf_response;
int i_resp;
sem_t* sem_requests;
sem_t* sem_new_msgs;
sem_t* sem_response;

void err_exit(std::string error_msg) {
	std::cout << "error: " << error_msg << std::endl;
	exit(1);
}

void* get_shm(std::string shm_name) {
	void* p = nullptr;
	// if we are not creating a new object, mode is equals to 0
	int fd = shm_open(shm_name.c_str(), O_RDWR, 0); //to be closed
	struct stat sb;
	if (fd == -1)
		err_exit("shm_open");
	/* Open existing object */
	/* Use shared memory object size as length argument for mmap()
	and as number of bytes to write() */
	if (fstat(fd, &sb) == -1)
		err_exit("fstat");
	p = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED)
		err_exit("mmap");
//	if (close(fd) == -1);
//		err_exit("close");
	return p;
}


struct circular_buffer get_circular_buffer() {
	//open shm
	void* buf = get_shm("circular_buffer");
	int* i = (int*) get_shm("index_buf");
	circular_buffer br;
	br.buf = buf;
	br.i = i;
	return br;
}

sem_t* get_sem_requests() {
	return sem_open("sem_requests", 0);
}
void* create_shm(std::string shm_name) {
	void* p = nullptr;
	// if we are not creating a new object, mode is equals to 0
	int fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR,  S_IRUSR | S_IWUSR); //to be closed
	struct stat sb;
	const int size = 4096;
	if (fd == -1)
		err_exit("shm_open");
	if (ftruncate(fd, size) == -1)
		err_exit("ftruncate");
	p = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED)
		err_exit("mmap");
//	if (close(fd) == -1);
//		err_exit("close");
	return p;
}


/*
 * This function must be called only once
 *
 */

static void mtrace_init(void) {
	real_open = (int (*)(const char*, int, ...)) dlsym(RTLD_NEXT, "open");
	if (NULL == real_open) {
		fprintf(stderr, "Error in `dlsym open`: %s\n", dlerror());
		return;	
	}
	real_read = (ssize_t (*)(int, void*, size_t)) dlsym(RTLD_NEXT, "read");
	if (NULL == real_read) {
		fprintf(stderr, "Error in `dlsym read`: %s\n", dlerror());
		return;	
	}
	real_write = (ssize_t (*)(int, const void*, size_t)) dlsym(RTLD_NEXT, "write");
	if (NULL == real_write) {
		fprintf(stderr, "Error in `dlsym write`: %s\n", dlerror());
		return;	
	}
	real_close = (int (*)(int)) dlsym(RTLD_NEXT, "close");
	if (NULL == real_close) {	
		fprintf(stderr, "Error in `dlsym close`: %s\n", dlerror());
		return;
	}
	buf_requests = get_circular_buffer();
	sem_requests = get_sem_requests();
	sem_new_msgs = sem_open("sem_new_msgs", O_RDWR);
	sem_response = sem_open(("sem_response" + std::to_string(getpid())).c_str(),  O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	buf_response = (int*) create_shm("buf_response" + std::to_string(getpid()));
	i_resp = 0;

}

int add_open_request(const char* pathname) {
	int fd;
	std::cout << "open request" << std::endl;
	sem_wait(sem_requests);
	std::string str ("open " + std::to_string(getpid()) + " " + std::string(pathname));
	const char* c_str = str.c_str();
	memcpy(buf_requests.buf + *buf_requests.i, c_str, strlen(c_str) + 1);
	std::cout << "open before response" << std::endl;
	char tmp_str[1024];
	printf("c_str %s, len c_str %i\n", c_str, strlen(c_str));
	sprintf(tmp_str, "%s", (char*) buf_requests.buf + *buf_requests.i);
	printf("add open msg sent: %s\n", tmp_str);

	//for (int i = 0; i < strlen(c_str) + 1; ++i) {
	//	std::cout << ((char*)buf_requests.buf)[i];
	//}
	std::cout << "before wait sem response" << std::endl;

	*buf_requests.i = *buf_requests.i + strlen(c_str) + 1;
	std::cout << "*buf_requests.i == " << *buf_requests.i << std::endl;
	sem_post(sem_requests);
	sem_post(sem_new_msgs);

	//wait for response
		
	sem_wait(sem_response);
	std::cout << "Open after response" << std::endl;
	fd = buf_response[i_resp];
	++i_resp;
	return fd; //works only with one file
}

int add_close_request(int fd) {
	std::cout << "close request" << std::endl;
	const char* c_str = ("clos " +std::to_string(getpid()) + " "  + std::to_string(fd)).c_str();
    sem_wait(sem_requests);
	memcpy(buf_requests.buf + *buf_requests.i, c_str, strlen(c_str) + 1);
	char tmp_str[1024];
	printf("c_str %s, len c_str %i\n", c_str, strlen(c_str));
	sprintf(tmp_str, "%s", (char*) buf_requests.buf + *buf_requests.i);
	printf("add read msg sent: %s\n", tmp_str);
	*buf_requests.i = *buf_requests.i + strlen(c_str) + 1;
	std::cout << "*buf_requests.i == " << *buf_requests.i << std::endl;
	sem_post(sem_requests);
	sem_post(sem_new_msgs);
	return 0;
}

int add_read_request(int fd, size_t count) {
	std::cout << "read request" << std::endl;
	std::string str = "read " + std::to_string(getpid()) + " " + std::to_string(fd) + " " + std::to_string(count);
	const char* c_str = str.c_str();
    sem_wait(sem_requests);
	memcpy(buf_requests.buf + *buf_requests.i, c_str, strlen(c_str) + 1);
	char tmp_str[1024];
	printf("c_str %s, len c_str %i\n", c_str, strlen(c_str));
	sprintf(tmp_str, "%s", (char*) buf_requests.buf + *buf_requests.i);
	printf("add read msg sent: %s\n", tmp_str);
	*buf_requests.i = *buf_requests.i + strlen(c_str) + 1;
	std::cout << "*buf_requests.i == " << *buf_requests.i << std::endl;
	sem_post(sem_requests);
	sem_post(sem_new_msgs);
	//read response (offest)
	std::cout << "read before wait sem response" << std::endl;
	sem_wait(sem_response);
	std::cout << "read after response" << std::endl;
	int offset = buf_response[i_resp];
	++i_resp;
	return offset;
}

void add_write_request(int fd, size_t count) {
	std::cout << "write request" << std::endl;
	std::string str = "writ " + std::to_string(getpid()) +  " " + std::to_string(fd) + " " + std::to_string(count);
	const char* c_str = str.c_str();    
	sem_wait(sem_requests);
	memcpy(buf_requests.buf + *buf_requests.i, c_str, strlen(c_str) + 1);
	std::cout << "write before response" << std::endl;
	std::cout << "i: " << *buf_requests.i << std::endl;
	char tmp_str[1024];
	printf("c_str %s, len c_str %i\n", c_str, strlen(c_str));
	sprintf(tmp_str, "%s", (char*) buf_requests.buf + *buf_requests.i);
	printf("add write msg sent: %s\n", tmp_str);
	*buf_requests.i = *buf_requests.i + strlen(c_str) + 1;
	std::cout << "*buf_requests.i == " << *buf_requests.i << std::endl;
	sem_post(sem_requests);
	sem_post(sem_new_msgs);
	

	

	//read response (offest)
	return;
}

void read_shm(void* shm, int offset, void* buffer, size_t count) {
	memcpy(buffer, shm + offset, count); 
}

void write_shm(void* shm, int offset, const void* buffer, size_t count) {	
	memcpy(shm + offset, buffer, count); 
}
		

extern "C" {

int open(const char *pathname, int flags, ...) {
	printf("Opening of the file %s captured\n", pathname);
	if (real_open == NULL)
		mtrace_init();
	const char* prefix = "file_";
	if (strncmp("file_", pathname, strlen(prefix)) == 0) {
		printf("calling my open...\n");
		//create shm
		int fd = add_open_request(pathname);
		files[fd] = std::pair<void*, int>(get_shm(pathname), 0);
		std::cout << "result of add_open_request" << std::endl;
		return fd;
	}
	else {
		printf("calling real open...\n");
		mode_t mode = 0;
		if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
			va_list ap;
			va_start(ap, flags);
			mode = va_arg(ap, mode_t);
			va_end(ap);
		}
		int fd = real_open(pathname, flags, mode);
		printf("fd %i \n", fd);
		return fd;
	}
}



int close(int fd) {
	if (real_close == NULL)
		mtrace_init();
	printf("Closing of the file %i captured\n", fd);
	if (fd <= -1) {
		printf("calling my close...\n");
		//only the CAPIO deamon will free the shared memory in the real implementation
		return add_close_request(fd);
	}
	else {
		printf("calling real close...\n");
		int res = real_close(fd);
		printf("result of real close: %i\n", res);
		return res;
	}
}

ssize_t read(int fd, void *buffer, size_t count) {
	printf("reading of the file %i captured\n", fd);
	if (fd <= -1) {
		printf("calling my read...\n");
		int offset = add_read_request(fd, count);
		read_shm(files[fd].first, offset, buffer, count);
		files[fd].second = offset;
		return count;
	}
	else { 
		printf("calling real read...\n");
		return real_read(fd, buffer, count);
	}
}

ssize_t write(int fd, const  void *buffer, size_t count) {
	printf("writing of the file %i captured\n", fd);
	if (fd <= -1) {
		add_write_request(fd, count);
		write_shm(files[fd].first, files[fd].second, buffer, count);
		files[fd].second += count;
		return count;
	}
	else {
		printf("calling real write\n");
		return real_write(fd, buffer, count);
	}
}


}
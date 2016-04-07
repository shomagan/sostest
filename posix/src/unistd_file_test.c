/* Copyright 2011; All Rights Reserved
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define NUM_READ_WRITE_TESTS 50

#define READ_WRITE_TEST 1
#define UTILS_TEST 1

#define EXEC_PATH "/app/flash/posix"

static int read_write_test(void); //read and writing data to files
static int utils_test(void); //link, unlink, remove, rename, etc

//Read Write Sub Tests
static int stdc_read_write_test(void);
static int posix_read_write_test(void);

static int access_test(void);

int unistd_file_test(void){

	if( access_test() < 0 ){
		return -1;
	}

	if ( READ_WRITE_TEST ){
		if ( read_write_test() ){
			return -1;
		}
	}

	if ( UTILS_TEST ){
		if ( utils_test() ){
			return -1;
		}
	}

	return 0;
}

int read_write_test(void){
	if ( stdc_read_write_test() ){
		return -1;
	}

	if ( posix_read_write_test() ){
		return -1;
	}

	return 0;
}

int posix_read_write_test(void){
	int i;
	int j;
	int fd;
	char buffer[64];
	int len;
	printf("Test file read/write...");
	fflush(stdout);
	for(i=0; i < NUM_READ_WRITE_TESTS; i++){
		errno = 0;
		fd = open("/home/test.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //Create a new file test
		if ( fd < 0 ){
			printf("Failed to Create /home/test.txt on %d try (%d)\n", i, errno);
			return -1;
		}

		sprintf(buffer, "This is a write Test\n");
		len = strlen(buffer);
		for(j=0; j < 50; j++){

			if ( write(fd, buffer, len) < 0 ){
				printf("Error with fprintf (%d)\n", errno);
			}
		}
		close(fd);

		fd = open("/home/test.txt", O_RDONLY);
		if ( fd < 0 ){
			printf("Failed to open /home/test.txt for reading on %d try (%d)\n", i, errno);
			return -1;
		}

		for(j=0; j < 50; j++){
			if ( read(fd, buffer, len) != len ){
				printf("failed to read all %d bytes\n", len);
				close(fd);
				return -1;
			}
			if ( strcmp("This is a write Test\n", buffer) ){
				printf("write/read test failed on %d try (%s)", i, buffer);
				close(fd);
				return -1;
			}
		}

		close(fd);

		if ( (i % 10) == 0 ){
			printf(".");
			fflush(stdout);
		}
	}
	printf("passed\n");

	return 0;
}


int stdc_read_write_test(void){
	int i;
	int j;
	FILE * f;
	char buffer[64];
	printf("Test file read/write...");
	fflush(stdout);
	for(i=0; i < NUM_READ_WRITE_TESTS; i++){
		f = fopen("/home/test.txt", "w"); //Create a new file test
		if ( f == NULL ){
			printf("Failed to Create /home/test.txt on  %d try (%d)\n", i, errno);
			return -1;
		}

		for(j=0; j < 50; j++){
			if ( fprintf(f, "This is a write Test\n") < 0 ){
				printf("Error with fprintf (%d)\n", errno);
			}
		}
		fclose(f);

		f = fopen("/home/test.txt", "r");
		if ( f == NULL ){
			printf("Failed to open /home/test.txt for reading on %d try (%d)\n", i, errno);
			return -1;
		}

		for(j=0; j < 50; j++){
			fgets(buffer, 64, f);
			if ( strcmp("This is a write Test\n", buffer) ){
				printf("fprintf/fgets test failed on %d try (%s)", i, buffer);
				fclose(f);
				return -1;
			}
		}

		fclose(f);

		if ( (i % 10) == 0 ){
			printf(".");
			fflush(stdout);
		}
	}
	printf("passed\n");

	return 0;
}

int utils_test(void){
	FILE * f;
	struct stat st;
	int fd;
	int tmp;
	bool fail;
	char buffer[64];

	//Test link, unlink, remove, rename, stat, fcntl etc
	f = fopen("/home/test.txt", "w");
	if ( f == NULL ){
		perror("failed to create /home/test.txt");
		return -1;
	}
	sprintf(buffer, "This is a write Test\n");
	fprintf(f, buffer);
	fclose(f);

	//Test stat
	printf("Test stat()...");
	if ( stat("/home/test.txt", &st) < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}

	if ( (st.st_mode & S_IFMT) != S_IFREG ){
		printf("/home/test.txt is not a regular file (0%o)\n", st.st_mode);
		return -1;
	}

	if ( st.st_size != strlen(buffer) ){
		printf("/home/test.txt is not %d bytes (%d)\n", (int)strlen(buffer), (int)st.st_size);
		return -1;
	}
	printf("passed\n");


	errno = 0;
	printf("Test open()...ENAMETOOLONG...");
	fd = open("/home/1234567890123456789012345678901234567890", O_RDONLY);
	if ( errno == ENAMETOOLONG ){
		printf("passed\n");
		errno = 0;
	} else {
		printf("failed (errno %d != %d)\n", errno, ENAMETOOLONG);
		return -1;
	}

	errno = 0;
	printf("Test open()...ENAMETOOLONG...");
	fd = open("/1234567890123456789012345678901234567890", O_RDONLY);
	if ( errno == ENAMETOOLONG ){
		printf("passed\n");
		errno = 0;
	} else {
		printf("failed (errno %d != %d)\n", errno, ENAMETOOLONG);
		return -1;
	}

	errno = 0;
	printf("Test open()...ENOENT...");
	fd = open("/home/nofile", O_RDONLY);
	if ( errno == ENOENT ){
		printf("passed\n");
		errno = 0;
	} else {
		printf("failed (errno %d != %d)\n", errno, ENOENT);
		return -1;
	}

	errno = 0;
	printf("Test open()...ENOTSUP...");
	fd = open("/nodir/nofile", O_RDONLY);
	if ( errno == ENOTSUP ){
		printf("passed\n");
		errno = 0;
	} else {
		printf("failed (errno %d != %d)\n", errno, ENOTSUP);
		return -1;
	}

	printf("Test open()...");
	fd = open("/home/test.txt", O_RDONLY);
	if ( fd < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}
	printf("passed\n");

	//Test fstat
	printf("Test fstat()...");
	if ( fstat(fd, &st) < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}

	if ( (st.st_mode & S_IFMT) != S_IFREG ){
		printf("/home/test.txt is not a regular file (0%o)\n", st.st_mode);
		return -1;
	}

	if ( st.st_size != 21 ){
		printf("/home/test.txt is not 21 bytes (%d)\n", (int)st.st_size);
		return -1;
	}
	printf("passed\n");


	//Test fcntl
	fail = false;
	printf("test fcntl()...F_GETFL...");

	tmp = fcntl(fd, F_GETFL);
	if ( tmp != O_RDONLY ){
		printf("F_GETFL access is not O_RDONLY (0x%X)\n", tmp);
		fail = true;
	}

	if ( !fail ){
		printf("passed\n");
	}

	if ( close(fd) < 0 ){
		printf("Failed to close /home/test.txt (%d)\n", errno);
		return -1;
	}


	if ( fail ){
		return -1;
	}

	printf("Test rename()...");
	if ( rename("/home/test.txt", "/home/test0.txt") < 0 ){
		if ( errno == ENOTSUP ){
			printf("Not Supported...");
		} else {
			fflush(stdout);
			perror("failed");
			return -1;
		}
	}
	printf("passed\n");

	if( errno != ENOTSUP ){
		printf("Test rename() ENOENT...");
		if ( rename("/home/test.txt", "/home/test0.txt") == 0 ){
			printf("rename /home/test.txt should have failed\n");
			return -1;
		} else {
			if ( errno != ENOENT ){
				printf("rename /home/test.txt did not give ENOENT (%d)\n", errno);
				return -1;
			}
		}
		printf("passed\n");
	}

	printf("Test remove() ENOENT...");
	if ( remove("/home/test5.txt") == 0 ){
		printf("remove /home/test5.txt should have failed\n");
		return -1;
	} else {
		if ( errno != ENOENT ){
			printf("remove /home/test5.txt should produced ENOENT (%d)\n", errno);
			return -1;
		}
	}
	printf("passed\n");

	printf("Test remove()...");
	if ( remove("/home/test.txt") < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}
	printf("passed\n");

	return 0;
}

int access_test(void){
	//check the access to the executable

	printf("Test access F_OK (exists)...");
	if( access(EXEC_PATH, F_OK) < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}
	printf("passed\n");

	printf("Test access F_OK (doesn't exist)...");
	if( access("/nomount/some/file", F_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != ENOENT ){
		printf("failed (errno should be ENOENT not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access ENAMETOOLONG (name)...");
	if( access("/0123456789012345678901234567890123456789012345678901234567890123456789", F_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != ENAMETOOLONG ){
		printf("failed (errno should be ENAMETOOLONG not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access ENAMETOOLONG (path)...");
	if( access("/0123456789/01234567890/123456789/0123/4567890/123456789012345/67890123/456/789", F_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != ENAMETOOLONG ){
		printf("failed (errno should be ENAMETOOLONG not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access W_OK...");
	if( access(EXEC_PATH, W_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != EACCES ){
		printf("failed (errno should be EACCES not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access W_OK (no write)...");
	if( access("/", W_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != EACCES ){
		printf("failed (errno should be EACCES not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access R_OK...");
	if( access(EXEC_PATH, R_OK) < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}
	printf("passed\n");

	printf("Test access R_OK (no read)...");
	if( access("/app/.install", R_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != EACCES ){
		printf("failed (errno should be EACCES not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	printf("Test access X_OK...");
	if( access(EXEC_PATH, X_OK) < 0 ){
		fflush(stdout);
		perror("failed");
		return -1;
	}
	printf("passed\n");

	printf("Test access X_OK (no exec)...");
	if( access("/app/.install", X_OK) == 0 ){
		printf("failed (should have indicated an error)\n");
		return -1;
	}
	if( errno != EACCES ){
		printf("failed (errno should be EACCES not %d)\n", errno);
		return -1;
	}
	printf("passed\n");

	return 0;
}



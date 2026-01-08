#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "parse.h"
#include "common.h"

int validate_db_header(int fd, struct dbheader_t **headerOut) {
	if (fd < 0) {
		printf("Passed wrong value for fd in validate_db_header\n");
		return STATUS_ERROR;
	}

	lseek(fd, 0, SEEK_SET);

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Calloc failed to create new database header\n");
		return STATUS_ERROR;
	}

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->magic = ntohl(header->magic);
	header->version = ntohs(header->version);
	header->filesize = ntohl(header->filesize);
	header->count = ntohs(header->count);

	if (header->magic != HEADER_MAGIC) {
		printf("Improper header magic\n");
		free(header);
		return STATUS_ERROR;
	}
	if (header->version != 1) {
		printf("Improper header version\n");
		free(header);
		return STATUS_ERROR;
	}
	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if (header->filesize != dbstat.st_size) {
		printf("Corrupted database file\n");
		free(header);
		return STATUS_ERROR;
	}

	*headerOut = header;
	return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Calloc failed to create new database header\n");
		return STATUS_ERROR;
	}


	header->magic = HEADER_MAGIC;
	header->count = 0;
	header->filesize = sizeof(struct dbheader_t);
	header->version = 0x1;

	*headerOut = header;

	return STATUS_SUCCESS;

}


void output_file(int fd, struct dbheader_t *dbhdr) {

	int bytes_written = 0;

	if (fd < 0) {
		printf("Passed wrong file descriptor to output_file\n");
		return;
	}

	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(dbhdr->filesize);
	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htons(dbhdr->count);

	lseek(fd, 0, SEEK_SET);

	bytes_written = write(fd, dbhdr, sizeof(struct dbheader_t));
	if (bytes_written == -1){
		perror("write");
		return;
	}
	

	return;

}

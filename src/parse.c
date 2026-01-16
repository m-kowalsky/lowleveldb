#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "parse.h"
#include "common.h"

int find_employee_by_name(struct dbheader_t *dbhdr, struct employee_t *employees, char *employee_name) {

	for (int i = 0; i < dbhdr->count; i++) {
		if (strncmp(employee_name, employees[i].name, sizeof(employees[i].name)) == 0) {
			return i;
		}
	}
	printf("Could not find emloyee %s in database.  Please list employees with -l argument to find exact name and try command again.\n", employee_name);
	return STATUS_ERROR;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
	if (NULL == dbhdr) return STATUS_ERROR;
	if (NULL == *employees) return STATUS_ERROR;
	if (NULL == employees) return STATUS_ERROR;
	if (NULL == addstring) return STATUS_ERROR;

	char *name = strtok(addstring, ",");
	if (name == NULL) return STATUS_ERROR;

	char *address = strtok(NULL, ",");
	if (address == NULL) return STATUS_ERROR;
	
	char *hours = strtok(NULL, ",");
	if (hours == NULL) return STATUS_ERROR;

	struct employee_t *e = *employees;
	e = realloc(e, sizeof(struct employee_t)*(dbhdr->count+1));
	if (e == NULL){
		return STATUS_ERROR;
	}

	dbhdr->count++;

	strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name)-1);
	// e[dbhdr->count-1].name[sizeof(e[dbhdr->count].name)-1] = '\0';
	strncpy(e[dbhdr->count-1].address, address, sizeof(e[dbhdr->count-1].address));
	// e[dbhdr->count-1].address[sizeof(e[dbhdr->count].address)-1] = '\0';

	e[dbhdr->count-1].hours = atoi(hours);

	*employees = e;

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	if (fd < 0) {
		printf("Passed wrong value for fd in validate_db_header\n");
		return STATUS_ERROR;
	}

	int count = dbhdr->count;

	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if (employees == NULL) {
		printf("Malloc failed");
		return STATUS_ERROR;
	}

	read(fd, employees, count*sizeof(struct employee_t));

	for (int i = 0; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;

	return STATUS_SUCCESS;
}

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


void output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if (fd < 0) {
		printf("Passed wrong file descriptor to output_file\n");
		return;
	}
	struct dbheader_t hdr_cpy = *dbhdr;

	int realcount = dbhdr->count;

	hdr_cpy.magic = htonl(hdr_cpy.magic);
	hdr_cpy.filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);
	hdr_cpy.version = htons(hdr_cpy.version);
	hdr_cpy.count = htons(hdr_cpy.count);

	lseek(fd, 0, SEEK_SET);

	write(fd, &hdr_cpy, sizeof(hdr_cpy));

	for (int i = 0; i < realcount; i++) {
		struct employee_t tmp = employees[i];
		tmp.hours = htonl(tmp.hours);
		write(fd, &tmp, sizeof(tmp));
	}
	off_t new_size = sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount);
	ftruncate(fd, new_size);
	return;

}

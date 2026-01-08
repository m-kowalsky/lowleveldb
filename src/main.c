#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "common.h"
#include "file.h"
#include "parse.h"

int create_db_file(char *filepath);
int open_db_file(char *filename);
int validate_db_header(int fd, struct dbheader_t **headerOut);
int create_db_header(struct dbheader_t **headerOut);

void print_usage(char* argv[]) {
	printf("Usage: %s -n -f <filepath>\n", argv[0]);
	printf("-n: newfile");
	printf("\t-f: filepath\n");

	return;
}


int main (int argc, char *argv[])
{
	bool newfile = false;
	char *filepath = NULL;
	int c;
	int dbfd = -1;
	struct dbheader_t *dbhdr = NULL;


	while ((c = getopt(argc, argv, "nf:")) != -1) {
		switch (c) {
			case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
			case '?':
				printf("Usage: %s -n -f <filepath>\n", argv[0]);
			default:
				return -1;
		}
	}
	if (filepath == NULL) {
		printf("Filepath is a required argument\n");
		print_usage(argv);

		return 0;
	}

	if (newfile) {
		dbfd = create_db_file(filepath);
		if (dbfd == STATUS_ERROR) {
			printf("Unable to create database file: %s\n", filepath);
			return -1;
		}
		if (create_db_header(&dbhdr) != STATUS_SUCCESS) {
			printf("Failed to create new db header\n");
			return STATUS_ERROR;
		}
	} else {
		dbfd = open_db_file(filepath);
		if (dbfd == STATUS_ERROR) {
			printf("Unable to open database file: %s\n", filepath);
			return STATUS_ERROR;
		}
		if (validate_db_header(dbfd, &dbhdr) != STATUS_SUCCESS) {
			printf("Unable to validate header\n");
			return STATUS_ERROR;
		}
	}

	output_file(dbfd, dbhdr);

	return 0;
}

#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

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
	bool list_employees = false;

	char *filepath = NULL;
	char *addstring = NULL;
	char *employee_to_update = NULL;
	char *emp_name = NULL;
	char *employee_to_remove = NULL;
	
	int c;
	int dbfd = -1;
	int emp_index = 0;
	int updated_hours = 0;

	struct dbheader_t *dbhdr = NULL;
	struct employee_t *employees = NULL;



	while ((c = getopt(argc, argv, "na:f:lu:r:")) != -1) {
		switch (c) {
			case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
			case 'a':
				addstring = optarg;
				break;
			case 'l':
				list_employees = true;
				break;
			// remove emp by name - find by name and delete employee -r
			case 'r':
				employee_to_remove = optarg;
				break;
			// update hours - find by name and update hours -u
			case 'u':
				employee_to_update = optarg;
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

	if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
		printf("Failed to read employees");
		return 0;
	}

	if (addstring) {
		add_employee(dbhdr, &employees, addstring);
		printf("added employee %s\n", employees[dbhdr->count-1].name);
	}

	if (employee_to_update) {
		char buf[128];
		strncpy(buf, employee_to_update, sizeof(buf)-1);
		buf[sizeof(buf)-1] = '\0';

		emp_name = strtok(buf, ",");
		if (!emp_name) return STATUS_ERROR;

		char *hours_str = strtok(NULL, ",");
		updated_hours = atoi(hours_str);
		if (updated_hours <= 0) return STATUS_ERROR;

		emp_index = find_employee_by_name(dbhdr, employees, emp_name);
		if (emp_index < 0) {
			printf("Failed to get employee index while updating employee.");
			return STATUS_ERROR;
		}

		employees[emp_index].hours = updated_hours;

		printf("Employee %s's hours updated successfully\n", emp_name);
		// printf("Employee Id: %d\n", emp_index);
		// printf("\tName: %s\n", employees[emp_index].name);
		// printf("\tAddress: %s\n", employees[emp_index].address);
		// printf("\tHours: %d\n", employees[emp_index].hours);
	}

	if (list_employees) {
		for (int i = 0; i < dbhdr->count; i++){
			printf("Employee Id: %d\n", i);
			printf("\tName: %s\n", employees[i].name);
			printf("\tAddress: %s\n", employees[i].address);
			printf("\tHours: %d\n", employees[i].hours);
		}
	}

	if (employee_to_remove) {
		emp_index = find_employee_by_name(dbhdr, employees, employee_to_remove);
		if (emp_index < 0) {
			printf("Failed to get employee index while updating employee.");
			return STATUS_ERROR;
		}

		for (int i = emp_index; i < dbhdr->count; i++) {
			employees[i] = employees[i+1];
		}

		dbhdr->count--;

		output_file(dbfd, dbhdr, employees);
		printf("Employee %s removed successfully\n", employee_to_remove);
		return STATUS_SUCCESS;
	}

	output_file(dbfd, dbhdr, employees);

	return 0;
}

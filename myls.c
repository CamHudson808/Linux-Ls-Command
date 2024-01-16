#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

int ls_command(bool show_all, bool show_long_listing, char *files[], int files_length);
int directories_handler(bool show_all, bool show_long_listing, char *file);
int files_handler(bool show_all, bool show_long_listing, char *file);
int l_command_dir(char *file, bool show_all, DIR *directory);
int l_command_file(char *file, bool show_all);
int l_command(char* file);
char current_dir[256];
char* dir_ptr;

int main(int argc, char *argv[]) {

	//String to hold name of the current directory, which shouldn't be longer than 256 characters 
	
	if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
		perror("getcwd() error");
	}

	dir_ptr = current_dir;

	/*
	 * Collect All the non-option arguments (files and directories) and put them in an array
	 * for each of them, check if a is true then check if l is true. Then go to the ls function and pass those booleans to it, among other things
	 * In ls, if have a directory case and a file case
	 * for the directory case, for each element of the directory, add it to a list of dir_element filenames which will be of type char*
	 * 	- and in	crement a counter
	 * Then for loop through the list of dir_element filenames and do the stat stuff on it or whatever
	 */

	//For loop to get all of the directories listed in the arguments
	
	char *files[argc];
	int files_length = 0;
	for (int i = 1; i < argc; i++) {

		if (argv[i][0] != '-') {
			files[files_length] = argv[i];
			files_length++;
		}
	}

	int opt;
	bool show_all = false;
	bool show_long_listing = false;

	//Condition if myls is ran with no file arguments but has option arugments, puts currentdir inside Directories[]

	if ((files_length == 0)) {
		files[0] = current_dir;
		files_length += 1;
	}

	while ((opt = getopt(argc, argv, "al")) != -1) {

		switch (opt) {
			case 'a':
				show_all = true;
				break;
			case 'l':
				show_long_listing = true;
				break;			
		}

	}
	ls_command(show_all, show_long_listing, files, files_length);
	return 0;
}

int ls_command(bool show_all, bool show_long_listing, char *files[], int files_length) {

	for (int i = 0; i < files_length; i++) {
		struct stat checker;
		stat(files[i], &checker);

		if (!S_ISREG(checker.st_mode)) { //directory case
			if(files_length > 1) {
				printf("%s:\n", files[i]);
			}
			directories_handler(show_all, show_long_listing, files[i]);
			chdir(dir_ptr);
		}
		else {
			files_handler(show_all, show_long_listing, files[i]);
			chdir(dir_ptr);
		}

	}
	return 0;
}

int directories_handler(bool show_all, bool show_long_listing, char *file) {

	DIR *directory;
	struct dirent *dir_element;

	if ((directory = opendir(file)) == NULL) {
		perror("Could not open directory");
	}
	else {
		//printf("The directory after chdir: %s\n ", file);
		if (show_long_listing == false) {
			if (show_all == true) { //-a command
				while ((dir_element = readdir(directory)) != NULL) {
					printf("%s	", (dir_element->d_name));
				}
			}
			else { //No option arguments
				while ((dir_element = readdir(directory)) != NULL) {
					if ((dir_element->d_name)[0] != '.') {
						printf("%s	", (dir_element->d_name));
					}
				}
			}
		}
		else {
			l_command_dir(file, show_all, directory);
		}
	}

	printf("\n");
	return 0;
}

int files_handler(bool show_all, bool show_long_listing, char *file) {
			
	if ((show_long_listing == true)) {
		l_command_file(file, show_all);
	}
	else {
		printf("%s	\n", file);
	}

	printf("\n");
	return 0;
}

int l_command(char* file) {

	struct passwd *username_access;
	struct passwd *group_access;
	struct tm *stattime;
	char username_name[80];
	char group_name[80];
	char timebuf[80];
	struct stat buffer[sizeof(struct stat)];
	
	int stat_value = stat(file, buffer); 
	if (stat_value == -1) {
		perror("stat");
		return 0;
	}	

	username_access = getpwuid(buffer->st_uid);
	if (username_access != NULL) {
		sprintf(username_name, "%s", username_access->pw_name); 
	}
	else {
		sprintf(username_name, "%d", buffer->st_uid);
	}

	group_access = getpwuid(buffer->st_gid);
	if (group_access !=  NULL) {
		sprintf(group_name, "%s",  group_access->pw_name);
	}
	else {
		sprintf(group_name, "%d", buffer->st_gid);

	}

	stattime = localtime(&buffer->st_ctime);
	strftime(timebuf, 80, "%b %e %R", stattime);

	//Lists the directory permissions
	printf((S_ISDIR(buffer->st_mode)) ? "d" : "-");
	printf((S_IRUSR & (buffer->st_mode)) ? "r" : "-");
	printf((S_IWUSR & (buffer->st_mode)) ? "w" : "-");
	printf((S_IXUSR & (buffer->st_mode)) ? "x" : "-");
	printf((S_IRGRP & (buffer->st_mode)) ? "r" : "-");
	printf((S_IWGRP & (buffer->st_mode)) ? "w" : "-");
	printf((S_IXGRP & (buffer->st_mode)) ? "x" : "-");
	printf((S_IROTH & (buffer->st_mode)) ? "r" : "-");
	printf((S_IWOTH & (buffer->st_mode)) ? "w" : "-");
	printf((S_IXOTH & (buffer->st_mode)) ? "x" : "-");
	printf("	%lu	%s	%s	%jd	%s	%s\n", 
	buffer->st_nlink,
	username_name, group_name, 
	buffer->st_size, timebuf, file);
	return 0;
}

int l_command_dir(char *file, bool show_all, DIR *directory) {

	struct dirent *dir_element;
	chdir(file);

	if (show_all == true) {
		while ((dir_element = readdir(directory)) != NULL) {
			char* filename = dir_element -> d_name;
			l_command(filename);
		}
	}
	else {
		while((dir_element = readdir(directory)) != NULL) {
			char* filename = dir_element -> d_name;
			if(filename[0] != '.') {
				l_command(dir_element -> d_name);
				}
			}
		}
	return 0;

}

int l_command_file(char *file, bool show_all) {

	if (show_all == true) {
		l_command(file);
	}
	else {
		if(file[0] != '.') {
			l_command(file);
		}
	}
	return 0;
}

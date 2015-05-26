#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "imgur_config.h"
#include "idname.h"
#include "dbg.h"

char *mylist;
unsigned int **myint;
unsigned int mylist_malloc_size;

int init_idname_mylist(void)
{
	mylist_malloc_size = sizeof(unsigned int);
	mylist = (char*) malloc(mylist_malloc_size);
	// os primeiros sizeof(int) bytes vai ter o número total de bytes
	myint = (unsigned int**) &mylist;
	**myint = sizeof(unsigned int);
	//printf("**myint %u\n", **myint);
	return 1;
}

int
kill_idname_mylist(void)
{
	free(mylist);
	mylist = NULL;
	mylist_malloc_size = 0;

	return 1;
}

int
add_idname(char *id, char *name)
{
	unsigned int **mylocalint;
	char *temppointer;
	throw_error(NULL == name, ALBUM_NAME_ISNULL, "Album name is null");
	while(2 * sizeof(int) + strlen(id) + strlen(name) + 2 > mylist_malloc_size - **myint) {
		// must increase malloc
		mylist_malloc_size *= 2;
		temppointer = malloc(mylist_malloc_size);
		memcpy(temppointer, mylist, **myint);
		free(mylist);
		mylist = temppointer;
	}

	mylocalint = (unsigned int**) &temppointer;

	temppointer = mylist + **myint;
	//printf("inicio bloco novo %u\n", (unsigned int) (temppointer - mylist));
//	*temppointer = strlen(id);
	**mylocalint = (unsigned int) strlen(id) + 1;
	//printf("tamanho primeira palavra %s %u\n", id, **mylocalint);
	temppointer += sizeof(unsigned int);
	strcpy(temppointer, id);
	temppointer += strlen(id);
	*temppointer = '\0';
	temppointer++;

	**mylocalint = (unsigned int) strlen(name) + 1;
//	*temppointer = strlen(name);
	// printf("tamanho segunda palavra %s %u\n", name, **mylocalint);
	temppointer += sizeof(unsigned int);
	strcpy(temppointer, name);
	temppointer += strlen(name);
	*temppointer = '\0';
	temppointer++;

	// *mylist = (int) (temppointer - mylist + 1);
	//AQUI
	**myint = (unsigned int) (temppointer - mylist);
	// printf("tamnho total ate agora %u\n", **myint);
	return 1;
error:
	return 0;
}

int
print_albuns()
{
	char *pointer = mylist + sizeof(unsigned int);
	char *theend = mylist + **myint - 1;
	// unsigned int counter;
	// unsigned int **intpointer;
	// intpointer = (unsigned int **) &pointer;

	while(pointer < theend) {
		// counter = (unsigned int) **intpointer;
		pointer += sizeof(unsigned int);
		// printf("id(%u) =>%s<=", counter, pointer);
		while('\0' != *(pointer++));

		// counter = (unsigned int) **intpointer;
		pointer += sizeof(unsigned int);
		// printf(", titulo(%u) =>%s<=\n", counter, pointer);
		while('\0' != *(pointer++));
	}

	return 1;
}

int
search_by_name(char *name, char **id)
{
	char *pointer = mylist + sizeof(unsigned int);
	char *theend = mylist + **myint - 1;
	unsigned int counter;
	unsigned int **intpointer;
	intpointer = (unsigned int **) &pointer;

	if(NULL == name) {
		log_warn("cannot search for a null name");
		*id = NULL;
		return 1;
	}
	// printf("search_by_name TOTAL: %u\n", **myint);
	while(pointer < theend) {
		counter = (unsigned int) **intpointer;
		pointer += sizeof(unsigned int);
		*id = pointer;
		// printf("this is the id to return (%u) =>%s<=", counter, *id);
		pointer += counter;
		counter = (unsigned int) **intpointer;
		pointer += sizeof(unsigned int);
		// printf(", comparing %s to %s with %u bytes\n", name, pointer, counter);
		if(0 == strncmp(name, pointer, counter)) {
			return 1;
		} else {
			pointer += counter;
		}
	}
	log_warn("name \"%s\" not found", name);
	*id = NULL;
	return 1;
}

int
search_by_id(char *id, char **name)
{
	char *pointer = mylist + sizeof(unsigned int);
	char *theend = mylist + **myint - 1;
	unsigned int counter;
	unsigned int **intpointer;
	intpointer = (unsigned int **) &pointer;

	// printf("TOTAL: %u\n", **myint);
	while(pointer < theend) {
		counter = (unsigned int) **intpointer;
		pointer += sizeof(unsigned int);
		if(0 == strcmp(id, pointer)) {
			pointer += counter + sizeof(unsigned int);
			*name = pointer;
			return 1;
		} else {
			pointer += counter;
			counter = (unsigned int) **intpointer;
			pointer += counter + sizeof(unsigned int);
		}
	}
	log_warn("id \"%s\" not found", id);
	*name = NULL;
	return 1;
}

int
save_idname()
{
	FILE *idname_fp;
	idname_fp = fopen(ALBUM_DATA_FILE, "w");
	throw_error(NULL == idname_fp, CANT_OPEN_FILE, "Failed to open " ALBUM_DATA_FILE);

	// printf("mysize: %u\n", **myint);
	fwrite(mylist, sizeof(char), **myint, idname_fp);
	fclose(idname_fp);
	return 1;
error:
	return 0;

}

int
load_idname()
{
	unsigned int mysize;

	FILE *idname_fp;
	idname_fp = fopen(ALBUM_DATA_FILE, "r");

	throw_error(NULL == idname_fp, CANT_OPEN_FILE, "Failed to open " ALBUM_DATA_FILE);
	// get block size
	throw_error(0 == fread(&mysize, sizeof(unsigned int), 1, idname_fp), CANT_READ_FILE, "Failed reading " ALBUM_DATA_FILE);

	// printf("load_idname mysize: %u\n", mysize);
	rewind(idname_fp);
	kill_idname_mylist();
	mylist = malloc(mysize);
	throw_error(0 == fread(mylist, mysize, 1, idname_fp), CANT_READ_FILE, "Failed reading " ALBUM_DATA_FILE);
	fclose(idname_fp);
	myint = (unsigned int**) &mylist;
	mylist_malloc_size = **myint;
	// printf("load_idname mylist_malloc_size: %u\n", mylist_malloc_size);
	return 1;
error:
	return 0;

}


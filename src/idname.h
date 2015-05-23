#ifndef __IDNAME__h
#define __IDNAME__h 1

int
init_idname_mylist(void);

int
add_idname(char *id, char *name);

int
print_albuns(void);

int
search_by_id(char *id, char **name);

int
search_by_name(char *name, char **id);

int
save_idname();

int
load_idname();

int
kill_idname_mylist(void);

#endif

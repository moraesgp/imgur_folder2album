#include "dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#include "msdirent.h"
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <curl/curl.h>
#include <FreeImage.h>
#include "imgur_config.h"
#include "imgur_helper.h"
#include "resize_image.h"
#include "idname.h"

int get_imgur_dir(api_config *config_data)
{
	printf("********* entre o diretório que vai conter as fotos *********\n");
	fflush(stdout);

	scanf("%s", config_data->images_directory);

	return 1;
}

int compare_strings(const void *a, const void *b)
{
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	return strcasecmp(*ia, *ib);
}

int get_config_from_file(api_config *api_data)
{
	FILE *api_data_fp;
	// int rand_state;
	time_t t;

	srand((unsigned) time(&t));
	// rand_state = rand();

	api_data_fp = fopen(API_DATA_FILE, "r");

	// file doesn't exist we need to get the pin
	throw_error(NULL == api_data_fp, CONFIG_FILE_DOESNOT_EXIST, "file " API_DATA_FILE " doesn't exist");

	log_info("file " API_DATA_FILE " exists. Let's read it");
	fread(api_data, sizeof(api_config), 1, api_data_fp);
	fclose(api_data_fp);
	return 1;
error:
	return 0;
}

int save_config_data(api_config *config_data)
{
	FILE *api_data_fp;
	api_data_fp = fopen(API_DATA_FILE, "w");

	throw_error(NULL == api_data_fp, CANT_OPEN_FILE, "Could not open " API_DATA_FILE);
	fwrite(config_data, 1, sizeof(api_config), api_data_fp);
	fclose(api_data_fp);
	return 1;
error:
	return 0;
}

int file_finder(api_config *config, char *path, char *parent_dir)
{
	BYTE *bytes;
	DWORD size;
	photo_linked_list *temp, *head, *killme, *last;
	char *album_id;
	DIR *dp;
	struct stat st;
	struct dirent *dptr;
	char filename[DIRPATHSIZE];
	char **dirs_array;
	char **files_array;
	int dirs_array_size, files_array_size, dirs_index, files_index, i, upload_tries;

	dptr = NULL;
	upload_tries = 0;
	head = (photo_linked_list*) malloc(sizeof(photo_linked_list));
	memset(head->photo_path, 0, DIRPATHSIZE);
	head->next = NULL;
	dp = NULL;
	last = head;
	dirs_array_size = files_array_size = dirs_index = files_index = 0;

	throw_error(NULL == (dp = opendir(path)), CANT_OPEN_DIR, "Could not open %s", path);
	log_info("Entering %s", path);
	while(NULL != (dptr = readdir(dp))) {
		if(0 == strcmp(".", dptr->d_name))
			continue;
		if(0 == strcmp("..", dptr->d_name))
			continue;
		strncpy(filename, path, DIRPATHSIZE);
		strncat(filename, PATH_SEPARATOR, DIRPATHSIZE);
		strncat(filename, dptr->d_name, DIRPATHSIZE);

		stat(filename, &st);
		if(S_ISDIR(st.st_mode)) {
			strcpy(last->photo_path, filename);
			last->type = 0x01;
			dirs_array_size++;
			last->next = (photo_linked_list*) malloc(sizeof(photo_linked_list));
			last->next->next = NULL;
			last = last->next;
		} else {
			strcpy(last->photo_path, filename);
			last->type = 0x02;
			files_array_size++;
			last->next = (photo_linked_list*) malloc(sizeof(photo_linked_list));
			last->next->next = NULL;
			last = last->next;
		}
	}
	closedir(dp);
	temp = head;
	dirs_array = malloc(sizeof(char*) * dirs_array_size);
	files_array = malloc(sizeof(char*) * files_array_size);

	while(NULL != temp->next) {
		if(0x01 == temp->type) {
			dirs_array[dirs_index] = malloc(sizeof(char) * strlen(temp->photo_path) + 1);
			strcpy(dirs_array[dirs_index], temp->photo_path);
			dirs_index++;
		}
		if(0x02 == temp->type) {
			files_array[files_index] = malloc(sizeof(char) * strlen(temp->photo_path) + 1);
			strcpy(files_array[files_index], temp->photo_path);
			files_index++;
		}

		killme = temp;
		temp = temp->next;
		free(killme);
	}
	free(temp);

	qsort(dirs_array, dirs_array_size, sizeof(char*), compare_strings);
	qsort(files_array, files_array_size, sizeof(char*), compare_strings);

	for(i=0; i<dirs_array_size; i++) {
		throw_error(!file_finder(config, dirs_array[i], &dirs_array[i][strlen(path) + sizeof(char)]), last_error, "problem calling file_finder");
		free(dirs_array[i]);
	}

	for(i=0; i<files_array_size; i++) {
		log_info("running (%s): %s", parent_dir, files_array[i]);
		if(NULL == parent_dir) {
			log_info("=> No album. File %s should be uploaded without a album", files_array[i]);
			album_id = NULL;
		} else {
			throw_error(!get_album_by_name(config, parent_dir, &album_id),
					APP_ERROR,
					"problem calling get_album_by_name for %s",
					parent_dir);

			if(NULL == album_id) {
				log_info("=> Album %s doesn't exist. We must create it", parent_dir);
				throw_error(!create_new_album(config,
						parent_dir),
						APP_ERROR,
						"could not create new album %s", parent_dir);

				throw_error(!get_album_by_name(config,
							parent_dir,
							&album_id), APP_ERROR, "problem calling get_album_by_name for %s", parent_dir);

				throw_error(NULL == album_id, APP_ERROR, "Album was not created or could not be found. Must exit");
			} else {
				log_info("=> Album %s found. It's id is %s. Let's upload photo %s to album id %s", parent_dir, album_id, files_array[i], album_id);
			}
		}

		throw_error(!resize_jpeg(files_array[i], &bytes, &size), APP_ERROR, "Problem calling resize_jpeg on %s", files_array[i]);

		log_info("Begin upload %s", files_array[i]);
		while(!upload_photo_stream_to_album(config, bytes, size, album_id)) {
			throw_error(upload_tries++ > 2, APP_ERROR, "too many tries to upload image");
			if(OUTDATE_ACCESS_TOKEN == last_error) {
				log_warn("access_token esta desatualizado");
				throw_error(!get_access_token(config), APP_ERROR, "error calling get_access_token");
			}
		}
		log_info("Ended upload %s", files_array[i]);
		free(bytes);

		throw_error(unlink(files_array[i]) < 0,
				APP_ERROR,
				"could not delete file %s", files_array[i]);
		free(files_array[i]);
	}

	if(NULL != parent_dir)
		rmdir(path);

	free(dirs_array);
	free(files_array);

	return 1;
error:
	return 0;
}

int main()
{
	DIR *dirtry = NULL;
	api_config config_data;
	curl_version_info_data *curl_info_data;

	curl_global_init(CURL_GLOBAL_ALL);
	init_idname_mylist();
	FreeImage_Initialise(TRUE);
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	curl_info_data = curl_version_info(CURLVERSION_NOW);
	log_info("Imgur Folder2Album Uploader " VERSION_NUMBER);
	log_info("Using Curl %s", curl_info_data->version);
	log_info("Using FreeImage %s", FreeImage_GetVersion());

	memset(&config_data, 0, sizeof(api_config));

	if(!get_config_from_file(&config_data)) {
		if(CONFIG_FILE_DOESNOT_EXIST == last_error) {
			if(!get_imgur_pin(&config_data)) {
				return 1;
			}
			if(!get_access_token(&config_data)) {
				return 1;
			}
			if(!save_config_data(&config_data)) {
				return 1;
			}
		}
	}
	log_info("Welcome %s", config_data.account_username);
	if(!load_idname() && CANT_OPEN_FILE == last_error) {
		log_info("File " ALBUM_DATA_FILE " does not exist.");
	}

	while(NULL == (dirtry = opendir(config_data.images_directory))) {
		log_err("Could not open dir %s\n", config_data.images_directory);
		get_imgur_dir(&config_data);
	}
	closedir(dirtry);

	if(!save_config_data(&config_data)) {
		return 1;
	}

	file_finder(&config_data, config_data.images_directory, NULL);

	curl_global_cleanup();
	FreeImage_DeInitialise();
	if(!save_config_data(&config_data)) {
		return 1;
	}
	if(!save_idname()) {
		return 1;
	}
// 	print_albuns();
	kill_idname_mylist();
	log_info("Bye");
	return 0;
}


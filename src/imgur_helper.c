#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "imgur_config.h"
#include "dbg.h"
#include "curl_helper.h"
#include "cJSON.h"
#include "idname.h"
#include "imgur_helper.h"

static error_code get_imgur_api_error(cJSON *imgur_json, char *error_message)
{
	char *temp_print;
	if(!imgur_json) {
		strcpy(error_message, "json is invalid");
		return JSON_PARSE_ERROR;
	}

	temp_print = cJSON_Print(imgur_json);
	log_info("imgur_json: %s", temp_print);
	free(temp_print);

	strcpy(error_message, "NO_ERROR");
	if(!cJSON_KeyExists("success", imgur_json))
		return UNDETERMINED;

	if(cJSON_True == cJSON_GetObjectItem(imgur_json, "success")->type)
		return SUCCESS;

	strcpy(error_message, cJSON_GetObjectItem(cJSON_GetObjectItem(imgur_json, "data"), "error")->valuestring);

	if(0 == strcmp(error_message, "The access token provided is invalid."))
		return OUTDATE_ACCESS_TOKEN;

	return API_ERROR;
}

int create_new_album(api_config *config_data, char *new_album_name)
{
	// FILE *api_data_fp = NULL;
	cJSON *create_album_json = NULL;
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	char create_album_postdata[300];
	struct curl_slist *headerlist=NULL;
	char header_buffer[1000];
	char new_album_id[20];

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	sprintf(create_album_postdata, "title=%s&layout=grid", new_album_name);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, CONFIG_IMGUR_ALBUM);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, create_album_postdata);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(create_album_postdata));

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		headerlist = curl_slist_append(headerlist, strcat(strcpy(header_buffer, "Authorization: Bearer "), config_data->access_token));
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

		res = curl_easy_perform(curl);
		throw_error(res != CURLE_OK, CURL_ERROR, "curl_easy_perform() failed %s", curl_easy_strerror(res));
		if(chunk.memory) {
			create_album_json = cJSON_Parse(chunk.memory);

			if(cJSON_KeyExists("success", create_album_json)) {
				throw_error(cJSON_False == cJSON_GetObjectItem(create_album_json, "success")->type, API_ERROR, "%s", cJSON_GetObjectItem(cJSON_GetObjectItem(create_album_json, "data"), "error")->valuestring);
			}

			strcpy(new_album_id, cJSON_GetObjectItem(cJSON_GetObjectItem(create_album_json, "data"), "id")->valuestring);

			add_idname(new_album_id, new_album_name);
		}
		cJSON_Delete(create_album_json);
		free(chunk.memory);
		curl_easy_cleanup(curl);
		curl_slist_free_all (headerlist);
	}
	save_idname();

	return 1;
error:
	cJSON_Delete(create_album_json);
	free(chunk.memory);
	curl_easy_cleanup(curl);
	curl_slist_free_all (headerlist);
	return 0;
}

int upload_photo_stream_to_album(api_config *config_data, BYTE *bytes, DWORD size, char *album_id)
{
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	char header_buffer[255];

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	cJSON *imgur_ret_json;
	char temp_error_message[128];
	error_code api_error;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	/* Fill in the file upload field */
	curl_formadd(&formpost,
			   &lastptr,
			   CURLFORM_COPYNAME, "image",
			   CURLFORM_BUFFER, "megusta.png",
			   CURLFORM_CONTENTTYPE, "image/png",
			   CURLFORM_BUFFERPTR, (char*) bytes,
			   CURLFORM_BUFFERLENGTH, (long) size,
			   CURLFORM_END);

	/* Fill in the albumid field */
	if(NULL != album_id) {
		curl_formadd(&formpost,
				   &lastptr,
				   CURLFORM_COPYNAME, "album",
				   CURLFORM_COPYCONTENTS, album_id,
				   CURLFORM_END);
	}

	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

	headerlist = curl_slist_append(headerlist, strcat(strcpy(header_buffer, "Authorization: Bearer "), config_data->access_token));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	if(curl) {
		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, CONFIG_IMGUR_FILEUPLOAD);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */

		if(chunk.memory) {
			log_info("imgur_ret_json upload_photo_stream_to_album: %s", chunk.memory);
			imgur_ret_json = cJSON_Parse(chunk.memory);

			throw_error(SUCCESS != (api_error = get_imgur_api_error(imgur_ret_json, temp_error_message)),
					api_error, "%s", temp_error_message);
//			if(cJSON_KeyExists("success", imgur_ret_json)) {
//				throw_error(cJSON_False == cJSON_GetObjectItem(imgur_ret_json, "success")->type, API_ERROR, "%s", cJSON_GetObjectItem(cJSON_GetObjectItem(imgur_ret_json, "data"), "error")->valuestring);
//			}
		}
		free(chunk.memory);
		cJSON_Delete(imgur_ret_json);

		throw_error(res != CURLE_OK, CURL_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(formpost);
		/* free slist */
		curl_slist_free_all (headerlist);
	}

	log_info("stream successfully uploaded to %s", album_id);

	return 1;
error:
	log_err("imgur_ret_json: %s", chunk.memory);
	cJSON_Delete(imgur_ret_json);
	free(chunk.memory);
	curl_easy_cleanup(curl);
	curl_formfree(formpost);
	curl_slist_free_all (headerlist);
	return 0;
}
int upload_photo_to_album(api_config *config_data, char *photo_path, char *album_id)
{
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	char header_buffer[255];

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	cJSON *imgur_ret_json;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	/* Fill in the file upload field */
	curl_formadd(&formpost,
			   &lastptr,
			   CURLFORM_COPYNAME, "image",
			   CURLFORM_FILE, photo_path,
			   CURLFORM_END);

	/* Fill in the albumid field */
	if(NULL != album_id) {
		curl_formadd(&formpost,
				   &lastptr,
				   CURLFORM_COPYNAME, "album",
				   CURLFORM_COPYCONTENTS, album_id,
				   CURLFORM_END);
	}

	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

	headerlist = curl_slist_append(headerlist, strcat(strcpy(header_buffer, "Authorization: Bearer "), config_data->access_token));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	if(curl) {
		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, CONFIG_IMGUR_FILEUPLOAD);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */

		if(chunk.memory) {
			imgur_ret_json = cJSON_Parse(chunk.memory);

			if(cJSON_KeyExists("success", imgur_ret_json)) {
				throw_error(cJSON_False == cJSON_GetObjectItem(imgur_ret_json, "success")->type, API_ERROR, "%s", cJSON_GetObjectItem(cJSON_GetObjectItem(imgur_ret_json, "data"), "error")->valuestring);
			}
		}
		free(chunk.memory);

		throw_error(res != CURLE_OK, CURL_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(formpost);
		/* free slist */
		curl_slist_free_all (headerlist);
	}

	log_info("image %s successfully uploaded to %s", photo_path, album_id);

	return 1;
error:
	free(chunk.memory);
	curl_easy_cleanup(curl);
	curl_formfree(formpost);
	curl_slist_free_all (headerlist);
	return 0;
}

int update_albuns_list(api_config *config_data)
{
	cJSON *imgur_album_list = NULL;
	CURL *curl;
	CURLcode res;
	error_code api_error;
	struct MemoryStruct chunk;
	char temp_error_message[128];
	char imgur_album_url[255];
	char header_buffer[255];
	struct curl_slist *slist=NULL;
	cJSON *c;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */
	curl = curl_easy_init();

	if(curl) {
		sprintf(imgur_album_url, CONFIG_IMGUR_ALBUNS, config_data->account_username);
		curl_easy_setopt(curl, CURLOPT_URL, imgur_album_url);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

		slist = curl_slist_append(slist, strcat(strcpy(header_buffer, "Authorization: Bearer "), config_data->access_token));
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		res = curl_easy_perform(curl);
		throw_error(res != CURLE_OK, CURL_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));

		if(chunk.memory) {
			imgur_album_list = cJSON_Parse(chunk.memory);

			throw_error(SUCCESS != (api_error = get_imgur_api_error(imgur_album_list, temp_error_message)),
					api_error, "%s", temp_error_message);

			c = cJSON_GetObjectItem(imgur_album_list, "data")->child;
			while(c) {
				// printf("|%s| <==> |%s|\n", cJSON_GetObjectItem(c, "id")->valuestring, cJSON_GetObjectItem(c, "title")->valuestring);
				throw_error(!add_idname(cJSON_GetObjectItem(c, "id")->valuestring, cJSON_GetObjectItem(c, "title")->valuestring), last_error,
					"error calling add_idname");
				c = c->next;
			}
		}
		cJSON_Delete(imgur_album_list);
		free(chunk.memory);
		curl_slist_free_all(slist);
		curl_easy_cleanup(curl);
	}

	save_idname();

	return 1;
error:
	cJSON_Delete(imgur_album_list);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	free(chunk.memory);
	return 0;
}

int get_album_by_id(api_config *config_data, char *id, char **albumname)
{
	/**
	 * this function hasn't been tested"
	 **/

	int update_tries = 0;

	search_by_id(id, albumname);

	if(NULL == *albumname) {
		kill_idname_mylist();
		init_idname_mylist();
		while(!update_albuns_list(config_data)) {
			throw_error(update_tries++ > 2, APP_ERROR, "too many tries to update album");
			if(OUTDATE_ACCESS_TOKEN == last_error) {
				log_warn("access_token esta desatualizado");
				throw_error(!get_access_token(config_data), APP_ERROR, "error calling get_access_token");
			}
		}
	}
	//now that album is update we try again
	search_by_id(id, albumname);
	save_idname();
	return 1;
error:
	return 0;
}

int get_album_by_name(api_config *config_data, char *albumname, char **albumid)
{
	int update_tries = 0;

	search_by_name(albumname, albumid);

	if(NULL == *albumid) {
		kill_idname_mylist();
		init_idname_mylist();
		while(!update_albuns_list(config_data)) {
			throw_error(update_tries++ > 2, APP_ERROR, "too many tries to update album");
			if(OUTDATE_ACCESS_TOKEN == last_error) {
				log_warn("access_token esta desatualizado");
				throw_error(!get_access_token(config_data), APP_ERROR, "error calling get_access_token");
			} else {
				throw_error(ALBUM_NAME_ISNULL == last_error, ALBUM_NAME_ISNULL, "Album name is null. Fix it and the run again");
			}
		}
	}
	//now that album is update we try again
	search_by_name(albumname, albumid);
	save_idname();
	return 1;
error:
	return 0;
}

int get_access_token(api_config *config_data)
{
	// FILE *api_data_fp = NULL;
	cJSON *imgur_data;
	CURL *curl;
	CURLcode res;
	time_t expire_time;
	struct MemoryStruct chunk;
	char token_post_data[300];

	curl = curl_easy_init();
	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	if(*config_data->pin) {
		// o pin existe então vamos usa-lo
		sprintf(token_post_data, "client_id=" CONFIG_CLIENT_ID "&client_secret=" CONFIG_CLIENT_SECRET "&grant_type=pin&pin=%s",
				config_data->pin);
	} else if(*config_data->refresh_token) {
		// o refresh_token existe então vamos usa-lo
		sprintf(token_post_data, "client_id=" CONFIG_CLIENT_ID "&client_secret=" CONFIG_CLIENT_SECRET "&grant_type=refresh_token&refresh_token=%s",
				config_data->refresh_token);
	} else {
		// nao temos nem pin e nem refresh_token. Nao ha o que fazer
		throw_error(1, API_ERROR, "No pin nor refresh_token. Nothing to do.");
	}

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, CONFIG_IMGUR_TOKEN_URL);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, token_post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(token_post_data));

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl);
		throw_error(res != CURLE_OK, CURL_ERROR, "curl_easy_perform() failed %s", curl_easy_strerror(res));
		if(chunk.memory) {
			imgur_data = cJSON_Parse(chunk.memory);

			if(cJSON_KeyExists("success", imgur_data)) {
				throw_error(cJSON_False == cJSON_GetObjectItem(imgur_data, "success")->type, API_ERROR, "%s", cJSON_GetObjectItem(cJSON_GetObjectItem(imgur_data, "data"), "error")->valuestring);
			}
			expire_time = (time_t) time(NULL);
			config_data->valid_until = expire_time + (time_t) cJSON_GetObjectItem(imgur_data, "expires_in")->valueint;

			strcpy(config_data->access_token,
					cJSON_GetObjectItem(imgur_data, "access_token")->valuestring);
			strcpy(config_data->refresh_token, cJSON_GetObjectItem(imgur_data, "refresh_token")->valuestring);
			strcpy(config_data->account_username, cJSON_GetObjectItem(imgur_data, "account_username")->valuestring);

			//we must set pin to 0. Always. It can't be used twice anyway

			memset(config_data->pin, 0, WORD1SIZE);
		}
		cJSON_Delete(imgur_data);
		free(chunk.memory);
		curl_easy_cleanup(curl);
	}

	return 1;
error:
	cJSON_Delete(imgur_data);
	curl_easy_cleanup(curl);
	free(chunk.memory);
	return 0;
}

int get_imgur_pin(api_config *config_data)
{
	int rand_state;
	time_t t;

	srand((unsigned) time(&t));
	rand_state = rand();

	printf("********* copie e cole esta url no seu browser *********\n");
	printf(CONFIG_IMGUR_AUTHORIZE_URL "?client_id=" CONFIG_CLIENT_ID "&response_type=pin&state=%d\n\n", rand_state);

	printf("Qual é o seu pin?\n");
	fflush(stdout);

	scanf("%s", config_data->pin);

	return 1;
}




/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2013, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* Example source code to show how the callback function can be used to
 * download data into a chunk of memory instead of storing it in a file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl_helper.h"
#include "dbg.h"

size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *temp_memory;

//	printf("BEFORE [%llu] [%s] [%s]\n", mem->size, mem->memory, contents);
  temp_memory = (char*) malloc(mem->size + realsize + 1);
//  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(temp_memory == NULL) {
    /* out of memory! */
    log_err("not enough memory (malloc returned NULL)\n");
    return 0;
  }

  memcpy(temp_memory, mem->memory, mem->size);
 //  printf("BEFORE temp_memory: %s\n", temp_memory);
  //memcpy(&(mem->memory[mem->size]), contents, realsize);
  memcpy(temp_memory + sizeof(char) * mem->size, contents, realsize);
  //printf("AFTER temp_memory: %s\n", temp_memory);
  mem->size += realsize;
  temp_memory[mem->size] = 0;
  //mem->memory[mem->size] = 0;
  free(mem->memory);
  mem->memory = temp_memory;
//	printf("AFTER [%llu] [%s] [%s]\n", mem->size, mem->memory, contents);

  return realsize;
}

static
void dump_2(const char *text, FILE *stream, unsigned char *ptr, size_t size)
{
	int i = 0;
	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)n", text, (long)size, (long)size);

	while(i < size)
		fputc(ptr[i++], stream);

    fputc('\n', stream); /* newline */
    fputc('\n', stream); /* newline */
}

static
void dump(const char *text,
           FILE *stream, unsigned char *ptr, size_t size)
 {
   size_t i;
   size_t c;
   unsigned int width=0x10;


  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)n",
           text, (long)size, (long)size);


  for(i=0; i<size; i+= width) {
     fprintf(stream, "%4.4lx: ", (long)i);


    /* show hex to the left */
     for(c = 0; c < width; c++) {
       if(i+c < size)
         fprintf(stream, "%02x ", ptr[i+c]);
       else
         fputs("   ", stream);
     }


    /* show data on the right */
     for(c = 0; (c < width) && (i+c < size); c++)
       fputc((ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.', stream);


    fputc('\n', stream); /* newline */
   }
 }


int my_trace(CURL *handle, curl_infotype type,
              char *data, size_t size,
              void *userp)
 {
   const char *text;
   (void)handle; /* prevent compiler warning */


  switch (type) {
   case CURLINFO_TEXT:
     fprintf(stderr, "== Info: %s", data);
   default: /* in case a new one is introduced to shock us */
     return 0;


  case CURLINFO_HEADER_OUT:
     text = "=> Send header";
     break;
   case CURLINFO_DATA_OUT:
     text = "=> Send data";
     break;
   case CURLINFO_SSL_DATA_OUT:
     text = "=> Send SSL data";
     break;
   case CURLINFO_HEADER_IN:
     text = "<= Recv header";
     break;
   case CURLINFO_DATA_IN:
     text = "<= Recv data";
     break;
   case CURLINFO_SSL_DATA_IN:
     text = "<= Recv SSL data";
     break;
   }


  dump_2(text, stderr, (unsigned char *)data, size);
   return 0;
 }


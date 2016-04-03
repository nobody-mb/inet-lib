#ifndef INET_LIB_HTTP_H
#define INET_LIB_HTTP_H

#include <stdlib.h>
#include <ctype.h>

void http_alloc_data_array (char ****dst, size_t len);
void http_free_data_array (char ***dst, size_t len);
void http_add_data_header (const char ***dst, const char *field, const char *value, size_t index);

int http_form_get (char *dst, const char *dir, const char *version, 
		   const char ***head, size_t num_head);

int http_form_post (char *dst, const char *dir, const char *version, 
		    const char ***head, size_t num_head,
		    const char ***data, size_t num_data);

int http_decode_response (const char *src, int *status, 
			  char ***head, size_t num_head);

#endif
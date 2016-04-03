#include "http.h"

int url_encode (char *dst, const char *src) 
{
	static char hex[] = "0123456789abcdef";
	int count = 0;
	
	while (*src) {
		if (isalnum(*src) || *src == '-' || *src == '_' || *src == '.' || *src == '~') {
			dst[count++] = *src;
		} else if (*src == ' ') {
			dst[count++] = '+';
		} else {
			dst[count++] = '%';
			dst[count++] = hex[(*src >> 4) & 0xf];
			dst[count++] = hex[*src & 0xf];
		}
		src++;
	}
	
	dst[count] = '\0';
	
	return count;
}

void concat_single (char **dst, const char *src)
{
	if (src)
		while (*src)
			*(*dst)++ = *src++;
}

void concat_array (char **dst, const char **src, size_t num_src)
{
	while (num_src--)
		concat_single(dst, *src++);
}

int http_insert_header (char **dst, const char *request, const char *dir, 
			const char *version, const char ***head, size_t num_head)
{
	if (!dst || !request || !dir || !version || !head || !num_head)
		return -1;
	
	concat_array(dst, (const char *[])
		     {request, " /", dir, " HTTP/", version, "\r\n"}, 6);
	
	while (num_head--)
		concat_array(dst, (const char *[])
			     {(*head)[0], ": ", (*head++)[1], "\r\n"}, 4);
	
	*(unsigned short *)(*dst) = 0x0A0D;
	*dst += sizeof(unsigned short);
	
	return 0;
}

int http_insert_data (char **dst, const char ***data, size_t num_data)
{
	if (!data || !num_data)
		return -1;
	
	while (*data && num_data--) {
		*dst += url_encode(*dst, (*data)[0]);
		*(*dst)++ = '=';
		*dst += url_encode(*dst, (*data++)[1]);
		*(*dst)++ = '&';
	}
	
	*(unsigned short *)(*dst - 1) = 0x0A0D;
	
	return 0;
}



void http_alloc_data_array (char ****dst, size_t len)
{
	*dst = calloc(sizeof(char *), len);
	
	while (len--)
		(*dst)[len] = calloc(sizeof(char *), 2);
}

void http_free_data_array (char ***dst, size_t len)
{
	while (len--)
		free(dst[len]);
	
	free(dst);
}

void http_add_data_header (const char ***dst, const char *field, const char *value, size_t index)
{
	dst[index][0] = field;
	dst[index][1] = value;
}

int http_form_get (char *dst, const char *dir, const char *version, 
		   const char ***head, size_t num_head)
{
	return http_insert_header(&dst, "GET", dir, version, head, num_head);
}

int http_form_post (char *dst, const char *dir, const char *version, 
		    const char ***head, size_t num_head,
		    const char ***data, size_t num_data)
{
	if (http_insert_header(&dst, "POST", dir, version, head, num_head) < 0)
		return -1;

	return http_insert_data(&dst, data, num_data);
}

int http_decode_response (const char *src, int *status, 
			  char ***head, size_t num_head)
{
	int i, j;
	int count = 0;

	if (!src || !status || !head || !num_head)
		return -1;

	while (*src++ != ' ');
	
	for ((*status) = 0; *src != ' '; src++)
		(*status) = ((*status) * 10) + (*src - '0');

	do {
		while (*src++ != '\n');
		
		for (i = 0; src[i] != ':'; i++)
			if (src[i] == '\r' || src[i] == '\n' || src[i] == '\0')
				return count;
		head[count][0] = malloc(i + 1);
		for (j = 0; j < i; j++)
			head[count][0][j] = *src++;
		src += 2;
		
		for (i = 0; src[i] != '\r'; i++) 
			if (src[i] == '\n' || src[i] == '\0')
				return count;
		head[count][1] = malloc(i + 1);
		for (j = 0; j < i; j++)
			head[count][1][j] = *src++;
	} while (count++ < num_head);

	return count;
}

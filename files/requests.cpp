#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>     /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helper.hpp"
#include "requests.hpp"

char *compute_get_request(const char *host, const char *url, char *query_params,
							char **cookies, int cookies_count)
{
	char *message = (char*)calloc(BUFLEN, sizeof(char));
	char *line = (char*)calloc(LINELEN, sizeof(char));

	// Step 1: write the method name, URL, request params (if any) and protocol type
	if (query_params != NULL) {
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "GET %s HTTP/1.1", url);
	}

	compute_message(message, line);

	// Step 2: add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	// Step 3 (optional): add headers and/or cookies, according to the protocol format
	if (cookies != NULL) {
		sprintf(line, "Cookie: ");
		for (int i = 0; i < cookies_count; i++) {
			sprintf(line, "%s", cookies[i]);
			strcat(line, cookies[i]);
			if (i < cookies_count - 1) {
				strcat(line, "; ");
			}
		}
		compute_message(message, line);
	}
	// Step 4: add final new line
	compute_message(message, "");
	return message;
}

char *compute_post_request(const char *host, const char *url, const char* content_type, char **body_data,
							int body_data_fields_count, char **cookies, int cookies_count)
{
	char *message = (char*)calloc(BUFLEN, sizeof(char));
	char *line = (char*)calloc(LINELEN, sizeof(char));
	char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

	// Step 1: write the method name, URL and protocol type
	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);
	
	// Step 2: add the host
	memset(line, 0, LINELEN);
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	/* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
			in order to write Content-Length you must first compute the message size
	*/

	memset(line, 0, LINELEN);
   sprintf(line, "Content-Type: %s", content_type);
   compute_message(message, line);

	for (int i = 0; i < body_data_fields_count; i++) {
		strcat(body_data_buffer, body_data[i]);
		if (i < body_data_fields_count - 1) {
			strcat(body_data_buffer, "&");
		}
	}

	memset(line, 0, LINELEN);
	sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
	compute_message(message, line);

	// Step 4 (optional): add cookies
	if (cookies != NULL) {
		sprintf(line, "Cookie: ");
		for (int i = 0; i < cookies_count; i++) {
			strcat(line, cookies[i]);
			if (i < cookies_count - 1) {
				strcat(line, "; ");
			}
		}
		compute_message(message, line);
	}
	// Step 5: add new line at end of header
	compute_message(message, "");

	// Step 6: add the actual payload data
	memset(line, 0, LINELEN);
	strcat(message, body_data_buffer);

	//compute_message(message, body_data_buffer);
	free(line);
	return message;
}

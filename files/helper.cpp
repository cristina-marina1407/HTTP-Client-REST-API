#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include "helper.hpp"
#include "buffer.hpp"

using namespace std;

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

/* checks if the given string is a natural number */
bool is_natural_number(const string& id) {
	if (id.empty())
		return false;
	for (char c : id) {
		if (!isdigit(c))
			 return false;
	}
	return true;
}

/* returns the HTTP status code from the response string */
int get_status_code(const string &resp_str) {
	int status_code = -1;

	/* find the first space after HTTP/1.1 */
	size_t code_start = resp_str.find(" ") + 1;
	if (code_start != string::npos) {
		/* get the substring starting from the status code */
		string rest = resp_str.substr(code_start);

		/* find the next space */
		size_t code_end = rest.find(" ");

		if (code_end != string::npos) {
			/* extract the status code as a string and convert it to int */
			string code_str = rest.substr(0, code_end);
			status_code = stoi(code_str);
		}
	}
	return status_code;
}

/* returns the value of the cookie header from the response string */
string get_cookie(const string &resp_str) {
	/* search for the "Set-Cookie:" header in the response */
	size_t cookie_start = resp_str.find("Set-Cookie:");

	/* if the header is found, extract the substring after "Set-Cookie: " */
	if (cookie_start != string::npos) {
		string cookie = resp_str.substr(cookie_start + strlen("Set-Cookie: "));

		/* find the end for the cookie */
		size_t cookie_end = cookie.find(";");

		/* return the cookie header */
		if (cookie_end != string::npos) {
			return cookie.substr(0, cookie_end);
		}
	}
	return "";
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void compute_message(char *message, const char *line)
{
	strcat(message, line);
	strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
	struct sockaddr_in serv_addr;
	int sockfd = socket(ip_type, socket_type, flag);
	if (sockfd < 0)
		error("ERROR opening socket");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = ip_type;
	serv_addr.sin_port = htons(portno);
	inet_aton(host_ip, &serv_addr.sin_addr);

	/* connect the socket */
	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	return sockfd;
}

void close_connection(int sockfd)
{
	close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
	int bytes, sent = 0;
	int total = strlen(message);

	do
	{
		bytes = write(sockfd, message + sent, total - sent);
		if (bytes < 0) {
			error("ERROR writing message to socket");
		}

		if (bytes == 0) {
			break;
		}

		sent += bytes;
	} while (sent < total);
}

char *receive_from_server(int sockfd)
{
	char response[BUFLEN];
	buffer buffer = buffer_init();
	int header_end = 0;
	int content_length = 0;

	do {
		int bytes = read(sockfd, response, BUFLEN);

		if (bytes < 0){
			error("ERROR reading response from socket");
		}

		if (bytes == 0) {
			break;
		}

		buffer_add(&buffer, response, (size_t) bytes);
		
		header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

		if (header_end >= 0) {
			header_end += HEADER_TERMINATOR_SIZE;
			
			int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
			
			if (content_length_start < 0) {
				continue;           
			}

			content_length_start += CONTENT_LENGTH_SIZE;
			content_length = strtol(buffer.data + content_length_start, NULL, 10);
			break;
		}
	} while (1);
	size_t total = content_length + (size_t) header_end;
	
	while (buffer.size < total) {
		int bytes = read(sockfd, response, BUFLEN);

		if (bytes < 0) {
			error("ERROR reading response from socket");
		}

		if (bytes == 0) {
			break;
		}

		buffer_add(&buffer, response, (size_t) bytes);
	}
	buffer_add(&buffer, "", 1);
	return buffer.data;
}

char *basic_extract_json_response(char *str)
{
	return strstr(str, "{\"");
}

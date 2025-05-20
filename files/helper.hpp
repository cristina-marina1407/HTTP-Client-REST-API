#ifndef _HELPERS_
#define _HELPERS_

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

#define BUFLEN 4096
#define LINELEN 1000

/* checks if a given string is a natural */
bool is_natural_number(const string& id);

/* returns the HTTP status code from the response string */
int get_status_code(const string &resp_str);

/* returns the value of the cookie header from the response string */
string get_cookie(const string &resp_str);

/* shows the current error */
void error(const char *msg);

/* adds a line to a string message */
void compute_message(char *message, const char *line);

/* opens a connection with server host_ip on port portno, returns a socket */
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

/* closes a server connection on socket sockfd */
void close_connection(int sockfd);

/* send a message to a server */
void send_to_server(int sockfd, char *message);

/* receives and returns the message from a server */
char *receive_from_server(int sockfd);

/* extracts and returns a JSON from a server response */
char *basic_extract_json_response(char *str);

#endif

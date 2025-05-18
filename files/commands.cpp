#include <iostream>
#include <string>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>

#include "helper.hpp"
#include "requests.hpp"
#include "json.hpp"
#include "json_fwd.hpp"
#include "commands.hpp"

using namespace std;
using namespace nlohmann;

void login_admin(string &cookies, string &token) {
	/* Reading the username and password */
	string username, password;
	cout<< "username=";
	getline(cin, username);
	cout << "password=";
	getline(cin, password);

	/* Creating the JSON payload */
	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/admin/login";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* Check if the admin is already logged in */
	if (!cookies.empty()) {
		cout << "ERROR : Admin already logged in." << endl;
		return;
	}

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, NULL, 0, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	string resp_str(response);

	int status_code = -1;
	size_t first_space = resp_str.find(' ');
	size_t second_space = resp_str.find(' ', first_space + 1);
	if (first_space != string::npos && second_space != string::npos) {
		string status_code_str = resp_str.substr(first_space + 1,
										 		 second_space - first_space - 1);
		status_code = stoi(status_code_str);
	} 

	size_t cookie_start = resp_str.find("Set-Cookie:");
	if (cookie_start != string::npos) {
		string cookie = resp_str.substr(cookie_start + strlen("Set-Cookie: "));
		size_t cookie_end = cookie.find(";");
		if (cookie_end != string::npos) {
			cookies = cookie.substr(0, cookie_end);
		}
	}

	if (status_code == 200) {
	cout << "SUCCESS: " << status_code << " - OK" << endl;
	} else {
		char *json_response = basic_extract_json_response(response);
		if (json_response != NULL) {
			json parsed_response = json::parse(json_response, nullptr, false);
			if (!parsed_response.is_discarded() &&
				parsed_response.contains("error")) {
				string error_msg = parsed_response["error"].get<string>();
				if (error_msg.find("Invalid credentials") != string::npos) {
					cout << status_code
						 << " - ERROR: Credentials are not good!" << endl;
				} else {
					cout << status_code << " - ERROR: " << error_msg << endl;
				}
			} else {
				cout << status_code << " - ERROR: Unexpected response!"
					 << endl;
			}
		} else {
			cout << status_code << " - ERROR: No JSON response from server!"
				 << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void add_user() {

}

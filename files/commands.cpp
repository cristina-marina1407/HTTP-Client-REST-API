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

int admin = -1;

int get_status_code(const string &resp_str) {
	int status_code = -1;
	size_t code_start = resp_str.find(" ") + 1;
	if (code_start != string::npos) {
		string rest = resp_str.substr(code_start);
		size_t code_end = rest.find(" ");
		if (code_end != string::npos) {
			string code_str = rest.substr(0, code_end);
			status_code = stoi(code_str);
		}
	}
	return status_code;
}

string get_cookie(const string &resp_str) {
	size_t cookie_start = resp_str.find("Set-Cookie:");
	if (cookie_start != string::npos) {
		string cookie = resp_str.substr(cookie_start + strlen("Set-Cookie: "));
		size_t cookie_end = cookie.find(";");
		if (cookie_end != string::npos) {
			return cookie.substr(0, cookie_end);
		}
	}
	return "";
}

void login_admin(string &cookies, string &token) {
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
		cout << "ERROR: Admin already logged in" << endl;
		return;
	}

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: " << status_code << " - OK" << endl;

		admin = 1;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}

	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			if (error_msg.find("Invalid credentials") != string::npos) {
				cout << "ERROR: " << status_code << " Credentials are not good!" << endl;
			} else {
				cout << "ERROR: " << status_code << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
}

void add_user(string &cookies, string &token) {
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

	const char *access_route = "/api/v1/tema/admin/users";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	if (username.find(' ') != string::npos || password.find(' ') != string::npos) {
    	cout << "ERROR: Incomplete/Wrong information" << endl;
    	return;
	}

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	string resp_str(response);

	int status_code = get_status_code(resp_str);

	if (status_code == 201) {
		cout << "SUCCESS: " << status_code << " - OK" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}

	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			char *json_response = basic_extract_json_response(response);
			json parsed_response = json::parse(json_response);
			if (parsed_response.contains("error")) {
				string error_msg = parsed_response["error"].get<string>();
				cout << "ERROR: " << status_code << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_users(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/admin/users";

	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: Users:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		if (parsed_response.find("users") != parsed_response.end()) {
			auto users = parsed_response["users"];
			int index = 1;
			for (const auto& user : users) {
				//int id = user["id"];
				string username = user["username"];
				string password = user["password"];
				cout << "#" << index << " " << username << ":" << password << endl;
				index++;
			}
		}

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void delete_user(string &cookies, string &token) {
	string username;
	cout<< "username=";
	getline(cin, username);

	const char *access_route = "/api/v1/tema/admin/users/username";

	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	if (username.find(' ') != string::npos) {
    	cout << "ERROR: Incomplete/Wrong information" << endl;
    	return;
	}

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);

	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: " << status_code << " - OK" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void logout_admin(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/admin/logout";

	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: Admin logged off" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void login(string &cookies, string &token) {
	string admin_username, username, password;
	cout<< "admin_username=";
	getline(cin, admin_username);
	cout<< "username=";
	getline(cin, username);
	cout << "password=";
	getline(cin, password);

	/* Creating the JSON payload */
	json json_data;
	json_data["admin_username"] = admin_username;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/user/login";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: " << "Successfully logged in" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}

	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			if (error_msg.find("Invalid credentials") != string::npos) {
				cout << "ERROR: " << status_code << " Credentials are not good!" << endl;
			} else {
				cout << "ERROR: " << status_code << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
}

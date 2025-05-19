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
int movies_added_to_collection = 0;

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

	if (username.find(' ') != string::npos || password.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* Creating the JSON payload */
	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/admin/login";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* Check if the admin is already logged in */
	if (admin == 1) {
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
				cout << "ERROR: " << status_code << " " << error_msg << endl;
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
				cout << "ERROR: " << status_code << " " << error_msg << endl;
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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void delete_user(string &cookies, string &token) {
	string username;
	cout<< "username=";
	getline(cin, username);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/admin/users/%s", username.c_str());

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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void logout_admin(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/admin/logout";

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: Admin logged off" << endl;
		admin = -1;

		cookies.clear();
		token.clear();
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
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

	if (username.find(' ') != string::npos ||
	 	password.find(' ') != string::npos ||
		admin_username.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

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
				cout << "ERROR: " << status_code << " " << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_access(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/access";

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: Token JWT received" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("token")) {
			token = parsed_response["token"].get<string>();
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
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_movies(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/movies";

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	cout << response << endl;

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: Movies:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		if (parsed_response.find("movies") != parsed_response.end()) {
			auto movies = parsed_response["movies"];
			for (const auto& movie : movies) {
				int id = movie["id"]; 
				string title = movie["title"];
				cout << "#" << id << " " << title << endl;
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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_movie(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s", id.c_str());

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Movie details" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		if (parsed_response.contains("id") && parsed_response["id"].get<int>() == stoi(id)) {
			if (parsed_response.contains("title"))
				cout << "title: " << parsed_response["title"].get<string>() << endl;
			if (parsed_response.contains("year"))
				cout << "year: " << parsed_response["year"] << endl;
			if (parsed_response.contains("description"))
				cout << "description: " << parsed_response["description"].get<string>() << endl;
			if (parsed_response.contains("rating"))
				cout << "rating: " << parsed_response["rating"].get<string>()  << endl;
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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void add_movie(string &cookies, string &token) {
	string title, description, year_string, rating_string;
	cout<< "title=";
	getline(cin, title);
	cout<< "year=";
	getline(cin, year_string);
	cout << "description=";
	getline(cin, description);
	cout << "rating=";
	getline(cin, rating_string);

	if (year_string.find(' ') != string::npos ||
	 	rating_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	int year = stoi(year_string);
	double rating = stod(rating_string);

	/* Creating the JSON payload */
	json json_data;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/library/movies";
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

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		cout << "SUCCESS: Movie added" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void update_movie(string &cookies, string &token) {
	string title, description, year_string, rating_string, id;
	cout<< "id=";
	getline(cin, id);
	cout<< "title=";
	getline(cin, title);
	cout<< "year=";
	getline(cin, year_string);
	cout << "description=";
	getline(cin, description);
	cout << "rating=";
	getline(cin, rating_string);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s", id.c_str());

	if (year_string.find(' ') != string::npos ||
	 	rating_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	int year = stoi(year_string);
	double rating = stod(rating_string);

	/* Creating the JSON payload */
	json json_data;
	//json_data["id"] = id;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	string json_payload = json_data.dump();

	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_put_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Movie updated" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void delete_movie(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s", id.c_str());

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Movie deleted" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_collections(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/collections";

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collections:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		if (parsed_response.find("collections") != parsed_response.end()) {
			auto collections = parsed_response["collections"];
			for (const auto& collection : collections) {
				int id = collection["id"]; 
				string title = collection["title"];
				cout << "#" << id << " " << title << endl;
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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void get_collection(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s", id.c_str());

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collection details" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		if (parsed_response.contains("id") && parsed_response["id"].get<int>() == stoi(id)) {
			if (parsed_response.contains("title"))
				cout << "title: " << parsed_response["title"].get<string>() << endl;
			if (parsed_response.contains("owner"))
				cout << "owner: " << parsed_response["owner"].get<string>() << endl;
			if (parsed_response.find("movies") != parsed_response.end()) {
				auto movies = parsed_response["movies"];
				for (const auto& movie : movies) {
					int id = movie["id"]; 
					string title = movie["title"];
					cout << "#" << id << " " << title << endl;
				}
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
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void add_collection_helper(string &cookies, string &token, string id_collection_string, string movie_id_string) {
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies", id_collection_string.c_str());

	int id = stoi(movie_id_string);

	json json_data;
	json_data["id"] = id;
	string json_payload = json_data.dump();

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

	if (status_code == 201) {
		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		movies_added_to_collection++;
	}

	close_connection(sockfd);
	free(request);
}

void add_collection(string &cookies, string &token) {
	string title, num_movies_string;
	cout<< "title=";
	getline(cin, title);
	cout<< "num_movies=";
	getline(cin, num_movies_string);

	int num_movies = stoi(num_movies_string);

	vector<string> movie_id_string(num_movies);

	for (int i = 0; i < num_movies; i++) {
		cout << "movie_id[" << i << "]=";
		getline(cin, movie_id_string[i]);
		if (movie_id_string[i].find(' ') != string::npos) {
			continue;
		}
	}

	movies_added_to_collection = 0;

	const char *access_route = "/api/v1/tema/library/collections";

	json json_data;
	json_data["title"] = title;
	string json_payload = json_data.dump();

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

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		int id;
		if (parsed_response.contains("id")) {
			id = parsed_response["id"];
		}

		string id_string = to_string(id);

		for (int i = 0; i < num_movies; i++) {
			add_collection_helper(cookies, token, id_string, movie_id_string[i]);
		}

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		if (movies_added_to_collection == 0) {
			cout << "ERROR: " << status_code  << " " << "Empty collection" << endl;
		}

		cout << "SUCCESS: Added collection" << endl;
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void delete_collection(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s", id.c_str());

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collection deleted" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void add_movie_to_collection(string &cookies, string &token) {
	string id_collection_string, movie_id_string;
	cout << "collection_id=";
	getline(cin, id_collection_string);
	cout << "movie_id=";
	getline(cin, movie_id_string);

	if (id_collection_string.find(' ') != string::npos ||
	 	movie_id_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies", id_collection_string.c_str());

	int id = stoi(movie_id_string);

	json json_data;
	json_data["id"] = id;
	string json_payload = json_data.dump();

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

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		cout << "SUCCESS: Movie added to collection" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void delete_movie_from_collection(string &cookies, string &token) {
	string id_collection_string, movie_id_string;
	cout << "collection_id=";
	getline(cin, id_collection_string);
	cout << "movie_id=";
	getline(cin, movie_id_string);

	if (id_collection_string.find(' ') != string::npos ||
	 	movie_id_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies/%s", id_collection_string.c_str(), movie_id_string.c_str());

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collection deleted" << endl;

		string cookie = get_cookie(resp_str);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

void logout(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/user/logout";

	char *send_cookies = strdup(cookies.c_str());

	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string resp_str(response);
	
	int status_code = get_status_code(resp_str);

	if (status_code == 200) {
		cout << "SUCCESS: User logged off" << endl;

		cookies.clear();
		token.clear();
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
}

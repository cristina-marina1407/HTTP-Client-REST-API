#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
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

/* function that logs in as admin by sending a POST request */
void login_admin(string &cookies, string &token) {
	string username, password;
	cout<< "username=";
	getline(cin, username);
	cout << "password=";
	getline(cin, password);

	/* checking input for spaces */
	if (username.find(' ') != string::npos ||
		password.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* creating the JSON payload */
	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/admin/login";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* check if the admin is already logged in */
	if (admin == 1) {
		cout << "ERROR: Admin already logged in" << endl;
		return;
	}

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* if the status code is successful, the admin is set to logged in */
		admin = 1;

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: " << status_code << " - OK" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			if (error_msg.find("Invalid credentials") != string::npos) {
				cout << "ERROR: " << status_code
					 << " Username/password is wrong " << endl;
			} else {
				cout << "ERROR: " << status_code << " " << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that adds a new user by sending a POST request */
void add_user(string &cookies, string &token) {
	string username, password;
	cout<< "username=";
	getline(cin, username);
	cout << "password=";
	getline(cin, password);

	/* checking input for spaces */
	if (username.find(' ') != string::npos || password.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the admin user is admin */
	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	/* creating the JSON payload */
	json json_data;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/admin/users";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);

	int status_code = get_status_code(response_string);

	if (status_code == 201) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: " << status_code << " - OK" << endl;
	} else {
		/* extract the error message */
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
	free(response);
	free(send_cookies);
}

/* function that gets the list of users by sending a GET request */
void get_users(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/admin/users";

	/* check if the admin user is admin */
	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL,
										&send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		cout << "SUCCESS: Users:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extracts the users from the response and prints them */
		if (parsed_response.find("users") != parsed_response.end()) {
			auto users = parsed_response["users"];
			int index = 1;
			for (const auto& user : users) {
				string username = user["username"];
				string password = user["password"];
				cout << "#" << index << " " << username << ":" << password << endl;
				index++;
			}
		}

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that delets an user by sending a DELETE request */
void delete_user(string &cookies, string &token) {
	string username;
	cout<< "username=";
	getline(cin, username);

	/* checking input for spaces */
	if (username.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the admin user is admin */
	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	/* concatenate the path with the username for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/admin/users/%s",
			 username.c_str());

	/* copy cookies to send with the request */		 
	char *send_cookies = strdup(cookies.c_str());

	/* send the delete request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string response_string(response);

	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: " << status_code << " - OK" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that logs out the admin by sending a GET request */
void logout_admin(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/admin/logout";

	/* check if the admin is logged in */
	if (admin == -1) {
		cout << "ERROR: User is not admin" << endl;
		return;
	}

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* change the admin to logged off */
		admin = -1;

		/* clear the cookies */
		cookies.clear();

		/* clear the token */
		token.clear();

		cout << "SUCCESS: Admin logged off" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that logs in a user by sending a POST request */
void login(string &cookies, string &token) {
	string admin_username, username, password;
	/* the admin username is necessary to log in a user as well */
	cout<< "admin_username=";
	getline(cin, admin_username);
	cout<< "username=";
	getline(cin, username);
	cout << "password=";
	getline(cin, password);

	/* checking input for spaces */
	if (username.find(' ') != string::npos ||
	 	password.find(' ') != string::npos ||
		admin_username.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* creating the JSON payload */
	json json_data;
	json_data["admin_username"] = admin_username;
	json_data["username"] = username;
	json_data["password"] = password;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/user/login";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: " << "Successfully logged in" << endl;
	} else {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			if (error_msg.find("Invalid credentials") != string::npos) {
				cout << "ERROR: " << status_code
				 	 << " Username/password is wrong " << endl;
			} else {
				cout << "ERROR: " << status_code << " " << error_msg << endl;
			}
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that gets the access token by sending a GET request */
void get_access(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/access";

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL,
										&send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* extract the token from the response */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("token")) {
			token = parsed_response["token"].get<string>();
		}

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Token JWT received" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that gets the list of movies by sending a GET request */
void get_movies(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/movies";

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL,
										&send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Movies:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extracts the movies from the response and prints them */
		if (parsed_response.find("movies") != parsed_response.end()) {
			auto movies = parsed_response["movies"];
			for (const auto& movie : movies) {
				int id = movie["id"]; 
				string title = movie["title"];
				cout << "#" << id << " " << title << endl;
			}
		}

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that gets the details of a movie by sending a GET request */
void get_movie(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	/* checking input for spaces */
	if (id.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the id is a natural number */
	if (!is_natural_number(id)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the movie id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s",
			id.c_str());

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL,
										&send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Movie details" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extract the movie details from the response and prints them*/
		if (parsed_response.contains("title"))
			cout << "title: " << parsed_response["title"].get<string>() << endl;
		if (parsed_response.contains("year"))
			cout << "year: " << parsed_response["year"] << endl;
		if (parsed_response.contains("description"))
			cout << "description: " << parsed_response["description"].get<string>() << endl;
		if (parsed_response.contains("rating"))
			cout << "rating: " << parsed_response["rating"].get<string>()  << endl;

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that adds a new movie by sending a POST request */
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

	/* checking input for spaces */
	if (year_string.find(' ') != string::npos ||
	 	rating_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	int year = stoi(year_string);
	double rating = stod(rating_string);

	/* creating the JSON payload */
	json json_data;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	string json_payload = json_data.dump();

	const char *access_route = "/api/v1/tema/library/movies";
	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Movie added" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that updates a movie by sending a PUT request */
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

	/* checking input for spaces */
	if (year_string.find(' ') != string::npos ||
	 	rating_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the id is a natural number */
	if (!is_natural_number(id)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s", id.c_str());

	int year = stoi(year_string);
	double rating = stod(rating_string);

	/* creating the JSON payload */
	json json_data;
	json_data["title"] = title;
	json_data["year"] = year;
	json_data["description"] = description;
	json_data["rating"] = rating;
	string json_payload = json_data.dump();

	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the PUT request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_put_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Movie updated" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that deletes a movie by sending a DELETE request */
void delete_movie(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	/* checking input for spaces */
	if (id.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the id is a natural number */
	if (!is_natural_number(id)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/movies/%s", id.c_str());

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the DELETE request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Movie deleted" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that gets the list of collections by sending a GET request */
void get_collections(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/library/collections";

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collections:" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extracts the collections from the response and prints them */
		if (parsed_response.find("collections") != parsed_response.end()) {
			auto collections = parsed_response["collections"];
			for (const auto& collection : collections) {
				int id = collection["id"]; 
				string title = collection["title"];
				cout << "#" << id << " " << title << endl;
			}
		}

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that gets the details of a collection by sending a GET request */
void get_collection(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	/* checking input for spaces */
	if (id.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the id is a natural number */
	if (!is_natural_number(id)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s", id.c_str());

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		cout << "SUCCESS: Collection details" << endl;

		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extract the collection details from the response and prints them */
		if (parsed_response.contains("title"))
			cout << "title: " << parsed_response["title"].get<string>() << endl;
		if (parsed_response.contains("owner"))
			cout << "owner: " << parsed_response["owner"].get<string>() << endl;

		/* print every movie in the collection */
		if (parsed_response.find("movies") != parsed_response.end()) {
			auto movies = parsed_response["movies"];
			for (const auto& movie : movies) {
				int id = movie["id"]; 
				string title = movie["title"];
				cout << "#" << id << " " << title << endl;
			}
		}
		
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function used to add the valid movies in the collection */
void add_collection_helper(string &cookies, string &token,
						   string id_collection_string, string movie_id_string) {
	/* concatenate the path with the movie id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies", id_collection_string.c_str());

	int id = stoi(movie_id_string);

	/* creating the JSON payload */
	json json_data;
	json_data["id"] = id;
	string json_payload = json_data.dump();

	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 201) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		/* increase the number of movies added in the collection */
		movies_added_to_collection++;
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}


/* function that adds a new collection and every movie in it by sending a POST request */
void add_collection(string &cookies, string &token) {
	string title, num_movies_string;
	cout<< "title=";
	getline(cin, title);
	cout<< "num_movies=";
	getline(cin, num_movies_string);

	int num_movies = stoi(num_movies_string);

	vector<string> movie_id_string(num_movies);

	/* add all of the movie ids in a vector */
	for (int i = 0; i < num_movies; i++) {
		cout << "movie_id[" << i << "]=";
		getline(cin, movie_id_string[i]);
		if (movie_id_string[i].find(' ') != string::npos) {
			continue;
		}
	}

	/* reset the movies added to every collection counter */
	movies_added_to_collection = 0;

	const char *access_route = "/api/v1/tema/library/collections";

	/* creating the JSON payload */
	json json_data;
	json_data["title"] = title;
	string json_payload = json_data.dump();

	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);

		/* extract the collection id from the response */
		int id;
		if (parsed_response.contains("id")) {
			id = parsed_response["id"];
		}

		/* transform the id to string to use it in the helper */
		string id_string = to_string(id);

		/* add every movie in the collection */
		for (int i = 0; i < num_movies; i++) {
			add_collection_helper(cookies, token, id_string, movie_id_string[i]);
		}

		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		/* check if the collection is complete*/
		if (movies_added_to_collection != num_movies) {
			cout << "ERROR: " << status_code  << " "
				 << "Not all of the movies were valid" << endl;
		}

		cout << "SUCCESS: Added collection" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that deletes a collection by sending a DELETE request */
void delete_collection(string &cookies, string &token) {
	string id;
	cout<< "id=";
	getline(cin, id);

	/* checking input for spaces */
	if (id.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the id is a natural number */
	if (!is_natural_number(id)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s", id.c_str());

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the DELETE request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Collection deleted" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that adds a movie to a collection by sending a POST request */
void add_movie_to_collection(string &cookies, string &token) {
	string id_collection_string, movie_id_string;
	cout << "collection_id=";
	getline(cin, id_collection_string);
	cout << "movie_id=";
	getline(cin, movie_id_string);

	/* checking input for spaces */
	if (id_collection_string.find(' ') != string::npos ||
	 	movie_id_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the ids are natural numbers */
	if (!is_natural_number(id_collection_string) ||
		!is_natural_number(movie_id_string)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the movie id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies",
		 	 id_collection_string.c_str());

	int id = stoi(movie_id_string);

	/* creating the JSON payload */
	json json_data;
	json_data["id"] = id;
	string json_payload = json_data.dump();

	char *body_data[1];
	body_data[0] = (char*)json_payload.c_str();

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the POST request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_post_request(HOST, access_route, PAYLOAD_TYPE,
										 body_data, 1, &send_cookies, 1, token);
	send_to_server(sockfd, request);
	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 201) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Movie added to collection" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code  << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that deletes a movie from a collection by sending a DELETE request */
void delete_movie_from_collection(string &cookies, string &token) {
	string id_collection_string, movie_id_string;
	cout << "collection_id=";
	getline(cin, id_collection_string);
	cout << "movie_id=";
	getline(cin, movie_id_string);

	/* checking input for spaces */
	if (id_collection_string.find(' ') != string::npos ||
	 	movie_id_string.find(' ') != string::npos) {
		cout << "ERROR: Incomplete/Wrong information" << endl;
		return;
	}

	/* check if the ids are natural numbers */
	if (!is_natural_number(id_collection_string) ||
		!is_natural_number(movie_id_string)) {
		cout << "ERROR: Invalid id" << endl;
		return;
	}

	/* concatenate the path with the collection and the movie id for the request */
	char access_route[256];
	snprintf(access_route, sizeof(access_route), "/api/v1/tema/library/collections/%s/movies/%s",
			 id_collection_string.c_str(), movie_id_string.c_str());

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the DELETE request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_delete_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	/* check if the user has access to the library */
	if (token.empty()) {
		cout << "ERROR: " << status_code << " " << "JWT token required" << endl;
		return;
	}

	if (status_code == 200) {
		/* save the session cookie */
		string cookie = get_cookie(response_string);
		if (!cookie.empty()) {
			cookies = cookie;
		}

		cout << "SUCCESS: Collection deleted" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

/* function that logs out a user by sending a GET request */
void logout(string &cookies, string &token) {
	const char *access_route = "/api/v1/tema/user/logout";

	/* copy cookies to send with the request */
	char *send_cookies = strdup(cookies.c_str());

	/* send the GET request */
	int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *request = compute_get_request(HOST, access_route, NULL, &send_cookies, 1, token);
	send_to_server(sockfd, request);

	char *response = receive_from_server(sockfd);

	/* convert to string */
	string response_string(response);
	
	int status_code = get_status_code(response_string);

	if (status_code == 200) {
		/* clear the cookies */
		cookies.clear();

		/* clear the token */
		token.clear();

		cout << "SUCCESS: User logged off" << endl;
	} else {
		/* extract the error message */
		char *json_response = basic_extract_json_response(response);
		json parsed_response = json::parse(json_response);
		if (parsed_response.contains("error")) {
			string error_msg = parsed_response["error"].get<string>();
			cout << "ERROR: " << status_code << " " << error_msg << endl;
		}
	}

	close_connection(sockfd);
	free(request);
	free(response);
	free(send_cookies);
}

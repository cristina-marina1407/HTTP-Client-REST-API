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

int main()
{
	string command;
	string cookies;
	string token;

	while (1) {
		/* Read the command from stdin */
		getline(cin, command);

		if (command == "exit") {
			return 0;
		} else if (command == "login_admin") {
			login_admin(cookies, token);
		} else if (command == "add_user") {
			add_user(cookies, token);
		} else if (command == "get_users") {
			get_users(cookies, token);
		} else if (command == "delete_users") {
			delete_user(cookies, token);
		} else if (command == "logout_admin") {
			logout_admin(cookies, token);
		} else if (command == "login") {
			login(cookies, token);
        } else if (command == "get_access") {
			get_access(cookies, token);
        } else if (command == "get_movies") {
			get_movies(cookies, token);
        } else if (command == "get_movie") {
			break;
        } else if (command == "add_movie") {
			break;
        } else if (command == "update_movie") {
			break;
        } else if (command == "delete_movie") {
			break;
        } else if (command == "delete_all_movies") {
			break;
        } else if (command == "add_collection") {
			break;
        } else if (command == "get_collections") {
			break;
        } else if (command == "get_collection") {
			break;
        } else if (command == "delete_collection") {
			break;
        } else if (command == "add_movie_to_collection") {
			break;
        } else if (command == "delete_movie_from_collection") {
			break;
        } else if (command == "logout") {
			logout(cookies, token);
		} else {
			cout << "Unknown command!" << endl;
		}
	}
	return 0;
}

#ifndef _COMMANDS_
#define _COMMANDS_

#include <string>
using namespace std;

#define HOST "63.32.125.183"
#define PORT 8081
#define PAYLOAD_TYPE "application/json"

void login_admin(string &cookies, string &token);
void add_user(string &cookies, string &token);
void get_users(string &cookies, string &token);
void delete_user(string &cookies, string &token);
void logout_admin(string &cookies, string &token);
void login(string &cookies, string &token);
void get_access(string &cookies, string &token);
void get_movies(string &cookies, string &token);
void get_movie(string &cookies, string &token);
void add_movie(string &cookies, string &token);
void delete_movie(string &cookies, string &token);
void update_movie(string &cookies, string &token);
void get_collections(string &cookies, string &token);
void get_collection(string &cookies, string &token);
void add_collection_helper(string &cookies, string &token,
						   string id_collection_string, string movie_id_string);
void add_collection(string &cookies, string &token);
void delete_collection(string &cookies, string &token);
void add_movie_to_collection(string &cookies, string &token);
void delete_movie_from_collection(string &cookies, string &token);
void logout(string &cookies, string &token);

#endif

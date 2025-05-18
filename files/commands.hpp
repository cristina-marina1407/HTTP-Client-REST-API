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

#endif

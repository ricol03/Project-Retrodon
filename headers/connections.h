#include "../curl/include/curl/curl.h"
#include "../cjson/cJSON.h"

#ifndef CONNECTIONS_H_
#define CONNECTIONS_H_

#define MAX_POSTS 64
#define MAX_STR 512

struct Memory {
    char * response;
    size_t size;
} typedef Memory;

struct Post {
    char createdAt[MAX_STR];
    char content[MAX_STR];
    char username[MAX_STR];
} typedef Post;

struct Account {
    char username[MAX_STR];
    char displayName[MAX_STR];
    char createdAt[MAX_STR];
    char note[MAX_STR];
    //char avatar[];
    //char header[];
    int followingNumber;
    int followersNumber;
} typedef Account;



static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp);

void createEndpoint(char * server, char * endpoint, char * argument);
void resetMemory(Memory * data);
void resetPosts(Post posts[]);

int accessPublicTimeline(char * server);
int accessPublicAccount(char * server, char * id);

int createApplication(char * server);
int getAccessToken(char * server);
int verifyCredentials(char * server);
int authorizeUser(char * server, HINSTANCE hinstance);

INT_PTR CALLBACK CodeDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);

#endif
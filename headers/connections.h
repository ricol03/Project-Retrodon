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
    char created_at[MAX_STR];
    char content[MAX_STR];
    char username[MAX_STR];
} typedef Post;

static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp);

void createEndpoint(char * server, char * endpoint);
void resetMemory(Memory * data, Post posts[]);

int accessPublicContent(char * server);
int createApplication(char * server);
int getAccessToken(char * server);
int verifyCredentials(char * server);
int authorizeUser(char * server, HINSTANCE hinstance);

INT_PTR CALLBACK CodeDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);

#endif
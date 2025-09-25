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
    wchar_t createdAt[MAX_STR];
    wchar_t content[MAX_STR];
    wchar_t username[MAX_STR];
} typedef Post;

struct Account {
    wchar_t username[MAX_STR];
    wchar_t displayName[MAX_STR];
    wchar_t createdAt[MAX_STR];
    wchar_t note[MAX_STR];
    //char avatar[];
    //char header[];
    int followingNumber;
    int followersNumber;
} typedef Account;



static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp);

void createEndpoint(wchar_t * server, wchar_t * endpoint, wchar_t * argument);
void resetMemory(Memory * data);
void resetPosts(Post posts[]);

int accessPublicTimeline(wchar_t * server);
int accessPublicAccount(wchar_t * server, wchar_t * id);

int createApplication(wchar_t * server);
int getAccessToken(wchar_t * server);
int verifyCredentials(wchar_t * server);
int authorizeUser(wchar_t * server, HINSTANCE hinstance);

#endif
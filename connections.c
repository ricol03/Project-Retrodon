#include "headers/tools.h"

//public content:       https://<server>/api/v1/timelines/public
//create application:   https://<server>/api/v1/apps
//access token:         https://<server>/oauth/token
//verify credentials:   https://<server>/api/v1/apps/verify_credentials
//authorize user:       https://<server>/oauth/authorize

Memory chunk = {0};
Memory chunk2 = {0};
Memory chunk3 = {0};
Memory data = {0};

char * client_id; 
char * client_secret;
char token[512];

wchar_t finallink[1024];
extern wchar_t authorizationCode[128];

Post posts[MAX_POSTS];
Account account;

extern PAINTSTRUCT ps;


static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp) {
    size_t totalSize = size * nmemb;
    struct Memory * mem = (struct Memory *)userp;

    char * ptr = realloc(mem->response, mem->size + totalSize + 1);
    if (ptr == NULL) return 0;  // out of memory

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, totalSize);
    mem->size += totalSize;
    mem->response[mem->size] = '\0';

    return totalSize;
}

void resetMemory(Memory * data) {
    memset(data, 0, sizeof(*data));
}

void resetPosts(Post posts[]) {
    memset(posts, 0, sizeof(posts));
}

//FIXME: need to fix the incorrect links that are created
void createEndpoint(wchar_t * server, wchar_t * endpoint, wchar_t * argument) {
    swprintf(finallink, sizeof(finallink) / sizeof(wchar_t), L"https://%s%s%s", server, endpoint, argument);
    MessageBox(NULL, finallink, L"Info", MB_ICONINFORMATION);
}


/* public connections */

int accessPublicTimeline(wchar_t * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        resetMemory(&data);
        resetPosts(posts);

        createEndpoint(server, L"/api/v1/timelines/public", L"?limit=64");
        curl_easy_setopt(curl, CURLOPT_URL, finallink);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        //TODO: do a while, while the curl perform failed and the user clicked retry
        /*while ()
        {}*/
        
        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Public content could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            //MessageBox(NULL, "Instance's info was retrieved", "Info", MB_ICONINFORMATION | MB_OK);

            cJSON * root = cJSON_Parse(data.response);

            if (root == NULL) {
                MessageBox(NULL, L"JSON is empty", L"Error", MB_ICONERROR);
            } else {
                if (!cJSON_IsArray(root)) {
                    printf("Error: expected array of posts\n");
                    return -1;
                }

                cJSON *item = NULL;
                size_t i = 0;

                cJSON_ArrayForEach(item, root) {
                    if (i >= MAX_POSTS) 
                        break;

                    cJSON * created  = cJSON_GetObjectItemCaseSensitive(item, "created_at");
                    cJSON * content  = cJSON_GetObjectItemCaseSensitive(item, "content");

                    cJSON * account  = cJSON_GetObjectItemCaseSensitive(item, "account");
                    cJSON * username = account ? cJSON_GetObjectItemCaseSensitive(account, "username") : NULL;

                    if (created && cJSON_IsString(created))
                        wcscpy(posts[i].createdAt, charToWchar(created->valuestring));
                    else
                        posts[i].createdAt[0] = '\0';

                    if (content && cJSON_IsString(content))
                        wcscpy(posts[i].content, charToWchar(content->valuestring));
                    else
                        posts[i].content[0] = '\0';

                    if (username && cJSON_IsString(username))
                        wcscpy(posts[i].username, charToWchar(username->valuestring));
                    else
                        posts[i].username[0] = '\0';

                    posts[i].createdAt[MAX_STR - 1] = '\0';
                    posts[i].content[MAX_STR - 1]    = '\0';
                    posts[i].username[MAX_STR - 1]   = '\0';

                    i++;
                }

                cJSON_Delete(root);
            }
        }
            
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    
    return 0;
}

int accessPublicAccount(wchar_t * server, wchar_t * id) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        resetMemory(&chunk2);
        createEndpoint(server, L"/api/v1/accounts/", id);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Instance's info could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            //MessageBox(NULL, client_id->valuestring, "Info", MB_ICONINFORMATION | MB_OK);

            printf("\n\n(TOKEN) Server response:\n%s\n", chunk2.response);

            cJSON *json = cJSON_Parse(chunk2.response);
            if (json) {

                cJSON * username  = cJSON_GetObjectItemCaseSensitive(json, "username");
                cJSON * display_name  = cJSON_GetObjectItemCaseSensitive(json, "display_name");

                cJSON * created_at  = cJSON_GetObjectItemCaseSensitive(json, "created_at");
                cJSON * note = cJSON_GetObjectItemCaseSensitive(json, "note");

                cJSON * following = cJSON_GetObjectItemCaseSensitive(json, "following_count");
                cJSON * followers = cJSON_GetObjectItemCaseSensitive(json, "followers_count");

                wcscpy(account.username, charToWchar(username->valuestring));
                wcscpy(account.displayName, charToWchar(display_name->valuestring));
                wcscpy(account.createdAt, charToWchar(created_at->valuestring));
                account.followingNumber = following->valueint;
                account.followersNumber = followers->valueint;

                char * final = malloc(MAX_STR * sizeof(char));
                removeHtml(note->valuestring, final);
                

                wcscpy(account.note, charToWchar(final));


                /*cJSON *access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
                if (cJSON_IsString(access_token) && (access_token->valuestring != NULL)) {
                    MessageBox(NULL, "Deu token", "Aviso", MB_ICONEXCLAMATION);
                    printf("Access token: %s\n", access_token->valuestring);
                    strcpy(token, access_token->valuestring);
                } else {
                    printf("No access_token in JSON\n");
                }*/
                cJSON_Delete(json);
            }


        }            
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

/*int loginProcedure() {
    createApplication(char * server)
    getAccessToken(char * server)
    verifyCredentials(char * server)
}*/

int createApplication(wchar_t * server) {
    /*curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/api/v1/apps");
        curl_easy_setopt(curl, CURLOPT_URL, finallink);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

        curl_mime * mime = curl_mime_init(curl);

        curl_mimepart * part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_name");
        curl_mime_data(part, "Retrodon", CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "redirect_uris");
        curl_mime_data(part, "urn:ietf:wg:oauth:2.0:oob", CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "scopes");
        curl_mime_data(part, "read write push", CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, "Instance's info could not be retrieved", "Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            MessageBox(NULL, "Instance's info was retrieved", "Info", MB_ICONINFORMATION | MB_OK);
            
            cJSON * json = cJSON_Parse(chunk.response);

            if (json == NULL) {
                MessageBox(NULL, "JSON is empty", "Error", MB_ICONERROR);
            } else {
                cJSON * id = cJSON_GetObjectItemCaseSensitive(json, "client_id");
                cJSON * secret = cJSON_GetObjectItemCaseSensitive(json, "client_secret");

                client_id = id->valuestring;
                client_secret = secret->valuestring;

                MessageBox(NULL, client_secret, client_id, MB_ICONINFORMATION);

                cJSON_Delete(json);
            }
        }
            
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();*/

    // instances usually rate limit the creation of these; manually add them here for now
    client_id = "ID_HERE";
    client_secret = "SECRET_HERE";

    return 0;
}   

int getAccessToken(wchar_t * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/oauth/token", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, finallink);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

        curl_mime * mime = curl_mime_init(curl);

        curl_mimepart * part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_id");
        curl_mime_data(part, client_id, CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_secret");
        curl_mime_data(part, client_secret, CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "grant_type");
        curl_mime_data(part, "client_credentials", CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Instance's info could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
            
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        } else {
            //printf("\n\n(TOKEN) Server response:\n%s\n", chunk2.response);

            cJSON *json = cJSON_Parse(chunk2.response);
            if (json) {
                cJSON *access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
                if (cJSON_IsString(access_token) && (access_token->valuestring != NULL)) {
                    MessageBox(NULL, L"Deu token", L"Aviso", MB_ICONEXCLAMATION);
                    printf("Access token: %s\n", access_token->valuestring);
                    strcpy(token, access_token->valuestring);
                } else {
                    printf("No access_token in JSON\n");
                }
                cJSON_Delete(json);
            }
        }            
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

int verifyCredentials(wchar_t * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/api/v1/apps/verify_credentials", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, finallink);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        char header_string[512];

        snprintf(header_string, sizeof(header_string), "Authorization: Bearer %s", token);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, header_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk3);

        CURLcode result = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Info could not be verified", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
            return -1;
        } else {
            printf("\n\nServer response:\n%s\n", chunk3.response);
        }            
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

int authorizeUser(wchar_t * server, HINSTANCE hinstance) {

    MessageBox(NULL, server, L"Info", MB_ICONASTERISK);

    wchar_t auth_url[512];
    swprintf(auth_url, sizeof(auth_url),
        L"https://%s/oauth/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=read+write+push",
        server, client_id, "urn:ietf:wg:oauth:2.0:oob");
    ShellExecute(NULL, L"open", auth_url, NULL, NULL, SW_SHOWNORMAL);

    int result = showCodeDialog(hinstance);

    if (result == IDB_CONTINUE_C) {
        MessageBox(NULL, authorizationCode, L"Info", MB_ICONEXCLAMATION);
        return 0;
    } else
        MessageBox(NULL, L"erro", L"erro", MB_ICONERROR);
        
    return -1;

}

/* login */


/*int getUserToken(char * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, "/oauth/token");
        curl_easy_setopt(curl, CURLOPT_URL, finallink);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

        curl_mime * mime = curl_mime_init(curl);

        curl_mimepart * part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_id");
        curl_mime_data(part, client_id, CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_secret");
        curl_mime_data(part, client_secret, CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "grant_type");
        curl_mime_data(part, "authorization_code", CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "redirect_uris");
        curl_mime_data(part, "urn:ietf:wg:oauth:2.0:oob", CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "code");
        curl_mime_data(part, authorizationCode, CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, "Instance's info could not be retrieved", "Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            //MessageBox(NULL, client_id->valuestring, "Info", MB_ICONINFORMATION | MB_OK);

            printf("\n\n(TOKEN) Server response:\n%s\n", chunk2.response);

            cJSON *json = cJSON_Parse(chunk2.response);
            if (json) {
                cJSON *access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
                if (cJSON_IsString(access_token) && (access_token->valuestring != NULL)) {
                    MessageBox(NULL, "Deu token", "Aviso", MB_ICONEXCLAMATION);
                    printf("Access token: %s\n", access_token->valuestring);
                    strcpy(token, access_token->valuestring);
                } else {
                    printf("No access_token in JSON\n");
                }
                cJSON_Delete(json);
            }


        }            
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}*/



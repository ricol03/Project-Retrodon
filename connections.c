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

char finallink[128];
extern char authorizationCode[128];

Post posts[MAX_POSTS];

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


void resetMemory(Memory * data, Post posts[]) {
    memset(data, 0, sizeof(*data));
    memset(posts, 0, sizeof(posts));
}

void createEndpoint(char * server, char * endpoint) {
    snprintf(finallink, sizeof(finallink), "https://%s%s", server, endpoint);
    MessageBox(NULL, finallink, "Info", MB_ICONINFORMATION);
}

int accessPublicContent(char * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        resetMemory(&data, posts);
        createEndpoint(server, "/api/v1/timelines/public?limit=64");
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
            MessageBox(NULL, "Public content could not be retrieved", "Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            //MessageBox(NULL, "Instance's info was retrieved", "Info", MB_ICONINFORMATION | MB_OK);

            cJSON * root = cJSON_Parse(data.response);

            if (root == NULL) {
                MessageBox(NULL, "JSON is empty", "Error", MB_ICONERROR);
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
                        strncpy(posts[i].created_at, created->valuestring, MAX_STR - 1);
                    else
                        posts[i].created_at[0] = '\0';

                    if (content && cJSON_IsString(content))
                        strncpy(posts[i].content, content->valuestring, MAX_STR - 1);
                    else
                        posts[i].content[0] = '\0';

                    if (username && cJSON_IsString(username))
                        strncpy(posts[i].username, username->valuestring, MAX_STR - 1);
                    else
                        posts[i].username[0] = '\0';

                    posts[i].created_at[MAX_STR - 1] = '\0';
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

int createApplication(char * server) {
    /*curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, "/api/v1/apps");
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

int getAccessToken(char * server) {
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
        curl_mime_data(part, "client_credentials", CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, "Instance's info could not be retrieved", "Error", MB_ICONERROR | MB_RETRYCANCEL);
            
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        } else {
            //printf("\n\n(TOKEN) Server response:\n%s\n", chunk2.response);

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
}

int verifyCredentials(char * server) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, "/api/v1/apps/verify_credentials");
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
            MessageBox(NULL, "Info could not be verified", "Error", MB_ICONERROR | MB_RETRYCANCEL);
            return -1;
        } else {
            printf("\n\nServer response:\n%s\n", chunk3.response);
        }            
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

int authorizeUser(char * server, HINSTANCE hinstance) {

    MessageBox(NULL, server, "Info", MB_ICONASTERISK);

    char auth_url[512];
    snprintf(auth_url, sizeof(auth_url),
        "https://%s/oauth/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=read+write+push",
        server, client_id, "urn:ietf:wg:oauth:2.0:oob");
    ShellExecuteA(NULL, "open", auth_url, NULL, NULL, SW_SHOWNORMAL);

    int result = showCodeDialog(hinstance);

    if (result == IDB_CONTINUE_C) {
        MessageBox(NULL, authorizationCode, "Info", MB_ICONEXCLAMATION);
        return 0;
    } else
        MessageBox(NULL, "erro", "erro", MB_ICONERROR);
        
    return -1;

}

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



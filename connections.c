#include "headers/tools.h"

//public content:       https://<server>/api/v1/timelines/public
//create application:   https://<server>/api/v1/apps
//access token:         https://<server>/oauth/token
//verify credentials:   https://<server>/api/v1/apps/verify_credentials
//authorize user:       https://<server>/oauth/authorize

// boolean that checks if the application was already created
BOOL createdApplication = FALSE;

Memory chunk = {0};
Memory chunk2 = {0};
Memory chunk3 = {0};
Memory data = {0};
Memory imageData = {0};

wchar_t client_id[128] = {0};  
wchar_t client_secret[128] = {0};
char public_token[512];
wchar_t user_token[128] = {0};

wchar_t finallink[2048];
extern wchar_t authorizationCode[256];

Post posts[MAX_POSTS];
Account userAccount;
Account account;

extern HWND hwindow[4];

extern PAINTSTRUCT ps;

BOOL runningCodeDialog = TRUE;


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
    memset(posts, 0, sizeof(Post) * MAX_POSTS);
}

void resetAccount(Account * account) {
    memset(account, 0, sizeof(* account));
}

void createEndpoint(wchar_t * server, wchar_t * endpoint, wchar_t * argument) {
    if (argument == NULL)
        swprintf(finallink, _countof(finallink), L"https://%ls%ls", server, endpoint);
    else
        swprintf(finallink, _countof(finallink), L"https://%ls%ls%ls", server, endpoint, argument);
}

void getImage(wchar_t * link) {
    resetMemory(&imageData);
    CURL * curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(link));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&imageData);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            MessageBox(NULL, link, L"ERROR", MB_ICONASTERISK);
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Instance's info could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else
            curl_easy_cleanup(curl);
    }
}

/* public connections */

int accessPublicTimeline(wchar_t * server) {

    CURL * curl = curl_easy_init();

    if (curl) {
        resetMemory(&data);
        resetPosts(posts);

        createEndpoint(server, L"/api/v1/timelines/public", L"?limit=64");
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        //TODO: do a while, while the curl performs failed and the user clicks retry
        /*while ()
        {}*/
        
        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Public content could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            cJSON * root = cJSON_Parse(data.response);

            if (root == NULL) {
                MessageBox(NULL, L"JSON is empty", L"Error", MB_ICONERROR);
            } else {
                if (!cJSON_IsArray(root)) {
                    printf("Error: expected array of posts\n");
                    return -1;
                }

                cJSON * item = NULL;
                size_t i = 0;

                cJSON_ArrayForEach(item, root) {
                    if (i >= MAX_POSTS)
                        break;

                    cJSON * created  = cJSON_GetObjectItemCaseSensitive(item, "created_at");
                    cJSON * content  = cJSON_GetObjectItemCaseSensitive(item, "content");

                    cJSON * account  = cJSON_GetObjectItemCaseSensitive(item, "account");
                    cJSON * username = account ? cJSON_GetObjectItemCaseSensitive(account, "username") : NULL;
                    
                    cJSON * id = account ? cJSON_GetObjectItemCaseSensitive(account, "id") : NULL;

                    if (created && cJSON_IsString(created)) {
                        wcscpy(posts[i].createdAt, charToWchar(removeLetters(created->valuestring)));
                    } else
                        posts[i].createdAt[0] = '\0';

                    if (content && cJSON_IsString(content))
                        wcscpy(posts[i].content, charToWchar(removeHtml(content->valuestring)));
                    else
                        posts[i].content[0] = '\0';

                    if (username && cJSON_IsString(username))
                        wcscpy(posts[i].username, charToWchar(username->valuestring));
                    else
                        posts[i].username[0] = '\0';

                    if (id && cJSON_IsString(id))
                        wcscpy(posts[i].id, charToWchar(id->valuestring));
                    else
                        posts[i].id[0] = '\0';

                    posts[i].createdAt[MAX_STR - 1]  = '\0';
                    posts[i].content[MAX_STR - 1]    = '\0';
                    posts[i].username[MAX_STR - 1]   = '\0';
                    posts[i].id[MAX_STR - 1]         = '\0';
                    
                    i++;
                }

                cJSON_Delete(root);
            }
        }
            
        curl_easy_cleanup(curl);
    }

    //resetMemory(&data);

    return 0;
}

int accessPublicAccount(wchar_t * server, wchar_t * id) {

    CURL * curl = curl_easy_init();

    if (curl) {
        resetAccount(&account);
        resetMemory(&chunk2);
        createEndpoint(server, L"/api/v1/accounts/", id);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Instance's info could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            cJSON * json = cJSON_Parse(chunk2.response);
            if (json) {
                cJSON * id = cJSON_GetObjectItemCaseSensitive(json, "id");

                cJSON * username  = cJSON_GetObjectItemCaseSensitive(json, "username");
                cJSON * display_name  = cJSON_GetObjectItemCaseSensitive(json, "display_name");

                cJSON * created_at  = cJSON_GetObjectItemCaseSensitive(json, "created_at");
                cJSON * note = cJSON_GetObjectItemCaseSensitive(json, "note");

                cJSON * avatar_url = cJSON_GetObjectItemCaseSensitive(json, "avatar");
                cJSON * banner_url = cJSON_GetObjectItemCaseSensitive(json, "header");

                cJSON * following = cJSON_GetObjectItemCaseSensitive(json, "following_count");
                cJSON * followers = cJSON_GetObjectItemCaseSensitive(json, "followers_count");

                wcscpy(account.id, charToWchar(id->valuestring));
                wcscpy(account.username, charToWchar(username->valuestring));
                wcscpy(account.displayName, charToWchar(display_name->valuestring));
                wcscpy(account.createdAt, charToWchar(created_at->valuestring));
                wcscpy(account.note, charToWchar(removeHtml(note->valuestring)));
                wcscpy(account.avatarUrl, charToWchar(avatar_url->valuestring));
                wcscpy(account.bannerUrl, charToWchar(banner_url->valuestring));
                account.followingNumber = following->valueint;
                account.followersNumber = followers->valueint;
            }
        }            
        
        curl_easy_cleanup(curl);
    }

    return 0;
}



int createApplication(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/api/v1/apps", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
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
            MessageBox(NULL, L"Instance's info could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            MessageBox(NULL, L"Instance's info was retrieved", L"Info", MB_ICONINFORMATION | MB_OK);
            
            cJSON * json = cJSON_Parse(chunk.response);

            if (json == NULL) {
                MessageBox(NULL, L"JSON is empty", L"Error", MB_ICONERROR);
            } else {
                cJSON * id = cJSON_GetObjectItemCaseSensitive(json, "client_id");
                cJSON * secret = cJSON_GetObjectItemCaseSensitive(json, "client_secret");

                wcscpy(client_id, charToWchar(id->valuestring));
                wcscpy(client_secret, charToWchar(secret->valuestring));

                MessageBox(NULL, client_secret, client_id, MB_ICONINFORMATION);

                cJSON_Delete(json);
            }
        }
            
        curl_easy_cleanup(curl);
    }

    saveSecrets();

    return 0;
}   

int getAccessToken(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/oauth/token", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

        curl_mime * mime = curl_mime_init(curl);

        curl_mimepart * part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_id");
        curl_mime_data(part, wcharToChar(client_id), CURL_ZERO_TERMINATED);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "client_secret");
        curl_mime_data(part, wcharToChar(client_secret), CURL_ZERO_TERMINATED);

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
                    printf("Access token: %s\n", access_token->valuestring);
                    strcpy(public_token, access_token->valuestring);
                } else {
                    printf("No access_token in JSON\n");
                }
                cJSON_Delete(json);
            }
        }            
        
        curl_easy_cleanup(curl);
    }

    return 0;
}

int verifyCredentials(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/api/v1/apps/verify_credentials", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        char header_string[512];

        snprintf(header_string, sizeof(header_string), "Authorization: Bearer %s", public_token);

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

    return 0;
}

int authorizeUser(wchar_t * server) {

    wchar_t auth_url[512];
    swprintf(auth_url, sizeof(auth_url),
        L"https://%ls/oauth/authorize?response_type=code&client_id=%ls&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=read+write+push",
        server, client_id);
    ShellExecute(NULL, L"open", auth_url, NULL, NULL, SW_SHOWNORMAL);

    ShowWindow(hwindow[3], SW_SHOW);
    UpdateWindow(hwindow[3]);

    MSG msg;

    while (runningCodeDialog && GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwindow[4], &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
        
    if (wcslen(authorizationCode) > 0) {
        MessageBox(NULL, authorizationCode, L"Info", MB_OK);
        return 0;
    } else {
        MessageBox(NULL, L"Code not found", L"Error", MB_OK);
        return 1;
    }
    
}

/* login */

int loginProcedure(wchar_t * server) {

    if (!createdApplication) {
        if (createApplication(server)) {
            MessageBox(hwindow[0], L"Could not create application!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);
            return 0;
        } else
            createdApplication = TRUE;
    }

    if (!authorizeUser(server)) {
        if (!getUserToken(server)) {
            saveToken();
            return 1;
        
        } else
            MessageBox(hwindow[0], L"Could not get user token!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);

    } else
        MessageBox(hwindow[0], L"Could not authorize user!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);

    return 0;
}

int getUserToken(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/oauth/token", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

        struct curl_slist * headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "User-Agent: curl/7.0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        char postfields[4096];
        snprintf(postfields, sizeof(postfields),
            "grant_type=authorization_code"
            "&client_id=%s"
            "&client_secret=%s"
            "&redirect_uri=urn:ietf:wg:oauth:2.0:oob"
            "&code=%s",
            wcharToChar(client_id),
            wcharToChar(client_secret),
            wcharToChar(authorizationCode));

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postfields));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
        } else {
            printf("\n(TOKEN) Server response:\n%s\n", chunk2.response);

            cJSON * json = cJSON_Parse(chunk2.response);
            if (json) {
                cJSON * access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
                if (cJSON_IsString(access_token) && access_token->valuestring) {
                    printf("Access token: %s\n", access_token->valuestring);
                    wcscpy(user_token, charToWchar(access_token->valuestring));
                }
                cJSON_Delete(json);
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    if (wcslen(user_token) > 0)
        return 0;
    else
        return 1;
    
}

/* user connections */

int getUserProfile(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        createEndpoint(server, L"/api/v1/accounts/verify_credentials", NULL);
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        char header_string[512];

        snprintf(header_string, sizeof(header_string), "Authorization: Bearer %ls", user_token);

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
            cJSON * json = cJSON_Parse(chunk3.response);
            if (json) {
                cJSON * id = cJSON_GetObjectItemCaseSensitive(json, "id");

                cJSON * username  = cJSON_GetObjectItemCaseSensitive(json, "username");
                cJSON * display_name  = cJSON_GetObjectItemCaseSensitive(json, "display_name");

                cJSON * created_at  = cJSON_GetObjectItemCaseSensitive(json, "created_at");
                cJSON * note = cJSON_GetObjectItemCaseSensitive(json, "note");

                cJSON * avatar_url = cJSON_GetObjectItemCaseSensitive(json, "avatar");
                cJSON * banner_url = cJSON_GetObjectItemCaseSensitive(json, "header");

                cJSON * following = cJSON_GetObjectItemCaseSensitive(json, "following_count");
                cJSON * followers = cJSON_GetObjectItemCaseSensitive(json, "followers_count");

                wcscpy(userAccount.id, charToWchar(id->valuestring));
                wcscpy(userAccount.username, charToWchar(username->valuestring));
                wcscpy(userAccount.displayName, charToWchar(display_name->valuestring));
                wcscpy(userAccount.createdAt, charToWchar(created_at->valuestring));
                wcscpy(userAccount.note, charToWchar(removeHtml(note->valuestring)));
                wcscpy(userAccount.avatarUrl, charToWchar(avatar_url->valuestring));
                wcscpy(userAccount.bannerUrl, charToWchar(banner_url->valuestring));
                userAccount.followingNumber = following->valueint;
                userAccount.followersNumber = followers->valueint;
            }
        }            
        
        curl_easy_cleanup(curl);
    }

    return 0;
}

int accessUserTimeline(wchar_t * server) {
    CURL * curl = curl_easy_init();

    if (curl) {
        resetMemory(&data);
        resetPosts(posts);

        createEndpoint(server, L"/api/v1/timelines/home", L"?limit=40");
        curl_easy_setopt(curl, CURLOPT_URL, wcharToChar(finallink));
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        char header_string[512];
        snprintf(header_string, sizeof(header_string), "Authorization: Bearer %ls", user_token);

        struct curl_slist * headers = NULL;
        headers = curl_slist_append(headers, header_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        //TODO: do a while, while the curl performs failed and the user clicks retry
        /*while ()
        {}*/
        
        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            MessageBox(NULL, L"Public content could not be retrieved", L"Error", MB_ICONERROR | MB_RETRYCANCEL);
        } else {
            //printf("JSON: %s", data.response);

            cJSON * root = cJSON_Parse(data.response);

            if (root == NULL) {
                MessageBox(NULL, L"JSON is empty", L"Error", MB_ICONERROR);
            } else {
                if (!cJSON_IsArray(root)) {
                    printf("Error: expected array of posts\n");
                    return -1;
                }

                cJSON * item = NULL;
                cJSON * reblog = NULL;
                size_t i = 0;

                cJSON_ArrayForEach(item, root) {
                    if (i >= MAX_POSTS)
                        break;

                    cJSON * created  = cJSON_GetObjectItemCaseSensitive(item, "created_at");
                    cJSON * content  = cJSON_GetObjectItemCaseSensitive(item, "content");

                    cJSON * account  = cJSON_GetObjectItemCaseSensitive(item, "account");
                    cJSON * username = account ? cJSON_GetObjectItemCaseSensitive(account, "username") : NULL;
                    
                    cJSON * id = account ? cJSON_GetObjectItemCaseSensitive(account, "id") : NULL;

                    cJSON * reblog  = cJSON_GetObjectItemCaseSensitive(item, "reblog");

                    if (reblog && cJSON_IsObject(reblog))
                        posts[i].reblog = TRUE;
                    else
                        posts[i].reblog = FALSE;

                    if (created && cJSON_IsString(created)) {
                        wcscpy(posts[i].createdAt, charToWchar(removeLetters(created->valuestring)));
                    } else
                        posts[i].createdAt[0] = '\0';

                    if (content && cJSON_IsString(content))
                        wcscpy(posts[i].content, charToWchar(removeHtml(content->valuestring)));
                    else
                        posts[i].content[0] = '\0';

                    if (username && cJSON_IsString(username))
                        wcscpy(posts[i].username, charToWchar(username->valuestring));
                    else
                        posts[i].username[0] = '\0';

                    if (id && cJSON_IsString(id))
                        wcscpy(posts[i].id, charToWchar(id->valuestring));
                    else
                        posts[i].id[0] = '\0';

                    posts[i].createdAt[MAX_STR - 1]  = '\0';
                    posts[i].content[MAX_STR - 1]    = '\0';
                    posts[i].username[MAX_STR - 1]   = '\0';
                    posts[i].id[MAX_STR - 1]         = '\0';
                    
                    i++;
                }

                cJSON_Delete(root);
            }
        }
            
        curl_easy_cleanup(curl);
    }

    //resetMemory(&data);

    return 0;
}

int accessLocalTimeline(wchar_t * server) {

}

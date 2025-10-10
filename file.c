#include "headers/tools.h"

FILE * fr;
FILE * fw;

extern size_t stringsize;

extern DWORD wversion, wmajorversion, wminorversion, wbuild;

extern wchar_t serverAddress[128];
extern wchar_t client_id[128];  
extern wchar_t client_secret[128];

extern wchar_t user_token[128];

wchar_t provider[32];
wchar_t protocol[6];
wchar_t port[6];

wchar_t lang[5];

wchar_t appdata[128], filepath[128];

void createDirectory() {
    if (wmajorversion == 5) {
        HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata);
        wcscat(appdata, L"\\ProjectRetrodon\\");

        if (checkDirectory(appdata)) {
            BOOL check = CreateDirectory((LPCWSTR)appdata, NULL);

            if (check == 0) {
                MessageBox(NULL, L"The folder could not be created", L"Error", MB_ICONERROR);
                printf("\n\nGetLastError: %lu\n\n", GetLastError());
            }
        }
    } else {
        HRESULT result = SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, appdata);
        wcscat(appdata, L"\\AppData\\Local\\ProjectRetrodon\\");

        if (checkDirectory(appdata) == -1) {
            BOOL check = CreateDirectory((LPCWSTR)appdata, NULL);

            if (check == 0) {
                MessageBox(NULL, L"The folder could not be created", L"Error", MB_ICONERROR);
                printf("\n\nGetLastError: %lu\n\n", GetLastError());
            }
        }
    }
}

int checkDirectory(wchar_t pathname[]) {
    DWORD check = GetFileAttributes(pathname);
    return (int)check;
}

wchar_t * checkImage(wchar_t * showid) {
    /*wchar_t * imagepath = (wchar_t *)malloc(128 * sizeof(wchar_t));
    wchar_t * imagepathaux = (wchar_t *)malloc(128 * sizeof(wchar_t));

    wcscpy(imagepath, appdata);
    wcscat(imagepath, showid);

    wcscpy(imagepathaux, imagepath);
    wcscat(imagepath, L".jpg");

    printf("\n\nchegou ao checkimage");

    DWORD check = GetFileAttributes(imagepath);
    if (check != -1) {
        printf("   \n\n sabeque é jpg");
        return imagepath;
    } else {
        wcscpy(imagepath, imagepathaux);
        wcscat(imagepath, L".png");

        DWORD check = GetFileAttributes(imagepath);
        if (check != -1) {
            return imagepath;
        } else {
            wcscpy(imagepath, imagepathaux);
            wcscat(imagepath, L".gif");

            DWORD check = GetFileAttributes(imagepath);
            if (check == -1) {
                return NULL;
            } else {
                return imagepath;
            }
        }
    }*/
}


//TODO: I need to know the file extension of the file to create
wchar_t * createImagePath(wchar_t * showid, wchar_t * fileextension) {

    /*printf("\n\n\nfoi corajoso e chegou aqui");

    wchar_t * filepath = (wchar_t *)malloc(128 * sizeof(wchar_t));
    wcscpy(filepath, appdata);
    wcscat(filepath, showid);
    wcscat(filepath, fileextension);

    return filepath;*/

    /*DWORD byteswritten;
    HANDLE hf = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    //FILE * fw = fopen(filepath, "w+b");

    if (hf != NULL) {
        WriteFile(hf, imagedata, stringsize, &byteswritten, NULL);z
        //fwrite(imagedata, 1, 1, fw);
        //fclose(fw);z
        CloseHandle(hf);
        return filepath;
    } else {
        MessageBox(NULL, L"The file could not be created", L"Error", MB_ICONERROR);
        fclose(fw);
        return NULL;
    }*/

    /*HANDLE hf = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, L"The file could not be created", L"Error", MB_ICONERROR);
        free(filepath);
        return NULL;
    }

    // Escreve os dados da imagem no arquivo
    DWORD byteswritten;
    BOOL result = WriteFile(hf, imagedata, stringsize, &byteswritten, NULL);
    if (!result || byteswritten != stringsize) {
        MessageBox(NULL, L"Failed to write the image data to the file", L"Error", MB_ICONERROR);
        CloseHandle(hf);
        free(filepath);
        return NULL;
    }

    // Fecha o arquivo
    CloseHandle(hf);*/

}

void readyingFile() {
    fr = _wfopen(SETTINGSFILENAME, L"r");
    //fw = fopen(SETTINGSFILENAME, "w");
}

int readSettings() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fr = _wfopen(filepath, L"r+b");

    if (fr != NULL) {
        wchar_t buffer[MAX_STR];
        wchar_t * line;
        int lineCount = 0;

        while (fgetws(buffer, MAX_STR, fr) != NULL) {
            buffer[wcslen(buffer) - 1] = L'\0';

            if (lineCount == 0) {
                wcscpy(serverAddress, buffer);
                fclose(fr);
                return 1;
            }
        }
    } else
        MessageBox(NULL, L"Settings not found", L"Error", MB_ICONERROR);
    
    fclose(fr);
    return 0;
}

int saveSettings() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fw = _wfopen(filepath, L"wb");
    
    if (serverAddress != NULL)
        fwprintf(fw, L"%ls\n", serverAddress);
    
    fclose(fw);
}

int saveSecrets() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fw = _wfopen(filepath, L"a+b");

    if (fw == NULL) {
        MessageBox(NULL, L"Failed to open file for writing", L"Error", MB_ICONERROR);
        return 0;
    }

    if (client_id != NULL && client_secret != NULL) {
        fwprintf(fw, L"%ls\n", client_id);
        fwprintf(fw, L"%ls\n", client_secret);
    }

    fclose(fw);
    return 1;
}

int readSecrets() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fr = _wfopen(filepath, L"r+b");

    if (fr == NULL) {
        MessageBox(NULL, L"Settings file not found", L"Error", MB_ICONERROR);
        return 0;
    }

    wchar_t buffer[MAX_STR];
    wchar_t * line;
    int lineCount = 0;

    while (fgetws(buffer, MAX_STR, fr) != NULL) {
        buffer[wcslen(buffer) - 1] = L'\0';

        printf("%s", buffer);

        if (lineCount == 1) {
            wcscpy(client_id, buffer);
            MessageBox(NULL, client_id, L"Info", MB_OK);
        } else if (lineCount == 2)
            wcscpy(client_secret, buffer);
        
        lineCount++;
        if (lineCount > 2) break;
    }

    MessageBox(NULL, client_id, L"Error", MB_ICONERROR);
    MessageBox(NULL, client_secret, L"Error", MB_ICONERROR);

    if (client_id[0] == L'\0' || client_secret[0] == L'\0')
        return 0;
    
    fclose(fr);
    return 1;
}

int saveToken() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fw = _wfopen(filepath, L"a+b");

    if (fw == NULL) {
        MessageBox(NULL, L"Failed to open file for writing", L"Error", MB_ICONERROR);
        return 0;
    }

    if (user_token != NULL)
        fwprintf(fw, L"%ls\n", user_token);
    
    fclose(fw);
    return 1;
}

int readToken() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fr = _wfopen(filepath, L"r+b");

    if (fr == NULL) {
        MessageBox(NULL, L"Settings file not found", L"Error", MB_ICONERROR);
        return 0;
    }

    wchar_t buffer[MAX_STR];
    wchar_t * line;
    int lineCount = 0;

    while (fgetws(buffer, MAX_STR, fr) != NULL) {
        buffer[wcslen(buffer) - 1] = L'\0';

        if (lineCount == 3)
            wcscpy(user_token, buffer);

        lineCount++;
        if (lineCount > 3) break;
    }

    MessageBox(NULL, user_token, L"Error", MB_ICONERROR);

    if (user_token[0] == L'\0')
        return 0;
    
    fclose(fr);
    return 1;
}
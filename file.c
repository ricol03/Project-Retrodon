#include "headers/tools.h"

FILE * fr;
FILE * fw;

extern size_t stringsize;

extern DWORD wversion, wmajorversion, wminorversion, wbuild;

extern wchar_t serverAddress[128];
extern char * client_id;  
extern char * client_secret;

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

    wchar_t * buffer = malloc(MAX_STR);

    if (fr != NULL) {
        int bytesread = fread(buffer, sizeof(wchar_t), MAX_STR - 1, fr);
        buffer[bytesread] = '\0';
        wcscpy(serverAddress, buffer);
        return 1;
    } else {
        MessageBox(NULL, L"Settings not found", L"Error", MB_ICONERROR);
    } 

    free(buffer);
    fclose(fr);
    return 0;
}

int saveSettings() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fw = _wfopen(filepath, L"wb");
    
    if (serverAddress != NULL)
        fwrite(serverAddress, _countof(serverAddress), 1, fw);
    
    printf("\n%ls", serverAddress);

    fclose(fw);
}

int saveSecrets() {
    swprintf(filepath, sizeof(filepath), L"%ls/%ls", appdata, SETTINGSFILENAME);
    fw = _wfopen(filepath, L"a+b");
    
    if (client_id != NULL && client_secret != NULL) {
        fwrite(client_id, sizeof(client_id), 1, fw);
        fwrite(client_secret, sizeof(client_secret), 1, fw);
    }

    fclose(fw);
}

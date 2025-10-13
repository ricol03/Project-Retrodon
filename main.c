#include "headers/tools.h"
#define STB_IMAGE_IMPLEMENTATION
#include "headers/stb_image.h"

//TODO: do i686 compilation
//TODO: solve ram leak

BOOL loggedIn = FALSE;
extern BOOL createdApplication;

const LPCWSTR MAIN_CLASS           = L"MainWndClass";
const LPCWSTR INSTANCE_CLASS       = L"InstanceWndClass";
const LPCWSTR ACCOUNT_CLASS        = L"AccountWndClass";
const LPCWSTR CODE_CLASS           = L"CodeWndClass";

// window handles
// 0 - main window
// 1 - instance dialog
// 2 - account dialog
// 3 - code dialog
HWND hwindow[4];

// main window controls

extern HWND hmainControls[6];

HWND hinstance_edit, hinstance_title, hinstance_subtitle, hinstance_button;

extern HWND hfollow_button, hfollowing_static, hfollowers_static, hdisplayname_static, hname_static, hnote_static, havatar_area, hbanner_area, hok_button;

extern HWND hcodeControls[12];

PAINTSTRUCT ps;

HINSTANCE glhinstance;

wchar_t serverAddress[128] = {0};
wchar_t authorizationCode[256] = {0};

DWORD wversion, wmajorversion, wminorversion, wbuild;

extern Post posts[64];
extern Account account;
extern Account userAccount;

extern Memory imageData;

extern wchar_t finallink[2048];

Image avatar;
Image banner;

extern char public_token[512];
extern wchar_t user_token[128];

extern BOOL runningCodeDialog;

BOOL clickedUserProfile = FALSE;



//TODO: could do a separate file for dialog windows 

/*int showInstanceDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_INSTANCE), hwndmain, InstanceDialogProc);
}
int showCodeDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_CODE), NULL, CodeDialogProc);
}
int showAccountDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_ACCOUNT), NULL, CodeDialogProc);
}*/

//TODO: could do a separate file for image manipulation

HBITMAP CreateHbitmapFromPixels(unsigned char * pixels, int srcW, int srcH, int dstW, int dstH) {
    HDC hdc = GetDC(NULL);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = srcW;
    bmi.bmiHeader.biHeight = -srcH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pvBits = NULL;
    HBITMAP hSrcBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (hSrcBmp && pvBits) {
        uint8_t *src = (uint8_t*)pixels;
        uint8_t *dst = (uint8_t*)pvBits;
        int pxCount = srcW * srcH;
        for (int i = 0; i < pxCount; ++i) {
            uint8_t r = src[4*i + 0];
            uint8_t g = src[4*i + 1];
            uint8_t b = src[4*i + 2];
            uint8_t a = src[4*i + 3];
            dst[4*i + 0] = b;
            dst[4*i + 1] = g;
            dst[4*i + 2] = r;
            dst[4*i + 3] = a;
        }
    }

    HDC hdcSrc = CreateCompatibleDC(hdc);
    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, hSrcBmp);

    BITMAPINFO dbmi = bmi;
    dbmi.bmiHeader.biWidth = dstW;
    dbmi.bmiHeader.biHeight = -dstH;

    void *dstBits = NULL;
    HBITMAP hDstBmp = CreateDIBSection(hdc, &dbmi, DIB_RGB_COLORS, &dstBits, NULL, 0);

    HDC hdcDst = CreateCompatibleDC(hdc);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hDstBmp);

    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rc = {0, 0, dstW, dstH};
    FillRect(hdcDst, &rc, hBrush);

    float aspect = (float)srcW / (float)srcH;

    int newW = dstW;
    int newH = (int)(dstW / aspect);

    if (newH < dstH) {
        newH = dstH;
        newW = (int)(dstH * aspect);
    }

    int x = (dstW - newW) / 2;
    int y = (dstH - newH) / 2;

    SetStretchBltMode(hdcDst, HALFTONE);
    StretchBlt(hdcDst, x, y, newW, newH,
               hdcSrc, 0, 0, srcW, srcH,
               SRCCOPY);

    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    DeleteObject(hSrcBmp);
    ReleaseDC(NULL, hdc);

    return hDstBmp;
}

int preparingApplication() {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    createDirectory();

    if (readSecrets())
        createdApplication = TRUE;

    if (readToken())
        loggedIn = TRUE;

    if (!readSettings()) {

        createFonts();

        #pragma region InstanceWindow

        WNDCLASS instancewindowclass = { 0 };

        instancewindowclass.style            = CS_OWNDC;
        instancewindowclass.lpfnWndProc      = InstanceWindowProc;
        instancewindowclass.hInstance        = glhinstance;
        instancewindowclass.lpszClassName    = (LPCWSTR)INSTANCE_CLASS;

        RegisterClass(&instancewindowclass);

        hwindow[1] = CreateWindowEx(
            0,
            INSTANCE_CLASS,
            L"Instance",
            WS_OVERLAPPED | WS_CAPTION,
            (GetSystemMetrics(SM_CXSCREEN) / 2) - 200 , (GetSystemMetrics(SM_CYSCREEN) / 2) - 90, 400, 180,
            NULL,
            NULL,
            glhinstance,
            NULL
        );
        
        if (hwindow[1] == NULL) {
            wchar_t error[512];
            swprintf(error, 512, L"Error: %lu", GetLastError());
            MessageBox(NULL, error, L"Error", MB_ICONERROR);
            MessageBox(NULL, L"Unable to create window", 
                    L"Error", MB_ICONERROR | MB_OK);
            return 0;
        }

        #pragma endregion

        ShowWindow(hwindow[1], SW_SHOW);
        UpdateWindow(hwindow[1]);

        MSG msg;
        while (IsWindow(hwindow[1]) && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    checkVersion();

    return 1;
}

void createAccountWindow() {
    hwindow[2] = CreateWindowEx(
        0,
        ACCOUNT_CLASS,
        L"Account",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - 300, (GetSystemMetrics(SM_CYSCREEN) / 2) - 208, 600, 415,
        NULL,
        NULL,
        glhinstance,
        NULL
    );
    
    if (hwindow[2] == NULL) {
        wchar_t error[512];
        swprintf(error, 512, L"Error: %lu", GetLastError());
        MessageBox(NULL, error, L"Error", MB_ICONERROR);
        MessageBox(NULL, L"Unable to create window", 
                L"Error", MB_ICONERROR | MB_OK);
        //return 0;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nShowCmd);
}

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PWSTR lpcmdline, int nshowcmd) {

    curl_global_init(CURL_GLOBAL_ALL);

    glhinstance = hinstance;

    if (preparingApplication()) {

        #pragma region MainWindow
        WNDCLASS mainwindowclass = { 0 };

        mainwindowclass.style            = CS_OWNDC;
        mainwindowclass.lpfnWndProc      = MainWindowProc;
        mainwindowclass.hInstance        = glhinstance;
        mainwindowclass.lpszClassName    = (LPCWSTR)MAIN_CLASS;

        RegisterClass(&mainwindowclass);

        hwindow[0] = CreateWindowEx(
            0,
            MAIN_CLASS,
            L"Retrodon",
            WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_ICONIC | WS_ACTIVECAPTION | WS_VISIBLE,
            (GetSystemMetrics(SM_CXSCREEN) / 2) - 250, (GetSystemMetrics(SM_CYSCREEN) / 2) - 250, 500, 500,
            NULL,
            NULL,
            glhinstance,
            NULL
        );
        
        if (hwindow[0] == NULL) {
            wchar_t error[512];
            swprintf(error, 512, L"Error: %lu", GetLastError());
            MessageBox(NULL, error, L"Error", MB_ICONERROR);
            MessageBox(NULL, L"Unable to create window", 
                    L"Error", MB_ICONERROR | MB_OK);
            return 0;
        }
            
        #pragma endregion

        #pragma region AccountWindow
        WNDCLASS accountwindowclass = { 0 };

        accountwindowclass.style            = CS_OWNDC;
        accountwindowclass.lpfnWndProc      = AccountWindowProc;
        accountwindowclass.hInstance        = glhinstance;
        accountwindowclass.lpszClassName    = (LPCWSTR)ACCOUNT_CLASS;

        RegisterClass(&accountwindowclass);

        #pragma endregion

        #pragma region CodeWindow
        WNDCLASS codewindowclass = { 0 };

        codewindowclass.style            = CS_OWNDC;
        codewindowclass.lpfnWndProc      = CodeWindowProc;
        codewindowclass.hInstance        = glhinstance;
        codewindowclass.lpszClassName    = (LPCWSTR)CODE_CLASS;

        RegisterClass(&codewindowclass);

        hwindow[3] = CreateWindowEx(
            0,
            CODE_CLASS,
            L"Code insertion",
            WS_OVERLAPPED | WS_CAPTION,
            (GetSystemMetrics(SM_CXSCREEN) / 2) - 200, (GetSystemMetrics(SM_CYSCREEN) / 2) - 90, 400, 180,
            NULL,
            NULL,
            glhinstance,
            NULL
        );
        
        if (hwindow[3] == NULL) {
            wchar_t error[512];
            swprintf(error, 512, L"Error: %lu", GetLastError());
            MessageBox(NULL, error, L"Error", MB_ICONERROR);
            MessageBox(NULL, L"Unable to create window", 
                    L"Error", MB_ICONERROR | MB_OK);
            return 0;
        }
        #pragma endregion

    }   

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } 

    curl_global_cleanup();

    return 0;
}

LRESULT CALLBACK MainWindowProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    
    switch (message) {
        case WM_CREATE: {
            if (loggedIn) {
                getUserProfile(serverAddress);
                accessUserTimeline(serverAddress);
            } else
                accessPublicTimeline(serverAddress);

            homeWindow(hwnd);

            for (int i = 0; i < MAX_POSTS; i++) {
                LVITEM item = {0};
                item.mask = LVIF_TEXT;
                item.iItem = i;
                item.pszText = L"Example post";
                ListView_InsertItem(hmainControls[3], &item);
            }

            ListView_SetItemCount(hmainControls[3], MAX_POSTS);

            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lparam;

            switch (pnmh->code) {
                case WM_VSCROLL: {

                    int topIndex = SendMessage(hmainControls[3], LB_GETTOPINDEX, 0, 0);
                    int visibleCount = SendMessage(hmainControls[3], LB_GETCOUNT, 0, 0);

                    RECT rc;
                    GetClientRect(hmainControls[3], &rc);

                    int itemHeight = SendMessage(hmainControls[3], LB_GETITEMHEIGHT, 0, 0);
                    int itemsVisible = (rc.bottom - rc.top) / itemHeight;

                    if (topIndex + itemsVisible >= visibleCount) {
                        printf("Reached end of list!\n");
                    }

                }


                case NM_DBLCLK: {
                    LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lparam;

                    // index of the selected item
                    int iItem = pia->iItem;
                    if (iItem >= 0) {
                        wchar_t buffer[MAX_STR];
                        ListView_GetItemText(pia->hdr.hwndFrom, iItem, 3, buffer, MAX_STR);

                        accessPublicAccount(serverAddress, buffer);

                        createAccountWindow();

                    }
                    break;
                }

                case LVN_GETDISPINFO: {
                    if (pnmh->idFrom == IDC_LISTVIEW) {
                        NMLVDISPINFO *plvdi = (NMLVDISPINFO *)lparam;
                        if (plvdi->item.mask & LVIF_TEXT) {

                            int row = plvdi->item.iItem;
                            int col = plvdi->item.iSubItem;

                            if (col == 0)
                                plvdi->item.pszText = (LPWSTR)posts[plvdi->item.iItem].username;
                            else if (col == 1 && posts[plvdi->item.iItem].reblog == FALSE)
                                plvdi->item.pszText = (LPWSTR)posts[plvdi->item.iItem].content;
                            else if (col == 1 && posts[plvdi->item.iItem].reblog == TRUE)
                                plvdi->item.pszText = (LPWSTR)L"(Reblogged post)";
                            else if (col == 2)
                                plvdi->item.pszText = (LPWSTR)posts[plvdi->item.iItem].createdAt;
                            else 
                                plvdi->item.pszText = (LPWSTR)posts[plvdi->item.iItem].id;
                        }
                    }
                }
            }
        }
        break;

        case WM_COMMAND: {
            switch (LOWORD(wparam)) {

                /* buttons */
                case IDP_AVATAR_M: {
                    if (HIWORD(wparam) == STN_CLICKED) {
                        clickedUserProfile = TRUE;
                        getUserProfile(serverAddress);

                        createAccountWindow();

                        clickedUserProfile = FALSE;
                    }
                }
                break;

                case IDB_LOGIN: {
                    if (loginProcedure(serverAddress)) {
                        loggedIn = TRUE;

                        resetHomeWindow();
                        getUserProfile(serverAddress);
                        
                        RECT rc;
                        GetClientRect(hwindow[0], &rc);

                        homeWindow(hwindow[0]);
                        SendMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));

                        accessUserTimeline(serverAddress);
                    }
                }
                break;

                case IDB_REFRESH: {
                    if (loggedIn)
                        accessUserTimeline(serverAddress);
                    else
                        accessPublicTimeline(serverAddress);

                    for (int i = 0; i < MAX_POSTS; i++) {
                        LVITEM item = {0};
                        item.mask = LVIF_TEXT;
                        item.iItem = i;
                        item.pszText = L"Example post";
                        ListView_InsertItem(hmainControls[3], &item);
                    }

                    ListView_SetItemCount(hmainControls[3], MAX_POSTS);

                }
                break;

                /* menus */
                case IDM_FILE_LOGOUT: {
                    if (MessageBox(hwindow[0], L"Are you sure you want to logout?", L"Confirmation", MB_ICONQUESTION | MB_YESNO) == IDYES) {
                        MessageBox(hwindow[0], L"Logout logic", L"Confirmation", MB_ICONQUESTION | MB_YESNO);
                    }
                }
                break;

                case IDM_FILE_CLOSE: {
                    DestroyWindow(hwindow[0]);
                }
                break;

                case IDM_TIMELINE_MAINPAGE: {

                    HMENU hmenu = GetMenu(hwnd);
                    
                    UINT statelocal = GetMenuState(hmenu, IDM_TIMELINE_LOCAL, MF_BYCOMMAND);
                    if (statelocal & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_LOCAL, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND | MF_CHECKED);

                        accessUserTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);

                    }
                    
                    UINT statefederation = GetMenuState(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND);
                    if (statefederation & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND | MF_CHECKED);

                        accessUserTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);
                    }
                }
                break;

                case IDM_TIMELINE_LOCAL: {
                    HMENU hmenu = GetMenu(hwnd);
                    
                    UINT statemain = GetMenuState(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND);
                    if (statemain & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_LOCAL, MF_BYCOMMAND | MF_CHECKED);

                        accessUserTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);

                    }
                    
                    UINT statefederation = GetMenuState(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND);
                    if (statefederation & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND | MF_CHECKED);

                        accessUserTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);
                    }
                }
                break;

                case IDM_TIMELINE_FEDERATION: {
                    HMENU hmenu = GetMenu(hwnd);
                    
                    UINT statemain = GetMenuState(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND);
                    if (statemain & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_MAINPAGE, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND | MF_CHECKED);

                        accessPublicTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);

                    }
                    
                    UINT statelocal = GetMenuState(hmenu, IDM_TIMELINE_LOCAL, MF_BYCOMMAND);
                    if (statelocal & MF_CHECKED) {
                        CheckMenuItem(hmenu, IDM_TIMELINE_LOCAL, MF_BYCOMMAND | MF_UNCHECKED);
                        CheckMenuItem(hmenu, IDM_TIMELINE_FEDERATION, MF_BYCOMMAND | MF_CHECKED);

                        accessPublicTimeline(serverAddress);

                        for (int i = 0; i < MAX_POSTS; i++) {
                            LVITEM item = {0};
                            item.mask = LVIF_TEXT;
                            item.iItem = i;
                            item.pszText = L"Example post";
                            ListView_InsertItem(hmainControls[3], &item);
                        }

                        ListView_SetItemCount(hmainControls[3], MAX_POSTS);
                    }
                }
                break;

                case IDM_ABOUT_ABOUT: {
                    MessageBox(hwindow[0], L"Project Retrodon: version 0.1\nAuthor: ricol03", L"About", MB_OK);
                }
                break;
            }
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);

            if (loggedIn) {
                MoveWindow(hmainControls[0], width - 65, 15, SMALL_BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
                MoveWindow(hmainControls[2], 65, 20, width - SMALL_BUTTON_WIDTH - 90, 25, TRUE);
                MoveWindow(hmainControls[5], 15, 15, 35, 35, TRUE);
            } else {
                MoveWindow(hmainControls[0], width - SMALL_BUTTON_WIDTH - 140, 15, SMALL_BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
                MoveWindow(hmainControls[1], width - 115, 15, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
                MoveWindow(hmainControls[2], 15, 15, width - SMALL_BUTTON_WIDTH - 175, BUTTON_HEIGHT, TRUE);
            }

            MoveWindow(hmainControls[3], 0, 65, width, height - 100, TRUE);
            SendMessage(hmainControls[4], WM_SIZE, 0, 0);
        }
        
        case WM_PAINT: {
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK InstanceWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_CREATE: {
            instanceWindow(hwnd);
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDB_CONTINUE_I: {
                    int len = GetWindowText(hinstance_edit, serverAddress, sizeof(serverAddress)/sizeof(WCHAR));
                    if (len == 0) {
                        MessageBox(hwnd, L"Deve introduzir um endereço válido.",
                                L"Erro", MB_ICONERROR | MB_OK);
                    } else {
                        saveSettings();
                        DestroyWindow(hwnd);
                    }
                }
                break;
            }
            break;

        case WM_PAINT: {
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
 
            return TRUE;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK AccountWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_CREATE: {
            accountWindow(hwnd);
            return 0;
        }

        case WM_SHOWWINDOW: {
            if (clickedUserProfile) {
                getImage(userAccount.avatarUrl);

                avatar.pixels = stbi_load_from_memory(
                    imageData.response, imageData.size, &avatar.width, &avatar.height, &avatar.channels, 4);

                free(imageData.response);

                getImage(userAccount.bannerUrl);
                
                banner.pixels = stbi_load_from_memory(
                    imageData.response, imageData.size, &banner.width, &banner.height, &banner.channels, 4);

                free(imageData.response);

                HBITMAP hbmpavatar = CreateHbitmapFromPixels(avatar.pixels, avatar.width, avatar.height, 112, 112);
                HBITMAP hbmpbanner = CreateHbitmapFromPixels(banner.pixels, banner.width, banner.height, 595, 110);

                if (hbmpavatar) {
                    SendMessage(havatar_area, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpavatar);
                } else {
                    MessageBox(NULL, L"Avatar cannot be shown", L"Error", MB_ICONERROR);
                }
                
                if (hbmpbanner) {
                    SendMessage(hbanner_area, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpbanner);
                } else {
                    MessageBox(NULL, L"Banner cannot be shown", L"Error", MB_ICONERROR);
                }

                wchar_t following[32];
                swprintf(following, sizeof(following), L"Following: %d", userAccount.followingNumber);

                wchar_t followers[32];
                swprintf(followers, sizeof(followers), L"Followers: %d", userAccount.followersNumber);

                SendMessage(hdisplayname_static, WM_SETTEXT, 0, (LPARAM)userAccount.displayName);
                SendMessage(hname_static, WM_SETTEXT, 0, (LPARAM)userAccount.username);
                SendMessage(hfollowing_static, WM_SETTEXT, 0, (LPARAM)following);
                SendMessage(hfollowers_static, WM_SETTEXT, 0, (LPARAM)followers);
                SendMessage(hnote_static, WM_SETTEXT, 0, (LPARAM)userAccount.note);
            } else {
                getImage(account.avatarUrl);

                avatar.pixels = stbi_load_from_memory(
                    imageData.response, imageData.size, &avatar.width, &avatar.height, &avatar.channels, 4);

                free(imageData.response);

                getImage(account.bannerUrl);
                
                banner.pixels = stbi_load_from_memory(
                    imageData.response, imageData.size, &banner.width, &banner.height, &banner.channels, 4);

                free(imageData.response);

                HBITMAP hbmpavatar = CreateHbitmapFromPixels(avatar.pixels, avatar.width, avatar.height, 112, 112);
                HBITMAP hbmpbanner = CreateHbitmapFromPixels(banner.pixels, banner.width, banner.height, 595, 110);

                if (hbmpavatar) {
                    SendMessage(havatar_area, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpavatar);
                } else {
                    MessageBox(NULL, L"Avatar cannot be shown", L"Error", MB_ICONERROR);
                }
                
                if (hbmpbanner) {
                    SendMessage(hbanner_area, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpbanner);
                } else {
                    MessageBox(NULL, L"Banner cannot be shown", L"Error", MB_ICONERROR);
                }

                wchar_t following[32];
                swprintf(following, sizeof(following), L"Following: %d", account.followingNumber);

                wchar_t followers[32];
                swprintf(followers, sizeof(followers), L"Followers: %d", account.followersNumber);

                SendMessage(hdisplayname_static, WM_SETTEXT, 0, (LPARAM)account.displayName);
                SendMessage(hname_static, WM_SETTEXT, 0, (LPARAM)account.username);
                SendMessage(hfollowing_static, WM_SETTEXT, 0, (LPARAM)following);
                SendMessage(hfollowers_static, WM_SETTEXT, 0, (LPARAM)followers);
                SendMessage(hnote_static, WM_SETTEXT, 0, (LPARAM)account.note);
            }
            
            
        }

        case WM_COMMAND:
            switch(wparam) {
                case IDB_OK_A: {
                    DestroyWindow(hwindow[2]);
                    return 0;
                }    
            }
            break;

        
        case WM_PAINT: {
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hwnd, &ps);
        
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
            return 0;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK CodeWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_CREATE: {
            codeWindow(hwnd);
            return 0;
        }

        case WM_COMMAND:
            switch(wparam) {
                case IDB_CANCEL_C:
                    ShowWindow(hwnd, SW_HIDE);
                    PostQuitMessage(0);
                    return TRUE;
                case IDB_CONTINUE_C: {
                    wchar_t text[128];
                    GetWindowText(hcodeControls[1], text, _countof(text)); 

                    if (text != NULL)
                        wcscpy(authorizationCode, text);
                    
                    MessageBox(NULL, authorizationCode, L"test", MB_OK);
                    
                    ShowWindow(hwnd, SW_HIDE);
                    runningCodeDialog = FALSE;
                    return TRUE;
                }    
            }
            break;

        case WM_PAINT: {
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
            return 0;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

// old code from another project; might verify Windows version or not

int checkVersion() {
    wversion = GetVersion();

    wmajorversion = (DWORD)(LOBYTE(LOWORD(wversion)));
    wminorversion = (DWORD)(HIBYTE(LOWORD(wversion)));

    /*char version[64];
    snprintf(version, sizeof(version), "Windows version: %d.%d", wmajorversion, wminorversion);*/

    if (wmajorversion >= 5 && wmajorversion < 11) {
        //MessageBox(NULL, version, "Info", MB_ICONINFORMATION);
        return 0;
    } else if (wmajorversion < 5) {
        MessageBox(NULL, L"This program only runs on Windows 2000 and later.", L"Error", MB_ICONERROR);
        return 1;
    } else {
        MessageBox(NULL, L"Unknown Windows version. Aborting.", L"Error", MB_ICONERROR);
        return 1;
    }
}

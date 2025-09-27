#include "headers/tools.h"
#define STB_IMAGE_IMPLEMENTATION
#include "headers/stb_image.h"

const LPCWSTR MAIN_CLASS       = (LPCWSTR)"MainWndClass";

//janelas
HWND hwndmain;
HWND hrefresh;
HWND hlogin;
HWND hsearch;
HWND hlist;
HWND hstatus;

PAINTSTRUCT ps;

HINSTANCE glhinstance;

wchar_t serverAddress[128] = {0};
wchar_t authorizationCode[128] = {0};

DWORD wversion, wmajorversion, wminorversion, wbuild;

extern Post posts[64];
extern Account account;

extern Memory imageData;

extern wchar_t finallink[2048];

Image avatar;
Image banner;

//TODO: could do a separate file for dialogs

int showInstanceDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_INSTANCE), hwndmain, InstanceDialogProc);
}
int showCodeDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_CODE), NULL, CodeDialogProc);
}
int showAccountDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_ACCOUNT), NULL, CodeDialogProc);
}

//TODO: could do a separate file for image manipulation

HBITMAP CreateHbitmapFromPixels(unsigned char * pixels, int srcW, int srcH, int dstW, int dstH) {
    HDC hdc = GetDC(NULL);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = srcW;
    bmi.bmiHeader.biHeight = -srcH; // top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void * pvBits = NULL;
    HBITMAP hSrcBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);

    if (hSrcBmp && pvBits) {
        memcpy(pvBits, pixels, srcW * srcH * 4);
    }

    HDC hdcSrc = CreateCompatibleDC(hdc);
    SelectObject(hdcSrc, hSrcBmp);

    BITMAPINFO dbmi = bmi;
    dbmi.bmiHeader.biWidth = dstW;
    dbmi.bmiHeader.biHeight = -dstH;

    void * dstBits = NULL;
    HBITMAP hDstBmp = CreateDIBSection(hdc, &dbmi, DIB_RGB_COLORS, &dstBits, NULL, 0);

    HDC hdcDst = CreateCompatibleDC(hdc);
    SelectObject(hdcDst, hDstBmp);

    SetStretchBltMode(hdcDst, HALFTONE);
    StretchBlt(hdcDst, 0, 0, dstW, dstH, hdcSrc, 0, 0, srcW, srcH, SRCCOPY);

    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    DeleteObject(hSrcBmp);
    ReleaseDC(NULL, hdc);

    //return hSrcBmp;
    return hDstBmp;
}

int preparingApplication() {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    if (!readSettings()) {
        int result = showInstanceDialog(glhinstance);

        if (result == IDB_CONTINUE_I) {
            saveSettings();
        } else
            MessageBox(NULL, L"Instance could not be saved", L"Error", MB_ICONERROR);
    }

    /* ------ test zone ------ */

    /*-------------------------*/

    createDirectory();
    checkVersion();
}

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PWSTR lpcmdline, int nshowcmd) {

    preparingApplication();

    #pragma region MainWindow
    WNDCLASS mainwindowclass = { 0 };

    mainwindowclass.style            = CS_OWNDC;
    mainwindowclass.lpfnWndProc      = MainWindowProc;
    mainwindowclass.hInstance        = glhinstance;
    mainwindowclass.lpszClassName    = (LPCWSTR)MAIN_CLASS;

    RegisterClass(&mainwindowclass);

    hwndmain = CreateWindowEx(
        0,
        MAIN_CLASS,
        L"Retrodon",
        WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_MINIMIZEBOX | WS_ICONIC | WS_ACTIVECAPTION | WS_VSCROLL | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL,
        NULL,
        glhinstance,
        NULL
    );
    
    if (hwndmain == NULL) {
        MessageBoxW(NULL, L"Unable to create window", 
                L"Error", MB_ICONERROR | MB_OK);
        return 0;
    } else {
        ShowWindow(hwndmain, SW_SHOW);
        UpdateWindow(hwndmain);
    }
        
    #pragma endregion

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } 

    return 0;
}

LRESULT CALLBACK MainWindowProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    
    switch (message) {
        case WM_CREATE: {
            homeWindow(hwnd);
            accessPublicTimeline(serverAddress);

            for (int i = 0; i < MAX_POSTS; i++) {
                LVITEM item = {0};
                item.mask = LVIF_TEXT;
                item.iItem = i;
                item.pszText = L"Example post";
                ListView_InsertItem(hlist, &item);
            }

            ListView_SetItemCount(hlist, MAX_POSTS);

            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lparam;

            switch (pnmh->code) {
                case NM_DBLCLK: {
                    LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lparam;

                    // index of the selected item
                    int iItem = pia->iItem;
                    if (iItem >= 0) {
                        wchar_t buffer[MAX_STR];
                        ListView_GetItemText(pia->hdr.hwndFrom, iItem, 3, buffer, MAX_STR);

                        accessPublicAccount(serverAddress, buffer);

                        HWND hdlg = CreateDialog(glhinstance, MAKEINTRESOURCE(IDD_DIALOG_ACCOUNT), NULL, AccountDialogProc);

                        ShowWindow(hdlg, SW_SHOW);
                        UpdateWindow(hdlg);

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
                            else if (col == 1)
                                plvdi->item.pszText = (LPWSTR)posts[plvdi->item.iItem].content;
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
                case IDB_LOGIN: {
                    if (!createApplication(serverAddress)) {
                        if (!getAccessToken(serverAddress)) {
                            if (!verifyCredentials(serverAddress)) {
                                if (authorizeUser(serverAddress, (HINSTANCE)GetWindowLongPtr(hwndmain, GWLP_HINSTANCE)))
                                    MessageBox(hwndmain, L"Could not authorize user!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);

                            } else
                                MessageBox(hwndmain, L"Could not verify credentials!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);

                        } else 
                            MessageBox(hwndmain, L"Could not get access token!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);

                    } else
                        MessageBox(hwndmain, L"Could not create application!\nConnection attempt cannot proceed.", L"Error", MB_ICONERROR);
                }

                case IDB_REFRESH: {
                    //TODO: this behaviour should change whether the timeline is local, public, etc.
                    accessPublicTimeline(serverAddress);

                    for (int i = 0; i < MAX_POSTS; i++) {
                        LVITEM item = {0};
                        item.mask = LVIF_TEXT;
                        item.iItem = i;
                        item.pszText = L"Example post";
                        ListView_InsertItem(hlist, &item);
                    }

                    ListView_SetItemCount(hlist, MAX_POSTS);
                }

            }
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);

            MoveWindow(hsearch, 15, 15, width - SMALL_BUTTON_WIDTH - 175, BUTTON_HEIGHT, TRUE);
            MoveWindow(hrefresh, width - SMALL_BUTTON_WIDTH - 140, 15, SMALL_BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
            MoveWindow(hlogin, width - 115, 15, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);

            MoveWindow(hlist, 0, 65, width, height - 100, TRUE);

            SendMessage(hstatus, WM_SIZE, 0, 0);
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

INT_PTR CALLBACK InstanceDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_COMMAND:
            switch(LOWORD(wparam)) {
                case IDB_CONTINUE_I: {
                    //FIXME: need a better check for url; like a regex in Java or something
                    if (GetDlgItemText(hdlg, IDE_INSTANCE_I, serverAddress, sizeof(serverAddress)) > 0) {
                        EndDialog(hdlg, IDB_CONTINUE_I); 
                    } else {
                        MessageBox(hdlg, L"Deve introduzir um endereço válido.", L"Erro", MB_ICONERROR | MB_OK);
                    }

                    return TRUE;
                }    
            }
            break;

        case WM_PAINT: {
            HDC hdc = BeginPaint(hdlg, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hdlg, &ps);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
            EndDialog(hdlg, IDB_CONTINUE_I); 
            return TRUE;
    }

    return TRUE;
}

INT_PTR CALLBACK CodeDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_COMMAND:
            switch(wparam) {
                case IDB_CANCEL_C:
                    EndDialog(hdlg, IDB_CANCEL_C); 
                    return TRUE;
                case IDB_CONTINUE_C: {
                    GetDlgItemText(hdlg, IDE_INSTANCE_C, authorizationCode, sizeof(authorizationCode));
                    EndDialog(hdlg, IDB_CONTINUE_C);
                    return TRUE;
                }    
            }
            break;

        case WM_PAINT: {
            HDC hdc = BeginPaint(hdlg, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_3DFACE+1));
            EndPaint(hdlg, &ps);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
            EndDialog(hdlg, IDB_CANCEL_C); 
            return TRUE;
    }

    return TRUE;
}

INT_PTR CALLBACK AccountDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        HBITMAP hbmpbanner;
        case WM_INITDIALOG: {
            //accessPublicAccount(serverAddress, L"114582230083967571");
            getImage(account.avatarUrl);

            avatar.pixels = stbi_load_from_memory(
                imageData.response, imageData.size, &avatar.width, &avatar.height, &avatar.channels, 4);

            free(imageData.response);

            getImage(account.bannerUrl);
            
            banner.pixels = stbi_load_from_memory(
                imageData.response, imageData.size, &banner.width, &banner.height, &banner.channels, 4);

            free(imageData.response);

            HBITMAP hbmpavatar = CreateHbitmapFromPixels(avatar.pixels, avatar.width, avatar.height, 112, 112);
            hbmpbanner = CreateHbitmapFromPixels(banner.pixels, banner.width, banner.height, 515, 100);

            if (hbmpavatar) {
                HWND havatar = GetDlgItem(hdlg, IDP_AVATAR_A);
                SendMessage(havatar, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpavatar);
            } else {
                MessageBox(NULL, L"Avatar cannot be shown", L"Error", MB_ICONERROR);
            }
            
            if (hbmpbanner) {
                HWND hbanner = GetDlgItem(hdlg, IDP_BANNER_A);
                SendMessage(hbanner, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpbanner);
            } else {
                MessageBox(NULL, L"Banner cannot be shown", L"Error", MB_ICONERROR);
            }

            HWND hname = GetDlgItem(hdlg, IDS_NAME_A);
            HWND hfollowing = GetDlgItem(hdlg, IDS_FOLLOWING_A);
            HWND hfollowers = GetDlgItem(hdlg, IDS_FOLLOWERS_A);
            HWND hnote = GetDlgItem(hdlg, IDS_NOTE_A);

            wchar_t following[32];
            swprintf(following, sizeof(following), L"Following: %d", account.followingNumber);

            wchar_t followers[32];
            swprintf(followers, sizeof(followers), L"Followers: %d", account.followersNumber);

            SendMessage(hname, WM_SETTEXT, 0, (LPARAM)account.displayName);
            SendMessage(hfollowing, WM_SETTEXT, 0, (LPARAM)following);
            SendMessage(hfollowers, WM_SETTEXT, 0, (LPARAM)followers);
            SendMessage(hnote, WM_SETTEXT, 0, (LPARAM)account.note);


        }

        case WM_COMMAND:
            switch(wparam) {
                case IDB_OK_A: {
                    EndDialog(hdlg, IDB_OK_A);
                    return TRUE;
                }    
            }
            break;

        


        case WM_PAINT: {
            HDC hdc = BeginPaint(hdlg, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) ((GetSysColor(COLOR_WINDOW)+1)));
            EndPaint(hdlg, &ps);
        
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY: 
            EndDialog(hdlg, IDB_OK_A); 
            return TRUE;
    }

    return TRUE;
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

#include "headers/tools.h"

const LPCSTR MAIN_CLASS       = (LPCSTR)"MainWndClass";

//janelas
HWND hwndmain;
HWND htestbutton;
HWND hsearch;
HWND hlist;
HWND hstatus;

PAINTSTRUCT ps;

HINSTANCE glhinstance;

char serverAddress[128] = {0};
char authorizationCode[128] = {0};

extern Post posts[64];


//TODO: could do a separate file for dialogs

int showInstanceDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_INSTANCE), hwndmain, InstanceDialogProc);
}
int showCodeDialog(HINSTANCE hinstance) {
    return DialogBox(hinstance, MAKEINTRESOURCE(IDD_DIALOG_CODE), NULL, CodeDialogProc);
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
            MessageBox(NULL, "Instance could not be saved", "Error", MB_ICONERROR);
    }
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR lpcmdline, int nshowcmd) {

    preparingApplication();

    #pragma region MainWindow
    WNDCLASS mainwindowclass = { 0 };

    mainwindowclass.style            = CS_OWNDC;
    mainwindowclass.lpfnWndProc      = MainWindowProc;
    mainwindowclass.hInstance        = glhinstance;
    mainwindowclass.lpszClassName    = (LPCSTR)MAIN_CLASS;

    RegisterClass(&mainwindowclass);

    hwndmain = CreateWindowEx(
        0,
        MAIN_CLASS,
        "Retrodon",
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
            accessPublicContent(serverAddress);

            for (int i = 0; i < 100; i++) {
                LVITEM item = {0};
                item.mask = LVIF_TEXT;
                item.iItem = i;
                item.pszText = "Example post";
                ListView_InsertItem(hlist, &item);
            }

            ListView_SetItemCount(hlist, 100);

            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lparam;
            if (pnmh->idFrom == IDC_LISTVIEW && pnmh->code == LVN_GETDISPINFO) {
                NMLVDISPINFO *plvdi = (NMLVDISPINFO *)lparam;
                if (plvdi->item.mask & LVIF_TEXT) {

                    int row = plvdi->item.iItem;
                    int col = plvdi->item.iSubItem;

                    if (col == 0)
                        plvdi->item.pszText = (LPSTR)posts[plvdi->item.iItem].username;
                    else
                        plvdi->item.pszText = (LPSTR)posts[plvdi->item.iItem].content;

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
                                    MessageBox(hwndmain, "Could not authorize user!\nConnection attempt cannot proceed.", "Error", MB_ICONERROR);

                            } else
                                MessageBox(hwndmain, "Could not verify credentials!\nConnection attempt cannot proceed.", "Error", MB_ICONERROR);

                        } else 
                            MessageBox(hwndmain, "Could not get access token!\nConnection attempt cannot proceed.", "Error", MB_ICONERROR);

                    } else
                        MessageBox(hwndmain, "Could not create application!\nConnection attempt cannot proceed.", "Error", MB_ICONERROR);
                }
            }
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);

            MoveWindow(htestbutton, width - 125, 25, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
            MoveWindow(hsearch, 25, 25, width - BUTTON_WIDTH - 75, BUTTON_HEIGHT, TRUE);
            MoveWindow(hlist, 0, 75, width, height - 100, TRUE);

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
                        MessageBox(hdlg, "Deve introduzir um endereço válido.", "Erro", MB_ICONERROR | MB_OK);
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

// old code from another project; might verify Windows version or not

/*int checkVersion() {
    wversion = GetVersion();

    wmajorversion = (DWORD)(LOBYTE(LOWORD(wversion)));
    wminorversion = (DWORD)(HIBYTE(LOWORD(wversion)));

    if (wmajorversion >= 5 && wmajorversion < 11) {
        return 0;
    } else if (wmajorversion < 5) {
        MessageBox(NULL, L"This program only runs on Windows 2000 and later.", L"Error", MB_ICONERROR);
        return 1;
    } else {
        MessageBox(NULL, L"Unknown Windows version. Aborting.", L"Error", MB_ICONERROR);
        return 1;
    }
}*/

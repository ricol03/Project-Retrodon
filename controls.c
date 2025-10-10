#include "headers/tools.h"

extern BOOL loggedIn;

extern wchar_t serverAddress[128];

// main window controls
// 0 - refresh button
// 1 - login button
// 2 - search bar
// 3 - activity list
// 4 - status bar
// 5 - profile button
HWND hmainControls[6];

extern HWND hrefresh;
extern HWND hlogin;
extern HWND hsearch;
extern HWND hlist;
extern HWND hstatus;

extern HWND hinstance_edit, hinstance_title, hinstance_subtitle, hinstance_button;

HWND hfollow_button, hfollowing_static, hfollowers_static, hdisplayname_static, hname_static, hnote_static, havatar_area, hbanner_area, hok_button;

// code window controls
// 0 - static text
// 1 - edit control
// 2 - cancel button
// 3 - continue button
HWND hcodeControls[12];

//fonts
//0 - title font
//1 - text font
//2 - account title font
HFONT hfont[3];


HINSTANCE hinstance;

int createFonts() {
    hfont[0] = CreateFont(
        32,
        0,
        0,
        0,
        FW_BOLD,
        0, 0, 0, 
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, 
        ANTIALIASED_QUALITY, 
        FF_DONTCARE,
        TEXT("Arial")
    );

    hfont[1] = CreateFont(
        12,
        0,
        0,
        0,
        FW_NORMAL,
        0, 0, 0, 
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, 
        ANTIALIASED_QUALITY, 
        FF_DONTCARE,
        TEXT("Arial")
    );

    hfont[2] = CreateFont(
        18,
        0,
        0,
        0,
        FW_BOLD,
        0, 0, 0, 
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, 
        ANTIALIASED_QUALITY, 
        FF_DONTCARE,
        TEXT("Arial")
    );
}

int homeWindow(HWND hwnd) {
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    if (!loggedIn) {
        hmainControls[1] = CreateWindow(
            WC_BUTTON,
            TEXT("Login"),
            WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
            rcClient.left + 25, rcClient.bottom - 25, 125, 35,
            hwnd,
            (HMENU)IDB_LOGIN,
            GetModuleHandle(NULL),
            NULL
        );
    } else {
        hmainControls[5] = CreateWindow(
            WC_STATIC,
            L"",
            SS_BITMAP | WS_VISIBLE | WS_CHILD,
            25, 25, 32, 32,
            hwnd,
            (HMENU) IDP_AVATAR_M,
            GetModuleHandle(NULL),
            NULL
        );
    }

    hmainControls[0] = CreateWindow(
        WC_BUTTON,
        L"Refresh",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | BS_ICON | WS_VISIBLE,
        25, 25, 50, 35,
        hwnd,
        (HMENU)IDB_REFRESH,
        GetModuleHandle(NULL),
        NULL
    );

    //FIXME: example icon
    HICON hicon = LoadIcon(hinstance, IDI_WARNING);
    SendMessage(hmainControls[0], BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hicon);

    

    hmainControls[2] = CreateWindow(
        WC_EDIT,
        L"Search the Fediverse",
        WS_VISIBLE | WS_DISABLED | WS_CHILD,
        150, 15, 200, 25,
        hwnd,
        (HMENU) 10,
        GetModuleHandle(NULL),
        NULL
    );

    hmainControls[3] = CreateWindow(
        WC_LISTVIEW, 
        L"",         
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA,
        50, 0, 480, 300,
        hwnd, 
        (HMENU)IDC_LISTVIEW, 
        GetModuleHandle(NULL), 
        NULL
    );

    LVCOLUMN col = {0};
    col.mask = LVCF_WIDTH | LVCF_TEXT;
    col.cx = 100;
    col.pszText = L"User";
    ListView_InsertColumn(hmainControls[3], 0, &col);

    col.cx = 350;
    col.pszText = L"Content";
    ListView_InsertColumn(hmainControls[3], 1, &col);

    col.cx = 150;
    col.pszText = L"Posted at";
    ListView_InsertColumn(hmainControls[3], 2, &col);

    col.cx = 0;
    ListView_InsertColumn(hmainControls[3], 3, &col);

    ListView_SetIconSpacing(hmainControls[3], 100, 60);

    hmainControls[4] = CreateWindowEx(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

    int parts[] = {30, 200, -1};
    SendMessage(hmainControls[4], SB_SETPARTS, 3, (LPARAM)parts);

    wchar_t celltext[512];
    swprintf(celltext, sizeof(celltext), L"Instance: %ls", serverAddress);

    //FIXME: example icon
    SendMessage(hmainControls[4], SB_SETICON, 0, (LPARAM)LoadIcon(NULL, IDI_WARNING));
    SendMessage(hmainControls[4], SB_SETTEXT, 1, (LPARAM)charToWchar("Logged out"));
    SendMessage(hmainControls[4], SB_SETTEXT, 2, (LPARAM)celltext);

    HMENU hmenu = CreateMenu();
    HMENU hsubmenufile        = CreatePopupMenu();
    HMENU hsubmenutimefile    = CreatePopupMenu();
    HMENU hsubmenusearch      = CreatePopupMenu();
    HMENU hsubmenusettings    = CreatePopupMenu();
    HMENU hsubmenuabout       = CreatePopupMenu();

    if (loggedIn)
        AppendMenu(hsubmenufile, MF_STRING, IDM_FILE_LOGOUT, L"Logout...");
    else
        AppendMenu(hsubmenufile, MF_STRING | MF_GRAYED, IDM_FILE_LOGOUT, L"Logout...");  
    AppendMenu(hsubmenufile, MF_STRING, IDM_FILE_CLOSE, L"Exit");
    
    if (loggedIn) { 
        AppendMenu(hsubmenutimefile, MF_STRING | MF_CHECKED, 100, L"Main page");
        AppendMenu(hsubmenutimefile, MF_STRING, 101, L"Local");
        AppendMenu(hsubmenutimefile, MF_SEPARATOR, 0, NULL);
        AppendMenu(hsubmenutimefile, MF_STRING, 102, L"Federation");
    } else {
        AppendMenu(hsubmenutimefile, MF_STRING | MF_GRAYED, 100, L"Main page");
        AppendMenu(hsubmenutimefile, MF_STRING | MF_GRAYED, 101, L"Local");
        AppendMenu(hsubmenutimefile, MF_SEPARATOR, 0, NULL);
        AppendMenu(hsubmenutimefile, MF_STRING | MF_CHECKED, 102, L"Federation");
    }
    
    AppendMenu(hsubmenusearch, MF_STRING | MF_GRAYED, IDM_SEARCH_SEARCHBOX, L"Test");
    AppendMenu(hsubmenusettings, MF_STRING | MF_GRAYED, IDM_SETTINGS_SETTINGS, L"Preferences...");

    AppendMenu(hsubmenuabout, MF_STRING | MF_GRAYED, IDM_ABOUT_HELP, L"Help");
    AppendMenu(hsubmenuabout, MF_SEPARATOR, 0, NULL);
    AppendMenu(hsubmenuabout, MF_STRING, IDM_ABOUT_ABOUT, L"About");

    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenufile, L"File");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenutimefile, L"Timeline");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenusearch, L"View");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenusettings, L"Options");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenuabout, L"Help");

    SetMenu(hwnd, hmenu);
}


int instanceWindow(HWND hwnd) {
    hinstance_title = CreateWindow(
        WC_STATIC,
        L"Introduza o endereço da instância:",
        WS_VISIBLE | WS_CHILD,
        25, 20, 250, 25,
        hwnd,
        (HMENU) 15,
        GetModuleHandle(NULL),
        NULL
    );

    hinstance_edit = CreateWindow(
        WC_EDIT,
        NULL,
        WS_VISIBLE | WS_CHILD,
        25, 50, 350, 20,
        hwnd,
        (HMENU) IDE_INSTANCE_I,
        GetModuleHandle(NULL),
        NULL
    );

    // TODO: XP only; must check version
    //SendMessage(hinstance_edit, EM_SETCUEBANNER, 0, (LPARAM)L"(ex. ""mastodon.social"")");

    hinstance_subtitle = CreateWindow(
        WC_STATIC,
        L"(ex. ""mastodon.social"")",
        WS_VISIBLE | WS_CHILD,
        225, 75, 250, 25,
        hwnd,
        (HMENU) 15,
        GetModuleHandle(NULL),
        NULL
    );

    hinstance_button = CreateWindow(
        WC_BUTTON,
        L"Continuar",
        WS_VISIBLE | WS_CHILD,
        295, 105, 80, 30,
        hwnd,
        (HMENU) IDB_CONTINUE_I,
        GetModuleHandle(NULL),
        NULL
    );

}

int accountWindow(HWND hwnd) {
    if (loggedIn) {
        hfollow_button = CreateWindow(
            WC_BUTTON,
            L"Follow",
            WS_VISIBLE | WS_CHILD,
            490, 145, 80, 30,
            hwnd,
            (HMENU) IDB_FOLLOW_A,
            GetModuleHandle(NULL),
            NULL
        );
    } else {
        hfollow_button = CreateWindow(
            WC_BUTTON,
            L"Follow",
            WS_VISIBLE | WS_DISABLED | WS_CHILD,
            490, 145, 80, 30,
            hwnd,
            (HMENU) IDB_FOLLOW_A,
            GetModuleHandle(NULL),
            NULL
        );
    }
    
    hfollowing_static = CreateWindow(
        WC_STATIC,
        L"Following: ",
        WS_VISIBLE | WS_CHILD,
        175, 175, 120, 18,
        hwnd,
        (HMENU) IDS_FOLLOWING_A,
        GetModuleHandle(NULL),
        NULL
    );

    hfollowers_static = CreateWindow(
        WC_STATIC,
        L"Followers: ",
        WS_VISIBLE | WS_CHILD,
        315, 175, 120, 18,
        hwnd,
        (HMENU) IDS_FOLLOWERS_A,
        GetModuleHandle(NULL),
        NULL
    );

    hdisplayname_static = CreateWindow(
        WC_STATIC,
        L"name",
        WS_VISIBLE | WS_CHILD,
        175, 125, 145, 18,
        hwnd,
        (HMENU) IDS_NAME_A,
        GetModuleHandle(NULL),
        NULL
    );


    hname_static = CreateWindow(
        WC_STATIC,
        L"name",
        WS_VISIBLE | WS_CHILD,
        175, 155, 145, 18,
        hwnd,
        (HMENU) IDS_NAME_A,
        GetModuleHandle(NULL),
        NULL
    );

    hnote_static = CreateWindow(
        WC_EDIT,
        L"note",
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        40, 200, 530, 125,
        hwnd,
        (HMENU) IDS_NOTE_A,
        GetModuleHandle(NULL),
        NULL
    );

    hbanner_area = CreateWindow(
        WC_STATIC,
        L"",
        SS_BITMAP | WS_VISIBLE | WS_CHILD,
        0, 0, 590, 110,
        hwnd,
        (HMENU) IDP_BANNER_A,
        GetModuleHandle(NULL),
        NULL
    );

    havatar_area = CreateWindow(
        WC_STATIC,
        L"",
        SS_BITMAP | WS_VISIBLE | WS_CHILD,
        40, 75, 112, 112,
        hwnd,
        (HMENU) IDP_AVATAR_A,
        GetModuleHandle(NULL),
        NULL
    );

    hok_button = CreateWindow(
        WC_BUTTON,
        L"OK",
        WS_VISIBLE | WS_CHILD,
        490, 335, 80, 30,
        hwnd,
        (HMENU) IDB_OK_A,
        GetModuleHandle(NULL),
        NULL
    );

    SendMessage(hdisplayname_static, WM_SETFONT, (WPARAM)hfont[2], TRUE);
    SendMessage(hok_button, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hfollow_button, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hfollowers_static, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hfollowing_static, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hname_static, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hnote_static, WM_SETFONT, (WPARAM)hfont[1], TRUE);

}

int codeWindow(HWND hwnd) {
    hcodeControls[0] = CreateWindow(
        WC_STATIC,
        L"Enter the authorization code received in the browser below:",
        WS_VISIBLE | WS_CHILD,
        20, 20, 260, 20,
        hwnd,
        (HMENU) 0,
        GetModuleHandle(NULL),
        NULL
    );

    hcodeControls[1] = CreateWindow(
        WC_EDIT,
        NULL,
        WS_VISIBLE | WS_CHILD,
        20, 70, 360, 20,
        hwnd,
        (HMENU) IDE_INSTANCE_C,
        GetModuleHandle(NULL),
        NULL
    );

    hcodeControls[2] = CreateWindow(
        WC_BUTTON,
        L"Cancel",
        WS_VISIBLE | WS_CHILD,
        240, 110, 60, 20,
        hwnd,
        (HMENU) IDB_CANCEL_C,
        GetModuleHandle(NULL),
        NULL
    );

    hcodeControls[3] = CreateWindow(
        WC_BUTTON,
        L"Continue",
        WS_VISIBLE | WS_CHILD,
        320, 110, 60, 20,
        hwnd,
        (HMENU) IDB_CONTINUE_C,
        GetModuleHandle(NULL),
        NULL
    );
}


#include "headers/tools.h"

extern BOOL loggedIn;
extern BOOL clickedUserProfile;

extern Account userAccount;
extern Memory imageData;
extern Image avatar;

extern wchar_t serverAddress[128];

// main window controls
// 0 - refresh button
// 1 - login button
// 2 - search bar
// 3 - activity list
// 4 - status bar
// 5 - profile button
HWND hmainControls[6];

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

    wchar_t systemFont[128];

    LOGFONT lf;
    HFONT hfontobject = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hfontobject && GetObject(hfontobject, sizeof(LOGFONT), &lf)) {
        wcscpy(systemFont, lf.lfFaceName);
    }

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
        systemFont
    );

    hfont[1] = CreateFont(
        18,
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
        systemFont
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
        systemFont
    );
}

void resetHomeWindow() {
    for (int i = 0; i < 7; i++) {
        DestroyWindow(hmainControls[i]);
        hmainControls[i] = NULL;
    }
}

int homeWindow(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    if (!loggedIn) {
        hmainControls[1] = CreateWindow(
            WC_BUTTON,
            TEXT("Login"),
            WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
            rc.left + 25, rc.bottom - 25, 125, 35,
            hwnd,
            (HMENU)IDB_LOGIN,
            GetModuleHandle(NULL),
            NULL
        );
    } else {
        hmainControls[5] = CreateWindow(
            WC_STATIC,
            L"",
            SS_BITMAP | SS_NOTIFY | WS_VISIBLE | WS_CHILD,
            15, 15, 35, 35,
            hwnd,
            (HMENU) IDP_AVATAR_M,
            GetModuleHandle(NULL),
            NULL
        );

        getImage(userAccount.avatarUrl);

        avatar.pixels = stbi_load_from_memory(
            imageData.response, imageData.size, &avatar.width, &avatar.height, &avatar.channels, 4);

        free(imageData.response);

        HBITMAP hbmpavatar = CreateHbitmapFromPixels(avatar.pixels, avatar.width, avatar.height, 32, 32);

        if (hbmpavatar)
            SendMessage(hmainControls[5], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpavatar);

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

    HICON hrefreshicon = ExtractIconW(NULL, L"C:\\Windows\\System32\\shell32.dll", 238);
    SendMessageW(hmainControls[0], BM_SETIMAGE, IMAGE_ICON, (LPARAM)hrefreshicon);

    hmainControls[2] = CreateWindow(
        WC_EDIT,
        L"Search the Fediverse",
        WS_VISIBLE | WS_DISABLED | WS_CHILD,
        150, 20, 200, 20,
        hwnd,
        //TODO: add proper const
        (HMENU)12506,
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

    int parts[] = {20, 200, -1};
    SendMessage(hmainControls[4], SB_SETPARTS, 3, (LPARAM)parts);

    wchar_t celltext[512];
    swprintf(celltext, sizeof(celltext), L"Instance: %ls", serverAddress);

    HICON hshellicon = (HICON)LoadImageW(
        GetModuleHandleW(L"shell32.dll"),
        MAKEINTRESOURCEW(18),
        IMAGE_ICON,
        16, 16,
        LR_SHARED
    );

    SendMessage(hmainControls[4], SB_SETICON, 0, (LPARAM)hshellicon);

    if (loggedIn) {
        wchar_t statustext[128];
        swprintf(statustext, sizeof(statustext), L"Logged in as: %ls", userAccount.username);
        SendMessage(hmainControls[4], SB_SETTEXT, 1, (LPARAM)statustext);
    
    } else
        SendMessage(hmainControls[4], SB_SETTEXT, 1, (LPARAM)charToWchar("Logged out"));
    SendMessage(hmainControls[4], SB_SETTEXT, 2, (LPARAM)celltext);

    HMENU hmenu               = CreateMenu();
    HMENU hsubmenufile        = CreatePopupMenu();
    HMENU hsubmenutimeline    = CreatePopupMenu();
    HMENU hsubmenuview        = CreatePopupMenu();
    HMENU hsubmenuoptions     = CreatePopupMenu();
    HMENU hsubmenuhelp        = CreatePopupMenu();

    if (loggedIn)
        AppendMenu(hsubmenufile, MF_STRING, IDM_FILE_LOGOUT, L"Logout...");
    else
        AppendMenu(hsubmenufile, MF_STRING | MF_GRAYED, IDM_FILE_LOGOUT, L"Logout...");
    AppendMenu(hsubmenufile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hsubmenufile, MF_STRING, IDM_FILE_CLOSE, L"Exit");
    
    if (loggedIn) { 
        AppendMenu(hsubmenutimeline, MF_STRING | MF_CHECKED, IDM_TIMELINE_MAINPAGE, L"Main page");
        AppendMenu(hsubmenutimeline, MF_STRING, IDM_TIMELINE_LOCAL, L"Local");
        AppendMenu(hsubmenutimeline, MF_SEPARATOR, 0, NULL);
        AppendMenu(hsubmenutimeline, MF_STRING, IDM_TIMELINE_FEDERATION, L"Federation");
    } else {
        AppendMenu(hsubmenutimeline, MF_STRING | MF_GRAYED, IDM_TIMELINE_MAINPAGE, L"Main page");
        AppendMenu(hsubmenutimeline, MF_STRING | MF_GRAYED, IDM_TIMELINE_LOCAL, L"Local");
        AppendMenu(hsubmenutimeline, MF_SEPARATOR, 0, NULL);
        AppendMenu(hsubmenutimeline, MF_STRING | MF_CHECKED, IDM_TIMELINE_FEDERATION, L"Federation");
    }
    
    AppendMenu(hsubmenuview, MF_STRING | MF_GRAYED, IDM_VIEW_SMTH, L"Test");
    AppendMenu(hsubmenuview, MF_STRING | MF_GRAYED, IDM_VIEW_SMTH2, L"Test");

    AppendMenu(hsubmenuoptions, MF_STRING | MF_GRAYED, IDM_OPTIONS_PREFERENCES, L"Preferences...");

    AppendMenu(hsubmenuhelp, MF_STRING | MF_GRAYED, IDM_ABOUT_HELP, L"Help");
    AppendMenu(hsubmenuhelp, MF_SEPARATOR, 0, NULL);
    AppendMenu(hsubmenuhelp, MF_STRING, IDM_ABOUT_ABOUT, L"About");

    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenufile, L"File");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenutimeline, L"Timeline");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenuview, L"View");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenuoptions, L"Options");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenuhelp, L"Help");

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
        WS_VISIBLE | WS_EX_CLIENTEDGE | WS_CHILD,
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

    SendMessage(hinstance_title, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hinstance_edit, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hinstance_subtitle, WM_SETFONT, (WPARAM)hfont[1], TRUE);
    SendMessage(hinstance_button, WM_SETFONT, (WPARAM)hfont[1], TRUE);
}

int accountWindow(HWND hwnd) {
    if (loggedIn) {
        //TODO: requires user profile from post list logic
        if (!clickedUserProfile) {
            hfollow_button = CreateWindow(
                WC_BUTTON,
                L"Follow",
                //TODO: remove disabled state in future version
                WS_VISIBLE | WS_DISABLED | WS_CHILD,
                490, 145, 80, 30,
                hwnd,
                (HMENU) IDB_FOLLOW_A,
                GetModuleHandle(NULL),
                NULL
            );
        }
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
        20, 20, 280, 50,
        hwnd,
        (HMENU) 0,
        GetModuleHandle(NULL),
        NULL
    );

    hcodeControls[1] = CreateWindow(
        WC_EDIT,
        NULL,
        WS_VISIBLE | ES_AUTOHSCROLL | WS_EX_CLIENTEDGE | WS_CHILD,
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


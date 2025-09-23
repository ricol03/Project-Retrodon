#include "headers/tools.h"

extern char serverAddress[128];

extern HWND hrefresh;
extern HWND hlogin;
extern HWND hsearch;
extern HWND hlist;
extern HWND hstatus;

int homeWindow(HWND hwnd) {
    //destroyVisibleChildWindows(hwnd);

    RECT rcClient;

    GetWindowRect(hwnd, &rcClient);
    
    /*HWND htesttext = CreateWindow(
        WC_STATIC, 
        "Under construction",
        WS_VISIBLE | WS_CHILD,
        250, 150, 450, 200,
        hwnd, 
        (HMENU)998,
        GetModuleHandle(NULL),
        NULL
    );*/

    //TODO: add icon instead of text
    hrefresh = CreateWindow(
        WC_BUTTON,
        TEXT("R"),
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
        rcClient.left + 25, rcClient.bottom - 25, 0, 0,
        hwnd,
        (HMENU)IDB_REFRESH,
        GetModuleHandle(NULL),
        NULL
    );

    hlogin = CreateWindow(
        WC_BUTTON,
        TEXT("Login"),
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
        rcClient.left + 25, rcClient.bottom - 25, 125, 35,
        hwnd,
        (HMENU)IDB_LOGIN,
        GetModuleHandle(NULL),
        NULL
    );


    //const WCHAR * labelText = "é`Àçõaz it works";
        
    /*HWND htext = CreateWindow(
        WC_STATIC,
        "Trending Content",
        //L"トレンドコンテンツ",
        //labelText,
        WS_VISIBLE | WS_CHILD,
        25, 25, 100, 50,
        hwnd,
        (HMENU)1,
        GetModuleHandle(NULL),
        NULL
    );*/

    hsearch = CreateWindow(
        WC_EDIT,
        "Search the Fediverse",
        WS_VISIBLE | WS_CHILD,
        25, 15, 450, 35,
        hwnd,
        (HMENU) 10,
        GetModuleHandle(NULL),
        NULL
    );

    hlist = CreateWindow(
        WC_LISTVIEW, 
        "",         
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
    col.pszText = "User";
    ListView_InsertColumn(hlist, 0, &col);

    col.cx = 350;
    col.pszText = "Content";
    ListView_InsertColumn(hlist, 1, &col);

    col.cx = 150;
    col.pszText = "Posted at";
    ListView_InsertColumn(hlist, 2, &col);

    ListView_SetIconSpacing(hlist, 100, 60);

    hstatus = CreateWindowEx(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

    int parts[] = {30, 200, -1};
    SendMessage(hstatus, SB_SETPARTS, 3, (LPARAM)parts);

    char celltext[512];
    snprintf(celltext, sizeof(celltext), "Instance: %s", serverAddress);

    SendMessage(hstatus, SB_SETICON, 0, (LPARAM)LoadIcon(NULL, IDI_WARNING));
    SendMessage(hstatus, SB_SETTEXT, 1, (LPARAM)"Logged out");   // first cell
    SendMessage(hstatus, SB_SETTEXT, 2, (LPARAM)celltext);     // second cell

    //SendMessage(htext, WM_SETFONT, (WPARAM)htitlefont, (LPARAM)NULL);

    /*HWND hbutton = CreateWindow(
        WC_BUTTON,
        "Search anime",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        400, 25, 120, 30,
        hwnd,
        (HMENU)IDW_MAIN_BUTTON_SEARCH,
        GetModuleHandle(NULL),
        NULL
    );*/

    //SendMessage(hbutton, WM_SETFONT, (WPARAM)hfont, (LPARAM)NULL);

    HMENU hmenu = CreateMenu();
    HMENU hsubmenufile        = CreatePopupMenu();
    HMENU hsubmenutimefile    = CreatePopupMenu();
    HMENU hsubmenusearch      = CreatePopupMenu();
    HMENU hsubmenusettings    = CreatePopupMenu();
    HMENU hsubmenuabout       = CreatePopupMenu();

    AppendMenu(hsubmenufile, MF_STRING, IDM_FILE_CLOSE, "Exit");
    
    AppendMenu(hsubmenutimefile, MF_STRING | MF_GRAYED, 100, "Main page");
    AppendMenu(hsubmenutimefile, MF_STRING | MF_GRAYED, 101, "Local");
    AppendMenu(hsubmenutimefile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hsubmenutimefile, MF_STRING | MF_CHECKED, 102, "Federation");
    
    AppendMenu(hsubmenusearch, MF_STRING | MF_GRAYED, IDM_SEARCH_SEARCHBOX, "Test");
    AppendMenu(hsubmenusettings, MF_STRING, IDM_SETTINGS_SETTINGS, "Preferences...");

    AppendMenu(hsubmenuabout, MF_STRING, IDM_ABOUT_HELP, "Help");
    AppendMenu(hsubmenuabout, MF_SEPARATOR, 0, NULL);
    AppendMenu(hsubmenuabout, MF_STRING, IDM_ABOUT_ABOUT, "About");

    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenufile, "File");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenutimefile, "Timeline");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenusearch, "View");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenusettings, "Options");
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenuabout, "Help");

    SetMenu(hwnd, hmenu);
}
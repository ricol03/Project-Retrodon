#include "tools.h"

#ifndef MAIN_H_
#define MAIN_H_

struct Image {
    int width;
    int height; 
    int channels;
    unsigned char * pixels;
} typedef Image;

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK InstanceWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK CodeDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK AccountWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);


int preparingApplication();
int checkVersion();

int showCodeDialog(HINSTANCE hinstance);
int showInstanceDialog(HINSTANCE hinstance);
int showAccountDialog(HINSTANCE hinstance);


int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PWSTR lpcmdline, int nshowcmd);

#endif
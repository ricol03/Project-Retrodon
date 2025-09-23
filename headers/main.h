#include "tools.h"

#ifndef MAIN_H_
#define MAIN_H_

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK InstanceDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK CodeDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK AccountDialogProc(HWND hdlg, UINT message, WPARAM wparam, LPARAM lparam);


int preparingApplication();
int checkVersion();

int showCodeDialog(HINSTANCE hinstance);
int showInstanceDialog(HINSTANCE hinstance);
int showAccountDialog(HINSTANCE hinstance);


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR lpcmdline, int nshowcmd);

#endif
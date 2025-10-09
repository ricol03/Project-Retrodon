#ifndef TOOLS_H_
#define TOOLS_H_

#define UNICODE
#define _UNICODE

#define _WIN32_IE 0x0400
#define _WIN32_WINNT 0x0500

//#define STB_IMAGE_IMPLEMENTATION

//constants
#include "const.h"

//system headers
#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <shlobj.h>

#include <wchar.h>

//library headers
#include "../cjson/cJSON.h"
#include "stb_image.h"

//file headers
#include "controls.h"
#include "connections.h"
#include "file.h"
#include "main.h"
#include "strings.h"

#endif
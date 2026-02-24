// resource.hpp
// Resource identifier definitions for FileMgr
// 
// This file defines numeric constants used in the Windows resource file
// (resource.rc) to identify icons and other UI elements.
// 
// Note: Many of these IDs are legacy from earlier Win32 versions of the
// application. Current version primarily uses the icon IDs (IDI_*).
// 

#pragma once

// -----------------------------------------------------------------------------
// Icon IDs (101-102)
// -----------------------------------------------------------------------------
#define IDI_SUPERN  101  // "Add" icon (forward navigation)
#define IDI_SUPERY  102  // "Sub" icon (back navigation)

// -----------------------------------------------------------------------------
// Control IDs (1001-1015)
// -----------------------------------------------------------------------------
#define ID_LISTVIEW  1001  // List view control (legacy)
#define ID_ADDRESS   1002  // Address bar control (legacy)
#define ID_EDIT      1003  // Edit control (legacy)
#define ID_UP        1004  // Up button
#define ID_REFRESH   1005  // Refresh button
#define ID_SUPER     1006  // Super button (legacy)
#define ID_YES       1007  // Yes button/icon
#define ID_NO        1008  // No button/icon
#define ID_NDIR      1009  // New directory icon
#define ID_NFILE     1010  // New file icon
#define ID_DELETE    1011  // Delete icon
#define ID_RENAME    1012  // Rename icon
#define ID_COPY      1013  // Copy icon
#define ID_CUT       1014  // Cut icon
#define ID_PASTE     1015  // Paste icon

// -----------------------------------------------------------------------------
// Menu command IDs (2001-2004)
// -----------------------------------------------------------------------------
#define IDM_OPEN       2001  // Open command
#define IDM_NOTEPAD    2002  // Open with Notepad command
#define IDM_CPFILEPATH 2003  // Copy file path command
#define IDM_STREAM     2004  // Stream command (legacy)

// -----------------------------------------------------------------------------
// Context menu command IDs (3001-3004)
// -----------------------------------------------------------------------------
#define IDM_REFRESH    3001  // Refresh command
#define IDM_CMD        3002  // Open command prompt here
#define IDM_EXPLORER   3003  // Open in Explorer command
#define IDM_CPDIRPATH  3004  // Copy directory path command

// -----------------------------------------------------------------------------
// Icon ID aliases (for resource.rc compatibility)
// -----------------------------------------------------------------------------
#define IDI_UP      ID_UP
#define IDI_REFRESH ID_REFRESH
#define IDI_YES     ID_YES
#define IDI_NO      ID_NO
#define IDI_NDIR    ID_NDIR
#define IDI_NFILE   ID_NFILE
#define IDI_DELETE  ID_DELETE
#define IDI_RENAME  ID_RENAME
#define IDI_COPY    ID_COPY
#define IDI_CUT     ID_CUT
#define IDI_PASTE   ID_PASTE
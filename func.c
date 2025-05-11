#include <Windows.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <string.h>
#include "ressource.h"

extern lua_State *gLua;  // Ensure this is declared in a header file
static HWND hEdit, hSaveBtn, hCloseBtn;
// Forward declaration
void InitializeLua();
void CleanupLua();

void ExtractConfigTableAndShow(HWND hWnd);

LRESULT CALLBACK ConfigWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont;

    switch (uMsg) {
    case WM_CREATE: {
        // Set font (Consolas)
        LOGFONT lf = { 0 };
        lf.lfHeight = -16;
        lf.lfWeight = FW_NORMAL;
        strcpy(lf.lfFaceName, "Consolas");
        hFont = CreateFontIndirect(&lf);

        // Create controls (Edit box, Save button, Close button)
        hEdit = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL,
            10, 10, 380, 200, hWnd, NULL, NULL, NULL
        );
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        hSaveBtn = CreateWindowA(
            "BUTTON", "Save",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            80, 220, 100, 30, hWnd, (HMENU)1, NULL, NULL
        );
        SendMessage(hSaveBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        hCloseBtn = CreateWindowA(
            "BUTTON", "Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, 220, 100, 30, hWnd, (HMENU)2, NULL, NULL
        );
        SendMessage(hCloseBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Extract config table from config.lua and display in the Edit box
        ExtractConfigTableAndShow(hWnd);

        return 0;
    }

    case WM_SIZE: {
        // Resize controls dynamically
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hEdit, 10, 10, width - 20, height - 60, TRUE);
        MoveWindow(hSaveBtn, (width - 200) / 2, height - 45, 100, 30, TRUE);
        MoveWindow(hCloseBtn, (width - 200) / 2 + 120, height - 45, 100, 30, TRUE);

        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: {  // Save button pressed
            int len = GetWindowTextLengthA(hEdit);
            char* text = malloc(len + 1);
            GetWindowTextA(hEdit, text, len + 1);
            FILE* f = fopen("config.lua", "w");
            if (f) {
                fwrite(text, 1, len, f);
                fclose(f);
            }
            free(text);

            // Reload Lua state
            if (luaL_dofile(gLua, "config.lua") != LUA_OK) {
                void ApplyLuaConfig(lua_State *L, HWND hMainWnd, HWND hEdit);
                MessageBoxA(hWnd, lua_tostring(gLua, -1), "Lua Error", MB_ICONERROR);
                lua_pop(gLua, 1);
            } else {
                MessageBoxA(hWnd, "Configuration reloaded.", "Info", MB_OK);
            }
            return 0;
        }

        case 2:  // Close button pressed
            DestroyWindow(hWnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        // Clean up resources when window is destroyed
        if (hFont) {
            DeleteObject(hFont);
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ExtractConfigTableAndShow(HWND hWnd) {
    // Open the Lua file and load it into Lua state
    if (luaL_dofile(gLua, "config.lua") != LUA_OK) {
        MessageBoxA(hWnd, lua_tostring(gLua, -1), "Lua Error", MB_ICONERROR);
        lua_pop(gLua, 1);
        return;
    }

    // Check if the 'config' table is defined in Lua
    lua_getglobal(gLua, "config");  // Push 'config' table onto the stack

    if (!lua_istable(gLua, -1)) {
        // If 'config' is not a table, show an error
        MessageBoxA(hWnd, "'config' table not found or is not a table in config.lua", "Error", MB_ICONERROR);
        lua_pop(gLua, 1);  // Pop the non-table value
        return;
    }

    // Now that we know 'config' is a table, proceed to iterate over it
    lua_pushnil(gLua);  // First key for iteration
    char buf[8192] = "";
    
    while (lua_next(gLua, -2)) {
        // For each entry in the 'config' table
        const char* key = lua_tostring(gLua, -2);
        const char* value = lua_tostring(gLua, -1);

        // Make sure the value is not nil
        if (value != NULL) {
            // Format the key-value pairs to a string
            strcat(buf, key);
            strcat(buf, " = ");
            strcat(buf, value);
            strcat(buf, "\n");
        } else {
            // If the value is nil, show a warning
            char warning[512];
            sprintf(warning, "Warning: Key '%s' has a nil value.", key);
            MessageBoxA(hWnd, warning, "Warning", MB_ICONWARNING);
        }

        // Pop value, leave key for the next iteration
        lua_pop(gLua, 1);
    }

    // Set the text of the Edit control
    SetWindowTextA(hEdit, buf);

    // Pop the 'config' table
    lua_pop(gLua, 1);
}


void SaveFunction(HWND hWnd, HWND hEdit) {
    OPENFILENAME ofn = { 0 };
    char szFileName[MAX_PATH] = "";

    // Initialize OPENFILENAME structure
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Save File";
    ofn.lpstrDefExt = "txt";

    // Get the text length and allocate buffer
    int length = GetWindowTextLength(hEdit) + 1;
    if (length <= 1) {
        MessageBox(hWnd, "No text to save!", "Save", MB_OK | MB_ICONINFORMATION);
        return;
    }

    char* buffer = (char*)malloc(length);
    if (!buffer) {
        MessageBox(hWnd, "Memory allocation failed!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get the text from edit control
    GetWindowText(hEdit, buffer, length);

    // Show save file dialog
    if (!GetSaveFileName(&ofn)) {
        free(buffer);
        return;
    }

    // Save to file
    FILE* file = fopen(szFileName, "w");
    if (!file) {
        MessageBox(hWnd, "Failed to create file! Check permissions.", "Error", MB_OK | MB_ICONERROR);
        free(buffer);
        return;
    }

    // Write the text and close the file
    fputs(buffer, file);
    fclose(file);

    // Clean up
    free(buffer);

    // Show success message
    MessageBox(hWnd, "File saved successfully!", "Success", MB_OK);
}


void OpenFunction(HWND hWnd, HWND hEdit) {
    OPENFILENAME ofn = { 0 };
    char szFileName[MAX_PATH] = "";

    // Initialize Open File dialog parameters
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Open File";
    ofn.lpstrDefExt = "txt";

    // Handle file open operation
    if (GetOpenFileName(&ofn)) {
        FILE* file = fopen(szFileName, "r");
        if (file) {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            rewind(file);

            char* buffer = (char*)malloc(fileSize + 1);
            if (buffer) {
                fread(buffer, fileSize, 1, file);
                buffer[fileSize] = '\0';
                fclose(file);  // Correctly close the file here
                SetWindowText(hEdit, buffer);
                free(buffer);

                // Extract just the filename from the full path
                char* fileNameOnly = strrchr(szFileName, '\\');
                if (fileNameOnly) {
                    fileNameOnly++; // Skip the backslash
                }
                else {
                    fileNameOnly = szFileName; // No path, just file name
                }

                // Create new window title
                char newTitle[256];
                snprintf(newTitle, sizeof(newTitle), "Super Pad | %s", fileNameOnly);
                SetWindowText(hWnd, newTitle);
            }
        }
    }
}

extern lua_State *gLua;

// Forward
BOOL CALLBACK ConfigDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ShowConfigDialog(HWND hParent) {
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ConfigWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowExA(
        0,
        "ConfigWindowClass",
        "Edit Config.lua",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        hParent, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hWnd, SW_SHOW);
}

BOOL CALLBACK ConfigDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    switch (uMsg) {
    case WM_INITDIALOG:
        hEdit = GetDlgItem(hDlg, IDC_CONFIG_EDIT);
        {
            FILE *f = fopen("config.lua", "r");
            if (f) {
                char buf[8192];
                size_t n = fread(buf, 1, sizeof(buf)-1, f);
                buf[n] = '\0';
                SetWindowTextA(hEdit, buf);
                fclose(f);
            }
        }
        SetFocus(hEdit);        // Give keyboard focus to the edit control
        return FALSE;           // FALSE = we set the focus ourselves

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CONFIG_SAVE: {
            // Save back to file
            int len = GetWindowTextLengthA(hEdit);
            char *text = malloc(len+1);
            GetWindowTextA(hEdit, text, len+1);
            FILE *f = fopen("config.lua", "w");
            if (f) {
                fwrite(text, 1, len, f);
                fclose(f);
            }
            free(text);

            // Reload Lua state
            if (luaL_dofile(gLua, "config.lua") != LUA_OK) {
                const char *err = lua_tostring(gLua, -1);
                MessageBoxA(hDlg, err, "Lua Error", MB_ICONERROR);
                lua_pop(gLua,1);
            } else {
                MessageBoxA(hDlg, "Configuration reloaded.", "Info", MB_ICONINFORMATION);
            }
            return TRUE;
        }
        case IDC_CONFIG_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;
        }
        
        break;
    }
    return FALSE;
}

void ApplyLuaConfig(lua_State *L, HWND hMainWnd, HWND hEdit) {
    // Push the global 'config' table from Lua
    lua_getglobal(L, "config");

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        MessageBoxA(NULL, "No 'config' table found in Lua!", "Lua Error", MB_ICONERROR);
        return;
    }

    // Debug: Check if 'config' table contains 'window_title'
    lua_getfield(L, -1, "window_title");
    if (lua_isstring(L, -1)) {
        const char *title = lua_tostring(L, -1);
        MessageBoxA(NULL, title, "Title from Lua", MB_OK);  // Debug: Show title before applying
        SetWindowTextA(hMainWnd, title);
    } else {
        MessageBoxA(NULL, "'window_title' is missing or invalid in config.lua!", "Lua Error", MB_ICONERROR);
    }
    lua_pop(L, 1);  // Pop window_title

    // Debug: Check if 'font_size' exists
    lua_getfield(L, -1, "font_size");
    if (lua_isnumber(L, -1)) {
        int fontSize = (int)lua_tointeger(L, -1);
        LOGFONT lf = {0};
        lf.lfHeight = -fontSize;
        strcpy(lf.lfFaceName, "Consolas");
        HFONT hNewFont = CreateFontIndirect(&lf);
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hNewFont, TRUE);
    } else {
        MessageBoxA(NULL, "'font_size' is missing or invalid in config.lua!", "Lua Error", MB_ICONERROR);
    }
    lua_pop(L, 1);  // Pop font_size

    lua_pop(L, 1); // Pop config table
}

        }
    }
}

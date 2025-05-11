#include <Windows.h>
#include <stdio.h>
#include "func.c"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>


HFONT hFont; // Global font handle for the custom font
// Control identifiers for UI elements
#define ID_EDITCHILD 1001     // Multiline edit control for text input/display
#define ID_SAVE_BUTTON 1002   // Button to trigger file save operation
#define ID_OPEN_BUTTON 1003   // Button to trigger file open operation
// Includes the lua config
#define CONFIG_PATH "D:/VSCODE/C/Notepad/config.lua"

/**
 * Saves the contents of the edit control to a file.
 *
 * @param hWnd   Handle to the parent window
 * @param hEdit  Handle to the edit control containing the text to save
 * @return      TRUE if save was successful, FALSE otherwise
 */



LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit, hSaveButton, hOpenButton;

    switch (uMsg) {
    case WM_CREATE: {
        // Initialize UI controls during window creation
        hSaveButton = CreateWindow(
            "BUTTON", "Save File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 100, 30, hWnd, (HMENU)ID_SAVE_BUTTON, GetModuleHandle(NULL), NULL
        );

        hOpenButton = CreateWindow(  
            "BUTTON", "Open File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 10, 100, 30, hWnd, (HMENU)ID_OPEN_BUTTON, GetModuleHandle(NULL), NULL
        );

        hEdit = CreateWindowEx(
            0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 52, 480, 358, hWnd, (HMENU)ID_EDITCHILD, GetModuleHandle(NULL), NULL
        );

        // Set custom font (e.g., Consolas, size 16)
        LOGFONT lf = { 0 };
        lf.lfHeight = -16; // Negative for character height
        lf.lfWeight = FW_NORMAL;
        strcpy(lf.lfFaceName, "Consolas"); // Change to your preferred font
        hFont = CreateFontIndirect(&lf);

        // Apply font to the edit control
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hSaveButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hOpenButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }
    case WM_SIZE: {
        // Handle window resize events and adjust control positions
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Maintain consistent positioning of buttons
        MoveWindow(hSaveButton, 10, 10, 100, 30, TRUE);
        MoveWindow(hOpenButton, 120, 10, 100, 30, TRUE);

        // Resize edit control to fill remaining space
        MoveWindow(hEdit, 10, 52, width - 20, height - 62, TRUE);

        // Force redraw after size adjustment
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_PAINT: {
        // Custom painting logic for visual elements
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Draw horizontal separator line
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        RECT rect;
        GetClientRect(hWnd, &rect);
        MoveToEx(hdc, 10, 50, NULL);
        LineTo(hdc, rect.right, 50);

        // Restore original pen
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        // Optimize background erasure
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
        return 1;
    }
    case WM_COMMAND: {
        // Process control-specific commands
        switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON: {
            SaveFunction(hWnd, hEdit);
            break;
        }
        case ID_OPEN_BUTTON: {
            OpenFunction(hWnd,hEdit);
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
        // Check if there's text to save
        int length = GetWindowTextLength(hEdit);
        if (length > 0) {
            int result = MessageBox(
                hWnd,
                "Do you want to save before closing the Notepad?",
                "Save",
                MB_YESNO | MB_ICONQUESTION
            );

            if (result == IDYES) {
                SaveFunction(hWnd, hEdit);
            }
        }

        // Clean up during window destruction
        if (hFont) {
            DeleteObject(hFont);
        }
        PostQuitMessage(0);
        return 0;
   
    default:
       return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


// Initialize Lua and load configuration
int InitializeLua(lua_State *L) {
    // Open Lua libraries
    luaL_openlibs(L);

    // Try loading the Lua config file
    if (luaL_dofile(L, CONFIG_PATH) != LUA_OK) {
        const char *error = lua_tostring(L, -1);
        MessageBoxA(NULL, error, "Lua Error", MB_ICONWARNING);
        lua_pop(L, 1); // remove error message from stack
        return -1; // If error occurs, return
    }

    // Try fetching the 'config' table from Lua
    lua_getglobal(L, "config");
    if (lua_isnil(L, -1)) {
        MessageBoxA(NULL, "Lua configuration not found!", "Error", MB_ICONERROR);
        lua_pop(L, 1); // pop the nil value from stack
        return -1;
    }

    // Here you can fetch specific properties like background opacity, font size, etc.
    lua_getfield(L, -1, "background_opacity");
    if (lua_isnumber(L, -1)) {
        float backgroundOpacity = lua_tonumber(L, -1);
        // Apply the configuration to your UI (for example)
        printf("Background Opacity: %f\n", backgroundOpacity);
    }
    lua_pop(L, 1); // pop the value

    // More config parsing goes here...

    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    lua_State *L = luaL_newstate();

    // Initialize Lua and load configuration
    if (InitializeLua(L) != 0) {
        // If Lua initialization fails, exit the program
        return 1;
    }

    // Register the window class
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "NotepadClass";

    RegisterClassA(&wc);

    // Create the main window
    HWND hWnd = CreateWindowA(
        "NotepadClass", "Super Pad | %", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL
    );

    // Show and update the window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Close the Lua state before exiting
    lua_close(L);

    return msg.wParam;
}

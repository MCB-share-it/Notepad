#include <Windows.h>
#include <stdio.h>
#include "func.c"

HFONT hFont; // Global font handle for the custom font
// Control identifiers for UI elements
#define ID_EDITCHILD 1001     // Multiline edit control for text input/display
#define ID_SAVE_BUTTON 1002   // Button to trigger file save operation
#define ID_OPEN_BUTTON 1003   // Button to trigger file open operation

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
            // Implement file open functionality
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
                        fclose(file);
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
                fclose(file);
            }
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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
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

    return msg.wParam;
}

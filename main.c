#include <Windows.h>
#include <stdio.h>

#define ID_EDITCHILD 1001 // Identifier for the Edit Control 
#define ID_SAVE_BUTTON 1002 // Identifier for the Save Button

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;       // Handle to the Edit Control
    static HWND hSaveButton; // Handle to the Save Button

    switch (uMsg) {
    case WM_CREATE: {
        // Create the Edit Control
        hEdit = CreateWindowEx(
            0,                     // Extended styles
            "EDIT",                // Predefined Edit Control
            "",                    // Default text
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 10, 480, 400,      // Position and size of the Edit Control
            hWnd,                  // Parent window handle
            (HMENU)ID_EDITCHILD,   // Control identifier
            GetModuleHandle(NULL), // Handle to the instance
            NULL                   // Pointer not needed
        );

        // Create the Save Button
        hSaveButton = CreateWindow(
            "BUTTON",               // Predefined class
            "Save",                 // Button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
            10, 420, 100, 30,       // Position and size of the button
            hWnd,                   // Parent window handle
            (HMENU)ID_SAVE_BUTTON,  // Control identifier
            GetModuleHandle(NULL),  // Handle to the instance
            NULL                    // Pointer not needed
        );
        break;
    }
        
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON: {
            // Save button clicked
            int length = GetWindowTextLength(hEdit) + 1; // Get text length
            char* buffer = (char*)malloc(length);        // Allocate buffer
            if (buffer == NULL) {
                MessageBox(hWnd, "Malloc failed!", "Error", MB_OK | MB_ICONERROR);
                break;
            }
            GetWindowText(hEdit, buffer, length);       // Get the text from the edit control

            // Save the text to a file
            FILE* file = fopen("output.txt", "w");
            if (file) {
                fputs(buffer, file);                    // Write buffer to file
                fclose(file);
                MessageBox(hWnd, "File saved successfully!", "Success", MB_OK);
            }
            else {
                MessageBox(hWnd, "Failed to save file.", "Error", MB_OK | MB_ICONERROR);
            }

            free(buffer); // Free the allocated buffer
            break;
        }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "NotepadClass";

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowA(
        "NotepadClass",
        "Super Pad",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500, // Adjusted window size
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

#include <Windows.h>
#include <stdio.h>

#define ID_EDITCHILD 1001 // Identifier for the Edit Control
#define ID_SAVE_BUTTON 1002 // Identifier for the Save Button

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;       // Handle to the Edit Control
    static HWND hSaveButton; // Handle to the Save Button

    switch (uMsg) {
    case WM_CREATE: {
        // Create the Save Button
        hSaveButton = CreateWindow(
            "BUTTON",              
            "Save",                
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
            10, 10, 100, 30,         
            hWnd,                   
            (HMENU)ID_SAVE_BUTTON,  
            GetModuleHandle(NULL), 
            NULL                   
        );

        // Create the Edit Control
        hEdit = CreateWindowEx(
            0,                     
            "EDIT",                
            "",                    // Default text
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 52, 480, 358,      // Adjusted position to make space for the separator line
            hWnd,                  // Parent window handle
            (HMENU)ID_EDITCHILD,   // Control identifier
            GetModuleHandle(NULL), // Handle to the instance
            NULL                   // Pointer not needed
        );
        break;
    }
    case WM_SIZE: {
        // Adjust the size and position of the Save Button and Edit Control
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hSaveButton, 10, 10, 100, 30, TRUE);
        MoveWindow(hEdit, 10, 52, width - 20, height - 62, TRUE);

        // Force the window to repaint to prevent black areas
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Draw a thin line between the button and the edit control
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // Black thin line
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        MoveToEx(hdc, 10, 50, NULL);
        LineTo(hdc, LOWORD(GetClientRect(hWnd, &ps.rcPaint)), 50);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_ERASEBKGND: {
        // Handle background erasing to prevent flickering
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
        return 1; // Indicate that the background has been erased
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Set default background brush
    wc.lpszClassName = "NotepadClass";

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowA(
        "NotepadClass",
        "Super Pad",
        WS_OVERLAPPEDWINDOW, // Ensures the window is resizable
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500, // Initial window size
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

// Required headers for Windows GUI functionality and standard input/output
#include <Windows.h>
#include <stdio.h>

// Control identifier constants for window elements
#define ID_EDITCHILD 1001     // Identifier for text edit area
#define ID_SAVE_BUTTON 1002   // Identifier for save file button
#define ID_OPEN_BUTTON 1003   // Identifier for open file button

/**
 * Window procedure callback function that handles all messages for the application window
 * @param hWnd   Handle to the current window
 * @param uMsg   Message being processed
 * @param wParam Additional message information
 * @param lParam Additional message information
 * @return Result of message processing
 */
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Static storage for window controls to maintain references across messages
    static HWND hEdit, hSaveButton, hOpenButton;

    switch (uMsg) {
    case WM_CREATE: {
        // Initialize UI controls during window creation
        
        // Create Save button with default pushbutton style
        hSaveButton = CreateWindow(
            "BUTTON", "Save File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 100, 30, hWnd, (HMENU)ID_SAVE_BUTTON, GetModuleHandle(NULL), NULL
        );

        // Create Open button positioned next to Save button
        hOpenButton = CreateWindow(
            "BUTTON", "Open File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 10, 100, 30, hWnd, (HMENU)ID_OPEN_BUTTON, GetModuleHandle(NULL), NULL
        );

        // Create multi-line edit control for text input/display
        hEdit = CreateWindowEx(
            0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 52, 480, 358, hWnd, (HMENU)ID_EDITCHILD, GetModuleHandle(NULL), NULL
        );
        break;
    }

    case WM_SIZE: {
        // Handle window resizing to maintain UI layout
        
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Update positions of buttons and edit control
        MoveWindow(hSaveButton, 10, 10, 100, 30, TRUE);
        MoveWindow(hOpenButton, 120, 10, 100, 30, TRUE);
        
        // Resize edit control to fill remaining space
        MoveWindow(hEdit, 10, 52, width - 20, height - 62, TRUE);

        // Force redraw after resize
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }

    case WM_PAINT: {
        // Custom painting handler for drawing window decorations
        
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Create and select pen for drawing horizontal separator line
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        // Draw horizontal separator below buttons
        MoveToEx(hdc, 10, 50, NULL);
        LineTo(hdc, LOWORD(GetClientRect(hWnd, &ps.rcPaint)), 50);

        // Restore original pen and cleanup
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
        break;
    }
    
    case WM_COMMAND: {
        // Handle button clicks and other control notifications
        
        switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON: {
            // Initialize OPENFILENAME structure for file saving dialog
            OPENFILENAME ofn;
            char szFileName[MAX_PATH] = "";

            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            
            // Set file filters and default extension
            ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST;
            ofn.lpstrTitle = "Save File";
            ofn.lpstrDefExt = "txt";

            if (GetSaveFileName(&ofn)) {
                // Retrieve text from edit control and save to file
                int length = GetWindowTextLength(hEdit) + 1;
                char* buffer = (char*)malloc(length);
                if (buffer != NULL) {
                    GetWindowText(hEdit, buffer, length);
                    FILE* file = fopen(szFileName, "w");
                    if (file) {
                        fputs(buffer, file);
                        fclose(file);
                        MessageBox(hWnd, "File saved successfully!", "Success", MB_OK);
                    }
                    free(buffer);
                }
            }
            break;
        }

        case ID_OPEN_BUTTON

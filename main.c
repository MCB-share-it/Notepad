#include <Windows.h>
#include <stdio.h>

#define ID_EDITCHILD 1001
#define ID_SAVE_BUTTON 1002
#define ID_OPEN_BUTTON 1003

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit, hSaveButton, hOpenButton;

    switch (uMsg) {
    case WM_CREATE: {
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
        break;
    }
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hSaveButton, 10, 10, 100, 30, TRUE);
        MoveWindow(hOpenButton, 120, 10, 100, 30, TRUE);
        MoveWindow(hEdit, 10, 52, width - 20, height - 62, TRUE);

        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        MoveToEx(hdc, 10, 50, NULL);
        LineTo(hdc, LOWORD(GetClientRect(hWnd, &ps.rcPaint)), 50);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
        break;
    }
    
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON: {
            OPENFILENAME ofn;
            char szFileName[MAX_PATH] = "";

            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST;
            ofn.lpstrTitle = "Save File";
            ofn.lpstrDefExt = "txt";

            if (GetSaveFileName(&ofn)) {
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

        case ID_OPEN_BUTTON: {
            OPENFILENAME ofn;
            char szFileName[MAX_PATH] = "";

            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            ofn.lpstrTitle = "Open File";
            ofn.lpstrDefExt = "txt";

            if (GetOpenFileName(&ofn)) {
                FILE* file = fopen(szFileName, "r");
                if (file) {
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    rewind(file);

                    char* buffer = (char*)malloc(fileSize + 1);
                    if (buffer != NULL) {
                        fread(buffer, fileSize, 1, file);
                        buffer[fileSize] = '\0';
                        fclose(file);
                        SetWindowText(hEdit, buffer);
                        free(buffer);
                    }
                }
            }
            break;
        }

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "NotepadClass";

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowA(
        "NotepadClass", "Super Pad | %", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
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
}

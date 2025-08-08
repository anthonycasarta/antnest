#include <windows.h>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

class FolderWatcher
{
public:
    FolderWatcher(const std::wstring &path) : dirPath(path), stopRequested(false) {}

    ~FolderWatcher()
    {
    }

    void start()
    {
        stopRequested = false;
        watcherThread = std::thread(&FolderWatcher::watch, this); // Call watch on a new thread
    }

    void stop()
    {
        stopRequested = true;
        if (watcherThread.joinable())
        {
            watcherThread.join(); // Wait for thread to finish execution
        }
    }

    void setCallback(std::function<void(const std::wstring &, const std::wstring &)> cb)
    {
        callback = cb;
    }

private:
    std::wstring dirPath;
    std::atomic<bool> stopRequested;
    std::thread watcherThread;
    std::function<void(const std::wstring &, const std::wstring &)> callback;

    void watch()
    {
        // Grab the Handle of a folder
        HANDLE dirHandle = CreateFileW(
            dirPath.c_str(),                                        // Folder path (wide string)
            FILE_LIST_DIRECTORY,                                    // Ask for dir change notifs
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // Let others access it
            NULL,                                                   // Default security
            OPEN_EXISTING,                                          // Opening existing folder
            FILE_FLAG_BACKUP_SEMANTICS,                             // Needed to open dirs
            NULL);                                                  // No template

        // Allocate a buffer to store file changes
        const DWORD bufferSize = 1024 * 10;
        BYTE buffer[bufferSize];
        DWORD bytesReturned; // Number of bytes used returned from Windows

        while (!stopRequested)
        {

            if (ReadDirectoryChangesW(
                    dirHandle,                                                                              // Folder Handle
                    buffer,                                                                                 // Memory to store results
                    bufferSize,                                                                             // Size of Memory
                    TRUE,                                                                                   // Watch all sub folders
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE, // Which events to watch
                    &bytesReturned,                                                                         // Windows returns number of bytes here
                    NULL,                                                                                   // Not using OVERLAPPED async mode
                    NULL))
            {
                // Grab the stored results
                FILE_NOTIFY_INFORMATION *pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);

                // Grab the filename
                std::wstring filename(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

                // Extract the changes made
                std::wstring action;
                switch (pNotify->Action)
                {
                case FILE_ACTION_ADDED:
                    action = L"added";
                    break;
                case FILE_ACTION_REMOVED:
                    action = L"removed";
                    break;
                case FILE_ACTION_MODIFIED:
                    action = L"modified";
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                    action = L"renamed_from";
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    action = L"renamed_to";
                    break;

                default:
                    action = L"unknown";
                    break;
                }

                // Return the changes made to the file without exiting the watch function
                if (callback)
                {
                    callback(action, filename);
                }

                // Handle multiple notfis in buffer
                if (pNotify->NextEntryOffset != 0)
                {
                    pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(
                        reinterpret_cast<BYTE *>(pNotify) + pNotify->NextEntryOffset);
                }
                else
                {
                    break;
                }
            }

            Sleep(100); // Prevents CPU from running at 100% if nothing happens
        }
        CloseHandle(dirHandle);
    }
};
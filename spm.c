#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PATH_SIZE 32767

void list_path();
void add_path(const char* new_path);
void remove_path_by_index(int index);
BOOL set_registry_path(const char* new_path_value);
char* get_current_path(char* buffer, DWORD size);
BOOL ensure_admin_privileges();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: spm <command> [arguments]\n");
        printf("Commands:\n");
        printf("  list           - List all paths with indexes\n");
        printf("  add <path>     - Add a new path\n");
        printf("  remove <index> - Remove path at specified index\n");
        return 1;
    }

    if (!ensure_admin_privileges()) {
        printf("Failed to obtain administrative privileges\n");
        printf("Please run as administrator\n");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list_path();
    } else if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            printf("Usage: spm add <path>\n");
            return 1;
        }
        add_path(argv[2]);
    } else if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            printf("Usage: spm remove <index>\n");
            return 1;
        }
        int index = atoi(argv[2]);
        remove_path_by_index(index);
    } else {
        printf("Unknown command: %s\n", argv[1]);
        printf("Available commands: list, add, remove\n");
        return 1;
    }

    return 0;
}

void list_path() {
    char path_value[MAX_PATH_SIZE];
    if (get_current_path(path_value, MAX_PATH_SIZE) == NULL) {
        printf("Error reading PATH: %ld\n", GetLastError());
        return;
    }

    printf("\nCurrent PATH entries:\n");
    int index = 0;
    char* token = strtok(path_value, ";");
    while (token != NULL) {
        printf("[%d] %s\n", index++, token);
        token = strtok(NULL, ";");
    }
}

void add_path(const char* new_path) {
    char current_path[MAX_PATH_SIZE];
    char full_path[MAX_PATH_SIZE];
    char new_path_value[MAX_PATH_SIZE];

    // Convert relative path to absolute
    if (GetFullPathName(new_path, MAX_PATH_SIZE, full_path, NULL) == 0) {
        printf("Error converting path '%s': %ld\n", new_path, GetLastError());
        return;
    }

    if (get_current_path(current_path, MAX_PATH_SIZE) == NULL) {
        printf("Error reading current PATH: %ld\n", GetLastError());
        return;
    }

    // Remove trailing semicolon if present in current_path
    size_t len = strlen(current_path);
    if (len > 0 && current_path[len - 1] == ';') {
        current_path[len - 1] = '\0';
    }

    if (strstr(current_path, full_path) != NULL) {
        printf("Path already exists: %s\n", full_path);
        return;
    }

    // Ensure we don't exceed MAX_PATH_SIZE
    if (snprintf(new_path_value, MAX_PATH_SIZE, "%s;%s", current_path, full_path) >= MAX_PATH_SIZE) {
        printf("Error: Resulting PATH would exceed maximum length\n");
        return;
    }

    if (set_registry_path(new_path_value)) {
        printf("Added path: %s\n", full_path);
    } else {
        printf("Failed to add path: %ld\n", GetLastError());
    }
}

void remove_path_by_index(int index) {
    char current_path[MAX_PATH_SIZE];
    char new_path_value[MAX_PATH_SIZE] = "";
    int current_index = 0;

    if (get_current_path(current_path, MAX_PATH_SIZE) == NULL) {
        printf("Error reading current PATH: %ld\n", GetLastError());
        return;
    }

    char* token = strtok(current_path, ";");
    while (token != NULL) {
        if (current_index != index) {
            if (strlen(new_path_value) > 0) {
                strcat(new_path_value, ";");
            }
            strcat(new_path_value, token);
        }
        current_index++;
        token = strtok(NULL, ";");
    }

    if (current_index <= index) {
        printf("Index %d not found\n", index);
        return;
    }

    if (set_registry_path(new_path_value)) {
        printf("Removed path at index %d\n", index);
    } else {
        printf("Failed to remove path: %ld\n", GetLastError());
    }
}

char* get_current_path(char* buffer, DWORD size) {
    HKEY h_key;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                     0, KEY_READ, &h_key) != ERROR_SUCCESS) {
        return NULL;
    }

    DWORD type;
    if (RegQueryValueEx(h_key, "Path", NULL, &type, 
                        (LPBYTE)buffer, &size) != ERROR_SUCCESS) {
        RegCloseKey(h_key);
        return NULL;
    }

    RegCloseKey(h_key);
    return buffer;
}

BOOL set_registry_path(const char* new_path_value) {
    HKEY h_key;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                              0, KEY_SET_VALUE, &h_key);
    if (result != ERROR_SUCCESS) {
        SetLastError(result);
        return FALSE;
    }

    result = RegSetValueEx(h_key, "Path", 0, REG_EXPAND_SZ,
                          (BYTE*)new_path_value, strlen(new_path_value) + 1);
    if (result == ERROR_SUCCESS) {
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                          (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    }

    RegCloseKey(h_key);
    return result == ERROR_SUCCESS;
}

BOOL ensure_admin_privileges() {
    HANDLE h_token;
    if (!OpenProcessToken(GetCurrentProcess(), 
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &h_token)) {
        return FALSE;
    }

    TOKEN_PRIVILEGES tkp;
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL success = AdjustTokenPrivileges(h_token, FALSE, &tkp, 0, NULL, NULL);
    CloseHandle(h_token);
    return success;
}

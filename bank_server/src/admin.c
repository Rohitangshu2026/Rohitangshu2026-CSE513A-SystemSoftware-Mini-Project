#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "../include/admin_utils.h"
#include "../include/admin.h"
#include "../include/common.h"

void adminMenu(int sock, UserLockInfo *lockInfo) {
    Admin currentAdmin;
    memset(&currentAdmin, 0, sizeof(Admin));
    char buf[256], choiceBuf[32];
    write(sock, "Enter username: ", 16);
    if (readLine(sock, currentAdmin.username, sizeof(currentAdmin.username)) <= 0) {
        return;
    }
    write(sock, "Enter password: ", 15);
    if (readLine(sock, currentAdmin.password, sizeof(currentAdmin.password)) <= 0) {
        return;
    }
    int result = validateAdmin(currentAdmin.username, currentAdmin.password, lockInfo);
    if (result == -2) {
        write(sock, "Admin already logged in from another terminal.\n", 47);
        return;
    } else if (result == 0) {
        write(sock, "Invalid credentials.\n", 21);
        return;
    }
    write(sock, "Login successful!\n", 18);
    while (1) {
        snprintf(buf, sizeof(buf),
                 "\n===== ADMIN MENU =====\n"
                 "1. Add New Bank Employee\n"
                 "2. Modify Customer/Employee Details\n"
                 "3. Manage User Roles\n"
                 "4. Change Admin Password\n"
                 "5. View Logs\n"
                 "6. Logout\n"
                 "Enter your choice: ");
        write(sock, buf, strlen(buf));
        memset(choiceBuf, 0, sizeof(choiceBuf));
        ssize_t bytes = readLine(sock, choiceBuf, sizeof(choiceBuf) - 1);
        if (bytes <= 0) {
            write(sock, "Connection closed by client.\n", 29);
            unlockAdmin(lockInfo);
            return;
        }
        choiceBuf[strcspn(choiceBuf, "\r\n")] = '\0';
        int valid = 1;
        if (strlen(choiceBuf) == 0) { 
             valid = 0;
        }
        for (int i = 0; choiceBuf[i] != '\0'; i++) {
            if (!isdigit((unsigned char)choiceBuf[i])) {
                valid = 0;
                break;
            }
        }
        if (!valid) {
            write(sock, "Invalid input. Please enter a number.\n", 38);
            continue;
        }
        int choice = atoi(choiceBuf);
        if (choice < 1 || choice > 6) {
            write(sock, "Invalid choice. Please enter 1-6.\n", 34);
            continue;
        }
        switch (choice) {
            case 1:
                addEmployee(sock);
                break;
            case 2:
                modifyUser(sock);
                break;
            case 3:
                manageUserRoles(sock);
                break;
            case 4:
                changeAdminPassword(sock, &currentAdmin, lockInfo);
                break;
            case 5:
                viewLogs(sock);
                break;
            case 6: 
                unlockAdmin(lockInfo);
                write(sock, "Logged out successfully.\n", 25);
                return;
            default:
                write(sock, "Unknown error occurred.\n", 24);
                break;
        }
    }
}
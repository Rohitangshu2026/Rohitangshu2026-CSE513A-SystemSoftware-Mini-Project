#ifndef ADMIN_UTILS_H
#define ADMIN_UTILS_H

#include "../include/common.h"

void addEmployee(int sock);
void deleteUser(int sock);
void viewLogs(int sock);
void adminMenu(int sock, UserLockInfo *lockInfo);
void unlockAdmin(UserLockInfo *lockInfo);
int validateAdmin(const char *username, const char *password, UserLockInfo *lockInfo);
void changeAdminPassword(int sock, Admin *admin, UserLockInfo *lockInfo);
void modifyUser(int sock);                    
void manageUserRoles(int sock);                  

#endif
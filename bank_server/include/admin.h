#ifndef ADMIN_UTILS_H
#define ADMIN_UTILS_H

#include "../include/common.h"
#include <sys/types.h> 

int validateAdmin(const char *username, const char *password, UserLockInfo *lockInfo);
void unlockAdmin(UserLockInfo *lockInfo);
void addUser(int sock);
void deleteUser(int sock);
void viewLogs(int sock);
void adminMenu(int sock, UserLockInfo *lockInfo);

#endif 

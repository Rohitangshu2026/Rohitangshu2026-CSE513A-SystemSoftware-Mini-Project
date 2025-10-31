#ifndef MANAGER_UTILS_H
#define MANAGER_UTILS_H

#include "common.h"

int validateManager(const char *username, const char *password);
void logoutManager(int session_fd);
int toggleCustomerStatus(int sock, int userId);
int assignLoanToEmployee(int sock, int userId);
int reviewCustomerFeedback(int sock, int userId);
int changeManagerPassword(int sock, int userId);

#endif
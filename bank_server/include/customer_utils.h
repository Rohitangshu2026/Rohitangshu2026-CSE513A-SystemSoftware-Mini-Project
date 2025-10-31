#ifndef CUSTOMER_UTILS_H
#define CUSTOMER_UTILS_H

#include "common.h"

int validateCustomer(const char *username, const char *password, Customer *customerOut);
void logoutCustomer(int session_fd);
int viewBalance(int sock, int accountId, int userId);
int depositMoney(int sock, int accountId, int userId);
int withdrawMoney(int sock, int accountId, int userId);
int transferFunds(int sock, int sourceAccountId, int userId);
int applyForLoan(int sock, int sourceAccountId, int userId);
int viewTransactionHistory(int sock, int sourceAccountId, int userId);
int changeCustomerPassword(int sock, int userId);
int addFeedback(int sock, int userId);



#endif 
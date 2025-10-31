#ifndef EMPLOYEE_UTILS_H
#define EMPLOYEE_UTILS_H

#include "common.h"


int validateEmployee(const char *username, const char *password);
void logoutEmployee(int session_fd);
void addNewCustomer(int sock);
void modifyCustomerDetails(int sock);
void processLoan(int sock, int employeeId);
void viewAssignedLoans(int sock, int employeeId);
void viewCustomerTransactions(int sock);
void changeEmployeePassword(int sock, int userId);

#endif
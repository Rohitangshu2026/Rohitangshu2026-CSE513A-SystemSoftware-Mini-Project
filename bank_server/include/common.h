#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>  
#include <sys/types.h> 
#include <time.h>     



#define USER_FILE     "data/users.txt"
#define LOG_FILE      "data/logs.txt"
#define ADMIN_FILE    "data/admins.txt"
#define CUSTOMER_FILE "data/customers.txt"
#define SESSION_FILE  "data/sessions.txt"
#define TRANSACTION_FILE "data/transactions.txt"
#define FEEDBACK_FILE "data/feedback.txt"
#define LOAN_FILE     "data/loans.txt" 

ssize_t readLine(int sock, char *buf, size_t size);
int isUserActive(int userId); 
int checkUserRole(int userId, const char* expectedRole); 

#define BUFFER_SIZE 1024
#define HASHKEY     "$6$saltsalt$"

#define MAX_NAME    50
#define MAX_PASS    50
#define MAX_ROLE    30
#define MAX_TXN_TYPE 12
#define MAX_FEEDBACK 512 
#define MAX_STATUS   20
#define MAX_ASSIGN   50 

typedef struct {
    int id;
    char username[MAX_NAME];
    char password[MAX_PASS];
    int isActive;
} Admin;

typedef struct {
    int id;
    char username[MAX_NAME];
    char password[MAX_PASS];
    char role[MAX_ROLE]; 
    int isActive;
} User;


typedef struct {
    int id;                
    int userId;            
    double balance;
    int isActive;          
} Customer; 

typedef struct {
    time_t timestamp;
    int accountId;             
    char type[MAX_TXN_TYPE];    
    double amount;
    int relatedAccountId;       
} Transaction;


typedef struct {
    int userId;     
    int fd;     
    pid_t pid;      
} Session;

typedef struct {
    int fd;
    off_t start;
    off_t len;
} UserLockInfo; 

typedef struct {
    time_t timestamp;
    int userId;
    char message[MAX_FEEDBACK];
} Feedback;

typedef struct {
    int loanId;
    int accountId;     
    double amount;
    time_t timestamp;
    char status[MAX_STATUS];     
    int employeeId;
} Loan;
#endif 
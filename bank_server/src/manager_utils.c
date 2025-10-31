#include "../include/common.h"
#include "../include/manager.h"
#include "../include/manager_utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h> 

int validateManager(const char *username, const char *password) {
    int fd_user, fd_session;
    User user;
    int foundUserId = -1;

    fd_user = open(USER_FILE, O_RDONLY);
    if (fd_user < 0) {
        perror("open user file");
        return -1; 
    }

    while (read(fd_user, &user, sizeof(User)) == sizeof(User)) {
        if (strcmp(user.username, username) == 0 &&
            strcmp(user.password, password) == 0 &&
            strcmp(user.role, "manager") == 0 &&
            user.isActive == 1) {
            foundUserId = user.id;
            break;
        }
    }
    close(fd_user);

    if (foundUserId == -1) {
        return 0; 
    }

    fd_session = open(SESSION_FILE, O_RDWR);
    if (fd_session < 0) {
        perror("open session file");
        return -1;
    }

    Session session;
    off_t offset = 0;
    while (read(fd_session, &session, sizeof(Session)) == sizeof(Session)) {
        if (session.userId == foundUserId) {
            struct flock lock = {0};
            lock.l_type = F_WRLCK; 
            lock.l_whence = SEEK_SET;
            lock.l_start = offset;
            lock.l_len = sizeof(Session);

            if (fcntl(fd_session, F_SETLK, &lock) == -1) {
                close(fd_session);
                return -2; 
            }
            return fd_session;
        }
        offset += sizeof(Session);
    }

    close(fd_session);
    return -1; 
}

void logoutManager(int session_fd) {
    if (session_fd < 0) return;
    close(session_fd);
}

static int getEmployeeIdByUsername(int sock, const char *username) {
    int fd_user = open(USER_FILE, O_RDONLY);
    if (fd_user < 0) {
        write(sock, "Error opening user file.\n", 25);
        return 0;
    }
    User user;
    int employeeId = 0;
    while(read(fd_user, &user, sizeof(User)) == sizeof(User)) {
        if (strcmp(user.username, username) == 0 &&
            strcmp(user.role, "employee") == 0 &&
            user.isActive == 1) {
            
            employeeId = user.id;
            break;
        }
    }
    
    close(fd_user);

    if (employeeId == 0) {
        write(sock, "Employee not found or is not an active employee.\n", 49);
    }
    
    return employeeId; 
}

int toggleCustomerStatus(int sock, int userId) {
    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0;
    }

    char username[MAX_NAME];
    write(sock, "Enter customer username to toggle status: ", 42);
    if (readLine(sock, username, sizeof(username)) <= 0)
        return 1; 
    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; 
    }

    int fd = open(USER_FILE, O_RDWR);
    if (fd < 0) {
        write(sock, "Error opening user file.\n", 25);
        return 1;
    }

    User user;
    off_t offset = 0;
    int found = 0;

    while (read(fd, &user, sizeof(User)) == sizeof(User)) {
        if (strcmp(user.username, username) == 0) {
            if (strcmp(user.role, "customer") != 0) {
                write(sock, "This user is not a customer.\n", 31);
                close(fd);
                return 1;
            }
            found = 1;
            break;
        }
        offset += sizeof(User);
    }

    if (!found) {
        write(sock, "Customer (user) not found.\n", 28);
        close(fd);
        return 1;
    }

    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(User);

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        write(sock, "Record locked by another user. Try again later.\n", 50);
        close(fd);
        return 1;
    }

    lseek(fd, offset, SEEK_SET);
    if(read(fd, &user, sizeof(User)) != sizeof(User)) {
         write(sock, "Error reading record after lock.\n", 34);
         goto unlock_and_close; 
    }
    
    user.isActive = !user.isActive; 
    
    lseek(fd, offset, SEEK_SET);
    if (write(fd, &user, sizeof(User)) != sizeof(User)) {
        write(sock, "Failed to update status.\n", 26);
    } else {
        fsync(fd);
        if (user.isActive)
            write(sock, "Customer account activated successfully.\n", 43);
        else
            write(sock, "Customer account deactivated successfully.\n", 45);
    }

unlock_and_close:
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return 1; 
}

int assignLoanToEmployee(int sock, int userId) {
    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; 
    }
    
    char buffer[128];
    char employeeUsername[MAX_NAME];
    int loanId, employeeId;


    int fd_loan = open(LOAN_FILE, O_RDONLY);
    if (fd_loan < 0) {
        write(sock, "No loans found in the system.\n", 32);
    } 
    else {
        struct flock lock = {0};
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        
        if (fcntl(fd_loan, F_SETLK, &lock) == -1) { 
            write(sock, "Loan file is busy (a customer may be applying). Try again.\n", 58);
            close(fd_loan);
            return 1; 
        }

        write(sock, "\n--- Unassigned PENDING Loans ---\n", 36);
        Loan loan;
        int count = 0;
        while(read(fd_loan, &loan, sizeof(Loan)) == sizeof(Loan)) {
            if (strcmp(loan.status, "PENDING") == 0 && loan.employeeId == -1) {
                snprintf(buffer, sizeof(buffer), "  - Loan ID: %d | Account: %d | Amount: $%.2f\n",
                         loan.loanId, loan.accountId, loan.amount);
                write(sock, buffer, strlen(buffer));
                count++;
            }
        }
        
        if (count == 0) {
            write(sock, "  (No unassigned pending loans found)\n", 39);
        }
        write(sock, "----------------------------------\n", 35);

        lock.l_type = F_UNLCK;
        fcntl(fd_loan, F_SETLK, &lock);
        close(fd_loan);
    }
    
    write(sock, "Enter Loan ID to assign: ", 26);
    if (readLine(sock, buffer, sizeof(buffer)) <= 0) return 1;
    loanId = atoi(buffer);
    if (loanId <= 0) {
        return 1;
    }
    
    write(sock, "Enter Employee username to assign to: ", 40);
    if (readLine(sock, employeeUsername, sizeof(employeeUsername)) <= 0) return 1;

    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; 
    }

    employeeId = getEmployeeIdByUsername(sock, employeeUsername);
    if (employeeId == 0) {
        return 1; 
    }

    fd_loan = open(LOAN_FILE, O_RDWR);
    if (fd_loan < 0) {
        write(sock, "Error opening loan file.\n", 26);
        return 1;
    }
    
    Loan loan;
    off_t offset = 0;
    int found = 0;
    
    while(read(fd_loan, &loan, sizeof(Loan)) == sizeof(Loan)) {
        if (loan.loanId == loanId) {
            found = 1;
            break;
        }
        offset += sizeof(Loan);
    }
    
    if (!found) {
        write(sock, "Loan ID not found.\n", 20);
        close(fd_loan);
        return 1;
    }


    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(Loan);

    if (fcntl(fd_loan, F_SETLKW, &lock) == -1) {
        write(sock, "Loan record is busy. Try again later.\n", 40);
        close(fd_loan);
        return 1;
    }

    if (loan.employeeId != -1) { 
        snprintf(buffer, sizeof(buffer), "This loan is already assigned to employee ID %d.\n", loan.employeeId);
        write(sock, buffer, strlen(buffer));
    } else if (strcmp(loan.status, "PENDING") != 0) {
         snprintf(buffer, sizeof(buffer), "This loan is already %s and cannot be assigned.\n", loan.status);
        write(sock, buffer, strlen(buffer));
    } else {
        loan.employeeId = employeeId;
        
        lseek(fd_loan, offset, SEEK_SET);
        if (write(fd_loan, &loan, sizeof(Loan)) != sizeof(Loan)) {
            write(sock, "Error writing update to loan file.\n", 36);
        } 
        else {
            fsync(fd_loan);
            snprintf(buffer, sizeof(buffer), "Loan %d successfully assigned to %s (ID: %d)\n",
                     loanId, employeeUsername, employeeId);
            write(sock, buffer, strlen(buffer));
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_loan, F_SETLK, &lock);
    close(fd_loan);
    
    return 1; 
}

int reviewCustomerFeedback(int sock, int userId) {
    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; 
    }

    int fd_fb = open(FEEDBACK_FILE, O_RDONLY);
    if (fd_fb < 0) {
        write(sock, "No feedback has been submitted yet.\n", 38);
        return 1;
    }

    struct flock lock = {0};
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    
    if (fcntl(fd_fb, F_SETLK, &lock) == -1) {
        write(sock, "Feedback file is busy. Try again later.\n", 42);
        close(fd_fb);
        return 1;
    }

    Feedback fb;
    User user;
    char buffer[1024];
    char username[MAX_NAME];
    int count = 0;

    write(sock, "\n--- Customer Feedback Log ---\n", 31);

    while (read(fd_fb, &fb, sizeof(Feedback)) == sizeof(Feedback)) {
        char timeStr[30];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&fb.timestamp));
        
        strcpy(username, "Unknown User");
        int fd_user = open(USER_FILE, O_RDONLY);
        if (fd_user >= 0) {
            while (read(fd_user, &user, sizeof(User)) == sizeof(User)) {
                if (user.id == fb.userId) {
                    strncpy(username, user.username, MAX_NAME);
                    break;
                }
            }
            close(fd_user);
        }

        snprintf(buffer, sizeof(buffer),
                 "------------------------------\n"
                 "[%s] From: %s (ID: %d)\n"
                 "Message: %s\n",
                 timeStr, username, fb.userId, fb.message);

        write(sock, buffer, strlen(buffer));
        count++;
    }

    if (count == 0) {
        write(sock, "No feedback has been submitted yet.\n", 38);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_fb, F_SETLK, &lock);
    close(fd_fb);
    
    return 1; 
}

int changeManagerPassword(int sock, int userId) {
    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; 
    }

    char newPass[MAX_PASS], confirmPass[MAX_PASS];

    write(sock, "Enter new password: ", 21);
    if (readLine(sock, newPass, sizeof(newPass)) <= 0) return 1;

    write(sock, "Confirm new password: ", 23);
    if (readLine(sock, confirmPass, sizeof(confirmPass)) <= 0) return 1;

    if (!checkUserRole(userId, "manager")) {
        write(sock, "Your role has been changed. Logging out.\n", 42);
        return 0; // 
    }

    if (strcmp(newPass, confirmPass) != 0) {
        write(sock, "Passwords do not match.\n", 25);
        return 1;
    }

    int fd = open(USER_FILE, O_RDWR);
    if (fd < 0) {
        write(sock, "Error opening user file.\n", 25);
        return 1;
    }

    User user;
    off_t offset = 0;
    int found = 0;

    while (read(fd, &user, sizeof(User)) == sizeof(User)) {
        if (user.id == userId) {
            found = 1;
            break;
        }
        offset += sizeof(User);
    }

    if (!found) {
        write(sock, "Manager record not found.\n", 28);
        close(fd);
        return 1;
    }

    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(User);

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        write(sock, "Record is locked by another user. Try again later.\n", 52);
        close(fd);
        return 1;
    }

    lseek(fd, offset, SEEK_SET);
    if(read(fd, &user, sizeof(User)) != sizeof(User)) {
         write(sock, "Error reading record after lock.\n", 34);
         goto unlock_and_close_pw;
    }
    
    strncpy(user.password, newPass, sizeof(user.password) - 1);
    user.password[sizeof(user.password) - 1] = '\0';

    lseek(fd, offset, SEEK_SET);
    if (write(fd, &user, sizeof(User)) != sizeof(User)) {
        write(sock, "Failed to update password.\n", 28);
    } else {
        write(sock, "Password updated successfully.\n", 33);
        fsync(fd);
    }

unlock_and_close_pw:
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    
    return 1; 
}
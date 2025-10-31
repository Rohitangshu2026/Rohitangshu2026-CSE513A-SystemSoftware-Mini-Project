#include <stdio.h>
#include <unistd.h>      
#include <sys/types.h>   
#include <string.h>     
#include "../include/common.h"
#include <fcntl.h>
ssize_t readLine(int sock, char *buf, size_t size) {
    if (size == 0) return -1;
    ssize_t i = 0;
    char ch;
    while (i < (ssize_t)(size - 1)) {
        ssize_t n = read(sock, &ch, 1);
        if (n <= 0) break;
        if (ch == '\n' || ch == '\r') break;
        buf[i++] = ch;
    }
    buf[i] = '\0';
    return i;
}

int isUserActive(int userId) {
    int fd = open(USER_FILE, O_RDONLY);
    if (fd < 0) {
        perror("isUserActive: open USER_FILE");
        return 0; 
    }

    User user;
    int isActive = 0;
    while(read(fd, &user, sizeof(User)) == sizeof(User)) {
        if (user.id == userId) {
            if (user.isActive == 1) {
                isActive = 1;
            }
            break;
        }
    }
    close(fd);
    return isActive;
}

int checkUserRole(int userId, const char* expectedRole) {
    int fd = open(USER_FILE, O_RDONLY);
    if (fd < 0) return 0;

    User user;
    int hasRole = 0;
    while(read(fd, &user, sizeof(User)) == sizeof(User)) {
        if (user.id == userId) {
            if (strcmp(user.role, expectedRole) == 0) {
                hasRole = 1;
            }
            break;
        }
    }
    close(fd);
    return hasRole;
}
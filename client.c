#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <termios.h>  

#define SERVER_IP "127.0.0.1"
#define PORT_NO 8080
#define BUFFER_SIZE 1024

void connectionHandler(int socketFileDescriptor);
void hide_password(char *buffer, int size);
void printDivider();

int main() {
    int sd;
    struct sockaddr_in serv;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT_NO);
    serv.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (connect(sd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect failed");
        close(sd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server at %s:%d\n", SERVER_IP, PORT_NO);
    connectionHandler(sd);
    close(sd);
    printf("Connection closed.\n");
    return 0;
}
void connectionHandler(int socketFileDescriptor) {
    char readBuffer[BUFFER_SIZE], writeBuffer[BUFFER_SIZE];
    ssize_t readBytes, writeBytes;
    while (1) {
        memset(readBuffer, 0, sizeof(readBuffer));
        readBytes = read(socketFileDescriptor, readBuffer, sizeof(readBuffer) - 1);
        if (readBytes <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        readBuffer[readBytes] = '\0';
        printf("%s", readBuffer);
        fflush(stdout);
        if (strstr(readBuffer, "Client logging out") != NULL)
        {
            break;
        }
        memset(writeBuffer, 0, sizeof(writeBuffer));
        if (strstr(readBuffer, "Enter password") != NULL ||
            strstr(readBuffer, "Enter new password") != NULL ||
            strstr(readBuffer, "Confirm new password") != NULL)
        {
            hide_password(writeBuffer, sizeof(writeBuffer));
        }
        else if (strstr(readBuffer, "Enter username") != NULL ||
                 strstr(readBuffer, "Enter your choice") != NULL ||
                 strstr(readBuffer, "Enter") != NULL ||
                 strstr(readBuffer, "> ") != NULL) {
            fflush(stdout);
            if (fgets(writeBuffer, sizeof(writeBuffer), stdin) == NULL)
                continue;
            writeBuffer[strcspn(writeBuffer, "\r\n")] = '\0';  
        }    
        else
        {
            continue;
        }
        strcat(writeBuffer, "\n");

        writeBytes = write(socketFileDescriptor, writeBuffer, strlen(writeBuffer));
        if (writeBytes < 0) {
            perror("Unable to write to server");
            break;
        }
    }
}

void hide_password(char *buffer, int size) {
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fflush(stdout);

    if (fgets(buffer, size, stdin) != NULL)
        buffer[strcspn(buffer, "\r\n")] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n"); 
}

void printDivider() {
    printf("--------------------------------------------------\n");
    fflush(stdout);
}
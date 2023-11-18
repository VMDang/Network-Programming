#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#include "account.h"

#define BUFF_SIZE 1024
#define BACKLOG 5

char numbers[BUFF_SIZE];
char alphabets[BUFF_SIZE];

void removeLastCharacter(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            return;
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        printf("PortNumber is required. Pleae enter PortNumber\n");
        return 0;
    }
    else if (argc >= 3)
    {
        printf("Enter only one PortNumber\n");
        return 0;
    }

    int listen_sock, conn_sock;
    int bytes_sent, bytes_received;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;
    int pid;
    char *fileName = "account.txt";

    // Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Bind address to socket
    memset(&server, '\0', sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Communicate with client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");
        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // pid
        pid = fork();
        if (pid < 0)
        {
            perror("Error: Cannot fork a child server");
            return 1;
        }

        if (pid == 0)
        {
            List *listAcc = createList();
            Node *foundAcc;
            char username[BUFF_SIZE], password[BUFF_SIZE], reply[BUFF_SIZE];

            if (!getAllAccount(listAcc, fileName))
            {
                printf("Cannot open file!\n");
                return 0;
            }

            if (isEmptyList(listAcc))
            {
                printf("List account is empty!\n");
                return 0;
            }

            // receive username
            bytes_received = recv(conn_sock, username, BUFF_SIZE - 1, 0);
            if (bytes_received < 0)
            {
                perror("Error: Cannot received data from client!");
                close(conn_sock);
                break;
            }
            username[bytes_received] = '\0';
            if (strcmp(username, "\0") == 0)
            {
                close(conn_sock);
                continue;
            }

            if ((foundAcc = searchByName(listAcc, username)) != NULL)
            {
                List *newestList = createList();
                getAllAccount(newestList, fileName);
                listAcc = newestList;
                Node *tmp = searchByName(listAcc, foundAcc->user.name);
                if ((foundAcc->user.status == 1) && tmp->user.loginStatus == 0)
                {
                    strcpy(reply, "OK");
                }
                else if ((foundAcc->user.status == 1) && tmp->user.loginStatus == 1)
                {
                    strcpy(reply, "Username is logged");
                }

                if (foundAcc->user.status == 0)
                {
                    strcpy(reply, "Your account has been locked");
                }
            }
            else
            {
                strcpy(reply, "Username not exsits");
            }

            bytes_sent = send(conn_sock, reply, strlen(reply), 0);
            if (bytes_sent < 0)
            {
                perror("Error: Cannot send data to client!");
                close(conn_sock);
                break;
            }

            int attempt = 0;
            while (strcmp(reply, "Logged in") != 0)
            {
                memset(password, '\0', BUFF_SIZE);
                bytes_received = recv(conn_sock, password, BUFF_SIZE - 1, 0);
                if (bytes_received < 0)
                {
                    perror("Error: Cannot received data from client!");
                    close(conn_sock);
                    break;
                }
                password[bytes_received] = '\0';
                if (strcmp(password, "\0") == 0)
                {
                    close(conn_sock);
                    break;
                }

                if (strcmp(foundAcc->user.password, password) == 0)
                {
                    updatedLoginStatus(listAcc, foundAcc->user.name, 1);
                    storeAccount(listAcc, fileName);
                    strcpy(reply, "Logged in");
                }
                else
                {
                    attempt++;
                    if (attempt == 3)
                    {
                        strcpy(reply, "Block account");
                        List *newestList = createList();
                        getAllAccount(newestList, fileName);
                        listAcc = newestList;
                        updatedStatusAccount(listAcc, foundAcc->user.name, 0);
                        storeAccount(listAcc, fileName);
                    }
                    else
                    {
                        strcpy(reply, "Password incorrect");
                    }
                }

                bytes_sent = send(conn_sock, reply, strlen(reply), 0);
                if (bytes_sent < 0)
                {
                    printf("\nConnection closed\n");
                    break;
                }
            }

            char logoutFlag[BUFF_SIZE];
            bytes_received = recv(conn_sock, logoutFlag, BUFF_SIZE - 1, 0);
            if (bytes_received < 0)
            {
                perror("Error: Cannot received data from client!");
                close(conn_sock);
                break;
            }
            logoutFlag[bytes_received] = '\0';
            if (strcmp(logoutFlag, "logout") == 0)
            {
                List *newestList = createList();
                getAllAccount(newestList, fileName);
                listAcc = newestList;
                updatedLoginStatus(listAcc, foundAcc->user.name, 0);
                storeAccount(listAcc, fileName);
                close(conn_sock);
            }
        }
        close(conn_sock);
    }
    close(listen_sock);
    return 0;
}

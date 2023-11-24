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
#include <pthread.h>
#include <semaphore.h>

#include "account.h"

#define BUFF_SIZE 1024
#define BACKLOG 5

char numbers[BUFF_SIZE];
char alphabets[BUFF_SIZE];
char *fileName = "account.txt";
List *listAcc;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;

void *client_handler(void *arg)
{
    int clientfd;
    int sendBytes, rcvBytes;

    pthread_detach(pthread_self());
    clientfd = *((int *)arg);
    free(arg);

    while (1)
    {
        char selectedMenu[BUFF_SIZE];
        rcvBytes = recv(clientfd, selectedMenu, BUFF_SIZE - 1, 0);
        if (rcvBytes < 0)
        {
            perror("Error: Cannot received data from client!");
            close(clientfd);
            break;
        }
        selectedMenu[rcvBytes] = '\0';

        if (strcmp(selectedMenu, "1") == 0)
        {
            sendBytes = send(clientfd, "1", strlen("1"), 0);
            Node *foundAcc;
            char username[BUFF_SIZE], password[BUFF_SIZE], reply[BUFF_SIZE];

            // receive username
            rcvBytes = recv(clientfd, username, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            username[rcvBytes] = '\0';
            if (strcmp(username, "\0") == 0)
            {
                close(clientfd);
                continue;
            }

            pthread_mutex_lock(&mutex);
            if ((foundAcc = searchByName(listAcc, username)) != NULL)
            {
                if ((foundAcc->user.status == 1) && (foundAcc->user.loginStatus == 0))
                {
                    strcpy(reply, "OK");
                }
                else if ((foundAcc->user.status == 1) && foundAcc->user.loginStatus == 1)
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
            pthread_mutex_unlock(&mutex);

            sendBytes = send(clientfd, reply, strlen(reply), 0);
            if (sendBytes < 0)
            {
                perror("Error: Cannot send data to client!");
                close(clientfd);
                break;
            }

            int attempt = 0;
            while (strcmp(reply, "Logged in") != 0)
            {
                memset(password, '\0', BUFF_SIZE);
                rcvBytes = recv(clientfd, password, BUFF_SIZE - 1, 0);
                if (rcvBytes < 0)
                {
                    perror("Error: Cannot received data from client!");
                    close(clientfd);
                    break;
                }
                password[rcvBytes] = '\0';
                if (strcmp(password, "\0") == 0)
                {
                    close(clientfd);
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.loginStatus == 1)
                {
                    strcpy(reply, "Username is logged");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.status == 0)
                {
                    strcpy(reply, "Your account has been locked");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (strcmp(foundAcc->user.password, password) == 0)
                {
                    pthread_mutex_lock(&mutex);
                    updatedLoginStatus(listAcc, foundAcc->user.name, 1);
                    pthread_mutex_unlock(&mutex);

                    strcpy(reply, "Logged in");
                }
                
                else
                {
                    attempt++;
                    if (attempt == 3)
                    {
                        pthread_mutex_lock(&mutex);
                        updatedStatusAccount(listAcc, foundAcc->user.name, 0);
                        storeAccount(listAcc, fileName);
                        pthread_mutex_unlock(&mutex);

                        strcpy(reply, "Block account");
                    }
                    else
                    {
                        strcpy(reply, "Password incorrect");
                    }
                }

                sendBytes = send(clientfd, reply, strlen(reply), 0);
                if (sendBytes < 0)
                {
                    printf("\nConnection closed\n");
                    break;
                }
            }

            char logoutFlag[BUFF_SIZE];
            rcvBytes = recv(clientfd, logoutFlag, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            logoutFlag[rcvBytes] = '\0';
            if (strcmp(logoutFlag, "logout") == 0)
            {
                pthread_mutex_lock(&mutex);
                updatedLoginStatus(listAcc, foundAcc->user.name, 0);
                pthread_mutex_unlock(&mutex);

                close(clientfd);
            }
        }
        else if (strcmp(selectedMenu, "2") == 0)
        {
            sendBytes = send(clientfd, "2", strlen("2"), 0);

            Node *foundAcc;
            char username[BUFF_SIZE], password[BUFF_SIZE], reply[BUFF_SIZE];

            // receive username
            rcvBytes = recv(clientfd, username, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            username[rcvBytes] = '\0';
            if (strcmp(username, "\0") == 0)
            {
                close(clientfd);
                continue;
            }

            sem_wait(&semaphore);
            if ((foundAcc = searchByName(listAcc, username)) != NULL)
            {
                if ((foundAcc->user.status == 1) && (foundAcc->user.loginStatus == 0))
                {
                    strcpy(reply, "OK");
                }
                else if ((foundAcc->user.status == 1) && foundAcc->user.loginStatus == 1)
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
            sem_post(&semaphore);

            sendBytes = send(clientfd, reply, strlen(reply), 0);
            if (sendBytes < 0)
            {
                perror("Error: Cannot send data to client!");
                close(clientfd);
                break;
            }

            int attempt = 0;
            while (strcmp(reply, "Logged in") != 0)
            {
                memset(password, '\0', BUFF_SIZE);
                rcvBytes = recv(clientfd, password, BUFF_SIZE - 1, 0);
                if (rcvBytes < 0)
                {
                    perror("Error: Cannot received data from client!");
                    close(clientfd);
                    break;
                }
                password[rcvBytes] = '\0';
                if (strcmp(password, "\0") == 0)
                {
                    close(clientfd);
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.loginStatus == 1)
                {
                    strcpy(reply, "Username is logged");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.status == 0)
                {
                    strcpy(reply, "Your account has been locked");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (strcmp(foundAcc->user.password, password) == 0)
                {
                    sem_wait(&semaphore);
                    updatedLoginStatus(listAcc, foundAcc->user.name, 1);
                    sem_post(&semaphore);

                    strcpy(reply, "Logged in");
                }
                else
                {
                    attempt++;
                    if (attempt == 3)
                    {
                        sem_wait(&semaphore);
                        updatedStatusAccount(listAcc, foundAcc->user.name, 0);
                        storeAccount(listAcc, fileName);
                        sem_post(&semaphore);

                        strcpy(reply, "Block account");
                    }
                    else
                    {
                        strcpy(reply, "Password incorrect");
                    }
                }

                sendBytes = send(clientfd, reply, strlen(reply), 0);
                if (sendBytes < 0)
                {
                    printf("\nConnection closed\n");
                    break;
                }
            }

            char logoutFlag[BUFF_SIZE];
            rcvBytes = recv(clientfd, logoutFlag, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            logoutFlag[rcvBytes] = '\0';
            if (strcmp(logoutFlag, "logout") == 0)
            {
                sem_wait(&semaphore);
                updatedLoginStatus(listAcc, foundAcc->user.name, 0);
                sem_post(&semaphore);

                close(clientfd);
            }
        }
        else if (strcmp(selectedMenu, "3") == 0)
        {
            sendBytes = send(clientfd, "3", strlen("3"), 0);

            Node *foundAcc;
            char username[BUFF_SIZE], password[BUFF_SIZE], reply[BUFF_SIZE];

            // receive username
            rcvBytes = recv(clientfd, username, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            username[rcvBytes] = '\0';
            if (strcmp(username, "\0") == 0)
            {
                close(clientfd);
                continue;
            }

            pthread_mutex_lock(&mutex);
            if ((foundAcc = searchByName(listAcc, username)) != NULL)
            {
                if ((foundAcc->user.status == 1) && (foundAcc->user.loginStatus == 0))
                {
                    strcpy(reply, "OK");
                }
                else if ((foundAcc->user.status == 1) && foundAcc->user.loginStatus == 1)
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
            pthread_mutex_unlock(&mutex);

            sendBytes = send(clientfd, reply, strlen(reply), 0);
            if (sendBytes < 0)
            {
                perror("Error: Cannot send data to client!");
                close(clientfd);
                break;
            }

            int attempt = 0;
            while (strcmp(reply, "Logged in") != 0)
            {
                memset(password, '\0', BUFF_SIZE);
                rcvBytes = recv(clientfd, password, BUFF_SIZE - 1, 0);
                if (rcvBytes < 0)
                {
                    perror("Error: Cannot received data from client!");
                    close(clientfd);
                    break;
                }
                password[rcvBytes] = '\0';
                if (strcmp(password, "\0") == 0)
                {
                    close(clientfd);
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.loginStatus == 1)
                {
                    strcpy(reply, "Username is logged");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (searchByName(listAcc, foundAcc->user.name)->user.status == 0)
                {
                    strcpy(reply, "Your account has been locked");

                    sendBytes = send(clientfd, reply, strlen(reply), 0);
                    if (sendBytes < 0)
                    {
                        printf("\nConnection closed\n");
                        break;
                    }
                    break;
                }

                if (strcmp(foundAcc->user.password, password) == 0)
                {
                    pthread_mutex_lock(&mutex);
                    updatedLoginStatus(listAcc, foundAcc->user.name, 1);
                    pthread_mutex_unlock(&mutex);

                    strcpy(reply, "Logged in");
                }
                else
                {
                    attempt++;
                    if (attempt == 3)
                    {
                        pthread_mutex_lock(&mutex);
                        updatedStatusAccount(listAcc, foundAcc->user.name, 0);
                        storeAccount(listAcc, fileName);
                        pthread_mutex_unlock(&mutex);

                        strcpy(reply, "Block account");
                    }
                    else
                    {
                        strcpy(reply, "Password incorrect");
                    }
                }

                sendBytes = send(clientfd, reply, strlen(reply), 0);
                if (sendBytes < 0)
                {
                    printf("\nConnection closed\n");
                    break;
                }
            }

            char logoutFlag[BUFF_SIZE];
            rcvBytes = recv(clientfd, logoutFlag, BUFF_SIZE - 1, 0);
            if (rcvBytes < 0)
            {
                perror("Error: Cannot received data from client!");
                close(clientfd);
                break;
            }
            logoutFlag[rcvBytes] = '\0';
            if (strcmp(logoutFlag, "logout") == 0)
            {
                pthread_mutex_lock(&mutex);
                updatedLoginStatus(listAcc, foundAcc->user.name, 0);
                pthread_mutex_unlock(&mutex);

                close(clientfd);
            }
        }
        else
        {
            break;
        }
    }

    close(clientfd);
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

    listAcc = createList();
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

    int listenfd;
    int *connfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;
    pthread_t tid;
    sem_init(&semaphore, 0, 1);

    // Construct a TCP socket to listen connection request
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Bind address to socket
    memset(&server, '\0', sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Listen request from client
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Communicate with client
    while (1)
    {
        connfd = malloc(sizeof(int));
        sin_size = sizeof(struct sockaddr_in);
        if ((*connfd = accept(listenfd, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");
        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        pthread_create(&tid, NULL, &client_handler, connfd);
    }

    sem_destroy(&semaphore);

    close(listenfd);
    return 0;
}

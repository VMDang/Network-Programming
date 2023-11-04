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
#include <sys/stat.h>
#include <openssl/md5.h>

#define BUFF_SIZE 1024
#define BACKLOG 2
#define STORAGE "./storage/"

char numbers[BUFF_SIZE];
char alphabets[BUFF_SIZE];

void md5_encode(const char *input, char *output)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    MD5_Update(&md5Context, input, strlen(input));
    MD5_Final(digest, &md5Context);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&output[i * 2], "%02x", (unsigned int)digest[i]);
    }
    output[MD5_DIGEST_LENGTH * 2] = '\0';
}

int checkInvalidCharacterInMessage(char *received)
{
    for (int i = 0; i < strlen(received); i++)
    {
        if (!isdigit(received[i]) && !isalpha(received[i]))
        {
            return 0;
        }
    }
    return 1;
}

void classifyCharacters(char *input)
{
    int numIndex = 0;
    int charIndex = 0;

    for (int i = 0; i < strlen(input); i++)
    {
        if (isdigit(input[i]))
        {
            numbers[numIndex++] = input[i];
        }
        else if (isalpha(input[i]))
        {
            alphabets[charIndex++] = input[i];
        }
    }

    numbers[numIndex] = '\0';
    alphabets[charIndex] = '\0';
}

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

    int listen_sock, server_sock; /* file descriptors */
    char buff[BUFF_SIZE];
    char status[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;
    char buffMD5[MD5_DIGEST_LENGTH * 2 + 1];

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));     /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((server_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");
        printf("You got a connection from %s", inet_ntoa(client.sin_addr)); /* prints client's IP */

        while (1)
        {
            bytes_received = recv(server_sock, buff, BUFF_SIZE - 1, 0); // blocking
            buff[bytes_received] = '\0';

            removeLastCharacter(buff);
            if (strcmp(buff, "\0") == 0)
            {
                exit(0);
            }

            if (strcmp(buff, "1") == 0)
            {
                strcpy(status, "1");
                bytes_sent = send(server_sock, status, strlen(status), 0);

                // receives message from client
                bytes_received = recv(server_sock, buff, BUFF_SIZE - 1, 0); // blocking
                buff[bytes_received] = '\0';

                removeLastCharacter(buff);
                printf("\nReceive: %s", buff);

                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
                else
                {
                    if (checkInvalidCharacterInMessage(buff) == 0)
                    {
                        strcpy(status, "Error: Message contains special character\n");
                        bytes_sent = send(server_sock, status, strlen(status), 0);
                        if (bytes_sent < 0)
                        {
                            perror("Error: ");
                            close(server_sock);
                            exit(0);
                        }
                    }
                    else
                    {
                        md5_encode(buff, buffMD5);
                        classifyCharacters(buffMD5);

                        if (strlen(numbers) > 0 && strlen(alphabets) > 0)
                        {
                            char tmp[BUFF_SIZE * 2 + 2];
                            snprintf(tmp, sizeof(tmp), "%s\n%s\n", numbers, alphabets);
                            bytes_sent = send(server_sock, tmp, strlen(tmp), 0);
                        }
                        else
                        {
                            strcpy(status, "Error: Server not available\n");
                            bytes_sent = send(server_sock, status, strlen(status), 0);
                            if (bytes_sent < 0)
                            {
                                perror("Error: ");
                                close(server_sock);
                                exit(0);
                            }
                            break;
                        }
                    }
                }
            }
            else if (strcmp(buff, "2") == 0)
            {
                struct stat st = {0};
                strcpy(status, "2");
                bytes_sent = send(server_sock, status, strlen(status), 0);

                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    exit(0);
                }
                else
                {
                    if (stat(STORAGE, &st) == -1)
                    {
                        mkdir(STORAGE, 0755);
                    }

                    bytes_received = recv(server_sock, buff, BUFF_SIZE - 1, 0);
                    if (bytes_received <= 0)
                    {
                        printf("\nConnection closed");
                        exit(0);
                    }

                    buff[bytes_received] = '\0';
                    removeLastCharacter(buff);
                    printf("\nReceive: %s", buff);


                    char nameFile[100];
                    strcpy(nameFile, STORAGE);
                    strcat(nameFile, buff);

                    FILE *f;
                    long sizeImage;
                    char content[BUFF_SIZE];

                    if ((f = fopen(nameFile, "rb")) != NULL)
                    {
                        strcpy(status, "File existed");
                        fclose(f);
                        bytes_sent = send(server_sock, status, strlen(status), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                            exit(0);
                        }
                        continue;
                    }
                    else
                    {
                        strcpy(status, "Start Upload image");
                        bytes_sent = send(server_sock, status, strlen(status), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                            exit(0);
                        }

                        bytes_received = recv(server_sock, &sizeImage, 20, 0);
                        if (bytes_received <= 0)
                        {
                            printf("\nConnection closed");
                            exit(0);
                        }


                        int sumByteSent=0;
                        f = fopen(nameFile, "wb");
                        while (sumByteSent < sizeImage)
                        {
                            bytes_received = recv(server_sock, content, BUFF_SIZE - 1, 0);
                            if (bytes_received <= 0)
                            {
                                printf("\nConnection closed");
                                exit(0);
                            }

                            sumByteSent += bytes_received;
                            fwrite(content, bytes_received, 1, f);
                            strcpy(content, "");
                        }
                        fclose(f);

                        char tmp[BUFF_SIZE];
                        snprintf(tmp, sizeof(tmp), "Upload image succesful!\nStored in %s\n", nameFile);
                        bytes_sent = send(server_sock, tmp, strlen(tmp), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                            exit(0);
                        }
                    }
                }
            }
            else
            {
                continue;
            }
        }
        close(server_sock);
    }

    close(listen_sock);

    return 0;
}

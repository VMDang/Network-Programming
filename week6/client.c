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

#define BUFF_SIZE 1024
#define STORAGE "./images/client/"

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

void showMenu()
{
    printf("\nMENU\n");
    printf("----------------------------------\n");
    printf("1. Gửi xâu bất kỳ\n");
    printf("2. Gửi nội dung một file\n");
    printf(">>> ");
}

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        printf("IPAdress and PortNumber are required. Pleae enter IPAddress and PortNumber\n");
        return 0;
    }
    else if (argc >= 4)
    {
        printf("Enter only IPAdress and PortNumber \n");
        return 0;
    }

    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received;
    // socklen_t sin_size;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }

    do
    {
        showMenu();
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);

        if (strcmp(buff, "\n") == 0)
        {
            exit(0);
        }

        switch ((int)buff[0] - 48)
        {
        case 1:
            do
            {
                bytes_sent = send(client_sock, "1", strlen(buff), 0);
                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                buff[bytes_received] = '\0';
                removeLastCharacter(buff);

                memset(buff, '\0', (strlen(buff) + 1));
                fgets(buff, BUFF_SIZE, stdin);
                if (strcmp(buff, "\n") == 0)
                {
                    exit(0);
                }

                bytes_sent = send(client_sock, buff, strlen(buff), 0);
                if (bytes_sent <= 0)
                {
                    printf("\nConnection closed!\n");
                    exit(0);
                }

                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("\nError!Cannot receive data from sever!\n");
                    break;
                }
                buff[bytes_received] = '\0';
                printf("%s\n", buff);

            } while (1);

            break;
        case 2:

            bytes_sent = send(client_sock, "2", strlen(buff), 0);
            bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
            buff[bytes_received] = '\0';
            removeLastCharacter(buff);

            printf("Nhập tên file (image.jpg): ");
            memset(buff, '\0', (strlen(buff) + 1));
            fgets(buff, BUFF_SIZE, stdin);

            if (strcmp(buff, "\n") == 0)
            {
                exit(0);
            }

            removeLastCharacter(buff);
            FILE *f;
            if ((f = fopen(buff, "rb")) == NULL)
            {
                printf("Error: File not found");
                fclose(f);
                exit(0);
            }

            bytes_sent = send(client_sock, buff, strlen(buff), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed");
                exit(0);
            }

            bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
            buff[bytes_received] = '\0';
            removeLastCharacter(buff);

            printf("%s\n", buff);
            if (strcmp(buff, "Start Upload image") == 0)
            {
                long sizeImage = 0;
                fseek(f, 0, SEEK_END);
                sizeImage = ftell(f);
                rewind(f);
                bytes_sent = send(client_sock, &sizeImage, 20, 0);
                if (bytes_sent <= 0)
                {
                    printf("\nConnection closed");
                    exit(0);
                }

                int sumByteSent = 0;
                while (sumByteSent < sizeImage)
                {

                    int byteNum = BUFF_SIZE;
                    if ((sumByteSent + BUFF_SIZE) > sizeImage)
                    {
                        byteNum = sizeImage - sumByteSent;
                    }

                    char content[BUFF_SIZE];
                    fread(content, byteNum, 1, f);
                    sumByteSent += byteNum;

                    bytes_sent = send(client_sock, content, byteNum, 0);
                    if (bytes_sent <= 0)
                    {
                        printf("\nConnection closed");
                        exit(0);
                    }
                }
                fclose(f);

                bytes_sent = send(client_sock, "Done", strlen("Done"), 0);

                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    exit(0);
                }
                buff[bytes_received] = '\0';

                printf("%s\n", buff);
            }

            break;
        default:
            printf("Sai lựa chọn. Hãy nhập lại\n");
            break;
        }
    } while (1);

    close(client_sock);

    return 0;
}

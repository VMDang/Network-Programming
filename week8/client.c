#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFF_SIZE 1024

void showMenu()
{
    printf("\nSelect Menu\n");
    printf("---------------------------------------------\n");
    printf("1. Inter-lock\n");
    printf("2. Semaphore\n");
    printf("3. Mutex\n");
    printf("Your choice (1-3, other to quit) >>  ");
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

    int selectedMenu;
    showMenu();
    scanf("%d", &selectedMenu);
    while (getchar() != '\n')
        ;

    int client_sock;
    struct sockaddr_in server_addr; /* server's address information */
    int bytes_sent;
    int bytes_received;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit immediately! ");
        return 0;
    }

    switch (selectedMenu)
    {
    case 1:
        bytes_sent = send(client_sock, "1", strlen("1"), 0);
        break;
    case 2:
        bytes_sent = send(client_sock, "2", strlen("2"), 0);
        break;
    case 3:
        bytes_sent = send(client_sock, "3", strlen("3"), 0);
        break;
    default:
        bytes_sent = send(client_sock, "0", strlen("0"), 0);
        return 0;
        break;
    }
    // send username to server
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char buff[BUFF_SIZE];

    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);

    printf("Username >> ");
    memset(username, '\0', (strlen(username) + 1));
    fgets(username, BUFF_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';
    if (strcmp(username, "\0") == 0)
    {
        return 0;
    }

    bytes_sent = send(client_sock, username, strlen(username), 0);
    if (bytes_sent < 0)
    {
        perror("Error: ");
        close(client_sock);
        return 0;
    }

    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
    if (bytes_received < 0)
    {
        printf("\nError!Cannot receive data from sever!\n");
        close(client_sock);
        return 0;
    }
    buff[bytes_received] = '\0';

    if (strcmp(buff, "Username not exsits") == 0)
    {
        printf("%s\n", buff);
        bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

        return 0;
    }
    else if (strcmp(buff, "Your account has been locked") == 0)
    {
        printf("%s\n", buff);
        bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

        return 0;
    }
    else if (strcmp(buff, "Username is logged") == 0)
    {
        printf("Username is logged\n");
        bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

        return 0;
    }

    // send password to server
    while (1)
    {
        printf("Insert Password >> ");
        memset(password, '\0', (strlen(password) + 1));
        fgets(password, BUFF_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0';

        bytes_sent = send(client_sock, password, strlen(password), 0);
        if (bytes_sent < 0)
        {
            perror("Error: Cannot send data to sever!");
            close(client_sock);
            return 0;
        }

        if (strcmp(password, "\0") == 0)
        {
            break;
        }

        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received < 0)
        {
            perror("\nError!Cannot receive data from sever!\n");
            close(client_sock);
            return 0;
        }
        buff[bytes_received] = '\0';

        if (strcmp(buff, "Password incorrect") == 0)
        {
            printf("%s\n", buff);

            continue;
        }
        else if (strcmp(buff, "Block account") == 0)
        {
            printf("Password incorrect 3rd time, your account has been blocked\n");
            bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

            return 0;
        }
        else if (strcmp(buff, "Username is logged") == 0)
        {
            printf("%s\n", buff);
            bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

            return 0;
        }
        else if (strcmp(buff, "Your account has been locked") == 0)
        {
            printf("%s\n", buff);
            bytes_sent = send(client_sock, "\0", strlen("\0"), 0);

            return 0;
        }
        else if (strcmp(buff, "Logged in") == 0)
        {
            printf("%s\n", buff);
            printf("Press ENTER to log out and exit\n");

            while (1)
            {
                char logout[BUFF_SIZE];
                memset(logout, '\0', (strlen(logout) + 1));
                fgets(logout, BUFF_SIZE, stdin);
                if (strcmp(logout, "\n") == 0)
                {
                    bytes_sent = send(client_sock, "logout", strlen("logout"), 0);
                    if (bytes_sent < 0)
                    {
                        perror("Error: Cannot send data to sever!");
                        close(client_sock);
                        return 0;
                    }
                    return 0;
                }
                else
                {
                    printf("Press ENTER to log out and exit\n");
                }
            }
        }
        else
        {
            printf("%s\n", buff);
            printf("Server error\n");
            return 1;
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}

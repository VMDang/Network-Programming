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

char numbers[BUFF_SIZE];
char alphabets[BUFF_SIZE];

int checkInvalidCharacterInMessage(char *received)
{
    for (int i = 0; i < strlen(received); i++)
    {
        if (!isdigit(received[i]) && !isalpha(received[i])) {
            return 0;
        }
    }
    return 1;
}

void sha256_encode(char *password, char *hashedPassword) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hashedPassword[i * 2], "%02x", hash[i]);
    }
    hashedPassword[SHA256_DIGEST_LENGTH * 2] = '\0';
}

void classifyCharacters(char *input) {
    int numIndex = 0;
    int charIndex = 0;

    for (int i = 0; i < strlen(input); i++) {
        if (isdigit(input[i])) {
            numbers[numIndex++] = input[i];
        } else if (isalpha(input[i])) {
            alphabets[charIndex++] = input[i];
        }
    }

    numbers[numIndex] = '\0';
    alphabets[charIndex] = '\0';
}

void removeLastCharacter(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            return;
        }
    }
}

int isSingletonList(List* list) {
    return (list->root != NULL && list->root->next == NULL);
}

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        printf("PortNumber is required. Pleae enter PortNumber\n");
        return 0;
    } else if (argc >= 3)
    {
        printf("Enter only one PortNumber\n");
        return 0;
    }

    char buff[BUFF_SIZE];
    char status[BUFF_SIZE];
    struct sockaddr_in server, client;
    int bytes_sent, bytes_recieved;
    int server_sock;
    socklen_t sin_size;
    char buffSHA[SHA_DIGEST_LENGTH*2 + 1];
    char *fileName = "account.txt";
    int loginStatus = 0;

    List *listAcc = createList();
    List *listLogin = createList();

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

    // step 1: construct socket
    if((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("\nError: ");
        exit(0);
    }

    //Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server.sin_zero), 8);
    
    if(bind(server_sock, (struct sockaddr*)&server, sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}
    
    //Step 3: Communicate with clients
	sin_size=sizeof(struct sockaddr_in);

    while (loginStatus == 0 && isSingletonList(listLogin)==0)
    {
        char *password;
        char *username;
        int attempt=1;
        Node *tmp;

        bytes_recieved = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);	
	    if (bytes_recieved < 0)
		    perror("\nError: ");
	    else{
    	    buff[bytes_recieved] = '\0';
		    printf("[%s:%d]: %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
	    }
        username = buff;
        removeLastCharacter(username);
        removeLastCharacter(buff);
        
        tmp = searchByName(listAcc, username);
        if (tmp == NULL)
        {
            strcpy(status, "Cannot find account\n");
            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
        } else
        {
            strcpy(status, "Insert Passowrd:\n");
            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
            do
            {
                bytes_recieved = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr*)&client, &sin_size);
                memset(status, '\0', sizeof(status));

                removeLastCharacter(buff);

                if (bytes_recieved < 0)
			        perror("\nError: ");
		        else{
			        buff[bytes_recieved] = '\0';
			        printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
                    password = buff;
                    removeLastCharacter(password);
                }
                
                if (strcmp(password, tmp->user.password)!=0)
                {
                    attempt++;
                    if(attempt > 3) break;
                    strcpy(status, "Not OK\n");
                    bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);         
                } else
                {
                    if (tmp->user.status == 0)
                    {
                        strcpy(status, "Account not ready\n");
                        bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
                        break;
                    }else
                    {
                        insertAtfterCurrent(listLogin, tmp->user);
                        loginStatus = 1;
                        attempt = 1;
                        strcpy(status, "OK\n");
                        bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);   
                    }
                }
                

            } while (strcmp(password, tmp->user.password)!=0 && attempt<=3);

            if (attempt > 3)
            {
                updatedStatusAccount(listAcc, tmp->user.name, 0);
                storeAccount(listAcc, fileName);
                attempt = 1;
                strcpy(status, "Account is blocked\n");
                bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
            }
            
        }
    };

    while (loginStatus == 1 && isSingletonList(listLogin))
    {
        bytes_recieved = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr*)&client, &sin_size);
        memset(status, '\0', sizeof(status));

		if (bytes_recieved < 0)
			perror("\nError: ");
		else{
			buff[bytes_recieved] = '\0';
		}
        
        removeLastCharacter(buff);
        if (strcmp(buff, "\0")==0)
        {
            continue;
        }
        
        
        if (strcmp(buff, "bye\0") == 0)
        {
            loginStatus = 0;
            char tmp[100];
            snprintf(tmp, sizeof(tmp), "Goodbye %s\n", listLogin->root->user.name);
            bytes_sent = sendto(server_sock, tmp, strlen(tmp), 0, (struct sockaddr *) &client, sin_size);
            break;
        }
        
        if (checkInvalidCharacterInMessage(buff) == 0)
        {
            strcpy(status, "Error: Message contains special character\n");
            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
			if(bytes_sent < 0){
				perror("Error: ");
				close(server_sock);
				exit(0);
			}
        } else
        {
            strcpy(status, "Password is valid\n");
            updatedPasswordAccount(listAcc, listLogin->root->user.name, buff);
            storeAccount(listAcc, fileName);

            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client, sin_size);
			if(bytes_sent < 0){
				perror("Error: ");
				close(server_sock);
				exit(0);
			}
            
            sha256_encode(buff, buffSHA);

            classifyCharacters(buffSHA);

            for (int i = 0; i < 2; i++)
            {
               if (i==0)
               {
                    if (strlen(numbers) > 0)
                    {
                        bytes_sent =sendto(server_sock, numbers, strlen(numbers), 0,(struct sockaddr *) &client, sin_size);
                        if (bytes_sent < 0)
                        {
                            perror("Error: ");
                            close(server_sock);
                            return 0;
                        }
                    }
               }
               
               if (i==1)
               {
                    if(strlen(alphabets) > 0){
                        bytes_sent =sendto(server_sock, alphabets, strlen(alphabets), 0,(struct sockaddr *) &client, sin_size);
                        if (bytes_sent < 0)
                        {
                            perror("Error: ");
                            close(server_sock);
                            return 0;
                        }
			        }
               }
               
            }
            
        }
        
    }
    close(server_sock);

    return 0;
}

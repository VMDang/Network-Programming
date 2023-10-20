#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h> 

#define BUFF_SIZE 1024

char numbers[BUFF_SIZE];
char alphabets[BUFF_SIZE];

int checkInvalidCharacterInMessage(char *received)
{
    for (int i = 0; i < strlen(received)-1; i++)
    {
        if (!isdigit(received[i]) && !isalpha(received[i])) {
            return 0;
        }
    }
    return 1;
}

void sha1_encode(const char *message, char *messageSHA) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, message, strlen(message));
    SHA1_Final(hash, &sha1);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(&messageSHA[i * 2], "%02x", hash[i]);
    }
    messageSHA[SHA_DIGEST_LENGTH * 2] = '\0';
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
    struct sockaddr_in server, client1, client2;
    int bytes_sent, bytes_recieved;
    int server_sock;
    socklen_t sin_size;
    char buffSHA[SHA_DIGEST_LENGTH*2 + 1];

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
    		
	bytes_recieved = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client2, &sin_size);
		
	if (bytes_recieved < 0)
		perror("\nError: ");
	else{
		buff[bytes_recieved] = '\0';
		printf("[%s:%d]: %s", inet_ntoa(client2.sin_addr), ntohs(client2.sin_port), buff);
	}

    while (1)
    {
        bytes_recieved = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr*)&client1, &sin_size);
        memset(status, '\0', sizeof(status));

		if (bytes_recieved < 0)
			perror("\nError: ");
		else{
			buff[bytes_recieved] = '\0';
			printf("[%s:%d]: %s", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
		}

        if (checkInvalidCharacterInMessage(buff) == 0)
        {
            strcpy(status, "Error: Message contains special character\n");
            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
			if(bytes_sent < 0){
				perror("Error: ");
				close(server_sock);
				exit(0);
			}
        } else
        {
            strcpy(status, "Message is valid\n");
            bytes_sent = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
			if(bytes_sent < 0){
				perror("Error: ");
				close(server_sock);
				exit(0);
			}
            
            
            sha1_encode(buff, buffSHA);

            classifyCharacters(buffSHA);
            if (strlen(numbers) > 0)
            {
                bytes_sent =sendto(server_sock, numbers, strlen(numbers), 0,(struct sockaddr *) &client2, sin_size);
				if (bytes_sent < 0)
				{
					perror("Error: ");
					close(server_sock);
					return 0;
				}
            }

            if(strlen(alphabets) > 0){
				bytes_sent =sendto(server_sock, alphabets, strlen(alphabets), 0,(struct sockaddr *) &client2, sin_size);
				if (bytes_sent < 0)
				{
					perror("Error: ");
					close(server_sock);
					return 0;
				}
			}
            
        }
    }
    close(server_sock);

    return 0;
}

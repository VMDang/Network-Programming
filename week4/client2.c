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

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        printf("IPAdress and PortNumber are required. Pleae enter IPAddress and PortNumber\n");
        return 0;
    } else if (argc >= 4)
    {
        printf("Enter only IPAdress and PortNumber \n");
        return 0;
    }

    int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received;
    socklen_t sin_size;

    //Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
		perror("\nError: ");
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	//Step 3: Communicate with server
    memset(buff,'\0', 30);
	strcpy(buff, "Connected\n");

    sin_size = sizeof(struct sockaddr);
        
    bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
    if(bytes_sent < 0) {
        perror("Error: ");
        close(client_sock);
        return 0;
    }

    while (1)
    {
        bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0,(struct sockaddr *) &server_addr, &sin_size); //receive message from server
		if(bytes_received < 0){
			perror("Error: ");
            close(client_sock);
			return 0;
		}
		buff[bytes_received] = '\0';
		printf("%s\n", buff);
    }
    printf("\n");
    close(client_sock);
    
    return 0;
}

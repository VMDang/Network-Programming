#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

int isValidIPAddress(char *input)
{
    int octet[4];
    int octetCount = sscanf(input, "%d.%d.%d.%d", &octet[0], &octet[1], &octet[2], &octet[3]);
    if (octetCount != 4) {
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        if (octet[i] < 0 || octet[i] > 255) {
            return 0;
        }
    }

    return 1;
}

int isIPAdress(char *input)
{
    if (isdigit(input[0]))
    {
        if (isValidIPAddress(input))
        {
            struct sockaddr_in sa;
            return inet_pton(AF_INET, input, &(sa.sin_addr)) != 0;
        } else
        {
            printf("Not found information\n");
            exit(0);
        }
    }

    return 0;
}

void getIP(char *hostname)
{
    struct hostent *hostent;
    struct in_addr **in_addr_list;

    if ((hostent = gethostbyname(hostname)) == NULL)
    {
        printf("Not found information\n");
        return;
    }

    in_addr_list = (struct in_addr **)hostent->h_addr_list;

    printf("Official IP: %s\n", inet_ntoa(*in_addr_list[0]));
    printf("Alias IP: \n");
    for (int i = 1; in_addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*in_addr_list[i]));
    }
}

void getHostname(struct in_addr ip)
{
    struct hostent *hostent;
    if ((hostent = gethostbyaddr((const void*)&ip, sizeof(ip), AF_INET)) == NULL)
    {
        printf("Not found information\n");
        return;
    }

    printf("Official name: %s\n", hostent->h_name);
    printf("Alias name: \n");
    for (int i = 0; hostent->h_aliases[i] != NULL; i++)
    {
        printf("%s\n", hostent->h_aliases[i]);
    }
    
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Hostname/IP is required. Pleae enter Hostname or IP\n");
        return 0;
    } else if (argc >= 3)
    {
        printf("Enter only one IP or Hostname\n");
    }
    
    char *parameter = argv[1];

    if (isIPAdress(parameter))
    {
        struct in_addr ip;
        inet_pton(AF_INET, argv[1], &ip);
        getHostname(ip);
        
    } else
    {
        char *hostname = argv[1];
        getIP(hostname);       
    }
    
    return 0;
}

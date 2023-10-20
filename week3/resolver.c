#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <regex.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <unistd.h>

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

void getIP(char *hostname, FILE *file)
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

    fprintf(file, "Official IP: %-20s", inet_ntoa(*in_addr_list[0]));
    fprintf(file, "Alias IP: ");
    for (int i = 1; in_addr_list[i] != NULL; i++)
    {
        fprintf(file, "%s, ", inet_ntoa(*in_addr_list[i]));
    }
    fprintf(file, "\n");
}

void getHostname(struct in_addr ip, FILE *file)
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

    fprintf(file, "Official name: %-50s", hostent->h_name);
    fprintf(file, "Alias names: ");
    for (int i = 0; hostent->h_aliases[i] != NULL; i++)
    {
        fprintf(file, "%s, ", hostent->h_aliases[i]);
    }
    fprintf(file, "\n");
}

char* addHttpPrefix(char* original) {
    size_t len1 = strlen("https://");
    size_t len2 = strlen(original);

    char* result = (char*)malloc(len1 + len2 + 1); 							// +1 for the null terminator

    if (result) {
        strcpy(result, "https://");
        strcat(result, original);
    }

    return result;															// return new concated string
}

void getWebsite(char *hostname, char *file_name)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, hostname);
	FILE *file = fopen(file_name, "w"); 					
    
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file); 
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	fclose(file);
}

int compareStrings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

void sortFile(char* inputFileName) {

    FILE *csv_file = fopen(inputFileName, "r");
    if (!csv_file) {
        fprintf(stderr, "Error opening file %s\n", inputFileName);
        return;
    }

    char **lines = NULL;
    char line[4096];
    size_t line_count = 0;

    while (fgets(line, sizeof(line), csv_file)) {
        lines = realloc(lines, (line_count + 1) * sizeof(char *));
        lines[line_count] = strdup(line);
        line_count++;
    }

    fclose(csv_file);

    qsort(lines, line_count, sizeof(char *), compareStrings);

    csv_file = fopen(inputFileName, "w");
    if (!csv_file) {
        fprintf(stderr, "Error opening file %s\n", inputFileName);
        return;
    }

    for (size_t i = 0; i < line_count; i++) {
        fprintf(csv_file, "%s", lines[i]);
        free(lines[i]);
    }

    free(lines);
    fclose(csv_file);
}

void saveLinkToFile(char *file_name)
{
	FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        return;
    }

    char line[4096];								

    FILE *csv_file = fopen("link.csv", "w");
    if (!csv_file) {
        fprintf(stderr, "Error opening link.csv\n");
        fclose(file);
        return;
    }

    regex_t regex;
    regmatch_t match[2];
    int reti;

    char pattern[] = "href=\"(https?://[^\"]+)\"";

    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        fclose(csv_file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        char *pos = line;

        while ((reti = regexec(&regex, pos, 2, match, 0)) == 0) {
            int start = match[1].rm_so + (pos - line);
            int end = match[1].rm_eo + (pos - line);

            char href_value[end - start + 1];
            strncpy(href_value, line + start, end - start);
            href_value[end - start] = '\0';

            fprintf(csv_file, "%s\n", href_value);

            pos += end;
        }
    }

    regfree(&regex);
    fclose(file);
    fclose(csv_file);

    sortFile("link.csv");
}

void saveTextToFile(char *file_name) {

	FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        return;
    }

    FILE *csv_file = fopen("text.csv", "w");
    if (!csv_file) {
        fprintf(stderr, "Error opening text.csv\n");
        fclose(file);
        return;
    }

    char line[4096];
    regex_t regex;
    regmatch_t match[2];
    int reti;

    char pattern[] = "\"text\":\"([^\"]+)\"";

    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        char *pos = line;

        while ((reti = regexec(&regex, pos, 2, match, 0)) == 0) {
            int start = match[1].rm_so + (pos - line);
            int end = match[1].rm_eo + (pos - line);

            char content_value[end - start + 1];
            strncpy(content_value, line + start, end - start);
            content_value[end - start] = '\0';

            fprintf(csv_file, "%s\n", content_value);

            pos += end;
        }
    }

    regfree(&regex);
    fclose(file);

    sortFile("text.csv");
}

void saveVideoToFile(char *file_name) {

	FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        return;
    }

    FILE *csv_file = fopen("video.csv", "w");
    if (!csv_file) {
        fprintf(stderr, "Error opening video.csv\n");
        fclose(file);
        return;
    }

    char line[4096];
    regex_t regex;
    regmatch_t match[2];
    int reti;

    char pattern[] = "\"url\":\"(/watch?[^\"]+)\"";

    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        char *pos = line;

        while ((reti = regexec(&regex, pos, 2, match, 0)) == 0) {
            int start = match[1].rm_so + (pos - line);
            int end = match[1].rm_eo + (pos - line);

            char content_value[end - start + 1];
            strncpy(content_value, line + start, end - start);
            content_value[end - start] = '\0';

            fprintf(csv_file, "%s\n", content_value);

            pos += end;
        }
    }

    regfree(&regex);
    fclose(file);

    sortFile("video.csv");
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

    FILE *outputFile = fopen("output.csv", "a");
    if (outputFile == NULL) {
        printf("Error opening output file.\n");
        return 1;
    }

    if (isIPAdress(parameter))
    {
        struct in_addr ip;
        inet_pton(AF_INET, argv[1], &ip);
        getHostname(ip, outputFile);
        
    } else
    {
        char *hostname = argv[1];
        getIP(hostname, outputFile);
        getWebsite(addHttpPrefix(hostname), "web.txt");
		saveLinkToFile("web.txt");
        saveTextToFile("web.txt");
        saveVideoToFile("web.txt");
    }

    fclose(outputFile);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h> // for dirname()/basename()
#include <time.h>

#define MAX 256
#define PORT 1234

char line[MAX], ans[MAX];
int n;

struct sockaddr_in saddr;
int sfd;

int tokenize(char *pathname, char *output[], char *token, int *num) // YOU have done this in LAB2
{                                                                   // YOU better know how to apply it from now on
    char *s;
    *num = 0;
    s = strtok(pathname, token);

    while (s)
    {
        output[(*num)++] = s; // token string pointers
        s = strtok(0, token);
    }
    output[*num] = 0; // arg[n] = NULL pointer
}

int ls(char *CWD)
{
    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    char buf[512];
    mydir = opendir(CWD);
    while ((myfile = readdir(mydir)) != NULL)
    {
        sprintf(buf, "%s/%s", CWD, myfile->d_name);
        stat(buf, &mystat);
        printf("%zu", mystat.st_size);
        printf(" %s\n", myfile->d_name);
    }
    closedir(mydir);
}

int main(int argc, char *argv[], char *env[])
{
    int n;
    char how[64];
    int i;
    const int cSize = 8; //  0        1         2     3       4      5     6        7
    char *commands[] = {"lmkdir", "lrmdir", "lrm", "lcd", "lpwd", "lls", "get", "put"};

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        printf("socket creation failed\n");
        exit(0);
    }

    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(PORT);

    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }

    printf("********  processing loop  *********\n");
    while (1)
    {
        char *tokCommands[MAX];
        char pathname[MAX];
        char CWD[MAX];
        char lineCpy[MAX];
        int tokNumber;
        int commandIndex;
        int found = 0;

        printf("input a line : ");
        bzero(line, MAX);        // zero out line[ ]
        fgets(line, MAX, stdin); // get a line (end with \n) from stdin
        getcwd(CWD, sizeof(CWD));

        line[strlen(line) - 1] = 0; // kill \n at end

        strcpy(lineCpy, line);
        tokenize(lineCpy, tokCommands, " ", &tokNumber);

        printf("%s%s", tokCommands[0], tokCommands[1]);
        // printf("%s\n", CWD);

        strcpy(pathname, CWD);

        for (int i = 0; i < cSize; i++)
        {
            if (!strcmp(commands[i], tokCommands[0]))
            {
                commandIndex = i;
                if (tokCommands[1])
                {
                    strcat(CWD, "/");
                    strcat(pathname, tokCommands[1]);
                }
                break;
            }
        }
        printf("%s", pathname);

        switch (commandIndex)
        {
        case 0:
            mkdir(pathname, 0755);
            found = 1;
            break;
        case 1:
            rmdir(pathname);
            found = 1;
            break;
        case 2:
            unlink(pathname);
            found = 1;
            break;
        case 4:
            chdir(pathname);
            found = 1;
            break;
        case 5:
            ls(pathname);
            found = 1;
            break;
        case 6:
            break;
        case 7:
            break;

        default:
            break;
        }

        if (line[0] == 0) // exit if NULL line
            exit(0);
        if (!found)
        {
            // Send ENTIRE line to server
            n = write(sfd, line, MAX);
            printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

            // Read a line from sock and show it
            n = read(sfd, ans, MAX);
            printf("client: read  n=%d bytes; echo=(%s)\n", n, ans);
        }
    }
}

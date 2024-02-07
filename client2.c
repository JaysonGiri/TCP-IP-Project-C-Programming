#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int socketid, portnum;
    char buffer[256];

    int length;

    struct sockaddr_in server_address;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(5);
    }

    portnum = atoi(argv[1]);

    socketid = socket(AF_INET, SOCK_STREAM, 0);

    if (socketid < 0)
    {
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }

    memset((char *)&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portnum);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(socketid, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "ERROR connecting\n");
        exit(2);
    }

    for (;;)
    {

        printf("Enter next command> ");

        fgets(buffer, sizeof(buffer) - 1, stdin);

        length = strlen(buffer);

        if (length >= sizeof(buffer))
        {
            printf("Message exceeds the buffer size\n");
        }

        if (strcmp(buffer, "save\n") == 0)
        {
            break;
        }

        send(socketid, buffer, length, 0);

        buffer[length] = '\0';

        while (1)
        {

            length = recv(socketid, &buffer, sizeof(buffer), 0);

            buffer[length] = '\0';

            if (strcmp(buffer, "end") == 0)
            {
                break;
            }
            else
            {
                printf("%s\n", buffer);
            }
        }
    }

    close(socketid);

    return 0;
}

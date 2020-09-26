#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define COMMAX 255 /* Longest allowed command */

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                         /* Socket */
    struct sockaddr_in serverAddress; /* Local address */
    struct sockaddr_in clientAddress; /* Client address */
    unsigned int cliAddrLen;          /* Length of incoming message */
    char commandBuffer[COMMAX];       /* Buffer for received message */
    unsigned short serverPort;        /* Server port */
    int recvMsgSize;                  /* Size of received message */
    char *command;

    if (argc != 2) /* Test for correct number of parameters */
    {
        fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    serverPort = atoi(argv[1]); /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&serverAddress, 0, sizeof(serverAddress));  /* Zero out structure */
    serverAddress.sin_family = AF_INET;                /* Internet address family */
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */

    printf("Before bind, IP address on the server is: %s\n", inet_ntoa(serverAddress.sin_addr));

    serverAddress.sin_port = htons(serverPort); /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        DieWithError("bind() failed");

    printf("After bind, IP address on the server is: %s\n", inet_ntoa(serverAddress.sin_addr));
    printf("Server is listening on port %d . . .\n", serverPort);

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(clientAddress);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, commandBuffer, COMMAX, 0, (struct sockaddr *)&clientAddress, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        commandBuffer[recvMsgSize] = '\0';

        printf("\nServer handling client at IP address: %s\n", inet_ntoa(clientAddress.sin_addr));
        printf("Server receives message: \n%s\n", commandBuffer);

        printf("Enter message to send: \n");
        fgets(commandBuffer, COMMAX, stdin);
        printf("Server sends message \n\n");

        /* Send message of acknowledgment back to the client */
        if (sendto(sock, commandBuffer, strlen(commandBuffer), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) != strlen(commandBuffer))
            DieWithError("sendto() sent a different number of bytes than expected");
        printf("Server listening on port %d . . .\n", serverPort);
    }
    /* NOT REACHED */
}

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define COMMAX 255 /* Longest acceptable command */

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                         /* Socket descriptor */
    struct sockaddr_in serverAddress; /* Server address */
    struct sockaddr_in fromAddr;      /* Source address of response */
    unsigned short serverPort;        /* Server port */
    unsigned int fromSize;            /* In-out of address size for recvfrom() */
    char *servIP;                     /* IP address of server */
    char commandBuffer[COMMAX + 1];   /* Message send/receive data item */
    int respStringLen;                /* Length of received response */

    if (argc < 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP Address> <Server Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    serverPort = atoi(argv[2]); /* Second arg: Use given port, if any */

    printf("Connecting to server at IP %s and port %d\n", servIP, serverPort);

    /* Create a datagram/UDP socket */

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */

    memset(&serverAddress, 0, sizeof(serverAddress));  /* Zero out structure */
    serverAddress.sin_family = AF_INET;                /* Internet addr family */
    serverAddress.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    serverAddress.sin_port = htons(serverPort);        /* Server port */

    for (;;) {
        printf("Enter message to send: \n");
        fgets(commandBuffer, COMMAX, stdin);
        printf("\nClient sends message \n");

        /* Send the struct to the server */
        if (sendto(sock, commandBuffer, strlen(commandBuffer), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != strlen(commandBuffer))
            DieWithError("sendto() sent a different number of bytes than expected");

        /* Receive a response */

        fromSize = sizeof(fromAddr);

        if ((respStringLen = recvfrom(sock, commandBuffer, COMMAX, 0, (struct sockaddr *)&fromAddr, &fromSize)) > COMMAX)
            DieWithError("recvfrom() failed");

        commandBuffer[respStringLen] = '\0';

        if (serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
        {
            fprintf(stderr, "\nError: received a packet from unknown source.\n");
            fprintf(stderr, "Source Address: %s\n", inet_ntoa(fromAddr.sin_addr));
            fprintf(stderr, "Expected Address: %s\n", inet_ntoa(serverAddress.sin_addr));
            exit(1);
        }

        printf("Client received response from server: \n%s\n", commandBuffer);
    }
    close(sock);
    exit(0);
}

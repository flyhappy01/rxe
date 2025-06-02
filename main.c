#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // For close()

#define BUFLEN 512 // Max length of buffer
#define PORT 4791  // The UDP port to listen on

void die(const char *s) {
    perror(s);
    exit(1);
}

void parse_bth(u_char *data, int len);

int ib_rcv(u_char *data, int len) {
    parse_bth(data, len);
    return 0;
}

int main() {
    struct sockaddr_in si_me, si_other;
    int s; // Socket file descriptor
    socklen_t slen = sizeof(si_other);
    char buf[BUFLEN];

    // Create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    // Zero out the structure
    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;         // IPv4
    si_me.sin_port = htons(PORT);       // Port to listen on (converted to network byte order)
    si_me.sin_addr.s_addr = INADDR_ANY; // Listen on all available network interfaces

    // Bind the socket to the port
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        die("bind");
    }

    printf("UDP listener started on port %d...\n", PORT);
    printf("Waiting for data...\n");

    // Keep listening for data
    while (1) {
        // Try to receive some data, this is a blocking call
        ssize_t recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen);
        if (recv_len == -1) {
            die("recvfrom");
        }

        // Print details of the client and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %.*s\n", (int)recv_len, buf); // Print data, ensure it's null-terminated if needed for string functions

        ib_rcv((u_char*)buf, recv_len);
    }

    close(s); // Close the socket (this part of the code is unreachable in the current loop)
    return 0;
}

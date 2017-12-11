#undef UNICODE

#include <winsock2.h>
#include <stdio.h>

void readFileAndSendData(SOCKET);

const char* fileNotFoundException = "Die Datei wurde nicht gefunden!\n";

int main(void) {
    WSADATA wsa;
    SOCKET workerSocket, new_socket;
    struct sockaddr_in server, client;
    int c;
    char *message;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    //Create a socket
    if ((workerSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(32000);

    //Bind
    if (bind(workerSocket, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    puts("Bind done");

    listen(workerSocket, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);

    while ((new_socket = accept(workerSocket, (struct sockaddr *) &client, &c)) != INVALID_SOCKET) {
        puts("Connection accepted");
        readFileAndSendData(new_socket);
    }

    if (new_socket == INVALID_SOCKET) {
        printf("accept failed with error code : %d", WSAGetLastError());
        return 1;
    }

    closesocket(workerSocket);
    WSACleanup();
}

void readFileAndSendData(SOCKET workerSocketDescriptor) {
    const int buffer_len = 250;
    char buffer[buffer_len];

    char* arg;
    char delimiter[] = "-, ";
    int nBytes = 0, n = 0, c = 0;
    FILE *fp;

    recv(workerSocketDescriptor, buffer, buffer_len, 0);

    nBytes = atoi(strtok(buffer, delimiter));
    arg = strtok(NULL, delimiter);

    while (arg != NULL) {
        fp = fopen(arg, "r");
        char charArray[nBytes + 1];

        if (fp == NULL) {
            send(workerSocketDescriptor, fileNotFoundException, strlen(fileNotFoundException), 0);
            fprintf(stderr, "Datei %s nicht gefunden\n", arg);
        } else {
            while (n < nBytes && (c = fgetc(fp)) != EOF) {
                charArray[n] = (char) c;
                n++;
            }

            charArray[n] = '\0';
            send(workerSocketDescriptor, (const char *) charArray, nBytes + 1, 0);

            fclose(fp);
            n = 0;
        }

        arg = strtok(NULL, delimiter);
    }

    closesocket(workerSocketDescriptor);
}
#if defined WIN32
#include <winsock2.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main(void) {
    char serverIP[64];
    int serverPort;

#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("error at WSAStartup\n");
        return -1;
    }
#endif

    
    printf("Inserisci l'IP dell'host (es. 127.0.0.1): ");
    if (scanf("%63s", serverIP) != 1) {
        ErrorHandler("Errore di input IP\n");
        ClearWinSock();
        return -1;
    }

    printf("Inserisci il numero di porta del Server: ");
    if (scanf("%d", &serverPort) != 1 || serverPort <= 0 || serverPort > 65535) {
        ErrorHandler("Numero di porta non valido\n");
        ClearWinSock();
        return -1;
    }

    
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(serverIP);
    if (sad.sin_addr.s_addr == INADDR_NONE) {
        ErrorHandler("Indirizzo IP non valido\n");
        ClearWinSock();
        return -1;
    }
    sad.sin_port = htons(serverPort);

    
    int Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Csocket < 0) {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    
    if (connect(Csocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("Failed to connect.\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    
    char buf[BUFFERSIZE];
    int bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
    if (bytesRcvd <= 0) {
        ErrorHandler("recv() failed or connection closed prematurely\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }
    buf[bytesRcvd] = '\0';
    printf("%s\n", buf);

    while (1) {
        int n1, n2, n3;
        char inputStr[128];

        
        printf("Inserisci tre interi (n1 n2 n3): ");
        if (scanf("%d %d %d", &n1, &n2, &n3) != 3) {
            ErrorHandler("Input non valido\n");
            break;
        }

        snprintf(inputStr, sizeof(inputStr), "%d %d %d", n1, n2, n3);

        if (send(Csocket, inputStr, (int)strlen(inputStr), 0) != (int)strlen(inputStr)) {
            ErrorHandler("send() failed\n");
            break;
        }

      
        bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
        if (bytesRcvd <= 0) {
            ErrorHandler("recv() failed\n");
            break;
        }
        buf[bytesRcvd] = '\0';
        printf("Ricevuto dal server: %s\n", buf);

        if (strcmp(buf, "QUIT") == 0) {
            break; 
        }
    }

    closesocket(Csocket);
    ClearWinSock();
    return 0;
}
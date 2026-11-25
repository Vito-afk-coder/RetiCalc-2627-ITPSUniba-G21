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

#define PROTOPORT 27015
#define QLEN 6
#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

void HandleTCPClient(int clientSocket) {
    char recvBuf[BUFFERSIZE];
    char *welcomeMsg = "connessione avvenuta";

    // passo 5: invio del messaggio di benvenuto
    if (send(clientSocket, welcomeMsg, (int)strlen(welcomeMsg), 0) != (int)strlen(welcomeMsg)) {
        ErrorHandler("send() failed\n");
        return;
    }

    while (1) {
        int n1, n2, n3, res;

        
        int bytesRcvd = recv(clientSocket, recvBuf, BUFFERSIZE - 1, 0);
        if (bytesRcvd <= 0) {
            break; 
        }
        recvBuf[bytesRcvd] = '\0';

        if (sscanf(recvBuf, "%d %d %d", &n1, &n2, &n3) != 3) {
            ErrorHandler("Errore nel formato dei dati ricevuti\n");
            break;
        }

        res = n1 + n2 - n3;
        printf("la somma di %d+%d-%d=%d\n", n1, n2, n3, res);
        
        if (res == 100) {
            char *quitMsg = "QUIT";
            if (send(clientSocket, quitMsg, (int)strlen(quitMsg), 0) != (int)strlen(quitMsg)) {
                ErrorHandler("send() failed\n");
            }
            break;
        } else {
            char resStr[32];
            snprintf(resStr, sizeof(resStr), "%d", res);
            if (send(clientSocket, resStr, (int)strlen(resStr), 0) != (int)strlen(resStr)) {
                ErrorHandler("send() failed\n");
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int port;


    if (argc > 1) {
        port = atoi(argv[1]);
    } else {
        port = PROTOPORT;
    }
    if (port <= 0 || port > 65535) {
        printf("bad port number\n");
        return 0;
    }

#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Error at WSAStartup()\n");
        return 0;
    }
#endif

    
    int MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MySocket < 0) {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }


    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    if (bind(MySocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("bind() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    if (listen(MySocket, QLEN) < 0) {
        ErrorHandler("listen() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in cad;
    int clientSocket;
#if defined WIN32
    int clientLen;
#else
    socklen_t clientLen;
#endif

    printf("Server in ascolto sulla porta %d...\n", port);


    while (1) {
        clientLen = sizeof(cad);
        clientSocket = accept(MySocket, (struct sockaddr *)&cad, &clientLen);
        if (clientSocket < 0) {
            ErrorHandler("accept() failed.\n");
            continue; // resta in ascolto
        }

       
        printf("connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));

        HandleTCPClient(clientSocket);

        closesocket(clientSocket);
    }

 
    closesocket(MySocket);
    ClearWinSock();
    return 0;
}
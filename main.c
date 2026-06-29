
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 1024
#define ADDRESS "127.0.0.1"
#define PORT 5500

void cleanup(SOCKET listener);
int readFile(const char *filename, char **output);
void handleClient(SOCKET client, char *getResponse, int getLen, char *postResponse, int postLen, int *running);
void parseAndPrintPost(char *recvbuf, int res, int *running);

int main(){
    WSADATA wsaData;
    SOCKET listener, client;
    struct sockaddr_in address;
    char *inputFileContents, *thankyouFileContents;
    int inputLen, thankyouLen, res;

    //INITIALIZING
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res) return 1;

    //SOCKET
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        cleanup(0); return 1;
    }

    //BINDING
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);

    res = bind(listener, (struct sockaddr*)&address, sizeof(address));
    if (res == SOCKET_ERROR) { 
        cleanup(listener); 
        return 1; 
    }

    //LISTENING
    res = listen(listener, SOMAXCONN);
    if (res == SOCKET_ERROR) { 
        cleanup(listener); 
        return 1;
    }

    //FILE READ
    inputLen = readFile("input.html", &inputFileContents);
    thankyouLen = readFile("thankyou.html", &thankyouFileContents);
    if (!inputLen || !thankyouLen) {
        cleanup(listener);
        return 1;
    }

    //READY TO ACCEPT
    printf("Accepting on %s:%d\n", ADDRESS, PORT);
    int running = 1;

    //POLLING TO ACCEPT
    while (running){
        client = accept(listener, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        handleClient(client, inputFileContents, inputLen, thankyouFileContents, thankyouLen, &running);
    }

    //SHUTTING DOWN
    cleanup(listener);
    printf("Shutting down.\nGood night.\n");
    return 0;
}



void cleanup(SOCKET listener){
    if (listener && listener != INVALID_SOCKET) closesocket(listener);
    WSACleanup();
}

void handleClient(SOCKET client, char *getResponse, int getLen, char *postResponse, int postLen, int *running)
{
    char recvbuf[BUFLEN + 1];

    int res = recv(client, recvbuf, BUFLEN, 0);

    if (res > 0)
    {
        recvbuf[res] = '\0';

        printf("\n========== REQUEST ==========\n");
        printf("%s\n", recvbuf);

        // GET Request
        if (strncmp(recvbuf, "GET", 3) == 0)
        {
            char header[512];

            sprintf(header,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    getLen);

            send(client, header, (int)strlen(header), 0);
            send(client, getResponse, getLen, 0);
        }

        // POST Request
        else if (strncmp(recvbuf, "POST", 4) == 0)
        {
            char header[512];

            sprintf(header,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    postLen);

            send(client, header, (int)strlen(header), 0);
            send(client, postResponse, postLen, 0);

            parseAndPrintPost(recvbuf, res, running);
        }
        else
        {
            char response[] =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";

            send(client, response, (int)strlen(response), 0);
        }
    }

    shutdown(client, SD_BOTH);
    closesocket(client);
}

void parseAndPrintPost(char *recvbuf, int res, int *running){
    // in POST response we get body as "field=value" from the form, so we move i pointer till the '=', after this is required value 
    int i = res - 1;
    while (i >= 0 && recvbuf[i] != '=') i--;
    i++;
    int len = 0;
    for (int j = i; j < res; j++){
        len++;
        if (recvbuf[j] == '%') j += 2;
    }

    //value is encrypted like this - "we+don%40t+have+that"
    char *msg = malloc(len + 1);
    for (int cursor = 0, j = i; cursor < len; cursor++, j++){
        char c = recvbuf[j];
        //HEX DECODE - for symbols
        if (c == '%') {   
            msg[cursor] = 0;
            for (int k = 1; k <= 2; k++) {
                c = recvbuf[j + k];
                c = (c >= 'A') ? c - 'A' + 10 : c - '0';
                msg[cursor] = (msg[cursor] << 4) | c;
            }
            j += 2;
        } 
        //SPACE
        else if (c == '+') { 
            msg[cursor] = ' ';
        } 
        //CHARACTER
        else {            
            msg[cursor] = c;
        }
    }
    msg[len] = 0;
    printf("Parsed (%d): %s\n", len, msg);
    if (!memcmp(msg, "/quit", 5)) *running = 0;
    free(msg);
}



int readFile(const char *filename, char **output)
{
    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        printf("Cannot open file: %s\n", filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    rewind(fp);

    *output = (char *)malloc(len + 1);

    fread(*output, 1, len, fp);

    (*output)[len] = '\0';

    fclose(fp);

    return len;
}
//
//  server.cpp
//  
//
//  Created by Ousyoukan on 16/6/11.
//
//

// OneServerMain.cpp

#include <iostream>
#include <cstdio>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <vector>
#include <iterator>
#include <algorithm>
#include <Winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
//#include <IPHlpApi.h>
using namespace std;
#pragma comment (lib, "Ws2_32.lib")
//#pragma execution_character_set("utf-8")


#define DEFAULT_PORT "27015"
HANDLE bufferMutex;		
SOCKET sockConn = INVALID_SOCKET;
SOCKET sockSrv = INVALID_SOCKET;

struct addrinfo hints;
struct addrinfo *addrSrv;
int iResult;
string  getHostTime();
SOCKADDR_IN clientAddr;
int clientSize = sizeof(clientAddr);
vector <SOCKET> clientSocketGroup;
vector <SOCKADDR_IN> clientAddrGroup;


string getHostTime()
{
    
    time_t t = time(0);
    char tmp[80];
    memset((void *)tmp, 0, sizeof(tmp));
    strftime(tmp, sizeof(tmp), "œ÷‘⁄ «%Y/%m/%d %X %A",localtime(&t));
    return (tmp);
    
}

int main()
{
    
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0) {
        printf("WSAStartUp failed: %d\n", iResult);
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return -1;
    }
    printf("version is %ld", wsaData.wVersion);
    cout << "WSAStartup success" << endl ;
    
    
    
   
    /*
     SOCKADDR_IN addrSrv;
     //ZeroMemory(&addrSrv, sizeof(addrSrv);
     addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // Ω´INADDR_ANY◊™ªªŒ™Õ¯¬Á◊÷Ω⁄–Ú£¨µ˜”√ htonl(long–Õ)ªÚhtons(’˚–Õ)
     addrSrv.sin_family = AF_INET;
     addrSrv.sin_port = htons(6000);
     */
    
    
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrSrv);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    cout << "get the address information success" << endl;
    
    
    sockSrv = socket(addrSrv->ai_family, addrSrv->ai_socktype, addrSrv->ai_protocol);
    if (sockSrv == INVALID_SOCKET)
    {
        printf("error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(addrSrv);
        WSACleanup();
        return 1;
    }
    cout << "create the socket" << sockSrv << " success" << endl;
    
    iResult = bind(sockSrv, addrSrv->ai_addr, (int)addrSrv->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", iResult);
        freeaddrinfo(addrSrv);
        closesocket(sockSrv);
        WSACleanup();
        return 1;
    }
    cout << "binding success!" << endl;
    cout << "the ai_addr" << addrSrv->ai_addr << endl;
    freeaddrinfo(addrSrv);
    /*
     if (SOCKET_ERROR == bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))){ // µ⁄∂˛≤Œ ˝“™«ø÷∆¿‡–Õ◊™ªª
     return -1;
     }
     */
    
    //listen(sockSrv, 20);
    
    iResult = listen(sockSrv, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(sockSrv);
        WSACleanup();
        return 1;
    }
    cout << "listen is OK!" << endl;
    
    cout << "∑˛ŒÒ∆˜“—≥…π¶æÕ–˜£¨»Ù∑˛ŒÒ∆˜œÎ∑¢ÀÕ–≈œ¢∏¯øÕªß∂À£¨ø…÷±Ω” ‰»Îƒ⁄»›∫Û∞¥ªÿ≥µ.\n";
   
    
    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    //cout << INADDR_ANY << endl;
    DWORD WINAPI SendMessageThread(LPVOID IpParameter);
    DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter);
    
    HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, NULL, 0, NULL);
    
    while (true){
        sockConn = accept(sockSrv, NULL, NULL);
        //sockConn
        if (SOCKET_ERROR != sockConn){
            clientSocketGroup.push_back(sockConn);
        }
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)sockConn, 0, NULL);
        WaitForSingleObject(bufferMutex, INFINITE);
        if (NULL == receiveThread) {
            printf("\nCreatThread AnswerThread() failed.\n");
        }
        else{
            printf("\nCreate Receive Client Thread OK.\n");
        }
        ReleaseSemaphore(bufferMutex, 1, NULL);
    }
    
    WaitForSingleObject(sendThread, INFINITE);
    CloseHandle(sendThread);
    CloseHandle(bufferMutex);
    WSACleanup();
    printf("\n");
    system("pause");
    return 0;
}


DWORD WINAPI SendMessageThread(LPVOID IpParameter)
{
    while (1){
        string talk;
        getline(cin, talk);
        WaitForSingleObject(bufferMutex, INFINITE);
        {
            talk.append("\n");
        }
        printf("I(server) Say:(\"quit\"to exit):");
        cout << talk;
        for (int i = 0; i < clientSocketGroup.size(); ++i){
            //		send(clientSocketGroup[i], talk.c_str(), talk.size(), 0);
            send(clientSocketGroup[i], talk.c_str(), 200, 0);
        }
        ReleaseSemaphore(bufferMutex, 1, NULL);
    }
    return 0;
}


DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter)
{
    SOCKET ClientSocket = (SOCKET)(LPVOID)IpParameter;
    while (1){
        char recvBuf[300];
        memset((void *)recvBuf, 0, sizeof(recvBuf));
        recv(ClientSocket, recvBuf, 200, 0);
        WaitForSingleObject(bufferMutex, INFINITE);
        /*
         int a = ClientSocket;
         char b[10];
         itoa(a, b, 10);
         send(ClientSocket, b, 10, 0);
         */
        
        if (recvBuf[0] == 'q' && recvBuf[1] == 'u' && recvBuf[2] == 'i' && recvBuf[3] == 't' && recvBuf[4] == '\0'){
            vector<SOCKET>::iterator result = find(clientSocketGroup.begin(), clientSocketGroup.end(), ClientSocket);
            clientSocketGroup.erase(result);
            closesocket(ClientSocket);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            printf("\nAttention: A Client has leave...\n", 200, 0);
            break;
        }
        else if (recvBuf[0] == 't' && recvBuf[1] == 'i' && recvBuf[2] == 'm' && recvBuf[3] == 'e'){
            cout << "time" << endl;
            send(ClientSocket, getHostTime().c_str(), 35, 0);
        }
        else if (recvBuf[0] == 'n' && recvBuf[1] == 'a' && recvBuf[2] == 'm' && recvBuf[3] == 'e'){
            char name[100];
            memset((void *)name, 0, sizeof(name));
            iResult = gethostname(name, sizeof(name));
            if (iResult != 0){
                printf("failed to get the host name %d\n", WSAGetLastError());
                return 1;
            }
            cout << "name" << endl;
            send(ClientSocket, name, sizeof(name), 0);
            //cout << "Server says: " << name << endl;
        }
        else if (recvBuf[0] == 'l' && recvBuf[1] == 'i' && recvBuf[2] == 's' && recvBuf[3] == 't'){
            char listClient[100];
            char clientId[10];
            char clientPort[8];
            memset((void *)listClient, 0, sizeof(listClient));
            memset((void *)clientId, 0, sizeof(clientId));
            memset((void *)clientPort, 0, sizeof(clientPort));
            cout << "list" << endl;
            for (int i = 0; i < clientSocketGroup.size(); ++i){
                iResult = getpeername(clientSocketGroup[i], (SOCKADDR *)&clientAddr, &clientSize);
                if (iResult != 0){
                    printf("getpeername failed: %d\n", WSAGetLastError());
                }
                itoa(clientSocketGroup[i], clientId, 10);
                strcpy(listClient, "ClientSocket_ID: ");
                strcat_s(listClient, 100, clientId);
                strcat_s(listClient, 100, " IP: ");
                strcat_s(listClient, 100, inet_ntoa(clientAddr.sin_addr));
                strcat_s(listClient, 100, " port: ");
                //strcat_s(listClient, 100, clientAddr.sin_port);
                ultoa(ntohs(clientAddr.sin_port), clientPort, 10);
                strcat_s(listClient, 100, clientPort);
                //		send(clientSocketGroup[i], talk.c_str(), talk.size(), 0);
                send(ClientSocket, listClient, 100, 0);
            }
        }
        else if (recvBuf[0] == 's' && recvBuf[1] == 'e' && recvBuf[2] == 'n' && recvBuf[3] == 'd'){
            char desClientId[10];
            char souClientId[10];
            char message[200];
            memset((void *)desClientId, 0, sizeof(desClientId));
            memset((void *)souClientId, 0, sizeof(souClientId));
            memset((void *)message, 0, sizeof(message));
            itoa(ClientSocket, souClientId, 10);
            cout << "send" << endl;
            if (recvBuf[5] == '<'){
                for (int i = 6; i < 20; i++){
                    if (recvBuf[i] == '>'){
                        strcpy(message, souClientId);
                        strcat_s(message,200, "says: ");
                        strcat_s(message, 200, recvBuf + i + 2);
                        cout << message << "to" << desClientId << endl;
                        strcat_s(message, 200, " to you.\n");
                        send(atoi(desClientId), message, 200, 0);
                        
                        break;
                    }
                    desClientId[i - 6] = recvBuf[i];
                }
            }
            else{
                char* normalFormat;
                strcpy(normalFormat,"Wrong format! The correct format is 'send<id> content'!");
                send(ClientSocket, normalFormat, 100, 0);
            }
        }
        else if (recvBuf[0] != '\0'){
            //mesg = recvBuf;
            cout << ClientSocket << " says:" << recvBuf << endl;
        }
        else{
            vector<SOCKET>::iterator result = find(clientSocketGroup.begin(), clientSocketGroup.end(), ClientSocket);
            clientSocketGroup.erase(result);
            closesocket(ClientSocket);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            printf("\nAttention: A Client has leave...\n", 200, 0);
            break;
        }	
        
        //printf("%s Says: %s\n", "One Client", recvBuf);
        ReleaseSemaphore(bufferMutex, 1, NULL);
    }
    return 0;
}


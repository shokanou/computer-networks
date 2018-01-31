//
//  123.cpp
//  
//
//  Created by Ousyoukan on 16/6/11.
//
//


#include "stdafx.h"
#include <Winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#define DATA_BUFFER 1024

int main(int argc, char * argv[])
{
    char command[100];
    char first_part[10],second_part[20],third_part[10];
    char time[100];
    char hostname[100];
    char sendtext[100];
    char message[100];
    char list[100];
    WSADATA wsaData;
    SOCKET sClient;
    struct sockaddr_in ser;
    int	iSend;
    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    
    memset(command, 0, sizeof(command));
    memset(first_part, 0, sizeof(first_part));
    memset(second_part, 0, sizeof(second_part));
    memset(third_part, 0, sizeof(third_part));
    memset(time, 0, sizeof(time));
    memset(hostname, 0, sizeof(hostname));
    memset(sendtext, 0, sizeof(sendtext));
    memset(list, 0, sizeof(list));
    
    
    
    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
    {
        printf("---Failed to load Winsock.\n");
        Sleep(3000);
        return -1;
    }
    sClient = socket(AF_INET,SOCK_STREAM,0);
    if(sClient == INVALID_SOCKET)
    {
        printf("---The function socket() has failed: %d\n",WSAGetLastError());
        Sleep(3000);
        return -1;
    }
    setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO,(const char *) &tv_out, sizeof(tv_out));
    printf("---please input your command\n ");
    gets(command);
    
    
    while(strcmp(command,"quit")!=0){
        memset(message,0,sizeof(message));
        recv(sClient,message,sizeof(message),0);
        if(strcmp(message,time)*strcmp(message,hostname)*strcmp(message,list)!=0)
            printf("%s\n",message);
        sscanf(command,"%[^ ]",first_part);
        sscanf(command,"%*s%s",second_part);
        sscanf(command,"%*s%*s%s",third_part);
        if(strcmp(first_part,"conn")==0){
          
            ser.sin_family = AF_INET;
            ser.sin_port = htons(atoi(third_part));
     
            ser.sin_addr.s_addr = inet_addr(second_part);
          
            
            if(connect(sClient,(struct sockaddr *)&ser,sizeof(ser)) == INVALID_SOCKET)
            {
                printf("---This time's connection Failed: %d\n",WSAGetLastError());
                Sleep(500);
                return -1;
            }
            else{
                printf("---connect success!\n");
            }
        }
        else if(strcmp(command,"disconn")==0){
            send(sClient,command,sizeof(command),0);
            closesocket(sClient);
            sClient = socket(AF_INET,SOCK_STREAM,0);
            if(sClient == INVALID_SOCKET)
            {
                printf("---The function socket() has failed: %d\n",WSAGetLastError());
                Sleep(500);
                return -1;
            }
            setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO,(const char *) &tv_out, sizeof(tv_out));
        }
        
        else if(strcmp(command,"time")==0||strcmp(command,"name")==0||strcmp(command,"list")==0){
            
            iSend = send(sClient,command,sizeof(command),0);
            if(iSend == SOCKET_ERROR)
            {
                printf("---The function send() has failed: %d\n",WSAGetLastError());
                Sleep(500);
                break;
            }
            else if(iSend == 0)
            {
                break;
            }
            else
            {
                printf("---right commamd: %s \n",command);
            }
            
            if(strcmp(command,"time")==0){
                recv(sClient,time,sizeof(time),0);
                printf("---The time from the server is: %s\n\n",time);
                Sleep(500);
            }
            
            else if(strcmp(command,"name")==0){
                recv(sClient,hostname,sizeof(hostname),0);
                printf("---The sever's name is: %s\n\n",hostname);
                Sleep(500);
            }
            
            else{
                recv(sClient,list,sizeof(list),0);
                printf("---The client list of server :\n%s\n",list);   
                Sleep(500);
            }
        }
        else if(strcmp(first_part,"send")==0){
            printf("---please input the message to send\n");
            gets(sendtext);
            send(sClient,command,sizeof(command),0);
            send(sClient,sendtext,sizeof(sendtext),0);
            printf("--The message has been sent!\n");
            Sleep(500);
        }
        else{
            printf("---sorry, %s is not a command\n",command);
        }
        
        memset(message,0,sizeof(message)); 
        recv(sClient,message,sizeof(message),0);
        if (strcmp(message, time)*strcmp(message, hostname)*strcmp(message, list) != 0){
            printf("%s\n", message);
            Sleep(500);
        }
        
        printf("---please input your command\n ");
        gets(command);
    }
    closesocket(sClient);
    WSACleanup();
    return 0;
}

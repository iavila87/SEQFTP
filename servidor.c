#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "defServidor.h"

char bufferOut[64];
char bufferIn[64];

int main(int argc, char* argv[]){
    
    if(argc != 2){
        perror("Debe introducir 1 argumento\n");
        exit(ERR_ARGS);
    }
    
    int sd;                                 // Socket descriptor
    int clientesd;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t longStruct = sizeof(server_address);

    sd = initSocket("127.0.0.1", argv[1], &server_address, longStruct);
    
    while(1){
        printf("Esperando conexion de un cliente...\n");
        clientesd = acceptClient(sd, &client_address, &longStruct);
        
        respCmd(clientesd);
        printf("Conectado con: %s\n",bufferIn);
        sendCmd(clientesd, DSC_OPEN, CMD_INIT);
        //printf("bufferOut: %s",bufferOut);
    
        while(1){
        
            respCmd(clientesd);
            printf("Redibo de comando: %s\r\n",bufferIn);
        
            if(strncmp( bufferIn, OPR_QUIT, (sizeof(OPR_QUIT)-1) ) == 0){
                memset(bufferOut, 0, sizeof(bufferOut));
                sprintf(bufferOut, "%s\r\n",CMD_OKQUIT);
                if(write(clientesd,bufferOut,sizeof(bufferOut)) < 0){
                    perror("Error en la escritura final\r\n");
                }
            
                close(clientesd);
                printf("Cliente desconectado.\n");
                printf("\n");
                printf("***********************************\n");
                break;
            }else{
                memset(bufferOut, 0, sizeof(bufferOut));
                sprintf(bufferOut, "%s %s %s\r\n", CMD_INIT, DSC_NAME, VERSION);
                write(clientesd,bufferOut,sizeof(bufferOut));
            }        
        }
    }
    
    close(sd);

    return END_OK;
}


int initSocket(char * ip, char * port, struct sockaddr_in * add, socklen_t l){
    (*add).sin_family = AF_INET;             // Protocolo IPv4
    (*add).sin_addr.s_addr = inet_addr(ip);  // IP
    (*add).sin_port = htons(atoi(port));     // Puerto

    int d= socket(AF_INET, SOCK_STREAM, 0); // En sd se almacena el socket descriptor
    if(d < 0){
        perror("No se pudo crear el socket.\n");
        exit(ERR_CREATESOCK);
    }

    if(bind(d,(struct sockaddr *)add, l)<0){
        perror("No se pudo realizar el bind.\n");
        exit(ERR_BIND);
    }
    if(listen(d,1)<0){
        perror("Fallo listen.\n");
        exit(ERR_LISTEN);
    }

    return d;
}

int acceptClient(int s, struct sockaddr_in * add, socklen_t * l){
    int d =accept(s,(struct sockaddr *)add, l);
    if(d < 0){
        perror("No se puedo acceptar al cliente\n");
    }
    return d;
}

void respCmd(int sockd){
    // Blanqueo el buffer de recepcion
    memset(bufferIn, 0, sizeof(bufferIn));
    // Lee el string desde el servidor
    if(read(sockd,bufferIn,sizeof(bufferIn)) <0){
        perror("No se obtuvo recepcion\n");
        exit(ERR_SENDSERV);
    }
}

void sendCmd(int sockd, char * cmd, char * dsc){
    // Blanqueo el buffer de recepcion
    memset(bufferOut, 0, sizeof(bufferOut));
    if(strcmp(cmd, CMD_OKQUIT) == 0){
        sprintf(bufferOut, "%s\r\n", CMD_OKQUIT);
    }
    else{
        if(strcmp(cmd, DSC_OPEN)==0){
            sprintf(bufferOut, "%s %s %s\r\n", CMD_INIT, DSC_OPEN, VERSION);
        }
    }
    
    // Escribe el string al servidor 
    if(write(sockd, bufferOut, sizeof(bufferOut)) < 0){
        perror("No se pudo escribir en el servidor.\n");
        exit(ERR_RECVSERV);
    }
}
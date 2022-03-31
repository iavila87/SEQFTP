#include <stdio.h>      //perror, printf
#include <stdlib.h>     //exit
#include <arpa/inet.h>  //socket
#include <string.h>     //memset
#include <unistd.h>     //file management

#include "defCliente.h"

char bufferIn[64];
char bufferOut[64];
char consoleIn[64];

int main(int argc, char* argv[]){
    // verifico la cantidad de argumentos
    if(argc != 3){
        perror("Debe introducir 2 argumentos\n");
        exit(ERR_ARGS);
    }

    int sd;         // socket descriptor
    int flagExit = 0;
    struct sockaddr_in addrSock;
    
    sd = initSocket(argv[1], argv[2], &addrSock);
    connectServer(sd, &addrSock);

    sendCmd(sd, DSC_OPEN, DSC_OPEN);

    respCmd(sd);
    unsigned int code = codeRecv(bufferIn);
    printf("code received: %d\r\n",code);

    do{
        printf("-> ");
        scanf("%s", bufferOut);

        if(strcmp(bufferOut, CMD_QUIT) == 0){
            sendCmd(sd, CMD_QUIT, DSC_OPEN);
        }
    
        // Lee el string desde el servidor
        respCmd(sd);
        if(strncmp(bufferIn, CMD_OKQUIT,(sizeof(CMD_OKQUIT))-1) == 0){
            flagExit = 1;
            printf("Cerrando conexion.\r\n");
        }
        //printf("llego del servidor: %s\n",bufferIn);

    }while(flagExit == 0);

    close(sd);
    return END_OK;
}

int initSocket(char * ip, char * port, struct sockaddr_in * add){
    (*add).sin_family = AF_INET;            // Protocolo IPv4
    (*add).sin_addr.s_addr = inet_addr(ip); // IP
    (*add).sin_port = htons(atoi(port));    // Puerto

    int d= socket(AF_INET, SOCK_STREAM, 0); // En sd se almacena el socket descriptor
    if(d < 0){
        perror("No se pudo crear el socket.\n");
        exit(ERR_CREATESOCK);
    }
    return d;
}

void connectServer(int sockd, struct sockaddr_in * add){
    // Realiza la conexion con el servidor, con los argumentos recibidos
    if(connect(sockd, (struct sockaddr *)(add), sizeof(*add)) < 0){
        perror("Conexion fallida con el servidor.\n");
        exit(ERR_CNNT_SERV);    
    }
}

void sendCmd(int sockd, char * cmd, char * dsc){
    // Blanqueo el buffer de recepcion
    //printf("\ncmd: %s\n",cmd);
    memset(bufferOut, 0, sizeof(bufferOut));
    if(strcmp(cmd, CMD_QUIT) == 0){
        sprintf(bufferOut, "%s\r\n", CMD_QUIT);
    }
    else{
        if(strcmp(cmd, DSC_OPEN)==0){
            sprintf(bufferOut, "%s %s %s\r\n", CMD_INIT, DSC_OPEN, VERSION);
        }else{
            if(strcmp(cmd, CMD_USER)==0){
                sprintf(bufferOut, "%s %s\r\n", CMD_USER, consoleIn);
            }else{
                if(strcmp(cmd, CMD_PASS)==0){
                    sprintf(bufferOut, "%s %s\r\n", CMD_PASS, consoleIn);
                }
                
            }
        }
    }
    
    // Escribe el string al servidor 
    if(write(sockd, bufferOut, sizeof(bufferOut)) < 0){
        perror("No se pudo escribir en el servidor.\n");
        exit(ERR_SENDCLIENT);
    }
}

void respCmd(int sockd){
    // Blanqueo el buffer de recepcion
    memset(bufferIn, 0, sizeof(bufferIn));
    // Lee el string desde el servidor
    if(read(sockd,bufferIn,sizeof(bufferIn)) <0){
        perror("No se obtuvo recepcion\n");
        exit(ERR_RECVCLIENT);
    }
}

unsigned int codeRecv(char * c){
    char Scode[3];
    strncpy(Scode,c,3);
    return atoi(Scode);
}

void clientAuntheticate(int s){
    printf("username: ");
    scanf("%s", consoleIn); // Solicito el nombre de usario por consola.
    sendCmd(s, CMD_USER, NULL); // envio la trama "USER 'username'"
    respCmd(s);                 // respuesta del servidor
    memset(consoleIn, 0, sizeof(consoleIn)); // Blanqueo consoleIn
    printf("%s",bufferIn);
    if(codeRecv(bufferIn)==OP_RPASS){   // de la trama recibida tomo los primeros ascii y los comparo para saber si necesito password o no.
        printf("passwd: ");             // 331 Password required for <nombreUsuario>\r\n
        scanf("%s", consoleIn);
        sendCmd(s, CMD_PASS, NULL);
        respCmd(s);
        if(codeRecv(bufferIn)==OP_LOGIN){ // 230 User <nombreUsuario> logged in\r\n
            /* code para cuanto me logueo correctamente*/
        }else{ 
            if(codeRecv(bufferIn)==OP_LOGOUT){ // 530 Login incorrect\r\n
            /* code */
            }
        }
    }
    //strchr

}

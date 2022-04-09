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
    struct sockaddr_in addrLocalSock; // estructura que contine los datos de la maquina local
    socklen_t addrLSLen = (sizeof(addrLocalSock));
    char cmds[10]; // contiene el comando recibido por teclado 
    char prms[50]; // contiene el parametro recibido por teclado

    /*******************************************/
    addrLocalSock.sin_family = AF_INET;         // Protocolo IPv4
    addrLocalSock.sin_addr.s_addr = INADDR_ANY; // IP
    addrLocalSock.sin_port = htons(0);          // Puerto
    /*******************************************/
    
    sd = initSocket(argv[1], argv[2], &addrSock);
    connectServer(sd, &addrSock);

    /*******************************************/
    getsockname(sd, (struct sockaddr *)&addrLocalSock, &addrLSLen);
    printf("mi puerto es: %d\n", ntohs(addrLocalSock.sin_port));
    /*******************************************/
    
    sendCmd(sd, DSC_OPEN, DSC_OPEN);

    respCmd(sd);
    unsigned int code = codeRecv(bufferIn);
    
    printf("%s",bufferIn);
    flagExit = clientAuntheticate(sd);
    
    while(flagExit == 0){
        
        printf("-> ");
        /**************************/
        // se quito scanf para poder adquirir string que contengan espacios en blanco
        fgets(bufferOut,sizeof(bufferOut),stdin); // ingreso de comandos por consola.
        if ((strlen(bufferOut) != 0) && (bufferOut[strlen (bufferOut) - 1] == '\n')){
            bufferOut[strlen (bufferOut) - 1] = '\0';
        }
        /**************************/

        extCmdParam(bufferOut, cmds, prms);
        
        if(strcmp(bufferOut, CMD_QUIT) == 0){
            sendCmd(sd, CMD_QUIT, DSC_OPEN);
        }else{
            if(strncmp(bufferOut, CMD_GET, strlen(CMD_GET)) == 0){
                sendCmd(sd, CMD_RETR, prms);
            }else{
                printf("Comando no reconocido.\n");
            }
        }
    
        // Lee el string desde el servidor
        respCmd(sd);
        if(strncmp(bufferIn, CMD_OKQUIT,(sizeof(CMD_OKQUIT))-1) == 0){
            flagExit = 1;
            printf("Cerrando conexion.\r\n");
        }else{
            if(codeRecv(bufferIn) == OP_FILEE){
                printf("%s", bufferIn);
            }else{
                if(codeRecv(bufferIn) == OP_FILENE){
                    printf("%s", bufferIn);
                }else{

                }
            }
        }

    }

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
                }else{
                    if(strcmp(cmd, CMD_RETR)==0){
                        sprintf(bufferOut, "%s %s\r\n", CMD_RETR, dsc);
                    }
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

void extCmdParam(char * buffer, char * c, char * p){
    char nb[64];
    char * aux;
    int n;  // contiene la longitud de la cadena a extraer

    memset(c, 0, sizeof(c)); // Blanqueo c
    memset(p, 0, sizeof(p)); // Blanqueo p
    aux = strchr(buffer,' ');
    
    if(aux == NULL){
        strcpy(c,buffer);
    }else{
        n = aux-buffer;
        printf("aux: %p - buffer: %p\n", aux, buffer);
        strncpy(c, buffer, n);
    }
    n += 1;
    strcpy(p,buffer+n);

    printf("cmmd: %s\n", c);
    printf("pram: %s\n", p);
}

//Retorna el codigo recibido
unsigned int codeRecv(char * c){
    char Scode[3];
    strncpy(Scode,c,3);
    return atoi(Scode);
}


int clientAuntheticate(int s){
    printf("username: ");

    /**************************/
    fgets(consoleIn,sizeof(consoleIn),stdin); // Solicito el nombre de usario por consola.
    if ((strlen(consoleIn) != 0) && (consoleIn[strlen (consoleIn) - 1] == '\n')){
        consoleIn[strlen (consoleIn) - 1] = '\0';
    }
    /**************************/

    sendCmd(s, CMD_USER, NULL); // envio la trama "USER 'username'"
    respCmd(s);                 // respuesta del servidor
    memset(consoleIn, 0, sizeof(consoleIn)); // Blanqueo consoleIn
    
    if(codeRecv(bufferIn)==OP_RPASS){   // de la trama recibida tomo los primeros ascii y los comparo para saber si necesito password o no.
        printf("%s",bufferIn);
        printf("passwd: ");             // 331 Password required for <nombreUsuario>\r\n
        
        /**************************/
        fgets(consoleIn,sizeof(consoleIn),stdin); // Solicito el nombre de usario por consola.
        if ((strlen(consoleIn) != 0) && (consoleIn[strlen (consoleIn) - 1] == '\n')){
            consoleIn[strlen (consoleIn) - 1] = '\0';
        }
        /**************************/

        sendCmd(s, CMD_PASS, NULL);
        respCmd(s);
        
        if(codeRecv(bufferIn) == OP_LOGIN){ // 230 User <nombreUsuario> logged in\r\n
            /* code para cuanto me logueo correctamente*/
            printf("%s", bufferIn);
            return 0;
        }else{ 
            if(codeRecv(bufferIn) == OP_LOGOUT){ // 530 Login incorrect\r\n
            /* code */
            printf("%s", bufferIn);
            return 1;
            }
        }
    }
    printf("%s",bufferIn);
    return 1;
   
}

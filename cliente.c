#include <stdio.h>      //perror, printf
#include <stdlib.h>     //exit
#include <arpa/inet.h>  //socket
#include <string.h>     //memset
#include <unistd.h>     //file management

#include "defCliente.h"

char bufferIn[64];
char bufferOut[64];
char consoleIn[64];
char ipport[100];

int main(int argc, char* argv[]){
    // verifico la cantidad de argumentos
    if(argc != 3){
        perror("Debe introducir 2 argumentos\n");
        exit(ERR_ARGS);
    }

    int sd;         // socket descriptor
    int sdData;
    int flagExit = 0;
    struct sockaddr_in addrSock;
    struct sockaddr_in addrLocalSock; // estructura que contine los datos de la maquina local
    struct sockaddr_in addrLocalServ;
    socklen_t addrLSLen = (sizeof(addrLocalSock));
    char cmds[10]; // contiene el comando recibido por teclado 
    char prms[50]; // contiene el parametro recibido por teclado
    char iplocal[16];
    int flagFTrnsf = 0;
    int portLocal;
    int portData;
    /*******************************************/
    addrLocalSock.sin_family = AF_INET;         // Protocolo IPv4
    addrLocalSock.sin_addr.s_addr = INADDR_ANY; // IP
    addrLocalSock.sin_port = htons(0);          // Puerto
    /*******************************************/
    
    sd = initSocket(argv[1], argv[2], &addrSock);
    connectServer(sd, &addrSock);

    /*******************************************/
    getsockname(sd, (struct sockaddr *)&addrLocalSock, &addrLSLen);
    inet_ntop(AF_INET,&(addrLocalSock.sin_addr), iplocal, INET_ADDRSTRLEN);
    printf("mi ip es: %s\n",iplocal);
    portLocal = ntohs(addrLocalSock.sin_port);
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
                // si llegue hasta aca entonces debo crear la conexion de datos
                // con el servidor para que me envie el archivo
                flagFTrnsf = 1;
            }else{
                if(codeRecv(bufferIn) == OP_FILENE){
                    printf("%s", bufferIn);
                }else{

                }
            }
        }


        if(flagFTrnsf){
            // abro un nuevo socket para los datos
            sdData = socket(AF_INET, SOCK_STREAM, 0); // En sd se almacena el socket descriptor
            if(sdData < 0){
                perror("No se pudo crear el socket.\n");
                exit(ERR_CREATESOCK);
            }
            // recupero el puerto actual para abrir el siguiente
            // de estar ocupado abro uno libre
            portLocal = ntohs(addrLocalSock.sin_port);
            addrLocalSock.sin_port = htons(portLocal+1); //le asigno el puerto siguiente
            // pruebo realizar un bind si es correcto utilizara este puerto en caso contrario
            // realizaremos otro bind con un puerto libre
            if(bind(sdData,(struct sockaddr *)&addrLocalSock, addrLSLen)<0){
                perror("No se pudo realizar el bind.\n");
                // cambio el puerto por uno libre y realizo nuevamente el bind
                addrLocalSock.sin_port = htons(0); //le asigno el puerto siguiente
                if(bind(sdData,(struct sockaddr *)&addrLocalSock, addrLSLen)<0){
                    perror("No se pudo realizar el bind.\n");
                    exit(ERR_BIND);
                }
                getsockname(sdData, (struct sockaddr *)&addrLocalSock, &addrLSLen);
                inet_ntop(AF_INET,&(addrLocalSock.sin_addr), iplocal, INET_ADDRSTRLEN);
                printf("mi ip de datos es: %s\n",iplocal);
                printf("mi puerto de datos es: %d\n", ntohs(addrLocalSock.sin_port));
            }

            getsockname(sdData, (struct sockaddr *)&addrLocalSock, &addrLSLen);
            inet_ntop(AF_INET,&(addrLocalSock.sin_addr), iplocal, INET_ADDRSTRLEN);
            printf("mi ip de datos es: %s\n",iplocal);
            printf("mi puerto de datos es: %d\n", ntohs(addrLocalSock.sin_port));

            if(listen(sdData,1)<0){
                perror("Fallo listen.\n");
                exit(ERR_LISTEN);
            }
            // envio PORT
            formatIpPort(ipport, iplocal, ntohs(addrLocalSock.sin_port));
            sendCmd(sd, CMD_PORT, ipport);
            // Lee el string desde el servidor
            respCmd(sd);
            // Me quedo esperando que se conecte el servidor para transferir el archivo
            if(codeRecv(bufferIn) == OP_CMMDOK){
                int sds =accept(sdData,(struct sockaddr *)&addrLocalServ, addrLSLen);
                if(sds < 0){
                    perror("No se puedo acceptar al cliente\n");
                }

                //soy servidor primero leo despues respondo
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
                    }else{
                        if(strcmp(cmd, CMD_PORT)==0){
                            sprintf(bufferOut, "%s %s\r\n", CMD_PORT, dsc);
                        }
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

// formatea el string para ser enviado con el comando PORT
void formatIpPort(char * strOut, char * ip, int port){
    char * aux;
    char * auxA;
    char ip1Aux[4];
    char ip2Aux[4];
    char ip3Aux[4];
    char ip4Aux[4];
    memset(ip1Aux,0,sizeof(ip1Aux));
    memset(ip2Aux,0,sizeof(ip2Aux));
    memset(ip3Aux,0,sizeof(ip3Aux));
    memset(ip4Aux,0,sizeof(ip4Aux));
    int portH;
    int portL;
    int n;
    aux = strchr(ip,'.');
    n = aux - ip;
    aux += 1;
    strncpy(ip1Aux, ip, n); // 1 octeto
    
    auxA = aux;
    aux = strchr(auxA,'.');
    n = aux - auxA;
    aux += 1;
    strncpy(ip2Aux, auxA, n); // 2 octeto

    auxA = aux;
    aux = strchr(auxA,'.');
    n = aux - auxA;
    aux += 1;
    strncpy(ip3Aux, auxA, n); // 3 octeto

    auxA = aux;
    aux = strchr(auxA,'\0');
    n = aux - auxA;
    aux += 1;
    strncpy(ip4Aux, auxA, n); // 4 octeto
    portH = (port >> 8) & 0x000000FF;
    portL = port &  0x000000FF;
    sprintf(strOut,"%s,%s,%s,%s,%d,%d", ip1Aux, ip2Aux, ip3Aux, ip4Aux, portH, portL);

}
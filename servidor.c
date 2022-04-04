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
    char * cmdRcv; // comando recibido
    char * pmtRcv; // parametro recibido
    char * fUser; // contiene el user en cuestion
    char * fPass; // contiene el password leido desde el archivo
    FILE * file; // contiene la struct que maneja el archivo

    sd = initSocket("127.0.0.1", argv[1], &server_address, longStruct);
    
    while(1){
        printf("Esperando conexion de un cliente...\n");
        clientesd = acceptClient(sd, &client_address, &longStruct);
        
        respCmd(clientesd);
        printf("Conectado con un nuevo cliente\n");
        sendCmd(clientesd, DSC_OPEN, CMD_INIT);
        //printf("bufferOut: %s",bufferOut);
    
        while(1){
        
            respCmd(clientesd);
            
            cmdRcv = extractCmd(bufferIn);
        
            if(strncmp( cmdRcv, OPR_QUIT, (sizeof(OPR_QUIT)-1) ) == 0){
                // Tratamiento comando QUIT
                memset(bufferOut, 0, sizeof(bufferOut));
                sprintf(bufferOut, "%s\r\n",CMD_OKQUIT);
                if(write(clientesd,bufferOut,sizeof(bufferOut)) < 0){
                    perror("Error en la escritura final\r\n");
                }
            
                close(clientesd);
                printf("Cliente desconectado.\n");
                printf("\n");
                printf("***********************************\n\n");
                break;
            }else{
                if(strncmp( cmdRcv, OPR_USER, (sizeof(OPR_USER)-1) ) == 0){
                    // Tratamiento comando USER
                    fUser = extract1Pmt(bufferIn); // extrae el parametro del buffer de entrada
                    file = openFile("ftpusers", "r");
                    fPass = searchUserFile(file, fUser);
                    if(fPass == NULL){
                        //responder que no es valido el usuario
                        memset(bufferOut, 0, sizeof(bufferOut));
                        sprintf(bufferOut, "%s %s\r\n", CMD_LOGERROR, TXT_LOGERROR);
                        write(clientesd,bufferOut,sizeof(bufferOut));
                        close(clientesd);
                        break;
                    }else{
                        //responder que solicito la pass
                        memset(bufferOut, 0, sizeof(bufferOut));
                        sprintf(bufferOut, "%s %s %s\r\n", CMD_UPASS, TXT_PASSREQ, fUser);
                        write(clientesd,bufferOut,sizeof(bufferOut));
                    }
                }else{
                    if(strncmp( cmdRcv, OPR_PASS, (sizeof(OPR_PASS)-1) ) == 0){
                        // Tratamiento comando PASS
                        pmtRcv = extract1Pmt(bufferIn); // extrae el parametro del buffer de entrada
                        if(strncmp(fPass, pmtRcv, strlen(fPass)-1) == 0){
                            //responder logueo exitoso
                            memset(bufferOut, 0, sizeof(bufferOut));
                            sprintf(bufferOut, "%s %s %s %s\r\n", CMD_LOGIN, TXT_LOGIN1, fUser, TXT_LOGIN2);
                            write(clientesd,bufferOut,sizeof(bufferOut));
                        }else{
                            //responder que no es correcta la pass
                            memset(bufferOut, 0, sizeof(bufferOut));
                            sprintf(bufferOut, "%s %s\r\n", CMD_LOGERROR, TXT_LOGERROR);
                            write(clientesd,bufferOut,sizeof(bufferOut));
                            close(clientesd);
                            break;
                        }
                    }else{
                        memset(bufferOut, 0, sizeof(bufferOut));
                        sprintf(bufferOut, "%s %s %s\r\n", CMD_INIT, DSC_NAME, VERSION);
                        write(clientesd,bufferOut,sizeof(bufferOut));
                    }
                    
                }
                
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

// retorna el comando recibido en el buffer de entrada
char * extractCmd(char * s){
    char * c;
    int n;
    // se libera la memoria luego en el flujo principal
    char * d = malloc(5); // reservo 4 bytes para los caracteres del comando y el 5 para \0
    memset(d, 0, sizeof(5));
    
    //printf("soy n del extractCmd %s\n",d);
    c = strchr(s,' ');

    if(c == NULL){
        return s;
    }else{
        n = c-s;
        strncpy(d, s, n);
        return d;
    }
}


// retorna el parametro recibido en el buffer de entrada
char * extract1Pmt(char * s){
    // se libera la memoria luego en el flujo principal
    char * d = malloc(100); // reservo 100 bytes para paremetros
    char * r;
    int end;
    memset(d, 0, sizeof(100));

    r = strchr(s,' ');
    r+=1;
    end = strchr(r,'\r') - r;
    strncpy(d,r,end);
    
    if(d == NULL){
        return NULL;
    }else{
        return d;
    }
}


// abre el archivo ascii, y retorna FILE *
FILE * openFile(char * path, char * type){
    return fopen (path, type);
}

// busca en el archivo el usuario y retorna su contraseña
char * searchUserFile(FILE * f, char * s){
    char linea[50];
    char * c;
    char *p = malloc(50);
    char fu[50];
    memset(fu,0,50);
    int n;

    while (feof(f) == 0){
 		fgets(linea,50,f);
        c = strchr(linea,':');
        n = c - linea;
        c = &c[0] +1;

        strncpy(fu, linea, n);

        if(strncmp(s, fu, n) == 0){
            strncpy(p, c, strlen(c));
            return p;
        }
 	}
    return NULL;
}

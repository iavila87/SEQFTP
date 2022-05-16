#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "defServidor.h"

char bufferOut[64];
char bufferIn[64];
char bufferData[512];

int main(int argc, char* argv[]){
    
    if(argc != 2){
        perror("Debe introducir 1 argumento\n");
        exit(ERR_ARGS);
    }
    
    int sd;                                 // Socket descriptor
    int clientesd;
    int sdData;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    struct sockaddr_in server_data;
    char ipclient[INET_ADDRSTRLEN]; // IP con la que se conecto el cliente
    char ipData[INET_ADDRSTRLEN]; // IP con la que se conecto el cliente para los datos
    int portclient; // puert con el que se conecto el cliente
    int portData;
    int flagPort;
    socklen_t longStruct = sizeof(server_address);
    char * cmdRcv; // comando recibido
    char pmtRcv[100]; // parametro recibido
    char fUser[100]; // contiene el user en cuestion
    char * fPass; // contiene el password leido desde el archivo
    FILE * file; // contiene la struct que maneja el archivo
    FILE * fSend = NULL; // contiene la struct que maneja el archivo de envio
    char fileName[50]; // nombre del archivo solicitado;
    int fileSize; // tamaño del archivo solicitado;
    char bufferData[512]; // buffer utilizado para el envio de archivos
    char path[512]; // buffer utilizado para almacenar el path de un directorio
    char cmdSys[512]; // buffer utilizado para almacenar el path de un directorio y un comando

    sd = initSocket("127.0.0.1", argv[1], &server_address, longStruct);

    system("pwd > pwd.temp");
    // abro archivo de pwd.temp para agregar la ruta recibida
    FILE * fp = openFile("pwd.temp", "r");

    struct stat sb;
    if(stat("pwd.temp", &sb) == -1) {
        perror("stat");
    }
    // leo el archivo
    char* fileTemp = malloc(sb.st_size);
    fread(fileTemp, sb.st_size, 1, fp);

    printf("antes: %s\n", fileTemp);
    if ((strlen(fileTemp) != 0) && (fileTemp[strlen (fileTemp) - 1] == '\n')){
        fileTemp[strlen (fileTemp) - 1] = '\0';
    }
    memset(path, 0 , sizeof(path));
    sprintf(path, "%s", fileTemp);
    printf("path: %s\n", path);
    
    fclose(fp);
    free(fileTemp);

    while(1){
        printf("Esperando conexion de un cliente...\n");
        clientesd = acceptClient(sd, &client_address, &longStruct);
        
        // obtengo la ip y puerto del cliente
        inet_ntop(AF_INET,&(client_address.sin_addr), ipclient, INET_ADDRSTRLEN);
        portclient = ntohs(client_address.sin_port);
        printf("conexion establecida con IP: %s y puerto: %d\n", ipclient, portclient);
        /************************/

        respCmd(clientesd);
        printf("Conectado con un nuevo cliente\n");
        sendCmd(clientesd, DSC_OPEN, CMD_INIT);
        //printf("bufferOut: %s",bufferOut);
    
        while(1){
            
            memset(bufferIn, 0, sizeof(bufferIn));
            respCmd(clientesd);
            
            cmdRcv = extractCmd(bufferIn);
            printf("cmdRcv: %s\n", cmdRcv);
            if(strncmp( cmdRcv, OPR_QUIT, (sizeof(OPR_QUIT)-1) ) == 0){
                // Tratamiento comando QUIT
                memset(bufferOut, 0, sizeof(bufferOut));
                sprintf(bufferOut, "%s %s\r\n",CMD_OKQUIT, TXT_OKQUIT);
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
                    extract1Pmt(bufferIn, fUser); // extrae el parametro del buffer de entrada
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
                        extract1Pmt(bufferIn, pmtRcv); // extrae el parametro del buffer de entrada
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
                        if(strncmp( cmdRcv, OPR_RETR, (sizeof(OPR_RETR)-1) ) == 0){
                            // Tratamiento comando RETR
                            
                            extract1Pmt(bufferIn, pmtRcv); // extrae el parametro del buffer de entrada
                            if(existFile(pmtRcv)){
                                memset(fileName, 0, sizeof(fileName));
                                sprintf(fileName, "%s", pmtRcv); // almaceno el nombre del archivo
                                fileSize = sizeFile(pmtRcv);     // almaceno el tamaño del archivo
                                // si existe el archivo respondo al cliente 
                                // "299 File <nombreArchivo> size <tamaño> bytes\r\n"
                                memset(bufferOut, 0, sizeof(bufferOut));
                                sprintf(bufferOut, "%s %s %s %s %d %s\r\n", CMD_FILEE, TXT_FILEE1, fileName, TXT_FILEE2, fileSize, TXT_FILEE3);
                                write(clientesd,bufferOut,sizeof(bufferOut));
                                // asigno un valor a la flag para luego en port reconocer la accion.
                                flagPort = 1; // Ver si lo uso o no

                            }else{
                                // si no existe el archivo respondo al cliente
                                // "550 <nombreFichero>: no such file or directory\r\n"
                                memset(bufferOut, 0, sizeof(bufferOut));
                                sprintf(bufferOut, "%s %s%s\r\n", CMD_FILENE, pmtRcv, TXT_FILENE);
                                write(clientesd,bufferOut,sizeof(bufferOut));
                            }

                        }else{
                            if(strncmp( cmdRcv, OPR_PORT, (sizeof(OPR_PORT)-1) ) == 0){
                                printf("entre al PORT\n");
                                // tratamiento del comando PORT
                                memset(bufferOut, 0, sizeof(bufferOut));
                                // 200 Command okay
                                sprintf(bufferOut, "%s %s\r\n", CMD_CMMDOK, TXT_CMMDOK);
                                write(clientesd,bufferOut,sizeof(bufferOut));

                                // Ahora debo conectarme al cliente para pasarle el archivo
                                recIpPort(ipData, &portData, bufferIn);
                            
                                server_data.sin_family = AF_INET;            // Protocolo IPv4
                                server_data.sin_addr.s_addr = inet_addr(ipData); // IP
                                server_data.sin_port = htons(portData);    // Puerto

                                sdData = socket(AF_INET, SOCK_STREAM, 0); // En sd se almacena el socket descriptor
                                if(sdData < 0){
                                    perror("No se pudo crear el socket para data.\n");
                                    exit(ERR_CREATESOCK);
                                }
                                
                                socklen_t addrLSLenData = (sizeof(server_data));

                                // Realiza la conexion con el servidor, con los argumentos recibidos
                                if(connect(sdData, (struct sockaddr *)(&server_data), sizeof(server_data)) < 0){
                                    perror("Conexion fallida con el servidor data.\n");
                                    exit(ERR_CNNT_SERV);    
                                }

                                // Envio archivo
                                sendFile(sdData, fSend, fileName, fileSize);
                                // Preparo buffer de salida
                                memset(bufferOut, 0, sizeof(bufferOut));
                                sprintf(bufferOut, "%s %s\r\n", CMD_TRNSFOK, TXT_TRNSFOK);
                                if(write(clientesd,bufferOut,sizeof(bufferOut)) < 0){
                                        perror("No se pudo escribir.\n");
                                        exit(ERR_SENDSERV);
                                }

                            }else{
                                if(strncmp( cmdRcv, OPR_NLST, (sizeof(OPR_NLST)-1) ) == 0){
                                    printf("entre al NLST\n");
                                    // creo un archivo con lo que devuelve ls
                                    memset(cmdSys, 0 , sizeof(cmdSys));
                                    sprintf(cmdSys,"ls -l %s > ls.temp", path);
                                    system(cmdSys);
                                    
                                    memset(fileName, 0, sizeof(fileName));
                                    sprintf(fileName, "%s", "ls.temp"); // almaceno el nombre del archivo
                                    fileSize = sizeFile("ls.temp");     // almaceno el tamaño del archivo 
                                    // "150 File <nombreArchivo> size <tamaño> bytes\r\n"
                                    memset(bufferOut, 0, sizeof(bufferOut));
                                    //sprintf(bufferOut, "%s %s\r\n", CMD_FOK, TXT_FOK);
                                    sprintf(bufferOut, "%s %s %s %s %d %s\r\n", CMD_FILEE, TXT_FILEE1, fileName, TXT_FILEE2, fileSize, TXT_FILEE3);
                                    printf("bufferOut: %s\n", bufferOut);
                                    write(clientesd,bufferOut,sizeof(bufferOut));

                                }else{
                                    
                                    if(strncmp( cmdRcv, OPR_CWD, (sizeof(OPR_CWD)-1) ) == 0){
                                        // Extrae el path
                                        printf("Llegue al CWD\n");
                                        memset(pmtRcv, 0, sizeof(pmtRcv));
                                        printf("CWD pathant: %s\n", pmtRcv);
                                        extract1Pmt(bufferIn, pmtRcv); // extrae el parametro del buffer de entrada
                                        
                                        if(strncmp(pmtRcv, "..", 2) == 0){
                                            char * auxChar = strrchr(path, '/');
                                            char auxfileTemp[100];
                                            int auxN = auxChar - path;
                                            memset(auxfileTemp, 0 , sizeof(auxfileTemp));
                                            strncpy(auxfileTemp, path, auxN);
                                            memset(path, 0 , sizeof(path));
                                            sprintf(path, "%s", auxfileTemp);
                                        }else{
                                            
                                            sprintf(path, "%s/%s", path, pmtRcv);
                                        }
                            
                                        printf("path: %s\n", path);
                                        memset(cmdSys, 0 , sizeof(cmdSys));
                                        sprintf(cmdSys,"cd %s", path);
                                        printf("cmdSys: %s\n", cmdSys);

                                        if(system(cmdSys) == 0){
                                            // respuesta OK
                                            printf("system(cmdSys) == 0\n");
                                            memset(bufferOut, 0, sizeof(bufferOut));
                                            sprintf(bufferOut, "%s %s %s\r\n", CMD_CWDOK, OPR_CWD, TXT_CDMNDOK);
                                            write(clientesd,bufferOut,sizeof(bufferOut));
                                        }else{
                                            // respuesta ERROR 
                                            
                                        }
                                        
                                    }else{
                                        if(strncmp( cmdRcv, OPR_MKD, (sizeof(OPR_MKD)-1) ) == 0){
                                            //aca mkdir
                                            // 257 "nombredeldirectorio" "se creo correctamente"
                                            printf("Entre a MKD\n");
                                            memset(pmtRcv, 0, sizeof(pmtRcv));
                                            extract1Pmt(bufferIn, pmtRcv); // extrae el parametro del buffer de entrada

                                            char auxfileTemp[100];
                                            memset(auxfileTemp, 0 , sizeof(auxfileTemp));
                                            memset(cmdSys, 0, sizeof(cmdSys));
                                            sprintf(auxfileTemp, "%s/%s", path, pmtRcv);
                                            sprintf(cmdSys,"mkdir %s", auxfileTemp);
                                            printf("cmdSys: %s\n", cmdSys);

                                            if(system(cmdSys) == 0){
                                                // respuesta OK
                                                printf("system(cmdSys) == 0\n");
                                                memset(bufferOut, 0, sizeof(bufferOut));
                                                sprintf(bufferOut, "%s %s %s\r\n", CMD_MKDOK, pmtRcv, TXT_MKDOK);
                                                write(clientesd,bufferOut,sizeof(bufferOut));
                                            }else{
                                                // respuesta ERROR
                                                //memset(bufferOut, 0, sizeof(bufferOut));
                                                //sprintf(bufferOut, "%s %s %s\r\n", CMD_MKDOK, pmtRcv, TXT_MKDOK);
                                                //write(clientesd,bufferOut,sizeof(bufferOut));
                                                
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
                        
                    }
                    
                }
                
            }        
        }
    }
    
    close(sd);

    return END_OK;
}

// envia un archivo
void sendFile(int sdd, FILE * fs, char * fname, int fsize){
    fs = openFile(fname,"rb"); // abro el archivo a transferir
    if(fs == NULL){
        perror("Error lectura archivo\n");
        exit(ERR_OPNFILE);
    }
    int cSize = fsize;
    memset(bufferData, 0, sizeof(bufferData));

    while(cSize>=0){
        if(cSize <=512){
            while(fread(bufferData,1,cSize,fs)!=NULL){
                if (write(sdd,bufferData,cSize)<0){
                    perror("Error al transferir el archivo\n");
                    exit(ERR_SENDSERV);
                }
                                                
                memset(bufferData,0,sizeof(bufferData));
            }
            cSize=-1;
        }else{   
            while(fread(bufferData,1,512,fs)!=NULL){
                if (write(sdd,bufferData,sizeof(bufferData))<0){
                    perror("Error al transferir el archivo\n");
                    exit(ERR_SENDSERV);
                }                            
                memset(bufferData, 0, sizeof(bufferData));
            }
            cSize = cSize - 512;
            if(cSize < 0){
                cSize = 512 + cSize;
            }
        }
    }
    closeFile(fs);
}

// reconstruye la ip y el puerto recibido en el comando PORT
void recIpPort(char * ip, int * port, char * str){
    // ejemplo: str = PORT 127,0,0,1,219,23
    char * aux;
    char * auxA;
    char ip1Aux[4];
    char ip2Aux[4];
    char ip3Aux[4];
    char ip4Aux[4];
    char sPortH[10];
    char sPortL[10];
    memset(ip1Aux,0,sizeof(ip1Aux));
    memset(ip2Aux,0,sizeof(ip2Aux));
    memset(ip3Aux,0,sizeof(ip3Aux));
    memset(ip4Aux,0,sizeof(ip4Aux));
    memset(sPortH,0,sizeof(sPortH));
    memset(sPortL,0,sizeof(sPortL));
    int portH;
    int portL;
    int n;

    aux = strchr(str,' '); //llego hasta "PORT "
    n = aux - str; 
    aux += 1; // estoy aca "127,0,0,1,219,23"
    auxA = aux;
    aux = strchr(auxA,',');
    n = aux - auxA;
    aux += 1; // estoy aca "0,0,1,219,23"
    strncpy(ip1Aux, auxA, n); // 1 octeto

    auxA = aux;
    aux = strchr(auxA,',');
    n = aux - auxA;
    aux += 1; // estoy aca "0,1,219,23"
    strncpy(ip2Aux, auxA, n); // 2 octeto

    auxA = aux;
    aux = strchr(auxA,',');
    n = aux - auxA;
    aux += 1; // estoy aca "1,219,23"
    strncpy(ip3Aux, auxA, n); // 3 octeto

    auxA = aux;
    aux = strchr(auxA,',');
    n = aux - auxA;
    aux += 1; // estoy aca "219,23"
    strncpy(ip4Aux, auxA, n); // 4 octeto

    auxA = aux;
    aux = strchr(auxA,',');
    n = aux - auxA;
    aux += 1; // estoy aca "23"
    strncpy(sPortH, auxA, n); // sPortH parte alta

    portH = atoi(sPortH);
    portH = portH << 8;
    auxA = aux;
    aux = strchr(auxA,'\r');
    n = aux - auxA;
    strncpy(sPortL, auxA, n); // sPortL parte baja
    portL = atoi(sPortL);
    *port = portH + portL;
    sprintf(ip,"%s.%s.%s.%s", ip1Aux, ip2Aux, ip3Aux, ip4Aux);

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
    memset(d, 0, 5);
    
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
void extract1Pmt(char * s, char * p){
    // se libera la memoria luego en el flujo principal
    char * d = malloc(100); // reservo 100 bytes para paremetros
    char * r;
    int end;
    memset(d, 0, 100);

    r = strchr(s,' ');
    r+=1;
    end = strchr(r,'\r') - r;
    printf("r: %s\n",r);
    printf("end: %d\n",end);
    printf("d: %s\n",d);
    strncpy(d,r,end);
    printf("dDes: %s\n",d);
    
    
    if(d == NULL){
        //*p = 0;
    }else{
        strncpy(p, d, strlen(d));
    }
}


// abre el archivo, y retorna FILE *
FILE * openFile(char * path, char * type){
    return fopen (path, type);
}

// cierra el archivo recibido como parametro
void closeFile(FILE * file){
    fclose(file);
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

// verifica la existencia de un archivo.
// 1=> Existe
// 0=> No Existe
int existFile(char * path){
    
    FILE * file = openFile(path, "r");
    
    if (openFile(path, "r") == NULL){
        return 0;
    }else{
        closeFile(file);
        return 1;
    }
    
}

// retorna el tamaño en bytes del archivo recibido por parametro
int sizeFile(char * path){

    struct stat stFile;

    if (stat(path, &stFile) == -1) {
        perror("No se pudo leer el tamaño del archivo\n");
        exit(ERR_STATFILE);
    }

    return stFile.st_size; // retorna el tamaño en bytes del archivo
}
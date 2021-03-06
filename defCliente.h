#define CMD_INIT   "220"
#define OP_FOK      150 // Listo para recibir el contenido del directorio actual
#define OP_CWDOK    250 // Comando CWD correcto
#define OP_RPASS    331 // Usuario requiere password.
#define OP_LOGIN    230 // Usuario logueado correctamente.
#define OP_LOGOUT   530 // Usuario No logueado correctamente.
#define OP_FILEE    299 // archivo solicitado existe en el servidor
#define OP_CMMDOK   200 // Comando ok
#define OP_MKDOK    257 // MKD Ok
#define OP_RMDOK    200 
#define OP_FILENE   550 // archivo solicitado no existe en el servidor
#define DSC_OPEN "clientFTP"
#define VERSION     "0.0"
#define CMD_QUIT    "QUIT"
#define CMD_QUSR    "quit"
#define CMD_GET     "get"
#define CMD_DIR     "dir"
#define CMD_CD      "cd"
#define CMD_CWD     "CWD"
#define CMD_MKDIR   "mkdir"
#define CMD_MKD     "MKD"
#define CMD_RMDIR   "rmdir"
#define CMD_RMD     "RMD"
#define CMD_NLST    "NLST"
#define CMD_RETR    "RETR"
#define CMD_PORT    "PORT"
#define CMD_USER    "USER"
#define CMD_PASS    "PASS"
#define CMD_OKQUIT "221"

#define END_OK          0
#define ERR_CREATESOCK  1
#define ERR_CNNT_SERV   2
#define ERR_SENDCLIENT  3
#define ERR_RECVCLIENT  4
#define ERR_BIND        5
#define ERR_LISTEN      6
#define ERR_OPNFILE     18
#define ERR_ARGS        20

int initSocket(char * ip, char * port, struct sockaddr_in * add);
int initSocketData(struct sockaddr_in * add, socklen_t l);
int acceptClientData(int s, struct sockaddr_in * add, socklen_t * l);
void connectServer(int sockd, struct sockaddr_in * add);
void sendCmd(int sockt, char * cmd, char * dsc);
void respCmd(int sockd);
void respData(int sockd, FILE * f, int fSize);
unsigned int codeRecv(char * c);
int clientAuntheticate(int s);
void extCmdParam(char * buffer, char * c, char * p, int sizec, int sizep);
void formatIpPort(char * strOut, char * ip, int port);
void extSize(int * fileSize, char * buff);
FILE * openFile(char * path, char * type);
void closeFile(FILE * file);
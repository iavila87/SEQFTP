#define CMD_INIT "220"
#define OP_RPASS    331 // Usuario requiere password.
#define OP_LOGIN    230 // Usuario logueado correctamente.
#define OP_LOGOUT   530 // Usuario No logueado correctamente.
#define OP_FILEE    299 // archivo solicitado existe en el servidor
#define OP_FILENE   550 // archivo solicitado no existe en el servidor
#define DSC_OPEN "clientFTP"
#define VERSION "0.0"
#define CMD_QUIT "QUIT"
#define CMD_GET "get"
#define CMD_RETR "RETR"
#define CMD_USER "USER"
#define CMD_PASS "PASS"
#define CMD_OKQUIT "OKQUIT"

#define END_OK          0
#define ERR_CREATESOCK  1
#define ERR_CNNT_SERV   2
#define ERR_SENDCLIENT  3
#define ERR_RECVCLIENT  4
#define ERR_ARGS        20

int initSocket(char * ip, char * port, struct sockaddr_in * add);
void connectServer(int sockd, struct sockaddr_in * add);
void sendCmd(int sockt, char * cmd, char * dsc);
void respCmd(int sockd);
unsigned int codeRecv(char * c);
int clientAuntheticate(int s);
void extCmdParam(char * buffer, char * c, char * p);
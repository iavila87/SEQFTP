#define CMD_INIT        "220"
#define CMD_UPASS       "331"
#define CMD_LOGIN       "230"
#define CMD_LOGERROR    "530"
#define DSC_NAME        "serverFTP"
#define VERSION         "0.0"
#define OPR_QUIT        "QUIT"
#define OPR_USER        "USER"
#define OPR_PASS        "PASS"
#define DSC_OPEN        "srvFTP"
#define CMD_OKQUIT      "OKQUIT"
#define TXT_PASSREQ     "Password required for"
#define TXT_LOGERROR    "Login incorrect"
#define TXT_LOGIN1      "User"
#define TXT_LOGIN2      "logged in"

#define END_OK          0
#define ERR_CREATESOCK  1
#define ERR_BIND        5
#define ERR_LISTEN      6
#define ERR_SENDSERV    7
#define ERR_RECVSERV    8
#define ERR_ARGS        20

int initSocket(char * ip, char * port, struct sockaddr_in * add, socklen_t l);
int acceptClient(int s, struct sockaddr_in * add, socklen_t * l);
void respCmd(int sockd);
void sendCmd(int sockd, char * cmd, char * dsc);
char * extractCmd(char * s);
char * extract1Pmt(char * s);

// funciones para el tratamiento de archivos
FILE * openFile(char * path, char * type);
char * searchUserFile(FILE * f, char * s);


#define CMD_INIT        "220"
#define CMD_CMMDOK      "200"
#define CMD_TRNSFOK     "226"
#define CMD_UPASS       "331"
#define CMD_LOGIN       "230"
#define CMD_FILEE       "299"
#define CMD_FILENE      "550"
#define CMD_LOGERROR    "530"
#define DSC_NAME        "serverFTP"
#define VERSION         "0.0"
#define OPR_QUIT        "QUIT"
#define OPR_USER        "USER"
#define OPR_PASS        "PASS"
#define OPR_RETR        "RETR"
#define OPR_PORT        "PORT"
#define DSC_OPEN        "srvFTP"
#define CMD_OKQUIT      "221"
#define TXT_OKQUIT      "Goodbye"
#define TXT_CMMDOK      "Command okay"
#define TXT_TRNSFOK     "Transfer complete"
#define TXT_PASSREQ     "Password required for"
#define TXT_LOGERROR    "Login incorrect"
#define TXT_LOGIN1      "User"
#define TXT_LOGIN2      "logged in"
#define TXT_FILEE1      "File"
#define TXT_FILEE2      "size"
#define TXT_FILEE3      "bytes"
#define TXT_FILENE      ": no such file or directory"
#define END_OK          0
#define ERR_CREATESOCK  1
#define ERR_CNNT_SERV   2
#define ERR_BIND        5
#define ERR_LISTEN      6
#define ERR_SENDSERV    7
#define ERR_RECVSERV    8
#define ERR_OPNFILE     18
#define ERR_STATFILE    19 
#define ERR_ARGS        20

int initSocket(char * ip, char * port, struct sockaddr_in * add, socklen_t l);
int acceptClient(int s, struct sockaddr_in * add, socklen_t * l);
void respCmd(int sockd);
void sendCmd(int sockd, char * cmd, char * dsc);
char * extractCmd(char * s);
char * extract1Pmt(char * s);
void recIpPort(char * ip, int * port, char * str);

// funciones para el tratamiento de archivos
FILE * openFile(char * path, char * type);
char * searchUserFile(FILE * f, char * s);
void closeFile(FILE * file);
int existFile(char * path);
int sizeFile(char * path);



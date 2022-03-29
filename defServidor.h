#define CMD_INIT "220"
#define DSC_NAME "serverFTP"
#define VERSION "0.0"
#define OPR_QUIT "QUIT"
#define DSC_OPEN "srvFTP"
#define CMD_OKQUIT "OKQUIT"
#define ERR_ARGS 5

int initSocket(char * ip, char * port, struct sockaddr_in * add, socklen_t l);
int acceptClient(int s, struct sockaddr_in * add, socklen_t * l);
void respCmd(int sockd);
void sendCmd(int sockd, char * cmd, char * dsc);


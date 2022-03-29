#define CMD_INIT "220"
#define DSC_OPEN "clientFTP"
#define VERSION "0.0"
#define CMD_QUIT "QUIT"
#define CMD_OKQUIT "OKQUIT"

#define ERR_ARGS 5

int initSocket(char * ip, char * port, struct sockaddr_in * add);
void connectServer(int sockd, struct sockaddr_in * add);
void sendCmd(int sockt, char * cmd, char * dsc);
void respCmd(int sockd);

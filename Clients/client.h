#ifndef __CLIENT_H
#define __CLIENT_H

int joinSS(struct ssDetails ss);
int sendRequest(char *input, int sockfd);
int joinNamingServerAsClient(char *ip, int port);
void *isNMConnected(void *args);

#endif

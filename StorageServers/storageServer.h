#ifndef __STORAGESERVER_H
#define __STORAGESERVER_H

void sendPathToNS(char *path, char perms[11], size_t size, int nmSock, time_t lastModifiedTime, time_t lastAccessTime);
void *takeInputsDynamically(void *args);
void *handleBackupCommand(void *args);
void *serveNM_Requests(void *args);
void *serveClient_Request(void *args);
void *acceptClients(void *args);
int initializeNMConnection(char *ip, int port, int nmPort, int cliPort);
int initializeNMConnectionForRecords(char *ip, int port);
int initialzeClientsConnection(int port);

#endif
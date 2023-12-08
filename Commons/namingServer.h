#ifndef __NAMINGSERVER_H
#define __NAMINGSERVER_H

struct record *getRecord(char *path);
void addToRecords(struct record *r);
void removeFromRecords(char *path);
void makeAccessibleAferCopy(struct record *r);
int backupCopy(struct record *curr_rec, char *main_path, char *mdir, char *base_dir, struct ssDetails *ss, struct ssDetails *ss_read);
int copyLocally(struct record *curr_rec, char *main_path, char *mdir, char *base_dir, struct ssDetails *ss, struct ssDetails *ss_read);
int backupRemove(struct record *curr_rec, struct ssDetails *ss);
void logMessage(const char *message, const char *ip, int port);
void *acceptClientRequests(void *args);
void *addPaths(void *args);
void addClient(int connfd, char *hostIP, int hostPort);
void createBackup(struct ssDetails *src, struct ssDetails *dest);
void addStorageServer(int connfd, char *hostIP, int hostPort);
void *acceptHost(void *args);
int initializeNamingServer(int port);

#endif
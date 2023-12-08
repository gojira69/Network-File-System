#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>
#include <sys/ioctl.h>


#define SA struct sockaddr
#define MAX_SIZE 1024
#define maxlen 1000

struct cDetails
{
    int id;
    int connfd;
    char ip[16];
    int port;
};

struct ssDetails
{
    int id;
    char ip[16];
    int nmPort;
    int cliPort;
    int connfd;
    int addPathfd;
    bool inBackup;
    struct ssDetails *backup1;
    struct ssDetails *backup2;
};

struct record
{
    char *path;
    struct ssDetails *originalSS;
    char originalPerms[11];
    bool isDir;
    char currentPerms[11]; // for backup
    size_t size;

    // for n-ary tree
    struct record *firstChild;
    struct record *nextSibling;
    struct record *parent;
    struct record *prevSibling;
    pthread_mutex_t record_lock;

    bool isValid; // only used in copy to prevent recursive dir copying
};

struct fileDetails
{
    char path[4096];
    char fileName[4096];
    char perms[11];
    size_t size;
    bool isDir;
    time_t lastAccessTime;
    time_t lastModifiedTime;
};

#include "api.h"
#include "cache.h"
#include "client.h"
#include "colors.h"
#include "errors.h"
#include "namingServer.h"
#include "search.h"
#include "storageServer.h"


#endif

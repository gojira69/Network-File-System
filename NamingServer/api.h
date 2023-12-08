#ifndef _API_H_
#define _API_H_

int isDirectory(const char *path);
int check_path_exists(const char *directoryPath);
void parse_input(char *array[], char *inputS);
mode_t reversePermissions(char *perms);
void convertPermissions(mode_t mode, char *str);
int removeFile(char *path);
int removeDirectory(char *path);
int makeDirectory(char *path, char* perms);
int makeFile(char *path, char* perms);
int writeFile(char *path, char *editor);
int readFile(char *path, char *editor);
int sendFile(char *path, int sockfd);
int receiveFile(char *path, int sockfd);
int sendFileCopy(char *path, int sockfd, bool finalAck);
int receiveFileCopy(char *path, int sockfd, char* perms, bool finalAck);
void removePrefix(char *str, const char *prefix);
void concatenateStrings(char *result, const char *A, const char *B, const char *C, const char *D);

#endif
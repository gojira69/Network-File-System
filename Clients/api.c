#include "headers.h"

int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int check_path_exists(const char *directoryPath)
{
    struct stat dirStat;
    if (stat(directoryPath, &dirStat) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void parse_input(char *array[], char *inputS)
{
    for (int i = 0; i < 3; i++)
    {
        array[i] = NULL;
    }

    char *inputString = strdup(inputS);
    char *token = strtok(inputString, " \t\n");
    int count = 0;

    while (token != NULL)
    {
        if (count >= 3)
        {
            break;
        }
        array[count] = malloc(sizeof(char) * 100);
        strcpy(array[count], token);
        count++;
        token = strtok(NULL, " \t\n");
    }
}

mode_t reversePermissions(char *perms)
{
    mode_t mode = 0;

    // Check if the file is a directory
    mode |= (perms[0] == 'd') ? S_IFDIR : 0;

    // Owner permissions
    mode |= (perms[1] == 'r') ? S_IRUSR : 0;
    mode |= (perms[2] == 'w') ? S_IWUSR : 0;
    mode |= (perms[3] == 'x') ? S_IXUSR : 0;

    // Group permissions
    mode |= (perms[4] == 'r') ? S_IRGRP : 0;
    mode |= (perms[5] == 'w') ? S_IWGRP : 0;
    mode |= (perms[6] == 'x') ? S_IXGRP : 0;

    // Others permissions
    mode |= (perms[7] == 'r') ? S_IROTH : 0;
    mode |= (perms[8] == 'w') ? S_IWOTH : 0;
    mode |= (perms[9] == 'x') ? S_IXOTH : 0;

    return mode;
}

void convertPermissions(mode_t mode, char *str)
{
    str[0] = S_ISDIR(mode) ? 'd' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

int removeFile(char *path)
{
    if (!check_path_exists(path))
    {
        return -1; // file_not_found
    }
    else
    {
        if (isDirectory(path))
        {
            return -2; // not_file
        }
        else
        {
            int ret = remove(path);
            if (ret == -1)
            {
                return -3; // remove system call
            }
        }
    }
    return 0;
}

int removeDirectory(char *path)
{
    if (!check_path_exists(path))
    {
        return -1; // dir_not_found
    }
    else
    {
        if (!isDirectory(path))
        {
            return -2; // not_dir
        }
        else
        {
            int ret = rmdir(path);
            if (ret == -1)
            {
                return -3; // rmdir system call
            }
        }
    }
    return 0;
}

int makeDirectory(char *path, char *perms)
{
    mode_t mode = reversePermissions(perms);
    char *baseDir = dirname(strdup(path));
    int retval = 0;
    if (check_path_exists(path))
    {
        retval = 0; // just need to make it accessible
    }
    else
    {
        if (!check_path_exists(baseDir))
        {
            retval = -1; // dir_not_found
        }
        else
        {
            if (!isDirectory(baseDir))
            {
                retval = -2; // not_dir
            }
            else
            {
                int ret = mkdir(path, mode);
                if (ret == -1)
                {
                    retval = -3; // mkdir syscall
                }
            }
        }
    }
    return retval;
}

int makeFile(char *path, char *perms)
{
    mode_t mode = reversePermissions(perms);
    char *baseDir = dirname(strdup(path));
    int retval = 0;
    if (check_path_exists(path))
    {
        retval = 0; // just need to make it accessible
    }
    else
    {
        if (!check_path_exists(baseDir))
        {
            retval = -1; // file_not_found
        }
        else
        {
            if (!isDirectory(baseDir))
            {
                retval = -1; // not_file
            }
            else
            {
                int ret = creat(path, mode);
                if (ret == -1)
                {
                    retval = -1; // creat syscall
                }
            }
        }
    }
    return retval;
}

int writeFile(char *path, char *editor)
{
    if (!check_path_exists(path))
    {
        handleFileOperationError("file_not_found");
        return -1;
    }
    else
    {
        if (!isDirectory(path))
        {
            int fd = open(path, O_WRONLY | O_APPEND);
            pid_t pid = fork();
            if (pid == 0) // Child process
            {
                char *editor_args[] = {editor, path, NULL};
                if (execvp(editor, editor_args) == -1)
                {
                    handleSYSErrors("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else // Parent process
            {
                int status;
                waitpid(pid, &status, 0);
            }
        }
        else
        {
            handleFileOperationError("not_file");
            return -2;
        }
    }
    return 0;
}

int readFile(char *path, char *editor)
{
    if (!check_path_exists(path))
    {
        handleFileOperationError("file_not_found");
        return -1;
    }
    else
    {
        if (!isDirectory(path))
        {
            int fd = open(path, O_RDONLY);
            pid_t pid = fork();
            if (pid == 0) // Child process
            {
                char *editor_args[] = {editor, path, NULL};
                if (execvp(editor, editor_args) == -1)
                {
                    handleSYSErrors("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else // Parent process
            {
                int status;
                waitpid(pid, &status, 0);
            }
        }
        else
        {
            handleFileOperationError("not_file");
            return -1;
        }
    }
    return 0;
}

int sendFile(char *path, int sockfd)
{
    if (!check_path_exists(path))
    {
        handleFileOperationError("file_not_found");
        return -1;
    }
    if (isDirectory(path))
    {
        handleFileOperationError("not_file");
        return -2;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        handleSYSErrors("open");
        return -3;
    }

    char buf[MAX_SIZE];
    ssize_t bytes_read = read(fd, buf, MAX_SIZE);

    while (bytes_read > 0)
    {
        ssize_t resp = send(sockfd, buf, bytes_read, 0);
        if (resp == -1)
        {
            handleNetworkErrors("send");
            return -4;
        }

        // Wait for acknowledgment from the client
        char recACK[MAX_SIZE];
        ssize_t ack_resp = recv(sockfd, recACK, sizeof(recACK), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("recv");
            return -5;
        }
        bzero(buf, MAX_SIZE);
        bytes_read = read(fd, buf, MAX_SIZE);
    }

    if (bytes_read == -1)
    {
        handleSYSErrors("read");
        return -6;
    }
    close(fd);

    // Send "STOP" signal
    char stopSignal[MAX_SIZE];
    strcpy(stopSignal, "STOP");
    ssize_t stop_resp = write(sockfd, stopSignal, strlen(stopSignal));

    if (stop_resp == -1)
    {
        handleSYSErrors("write");
        return -7;
    }
    return 0; // Success
}

int receiveFile(char *path, int sockfd)
{
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
    {
        handleSYSErrors("open");
        return -3;
    }

    char buf[MAX_SIZE];
    ssize_t bytes_read = recv(sockfd, buf, MAX_SIZE, 0);

    while (bytes_read > 0)
    {
        if (strstr(buf, "STOP"))
        {
            break;
        }

        ssize_t resp = write(fd, buf, bytes_read);

        // Send acknowledgment to the server
        char sendACK[MAX_SIZE] = "ACK";
        ssize_t ack_resp = send(sockfd, sendACK, sizeof(sendACK), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("send");
            return -4;
        }

        if (resp == -1 || ack_resp == -1)
        {
            handleSYSErrors("write");
            return -7;
        }

        bzero(buf, MAX_SIZE);
        bytes_read = recv(sockfd, buf, MAX_SIZE, 0);
        if (bytes_read == -1)
        {
            handleNetworkErrors("recv");
            return -5;
        }
    }

    if (bytes_read == -1)
    {
        handleNetworkErrors("recv");
        return -5;
    }
    close(fd);
    return 0; // Success
}

int sendFileCopy(char *path, int sockfd, bool finalAck)
{
    if (!check_path_exists(path))
    {
        handleFileOperationError("file_not_found");
        return -1;
    }
    if (isDirectory(path))
    {
        handleFileOperationError("not_file");
        return -2;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        handleSYSErrors("open");
        return -3;
    }

    char buf[MAX_SIZE];
    ssize_t bytes_read = read(fd, buf, MAX_SIZE);

    while (bytes_read > 0)
    {
        ssize_t resp = send(sockfd, buf, bytes_read, 0);
        if (resp == -1)
        {
            handleNetworkErrors("send");
            return -4;
        }

        // Wait for acknowledgment from the client
        char recACK[MAX_SIZE];
        ssize_t ack_resp = recv(sockfd, recACK, sizeof(recACK), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("recv");
            return -5;
        }

        bzero(buf, MAX_SIZE);
        bytes_read = read(fd, buf, MAX_SIZE);
    }

    if (bytes_read == -1)
    {
        handleSYSErrors("read");
        return -6;
    }

    close(fd);

    // Send "STOP" signal
    char stopSignal[MAX_SIZE];
    strcpy(stopSignal, "STOP");
    ssize_t stop_resp = write(sockfd, stopSignal, strlen(stopSignal));

    if (stop_resp == -1)
    {
        handleSYSErrors("write");
        return -7;
    }

    if (finalAck)
    {
        // get final ack
        ssize_t ack_resp = recv(sockfd, stopSignal, sizeof(stopSignal), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("recv");
            return -5;
        }
    }

    return 0; // Success
}

int receiveFileCopy(char *path, int sockfd, char *perms, bool finalAck)
{
    mode_t mode = reversePermissions(perms);
    int fd = open(path, O_WRONLY | O_CREAT, mode);
    if (fd == -1)
    {
        handleSYSErrors("open");
        return -3;
    }

    char buf[MAX_SIZE];
    ssize_t bytes_read = recv(sockfd, buf, MAX_SIZE, 0);

    while (bytes_read > 0)
    {
        if (strstr(buf, "STOP"))
        {
            break;
        }

        ssize_t resp = write(fd, buf, bytes_read);

        // Send acknowledgment to the server
        char sendACK[MAX_SIZE] = "ACK";
        ssize_t ack_resp = send(sockfd, sendACK, sizeof(sendACK), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("send");
            return -4;
        }

        if (resp == -1 || ack_resp == -1)
        {
            handleSYSErrors("write");
            return -7;
        }

        bzero(buf, MAX_SIZE);
        bytes_read = recv(sockfd, buf, MAX_SIZE, 0);
        if (bytes_read == -1)
        {
            handleNetworkErrors("recv");
            return -5;
        }
    }

    if (bytes_read == -1)
    {
        handleNetworkErrors("recv");
        return -5;
    }

    if (finalAck)
    {
        // send final ack
        ssize_t ack_resp = send(sockfd, "OVER", sizeof("OVER"), 0);
        if (ack_resp == -1)
        {
            handleNetworkErrors("send");
            return -4;
        }
    }

    close(fd);
    return 0; // Success
}

void removePrefix(char *str, const char *prefix)
{
    size_t prefixLen = strlen(prefix);
    size_t strLen = strlen(str);

    if (strLen >= prefixLen && strncmp(str, prefix, prefixLen) == 0)
    {
        memmove(str, str + prefixLen, strLen - prefixLen + 1); // +1 to include the null terminator
        if (str[0] == '/')
        {
            memmove(str, str + 1, strLen - prefixLen); // Remove the leading '/'
        }
    }
}

void concatenateStrings(char *result, const char *A, const char *B, const char *C, const char *D)
{
    strcpy(result, A);
    strcat(result, " ");
    strcat(result, B);
    strcat(result, "/");
    strcat(result, C);
    strcat(result, " ");
    strcat(result, D);
}

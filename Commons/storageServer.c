#include "headers.h"
// there should be an inifinite thread in the NS side that also checks and updates the paths
int nmSock1;

void sendPathToNS(char *path, char perms[11], size_t size, int nmSock, time_t lastModifiedTime, time_t lastAccessTime)
{
	struct fileDetails *det = (struct fileDetails *)malloc(sizeof(struct fileDetails));
	strcpy(det->path, path);
	strcpy(det->perms, perms);
	det->size = size;
	det->isDir = perms[0] == '-' ? false : true;
	det->lastAccessTime = lastAccessTime;
	det->lastModifiedTime = lastModifiedTime;
	strcpy(det->fileName, basename(path));

	int bytesSent = send(nmSock, det, sizeof(struct fileDetails), 0);
	if (bytesSent == -1)
	{
		handleNetworkErrors("send");
	}
	char status[100];
	bzero(status, sizeof(status));
	int bytesRecv = recv(nmSock, status, sizeof(status), 0);
	if (bytesRecv == -1)
	{
		handleNetworkErrors("recv");
	}
	if (strcmp(status, "ADDED") == 0 && strcmp(path, "backups") != 0)
	{
		printf(BLUE_COLOR "Added %s as an accessible path\n" RESET_COLOR, path);
	}
}

void *takeInputsDynamically(void *args)
{
	int nmSock = *(int *)(args);
	while (1)
	{
		// char *path;
		// path = malloc(sizeof(char) * 4096);
		// scanf("%s", path);
		char input[4096];
		fgets(input, 4096, stdin);
		char *token = strtok(input, " \t\n");
		if (strcmp(token, "ADD"))
		{
			printf(RED_COLOR "Invalid Syntax!\n" RESET_COLOR);
			continue;
		}
		char *path = strtok(NULL, " \n");
		if (check_path_exists(path))
		{
			char *finalPath = strdup(path);
			char *token = strtok(finalPath, "/");
			char *currPath = strdup("");

			while (token != NULL)
			{
				if (strlen(currPath) != 0)
				{
					strcat(currPath, "/");
				}
				strcat(currPath, token);

				struct stat dirStat;
				int r = stat(currPath, &dirStat);
				if (r == -1)
				{
					handleSYSErrors("stat");
					exit(1);
				}
				char perms[11];
				convertPermissions(dirStat.st_mode, perms);
				size_t size = dirStat.st_size;
				sendPathToNS(currPath, perms, size, nmSock, dirStat.st_mtime, dirStat.st_atime);

				token = strtok(NULL, "/");
			}

			free(finalPath);
			free(currPath);
		}
		else
		{
			printf(RED "Invalid path!\n" RESET_COLOR);
		}
	}
	return NULL;
}

void *handleBackupCommand(void *args)
{
	char *input = args;
	char *request_command = strtok(input, " \t\n");
	char *path = strtok(NULL, " \t\n");

	int status = -1;
	char statusMsg[4096];
	bzero(statusMsg, sizeof(statusMsg));

	if (strcmp(request_command, "BACKUP_MKFILE") == 0)
	{
		char *perms = strtok(NULL, " \t\n");
		if (perms == NULL)
		{
			perms = malloc(sizeof(char) * 1024);
			strcpy(perms, "-rw-r--r--");
		}

		status = makeFile(path, perms);
		send(nmSock1, "SUCCESS", strlen("SUCCESS"), 0);
		// if (status == 0)
		// {
		// 	printf(GRAY_COLOR "Created backup file %s" RESET_COLOR, path);
		// 	printf(WHITE_COLOR "\n" RESET_COLOR);
		// }
	}
	else if (strcmp(request_command, "BACKUP_MKDIR") == 0)
	{
		char *perms = strtok(NULL, " \t\n");
		if (perms == NULL)
		{
			perms = malloc(sizeof(char) * 1024);
			strcpy(perms, "drwxr-xr-x");
		}
		status = makeDirectory(path, perms);
		send(nmSock1, "SUCCESS", strlen("SUCCESS"), 0);
		if (status == 0)
		{
			printf(GRAY_COLOR "Created backup directory %s" RESET_COLOR, path);
			printf(WHITE_COLOR "\n" RESET_COLOR);
		}
	}
	else if (strcmp(request_command, "BACKUP_WRITEFILE") == 0)
	{
		status = sendFileCopy(path, nmSock1, true);
		send(nmSock1, "SUCCESS", strlen("SUCCESS"), 0);
	}
	else if (strcmp(request_command, "BACKUP_READFILE") == 0)
	{
		char *perms = strtok(NULL, " \t\n");
		if (perms == NULL)
		{
			perms = malloc(sizeof(char) * 1024);
			strcpy(perms, "-rw-r--r--");
		}
		status = receiveFileCopy(path, nmSock1, perms, false);
		send(nmSock1, "SUCCESS", strlen("SUCCESS"), 0);
		if (status == 0)
		{
			printf(GRAY_COLOR "Updated backup file %s" RESET_COLOR, path);
			printf(WHITE_COLOR "\n" RESET_COLOR);
		}
	}

	return NULL;
}

void *serveNM_Requests(void *args)
{
	int nmfd = *((int *)args);
	while (1)
	{

		char buffer[4096];
		bzero(buffer, 4096);
		int bytesRecv = recv(nmfd, buffer, sizeof(buffer), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			// perror("recv");
			break;
		}
		if (bytesRecv == 0)
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!" RESET_COLOR);
			printf(WHITE_COLOR "\n" RESET_COLOR);

			exit(0);
			break;
		}

		if (strncmp(buffer, "BACKUP", 6) != 0)
		{
			printf(CYAN_COLOR "Recieved command from NM: %s" RESET_COLOR, buffer);
			printf(WHITE_COLOR "\n" RESET_COLOR);
		}

		char *bufferCopy = strdup(buffer);
		char *request_command = strtok(buffer, " \t\n");
		char *path = strtok(NULL, " \t\n");

		int status = -1;
		char statusMsg[4096];
		bzero(statusMsg, sizeof(statusMsg));

		if (strncmp(request_command, "BACKUP", 6) == 0)
		{
			pthread_t backupCommandThread;
			pthread_create(&backupCommandThread, NULL, handleBackupCommand, (void *)(bufferCopy));
			pthread_join(backupCommandThread, NULL);
			continue;
		}
		else if (strcmp(request_command, "RMFILE") == 0)
		{
			// RMFILE path
			status = removeFile(path);
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("file_not_found");
				strcpy(statusMsg, "file_not_found");
				break;
			case -2:
				handleFileOperationError("not_file");
				strcpy(statusMsg, "not_file");
				break;
			case -3:
				handleSYSErrors("remove");
				strcpy(statusMsg, "remove");
				break;
			default:
				break;
			}
		}
		else if (strcmp(request_command, "MKDIR") == 0)
		{
			// MKDIR path
			char *perms = strtok(NULL, " \t\n");
			if (perms == NULL)
			{
				perms = malloc(sizeof(char) * 1024);
				strcpy(perms, "drwxr-xr-x");
			}
			// printf("perms: %s\n", perms);
			status = makeDirectory(path, perms);
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("dir_not_found");
				strcpy(statusMsg, "dir_not_found");
				break;
			case -2:
				handleFileOperationError("not_dir");
				strcpy(statusMsg, "not_dir");
				break;
			case -3:
				handleSYSErrors("mkdir");
				strcpy(statusMsg, "mkdir");
				break;
				break;
			default:
				break;
			}
		}
		else if (strcmp(request_command, "RMDIR") == 0)
		{
			// RMFILE path
			status = removeDirectory(path);
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("dir_not_found");
				strcpy(statusMsg, "dir_not_found");
				break;
			case -2:
				handleFileOperationError("not_dir");
				strcpy(statusMsg, "not_dir");
				break;
			case -3:
				handleSYSErrors("rmdir");
				strcpy(statusMsg, "rmdir");
				break;
			default:
				break;
			}
		}
		else if (strcmp(request_command, "MKFILE") == 0)
		{
			// RMFILE path
			char *perms = strtok(NULL, " \t\n");
			if (perms == NULL)
			{
				perms = malloc(sizeof(char) * 1024);
				strcpy(perms, "-rw-r--r--");
			}

			status = makeFile(path, perms);
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("file_not_found");
				strcpy(statusMsg, "file_not_found");
				break;
			case -2:
				handleFileOperationError("not_file");
				strcpy(statusMsg, "not_file");
				break;
			case -3:
				handleSYSErrors("creat");
				strcpy(statusMsg, "creat");
				break;
			default:
				break;
			}
		}
		else if (strcmp(request_command, "WRITEFILE") == 0)
		{
			status = sendFileCopy(path, nmfd, true); // error
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("file_not_found");
				strcpy(statusMsg, "file_not_found");
				break;
			case -2:
				handleFileOperationError("not_file");
				strcpy(statusMsg, "not_file");
				break;
			case -3:
				handleSYSErrors("creat");
				strcpy(statusMsg, "creat");
				break;
			default:
				break;
			}
		}
		else if (strcmp(request_command, "READFILE") == 0)
		{
			char *perms = strtok(NULL, " \t\n");
			if (perms == NULL)
			{
				perms = malloc(sizeof(char) * 1024);
				strcpy(perms, "-rw-r--r--");
			}
			status = receiveFileCopy(path, nmfd, perms, false); // error
			switch (status)
			{
			case 0:
				printf(YELLOW_COLOR "Command successfully executed" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);

				strcpy(statusMsg, "SUCCESS");
				break;
			case -1:
				handleFileOperationError("file_not_found");
				strcpy(statusMsg, "file_not_found");
				break;
			case -2:
				handleFileOperationError("not_file");
				strcpy(statusMsg, "not_file");
				break;
			case -3:
				handleSYSErrors("creat");
				strcpy(statusMsg, "creat");
				break;
			default:
				break;
			}
		}
		// backups

		else
		{
			continue;
		}
		int bytesSend = send(nmfd, statusMsg, strlen(statusMsg), 0);
		if (bytesSend == -1)
		{
			handleNetworkErrors("send");
		}
	}
	return NULL;
}

void *serveClient_Request(void *args)
{
	int connfd = *((int *)args);

	// recieve from client
	char buffer[4096];
	bzero(buffer, 4096);
	int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		handleNetworkErrors("recv");
	}
	printf(CYAN_COLOR "Recieved command from client: %s" RESET_COLOR, buffer);
	printf(WHITE_COLOR "\n" RESET_COLOR);

	char *arg_arr[3];
	parse_input(arg_arr, buffer);
	char *request_command = arg_arr[0];

	if (strcmp(arg_arr[0], "WRITE") == 0)
	{
		bool error = false;
		struct stat fileStat;
		char perms[11];
		if (!check_path_exists(arg_arr[1]))
		{
			handleFileOperationError("file_not_found");
			// strcpy(perms, "no_read_perm");
			error = true;
		}
		// if (!error)
		// {
		int r = stat(arg_arr[1], &fileStat);
		if (r == -1)
		{
			handleSYSErrors("stat");
			// strcpy(perms, "stat"); // add error in perms
			error = true;
		}
		else
		{
			convertPermissions(fileStat.st_mode, perms);
		}

		if (perms[1] == '-')
		{
			handleFileOperationError("no_read_perm");
			// strcpy(perms, "no_read_perm"); // add error in perms
			error = true;
		}
		if (perms[2] == '-')
		{
			handleFileOperationError("no_write_perm");
			// strcpy(perms, "no_write_perm"); // add error in perms
			error = true;
		}
		// }

		// send the perms first
		bytesRecv = send(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}
		if (error)
		{
			return NULL;
		}

		// send the file to client
		if (!sendFile(arg_arr[1], connfd))
		{
			printf(YELLOW_COLOR "Sent the file to client" RESET_COLOR);
			printf(WHITE_COLOR "\n" RESET_COLOR);
		}
		else
		{
			// printf(RED "File not sent\n" reset);
			handleFileOperationError("send_file");
			return NULL;
		}

		// receive the file from client
		if (!receiveFile(arg_arr[1], connfd))
		{
			printf(YELLOW_COLOR "Received the file from client" RESET_COLOR);
			printf(WHITE_COLOR "\n" RESET_COLOR);
		}
		else
		{
			handleFileOperationError("recv_file");
			return NULL;
		}

		// modify the mtime and actime manually since the user modified it while writing
		// struct timespec times[2];
		// struct stat file_stat;
		// if (stat(arg_arr[1], &file_stat) == -1)
		// {
		// 	handleSYSErrors("stat");
		// 	return NULL;
		// }
		// times[0].tv_sec = time(NULL);
		// times[0].tv_nsec = file_stat.st_atim.tv_nsec;
		// times[1].tv_sec = file_stat.st_mtime;
		// times[1].tv_nsec = file_stat.st_mtim.tv_nsec;
		// if (utimensat(AT_FDCWD, arg_arr[1], times, 0) == -1)
		// {
		// 	handleSYSErrors("utimensat");
		// 	return NULL;
		// }
	}
	else if (strcmp(arg_arr[0], "READ") == 0)
	{
		bool error = false;
		struct stat fileStat;
		char perms[11];
		if (!check_path_exists(arg_arr[1]))
		{
			handleFileOperationError("file_not_found");
			// strcpy(perms, "no_read_perm");
			error = true;
		}
		// if (!error)
		// {
		int r = stat(arg_arr[1], &fileStat);
		if (r == -1)
		{
			handleSYSErrors("stat");
			// strcpy(perms, "stat"); // add error in perms
			error = true;
		}
		else
		{
			convertPermissions(fileStat.st_mode, perms);
		}

		if (perms[1] == '-')
		{
			handleFileOperationError("no_read_perm");
			// strcpy(perms, "no_read_perm"); // add error in perms
			error = true;
		}
		if (perms[2] == '-')
		{
			handleFileOperationError("no_write_perm");
			// strcpy(perms, "no_write_perm"); // add error in perms
			error = true;
		}
		// }

		// send the perms first
		bytesRecv = send(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}
		if (error)
		{
			return NULL;
		}

		// send the file to client
		if (!sendFile(arg_arr[1], connfd))
		{
			printf(YELLOW_COLOR "Sent the file to client\n" RESET_COLOR);
		}
		else
		{
			// printf(RED "File not sent\n" reset);
			handleFileOperationError("send_file");
			return NULL;
		}
	}
	else if (strcmp(arg_arr[0], "FILEINFO") == 0)
	{
		bool error = false;
		char errMsg[100];
		char perms[11];

		if (!check_path_exists(arg_arr[1]))
		{
			handleFileOperationError("file_not_found");
			strcpy(errMsg, "file_not_found");
			error = true;
		}
		else
		{
			if (isDirectory(arg_arr[1]))
			{
				handleFileOperationError("not_file");
				strcpy(errMsg, "not_file");
				error = true;
			}
			else
			{
				struct stat dirStat;
				int r = stat(arg_arr[1], &dirStat);
				if (r == -1)
				{
					handleSYSErrors("stat");
					strcpy(errMsg, "stat");
					error = true;
				}
				if (error)
				{
					sendPathToNS(errMsg, "----------", 0, 0, 0, 0);
					return NULL;
				}

				convertPermissions(dirStat.st_mode, perms);
				size_t size = dirStat.st_size;
				// since it is connfd, this sends the file details to client
				sendPathToNS(arg_arr[1], perms, size, connfd, dirStat.st_mtime, dirStat.st_atime);
				printf(YELLOW_COLOR "Sent the file details to client" RESET_COLOR);
				printf(WHITE_COLOR "\n" RESET_COLOR);
			}
		}
	}
	return NULL;
}

void *acceptClients(void *args)
{
	int sockfd = *((int *)args);
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	while (1)
	{
		int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
		if (connfd < 0)
		{
			// perror("accept");
			handleNetworkErrors("accept");
			exit(0);
		}
		pthread_t serveClientThread;
		pthread_create(&serveClientThread, NULL, serveClient_Request, &connfd);
	}
	return NULL;
}

int initializeNMConnection(char *ip, int port, int nmPort, int cliPort)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		// perror("socket");
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = inet_addr(ip);
	cliaddr.sin_port = htons(nmPort);

	// to avoid bind error
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if ((bind(sockfd, (SA *)&cliaddr, sizeof(cliaddr))) != 0)
	{
		handleNetworkErrors("bind");
		// perror("bind");
		exit(0);
	}

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Storage Server";
	int bytesSent = send(sockfd, joinMsg, sizeof(joinMsg), 0);
	if (bytesSent == -1)
	{
		handleNetworkErrors("send");
	}
	char buffer[4096];
	int bytesRecv = recv(sockfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		handleNetworkErrors("recv");
	}
	if (strcmp(buffer, "ACCEPTED JOIN") != 0)
	{
		return -1;
	}

	// initialize detials
	struct ssDetails details;

	socklen_t addrlen = sizeof(cliaddr);
	if (getpeername(sockfd, (struct sockaddr *)&cliaddr, &addrlen) == 0)
	{
		inet_ntop(AF_INET, &cliaddr.sin_addr, details.ip, INET_ADDRSTRLEN);
	}
	details.nmPort = nmPort;
	details.cliPort = cliPort;

	bytesSent = send(sockfd, &details, sizeof(details), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}

	return sockfd;
}

int initializeNMConnectionForRecords(char *ip, int port)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}
	mkdir("backups", 0755);
	struct stat dirStat;
	int r = stat("backups", &dirStat);
	sendPathToNS("backups", "drwxr-xr-x", 0, sockfd, dirStat.st_mtime, dirStat.st_atime);

	return sockfd;
}

int initialzeClientsConnection(int port)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// to avoid bind error
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		// perror("bind");
		handleNetworkErrors("bind");
		exit(0);
	}
	if (listen(sockfd, 12) != 0)
	{
		handleNetworkErrors("listen");
	}

	return sockfd;
}

int main(int argc, char *argv[])
{
	// int nmPort = 6970;
	// int cliPort = 6971;
	if (argc != 4)
	{
		printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
		exit(0);
	}
	int mainPort = atoi(argv[1]);
	int nmPort = atoi(argv[2]);
	int cliPort = atoi(argv[3]);
	// printf("NMPORT: %d, CLIPORT:%d\n", nmPort, cliPort);
	int cliSock = initialzeClientsConnection(cliPort);
	printf(GREEN_COLOR "[+] Storage Server Initialized\n" RESET_COLOR);

	nmSock1 = initializeNMConnection("127.0.0.1", mainPort, nmPort, cliPort); // for feedback transfer
	int nmSock2 = initializeNMConnectionForRecords("127.0.0.1", mainPort);	  // for records

	printf(GREEN_COLOR "[+] Connected to Naming Server\n" RESET_COLOR);
	printf(YELLOW_COLOR "Port For NM Communication: %d\nPort for Client Communication: %d\n" RESET_COLOR, nmPort, cliPort);

	pthread_t nmThread, clientsThread, inputThread;
	pthread_create(&nmThread, NULL, serveNM_Requests, (void *)&nmSock1);
	pthread_create(&clientsThread, NULL, acceptClients, (void *)&cliSock);
	pthread_create(&inputThread, NULL, takeInputsDynamically, (void *)&nmSock2);

	pthread_join(inputThread, NULL);
	pthread_join(nmThread, NULL);
	pthread_join(clientsThread, NULL);

	return 0;
}

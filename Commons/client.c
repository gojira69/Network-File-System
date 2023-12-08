#include "headers.h"

int joinSS(struct ssDetails ss)
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
	servaddr.sin_addr.s_addr = inet_addr(ss.ip);
	servaddr.sin_port = htons(ss.cliPort);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}
	return sockfd;
}

int sendRequest(char *input, int sockfd)
{
	// send request
	char *arg_arr[3];
	parse_input(arg_arr, input);
	char *request_command = arg_arr[0];

	// privileged
	if (strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0 || strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0 || strcmp(request_command, "COPY") == 0)
	{
		int bytesSent = send(sockfd, input, strlen(input), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("send");
		}
		if (bytesSent == 0)
		{
			// nm has disconnected
			return -1;
		}

		// check for timout
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		bool isTimeout = false;
		int selectResult = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
		if (selectResult == -1)
		{
			handleSYSErrors("select");
		}
		else if (selectResult == 0)
		{
			// Timeout occurred
			handleNetworkErrors("timeout");
			isTimeout = true;
		}

		// recieve ack
		char ackMsg[4096];
		bzero(ackMsg, sizeof(ackMsg));
		int bytesRecv = recv(sockfd, ackMsg, sizeof(ackMsg), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}
		if (isTimeout)
		{
			return -2;
		}

		// only for output formatting
		if (strcmp(request_command, "MKDIR") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "Directory %s successfully created\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "Directory %s could not be created\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "COPY") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "%s successfully copied\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "%s could not be copied\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "MKFILE") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "File %s successfully created\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "File %s could not be created\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "RMDIR") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "Directory %s successfully removed\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				if (strcmp(ackMsg, "rmdir") == 0)
				{
					handleFileOperationError("dir_not_empty");
				}
				else
				{
					handleAllErrors(ackMsg);
				}

				printf(RED_COLOR "Directory %s could not be removed\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "RMFILE") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "File %s successfully removed\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "File %s could not be removed\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
	}

	else if (strcmp(request_command, "WRITE") == 0)
	{
		// request NS
		int bytesSent = send(sockfd, input, strlen(input), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("send");
		}
		if (bytesSent == 0)
		{
			// nm has disconnected
			return -1;
		}

		// recieve the ss details

		// check if currently something else is writing, give warning and wait
		// check for timout
		struct timeval timeout;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		int selectResult = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
		if (selectResult == -1)
		{
			handleSYSErrors("select");
		}
		else if (selectResult == 0)
		{
			// Timeout occurred
			handleFileOperationError("write_open");
			printf(YELLOW_COLOR "Waiting for other client to finish write\n" RESET_COLOR);
		}

		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id == 0) || bytesRecv == 0 || bytesRecv == -1)
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		else if (ss.id == -1)
		{
			handleFileOperationError("no_path");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}
		else if (ss.id == -2)
		{
			handleFileOperationError("not_file");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}
		else if (ss.id == -3)
		{
			handleFileOperationError("read_only");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}
		else if (ss.id == -4)
		{
			handleFileOperationError("no_backups");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		// printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);
		int connfd = joinSS(ss);

		// send same request to ss
		char ssRequest[1000];
		strcpy(ssRequest, input);
		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		// printf("\'%s\' sent to SS %s:%d\n", input, ss.ip, ss.cliPort);

		// receive the perms first
		char perms[11];
		bytesRecv = recv(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}
		if (strlen(perms) != 10)
		{
			handleSYSErrors(perms);
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		// printf("permissions: %s\n", perms);
		char recvFileName[1024];
		strcpy(recvFileName, basename(arg_arr[1]));
		strcpy(recvFileName + strlen(recvFileName), "_copy");
		// recieve file
		if (!receiveFile(recvFileName, connfd))
		{
			// printf("Client: received the file from ss\n");
		}
		else
		{
			handleFileOperationError("recv_file");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		mode_t mode = reversePermissions(perms);

		if (chmod(recvFileName, mode) == 0)
		{
			// printf("Permissions set successfully.\n");
		}
		else
		{
			handleSYSErrors("chmod");
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		char editor[1000];
		char editorList[4][100] = {"gedit", "vim", "nano", "emacs"};
		while (1)
		{
			printf(YELLOW_COLOR "Choose Editor to edit file:\n");
			printf("1. gedit\n");
			printf("2. vim\n");
			printf("3. nano\n");
			printf("4. emacs\n");
			printf("5. none (edit locally)\n" RESET_COLOR);
			scanf("%s", editor);
			if (!strcmp(editor, "gedit") || !strcmp(editor, "vim") || !strcmp(editor, "nano") || !strcmp(editor, "emacs") || !strcmp(editor, "none") || !strcmp(editor, "1") || !strcmp(editor, "2") || !strcmp(editor, "3") || !strcmp(editor, "4") || !strcmp(editor, "5"))

			{
				break;
			}
			else
			{
				printf(RED_COLOR "Invalid Editor!\n" YELLOW_COLOR "Choose Editor:\n" RESET_COLOR);
			}
		}

		if (!strcmp(editor, "none") || !strcmp(editor, "5"))
		{
			printf(YELLOW_COLOR "File has been added to the current directory\n");
			printf("Press ENTER after making changes locally to upload\n" RESET_COLOR);
			char c;
			scanf("%c", &c);
			scanf("%c", &c);
		}
		else
		{
			if (strlen(editor) == 1)
			{
				strcpy(editor, editorList[atoi(editor) - 1]);
			}
			if (writeFile(recvFileName, editor) == -1)
			{
				handleSYSErrors("write");
				char ackBUFFER[10];
				strcpy(ackBUFFER, "done");
				int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
				if (bSent == -1)
				{
					handleNetworkErrors("send");
				}
				return -2;
			}
		}

		// send the file to ss
		if (!sendFile(recvFileName, connfd))
		{
			printf(YELLOW_COLOR "Sent the file to Storage Server\n" reset);
		}
		else
		{
			handleFileOperationError("send_file");
			removeFile(recvFileName);
			char ackBUFFER[10];
			strcpy(ackBUFFER, "done");
			int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
			if (bSent == -1)
			{
				handleNetworkErrors("send");
			}
			return -2;
		}

		removeFile(recvFileName);
		// add code for sending ack to nm
		char ackBUFFER[10];
		strcpy(ackBUFFER, "done");
		int bSent = send(sockfd, ackBUFFER, strlen(ackBUFFER), 0);
		if (bSent == -1)
		{
			handleNetworkErrors("send");
		}
		return 0;
	}
	else if (strcmp(request_command, "READ") == 0)
	{
		// request NS
		int bytesSent = send(sockfd, input, strlen(input), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("send");
		}
		if (bytesSent == 0)
		{
			// nm has disconnected
			return -1;
		}

		// recieve the ss details
		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id == 0) || bytesRecv == 0 || bytesRecv == -1)
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		else if (ss.id == -1)
		{
			handleFileOperationError("no_path");
			return -2;
		}
		else if (ss.id == -2)
		{
			handleFileOperationError("not_file");
			return -2;
		}
		else if (ss.id == -3)
		{
			handleFileOperationError("read_only");
			return -2;
		}
		else if (ss.id == -4)
		{
			handleFileOperationError("no_backups");
			return -2;
		}

		// printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);
		int connfd = joinSS(ss);

		char ssRequest[1000];
		bzero(ssRequest, sizeof(ssRequest));
		if (ss.inBackup)
		{
			strcpy(ssRequest, "READ backups/");
			strcat(ssRequest, arg_arr[1]);
		}
		else
			// send same request to ss
			strcpy(ssRequest, input);

		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		// printf("\'%s\' sent to SS %s:%d\n", input, ss.ip, ss.cliPort);

		// receive the perms first
		char perms[11];
		bytesRecv = recv(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		if (strlen(perms) != 10)
		{
			handleSYSErrors(perms);
			return -2;
		}
		// disabling write permissions while reading
		perms[2] = '-';

		// printf("permissions: %s\n", perms);
		char recvFileName[1024];
		strcpy(recvFileName, basename(arg_arr[1]));
		strcpy(recvFileName + strlen(recvFileName), "_copy");
		// recieve file
		if (!receiveFile(recvFileName, connfd))
		{
			// printf("Client: received the file from ss\n");
		}
		else
		{
			handleFileOperationError("recv_file");
			return -2;
		}

		mode_t mode = reversePermissions(perms);

		if (chmod(recvFileName, mode) == 0)
		{
			// printf("Permissions set successfully.\n");
		}
		else
		{
			handleSYSErrors("chmod");
			return -2;
		}

		char editor[1000];
		char editorList[5][100] = {"gedit", "vim", "nano", "emacs", "cat"};
		printf(YELLOW_COLOR "Choose Editor to Read file:\n");
		printf("1. gedit\n");
		printf("2. vim\n");
		printf("3. nano\n");
		printf("4. emacs\n");
		printf("5. terminal\n" RESET_COLOR);
		while (1)
		{
			scanf("%s", editor);
			if (!strcmp(editor, "gedit") || !strcmp(editor, "vim") || !strcmp(editor, "nano") || !strcmp(editor, "emacs") || !strcmp(editor, "none") || !strcmp(editor, "1") || !strcmp(editor, "2") || !strcmp(editor, "3") || !strcmp(editor, "4") || !strcmp(editor, "5"))
			{
				break;
			}
			else
			{
				printf(RED_COLOR "Invalid Editor!\n" YELLOW_COLOR "Choose Editor:\n" RESET_COLOR);
			}
		}

		if (!strcmp(editor, "terminal") || !strcmp(editor, "5"))
		{
			if (readFile(recvFileName, "cat") == -1)
			{
				handleFileOperationError("read");
				return -2;
			}
			printf("\n");
		}
		else
		{
			if (strlen(editor) == 1)
			{
				strcpy(editor, editorList[atoi(editor) - 1]);
			}
			if (readFile(recvFileName, editor) == -1)
			{
				handleFileOperationError("read");
				return -2;
			}
		}
		removeFile(recvFileName);
		return 0;
	}
	else if (strcmp(request_command, "FILEINFO") == 0)
	{
		// request NS
		int bytesSent = send(sockfd, input, strlen(input), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("send");
		}
		if (bytesSent == 0)
		{
			// nm has disconnected
			return -1;
		}

		// recieve the ss details
		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id == 0) || bytesRecv == 0 || bytesRecv == -1)
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		else if (ss.id == -1)
		{
			handleFileOperationError("no_path");
			return -2;
		}
		else if (ss.id == -2)
		{
			handleFileOperationError("not_file");
			return -2;
		}
		else if (ss.id == -3)
		{
			handleFileOperationError("read_only");
			return -2;
		}
		else if (ss.id == -4)
		{
			handleFileOperationError("no_backups");
			return -2;
		}

		int connfd = joinSS(ss);

		char ssRequest[1000];
		bzero(ssRequest, sizeof(ssRequest));
		if (ss.inBackup)
		{
			strcpy(ssRequest, "FILEINFO backups/");
			strcat(ssRequest, arg_arr[1]);
		}
		else
			// send same request to ss
			strcpy(ssRequest, input);

		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		struct fileDetails det;
		bzero(&det, sizeof(det));
		bytesRecv = recv(connfd, &det, sizeof(det), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
			// break;
		}
		if (strcmp(det.perms, "----------") == 0)
		{
			handleAllErrors(det.path);
			return -2;
		}

		printf(CYAN_COLOR "File Name: %s\n", det.fileName);
		printf("File path: %s\n", det.path);
		printf("File permissions: %s\n", det.perms);
		printf("File size: %lu\n", det.size);
		printf("File last modified time: %s", ctime(&det.lastModifiedTime));
		printf("File last access time: %s\n" CYAN_COLOR, ctime(&det.lastAccessTime));
		return 0;
	}
	else if (strcmp(request_command, "HELP") == 0)
	{
		// using this to center text
		struct winsize w;
		ioctl(0, TIOCGWINSZ, &w);
		int columns = w.ws_col;
		int rows = w.ws_row;
		char *string = "USER COMMANDS";
		int stringLength = strlen(string) / 2;

		printf(ORANGE_COLOR "%*s\n\n", columns / 2 + stringLength, string);
		printf("1. READ <pathToFile> ====> Read a file \n");
		printf("2. WRITE <pathToFile> ====> Write to a file \n");
		printf("3. FILEINFO <pathToFile> ====> Get file information \n");
		printf("4. MKDIR <pathToDirectory> ====> Make a directory \n");
		printf("5. MKFILE <pathToFile> ====> Make a file \n");
		printf("6. RMDIR <pathToDirectory> ====> Remove a directory \n");
		printf("7. RMFILE <pathToFile> ====> Remove a file \n");
		printf("8. COPY <sourcePath> <destinationPathToDirectory> ====> Copy a directory or a file to another directory \n");
		printf("9. HELP ====> Print list of commands\n");
		printf("10. EXIT ====> Exit the NFS\n" reset);
		return 0;
	}
	else if ((strcasecmp(request_command, "EXIT") == 0))
	{
		exit(0);
	}
	else
	{
		printf(RED_COLOR "Invalid Input!\n" RESET_COLOR);
		return 0;
	}

	return 0;
}

int joinNamingServerAsClient(char *ip, int port)
{
	int sockfd;
	struct sockaddr_in servaddr;

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

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		// perror("connect");
		handleNetworkErrors("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Client";
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
	// printf("Client connected to naming server.\n");

	return sockfd;
}

void *isNMConnected(void *args)
{
	int sockfd = *(int *)(args);
	while (1)
	{
		sleep(1);
		int error;
		socklen_t len = sizeof(error);
		int ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

		if (ret == 0 && error == 0)
		{
			sleep(1);
			continue;
		}
		else
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
		exit(0);
	}

	int namingServerPort = atoi(argv[1]);
	// int namingServerPort = 6969;
	int sockfd = joinNamingServerAsClient("127.0.0.1", namingServerPort);
	printf(GREEN_COLOR "[+] Client connected to Naming Server\n" RESET_COLOR);

	pthread_t disconnectionThread;
	pthread_create(&disconnectionThread, NULL, isNMConnected, &sockfd);

	// using this to center text
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	int columns = w.ws_col;
	int rows = w.ws_row;
	char *string = "WELCOME TO NFS!";
	int stringLength = strlen(string) / 2;
	char *string2 = "Type 'HELP' to list all commands and their use";
	int stringLength2 = strlen(string2) / 2;
	printf("\n");
	printf(BBLU "%*s\n" reset, columns / 2 + stringLength, string);
	printf("%*s\n\n", columns / 2 + stringLength2, string2);
	while (1)
	{
		// char input[4096];
		printf(BLUE_COLOR "Enter command: " RESET_COLOR);
		char *input = malloc(sizeof(char) * 4096);
		if (fgets(input, 4096, stdin) == NULL)
		{
			break;
		}
		while (strlen(input) == 1)
		{
			if (fgets(input, 4096, stdin) == NULL)
			{
				break;
			}
		}

		if (input[strlen(input) - 1] == '\n')
		{
			input[strlen(input) - 1] = '\0'; // removing the \n char
		}

		// printf(".%ld.%s.\n", strlen(input), input);
		int status = sendRequest(input, sockfd);
		if (status == -1)
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
			break;
		}

		free(input);
	}
	pthread_join(disconnectionThread, NULL);
	return 0;
}

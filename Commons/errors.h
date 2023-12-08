#ifndef __ERRORS_H
#define __ERRORS_H

void handleFileOperationError(char *error);
void handleSYSErrors(char *error);
void handleNetworkErrors(char *error);
void handleAllErrors(char *error);

#endif
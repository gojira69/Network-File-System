#include "headers.h"

void handleAllErrors(char *error)
{
    handleFileOperationError(error);
    handleSYSErrors(error);
    handleNetworkErrors(error);
}

// file operations errors (all in red)
void handleFileOperationError(char *error)
{
    if (!strcmp(error, "file_not_found"))
        printf(RED "ERROR 501: FILE NOT FOUND (search)\n" reset);

    if (!strcmp(error, "dir_not_found"))
        printf(RED "ERROR 501: DIRECTORY NOT FOUND (search)\n" reset);

    if (!strcmp(error, "not_file"))
        printf(RED "ERROR 502: PATH IS NOT A FILE (isFile)\n" reset);

    if (!strcmp(error, "not_dir"))
        printf(RED "ERROR 503: PATH IS NOT A DIRECTORY (isDir)\n" reset);

    if (!strcmp(error, "file_exists"))
        printf(RED "ERROR 504: FILE ALREADY EXISTS (file_exists)\n" reset);

    if (!strcmp(error, "dir_exists"))
        printf(RED "ERROR 505: DIRECTORY ALREADY EXISTS (dir_exists)\n" reset);

    if (!strcmp(error, "no_path"))
        printf(RED "ERROR 506: PATH IS INVALID OR NOT ACCESSIBLE\n" reset);

    if (!strcmp(error, "no_read_perm"))
        printf(RED "ERROR 507: FILE DOES NOT HAVE READ PERMISSION\n" reset);

    if (!strcmp(error, "no_write_perm"))
        printf(RED "ERROR 508: FILE DOES NOT HAVE WRITE PERMISSION\n" reset);

    if (!strcmp(error, "recv_file"))
        printf(RED "ERROR 509: COULD NOT RECEIVE FILE (recv_file)\n" reset);

    if (!strcmp(error, "send_file"))
        printf(RED "ERROR 510: COULD NOT SEND FILE (send_file)\n" reset);

    if (!strcmp(error, "dir_not_empty"))
        printf(RED "ERROR 511: DIRECTORY IS NOT EMPTY (check_empty)\n" reset);

    if (!strcmp(error, "read_only"))
        printf(RED "ERROR 512: PATH IS READ ONLY (read_only)\n" reset);

    if (!strcmp(error, "no_backups"))
        printf(RED "ERROR 513: STORAGE SERVERS ARE DOWN (no_backups)\n" reset);
    
    if (!strcmp(error, "copy_failed"))
        printf(RED "ERROR 514: COULD NOT COPY (copy)\n" reset);
    
    if (!strcmp(error, "write_open"))
        printf(RED "ERROR 515: FILE IS CURRENTLY OPEN IN WRITE MODE (write)\n" reset);
}

// syscall errors (all in light red)
void handleSYSErrors(char *error)
{
    if (!strcmp(error, "malloc"))
        printf(LIGHT_RED_COLOR "ERROR 600: MALLOC FAILED (malloc)\n" reset);

    if (!strcmp(error, "remove"))
        printf(LIGHT_RED_COLOR "ERROR 601: REMOVE FAILED (remove)\n" reset);

    if (!strcmp(error, "stat"))
        printf(LIGHT_RED_COLOR "ERROR 602: UNABLE TO GET FILE DETAILS (stat)\n" reset);

    if (!strcmp(error, "rmdir"))
        printf(LIGHT_RED_COLOR "ERROR 603: REMOVE DIRECTORY FAILED (rmdir)\n" reset);

    if (!strcmp(error, "mkdir"))
        printf(LIGHT_RED_COLOR "ERROR 604: MAKE DIRECTORY FAILED (mkdir)\n" reset);

    if (!strcmp(error, "creat"))
        printf(LIGHT_RED_COLOR "ERROR 605: CREATE FILE FAILED (creat)\n" reset);

    if (!strcmp(error, "open"))
        printf(LIGHT_RED_COLOR "ERROR 606: COULD NOT OPEN FILE (open)\n" reset);

    if (!strcmp(error, "read"))
        printf(LIGHT_RED_COLOR "ERROR 607: COULD NOT READ FROM FILE (read)\n" reset);

    if (!strcmp(error, "write"))
        printf(LIGHT_RED_COLOR "ERROR 608: COULD NOT WRITE TO FILE (write)\n" reset);

    if (!strcmp(error, "utimensat"))
        printf(LIGHT_RED_COLOR "ERROR 609: COULD NOT CHANGE MODIFICATION TIME (utimensat)\n" reset);

    if (!strcmp(error, "chmod"))
        printf(LIGHT_RED_COLOR "ERROR 610: COULD NOT CHANGE PERMISSIONS (chmod)\n" reset);

    if (!strcmp(error, "execvp"))
        printf(LIGHT_RED_COLOR "ERROR 611: COULD NOT EXECUTE COMMAND (execvp)\n" reset);
    
    if (!strcmp(error, "select"))
        printf(LIGHT_RED_COLOR "ERROR 612: SELECT failed (select)\n" reset);
}

// network errors (all in red)
void handleNetworkErrors(char *error)
{
    if (!strcmp(error, "socket"))
        printf(RED "ERROR 901: UNABLE TO OPEN SOCKET (socket)\n" reset);

    if (!strcmp(error, "bind"))
        printf(RED "ERROR 902: UNABLE TO BIND TO SOCKET (bind)\n" reset);

    if (!strcmp(error, "listen"))
        printf(RED "ERROR 903: UNABLE TO LISTEN TO INCOMING CONNECTIONS (listen)\n" reset);

    if (!strcmp(error, "accept"))
        printf(RED "ERROR 904: UNABLE TO ACCEPT CONNECTION (accept)\n" reset);

    if (!strcmp(error, "connect"))
        printf(RED "ERROR 905: UNABLE TO CONNECT TO SERVER (connect)\n" reset);

    if (!strcmp(error, "recv"))
        printf(RED "ERROR 906: FAILED TO RECIEVE FROM SOCKET (recv)\n" reset);

    if (!strcmp(error, "send"))
        printf(RED "ERROR 907: FAILED TO SEND TO SOCKET (send)\n" reset);
    
    if (!strcmp(error, "timeout"))
        printf(RED "ERROR 908: COMMAND TIMED OUT (timeout)\n" reset);
}
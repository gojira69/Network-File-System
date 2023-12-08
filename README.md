# Network File System

## Introduction
This project implements a Network File System (NFS) consisting of a Naming `Server` (NS), `Storage Servers` (SS), and `Clients`. The system allows multiple clients to access and perform operations on files and directories stored across distributed storage servers. The NS maintains the metadata and routing information, while SS stores and manages the actual data.


## Usage
1. To build Naming Server 
```
./build.sh namingServer
```
2. To build Storage Server 1 (in `StorageServers/storageServer1`)
```
./build.sh storageServer1
```
3. To build Client 1 (in `Clients/client1`)
```
./build.sh client1
```

## Naming Server

1. The naming server is initialized in `initializeNamingServer()` where a new socket is created and binded to a fixed port.

2. The function `acceptHost()` runs on a thread allowing clients and storage servers to join. On joining, the host sends a message indicating if its a client or a storage server and their detials are stored in `clientDetials` and `storageServers` respectively.

3. After a storage server joins the naming server, then another socket is initialsed for listening to accessible paths dynamically on a separate thread running the `addToRecord()` function.

4. After a client joins the naming server, it continously listens for requests from the client on separate threads `acceptClientRequests()`. The details of the storage server responsible for the given path is determined by `getRecord()` function.

5. The data structure for storing the records are `Tries` and `LRU` cache. Both of these, store the details of each record by storing the `pointer` to each record in their own `struct node`.

6. Each `TrieNode` has the pointer to a record and an `array` of pointers. Using the index of each letter in the path provided, it searches for a record inside the `Trie`. If at any point, the pointer at any index is `NULL`, then that record does not exist. Given a pointer a record, it creates a

7. Another data structure for storing a record is an `LRU` cache. It uses an array for storing the least recently used records. The cache has a fixed size of 10, which can be changed accordingly by the user. Search for a record first happens in the `LRU`. If the record is found, then it is a `cache hit`. If the record is not found, then it is a `cache miss`. Then it proceeds to search in the `Trie` if it was a cache miss. If the record is found, then the record is added to the cache. At the same time, the least recently used record is discarded which is at the end of the array in which `LRU` is operating.

8. Overall `LRU` does search in O(1) time, since there are only a constant number of elements in the cache at any given time. `Tries` take a slighlty longer time, O(n) in the worst case, where n is the string length of the path provided.

9. The NM checks if the client request is a privileged operation or not. Accordingly it redirects the client. If it is a privileged operation (`RMDIR`, `RMFILE`, `MKDIR`, `MKFILE`, `COPY`), the NM itself handles this operation by extracting the corresponding record and from it the SS details.

10. If it is a non-privileged operation (`READ`, `WRITE`, `FILEINFO`, `HELP`), the NM sends the details of the SS where the file is stored by extracting the SS details from the record.

11. If at any point in time, no record is found, then it sends the relevent error message to the client.

12. After each operation, the client sends `ACK` accordingly to the NM if the operation was completed successfully or not. if not, then the client sends or receives the corresponding error message.

## Storage Server

1. The storage server is initialized by binding to two ports one for communication with all clients - `initialzeClientsConnection()` and another for communication with the naming server - `initializeNMConnection()`.

2. A thread running `takeInputsDynamically()` is created which continously listens on the command line for the list of accessible paths which is then sent to the naming server.

3. A thread for accepting clients is created which first accepts connection requests from clients and then responds to them using the `serveClient_Request()` thread.

4. The SS always receives the original request and accordingly performs the operation. If it is a privileged operation, then it performs the operation through the thread `serveNM_Requests`. After the operation is done, it sends the `ACK` to the NM, and in case an error happens, it sends the error message to the NM.

5. If it was a non-privileged request, then the SS connects with the client and performs the necessary operation using the thread `serveClient_Requested`. It also returns the necessary `ACK` strings and handles error if the operation was unsuccessfull.

## Client

1. The client is initialized using the `joinNamingServerAsClient()` and is accepted in the `acceptHost()` function in the naming server.

2. A continuous thread running the `isNMConnected()` function runs to check for the socket status of the naming server. Whenever the naming server goes down, the client is notified.

3. In a continous loop, first an input is taken from the user and sent to the `sendRequest()` function which processes the input and sends it to the naming server.

4. Each request is redirected to the NM and these requests are performed in the function `sendRequest`. There, if the request was privileged, the NM handles that request by connecting with the correct SS and returns the `ACK` to the client. If there were any errors with the operation, they are handled accordingly.

5. If the operation was not privileged, the NM on receiving the request returns the correct details of the SS. Upon receiving these details, the client connects with the SS using the function `joinSS` and makes the request again. The SS performs this operation and returns the `ACK`. If there were any errors, they are handled accordingly.

6. Depending on the return value of the fucntion `sendRequest`, the client accepts the next request.

## API Calls

1. `MKDIR` and `MKFILE`: enables the creation of directory with a given set of permissions. Implemented by the `makeDirectory` call which checks whether a given path exists or not, and using the `reversePermissions` call to get the permissions set by the user. Default permissions for a directory are `drwxr-xr-x` and for a file are `-rw-r--r--`. After the succesful completion of the call, the newly created path is added to the records.

2. `RMDIR` and `RMFILE`: deletes a file or a directory and the paths are removed from the records. For deletion of a directory, `rmdir` system call is used and for that of a file, `rmfile` command is used. Note that the expected behaviour of a directory that is not empty cannot be deleted is followed.

3. `COPY` is used to copy a file or a directory. We are recursively traversing the accessible path `n-ary tree` (`nextSibling` for all except the source itself and the `firstChild` for every intermediate node). If we encounter a directory, we use the already defined `MKDIR` call otherwise we use `MKFILE`. We then send over the contents of the file via the network and send it to the destination storage server. The temporarily created file at the naming server location is subsequently deleted.

4. `READ` and `WRITE`: are used to view and modify the files respectively. They are performed between the storage servers and the clients directly. Before sending any file, the storage server first sends the permsissions of the file requested to the client. Then the client checks if the file has enough file permissions and continues. The storage server sends the file in fixed size chunks using `send` and `recv` along with intermediate `ACK` packets. The client receives the file until the client receives the `STOP` packet. The contents of the file requested are stored in a `temp` file for reading and writing. For the case of `WRITE`, the modified file is sent back to the storage serber to update the changes.

5. `FILEINFO`: returns the information related to the file requested (no directories) by the client like permission, last modified time, path, etc. The information about the file is stored in the `struct fileDetails` and is sent by the storage server to the client.

## Error Codes

Custom error codes have been defined for almost all possible errors in the `errors.c` file.
|Error Code|Type Of Error|
|--|--|
|5xx|File Operation Errors|
|9xx|Network Errors|
|6xx|System Call Errors|

## Redundancy

1. To determine which storage servers would serve as the backup servers for a particular server, the policy used here is to assign the highest two SS ids among the currently active servers whenever a new storage server joins.

2. Reconnection of a storage server is tracked by the IP and Port of the storage server. It is used to match back the accessible paths of the disconnected storage server after reconnection.

3. Function `backupCopy()` and `backupRemove()` are used to update the backup files stored.
   `backupCopy()` is called whenever a new file/folder is added or modified by `MKDIR`, `MKFILE`, `WRITE` or when a new path is added to the list of accessible paths by the SS .
   `backupRemove()` is called whenever an existing file is removed by `RMDIR` or `RMFILE`

4. When the originalSS for a requested file is down, and the operation is a read operation like `READ` or `FILEPERMS`, then the backup SS detials are sent back to the client. In other cases, an error is raised stating that the requested path is in read-only mode and cannot be modified.

## Concurrency

1. The naming server can access requests from multiple clients and multiple storage servers at the same time. To prevent race condition on the critical sections of the code, locks have been implemented for global variables which are shared by multiple threads.

2. Each accessible file is associated with a lock which is aquired when an operation needs to take place on that file, and then released when the operation has completed.

3. If requested files have been blocked due to some reason, a timeout message is sent to the user to indicate the blockage. The basic case is when two clients request the same file to be written to, then the first client is given access first and the second has to wait until the first client has completed the write request.

## Logging

A function `logMessage()` has been implemented which displays detials about all network operations related to the naming server. It displays the timestamp, sender/receiver IP and port along with the log message of what is sent or received in the format -

```
[DD-MM-YYYY HH:MM:SS] [<ip>:<port>] -> <message>
```
### Note
To initialize the storage servers in specific directories, one can modify the `build.sh` shell-script to do that.
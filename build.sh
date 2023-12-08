#!/bin/bash

NM_PORT=6969
SS_PORTS=6970

client%() {
    cd "Clients" || exit
    mkdir -p "$1"
    cd "$1" || exit
    gcc ../client.c ../api.c ../errors.c -o "$1" -lpthread
    ./"$1" $NM_PORT
    rm "$1"
    cd ../..
}

storageServer%() {
    cd "StorageServers" || exit
    mkdir -p "$1"
    cd "$1" || exit
    gcc ../storageServer.c ../api.c ../errors.c -o "$1" -lpthread
    ./"$1" $NM_PORT $(((${1#storageServer} * 2) + SS_PORTS)) $(((${1#storageServer} * 2) + 1 + SS_PORTS))
    rm "$1"
    cd ../..
}

namingServer() {
    cd "NamingServer" || exit
    gcc namingServer.c search.c api.c cache.c errors.c -o namingServer -lpthread
    ./namingServer $NM_PORT
    rm namingServer
    cd ..
}


case "$1" in
    client*)
        client% "$1"
        ;;
    storageServer*)
        storageServer% "$1"
        ;;
    namingServer)
        namingServer
        ;;
    *)
        echo "Invalid target"
        ;;
esac
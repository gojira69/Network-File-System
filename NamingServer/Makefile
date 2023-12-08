NM_PORT := 6969

namingServer:
	gcc namingServer.c  search.c api.c cache.c errors.c -o namingServer -pthread
	./namingServer $(NM_PORT)
	rm namingServer

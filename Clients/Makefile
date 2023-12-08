NM_PORT := 6969

client:
	gcc client.c api.c errors.c -o client -pthread
	./client $(NM_PORT)
	rm client

client%:
	gcc client.c api.c  errors.c -o $@ -pthread
	./$@ $(NM_PORT)
	rm $@

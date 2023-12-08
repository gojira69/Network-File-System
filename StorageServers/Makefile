NM_PORT := 6969
SS_PORTS := 6970


storageServer:
	gcc storageServer.c api.c errors.c -o storageServer -pthread
	./storageServer $(NM_PORT) $(SS_PORTS) $$((1  + $(SS_PORTS)))
	rm storageServer

storageServer%: 
	gcc storageServer.c api.c errors.c -o $@ -pthread
	./$@ $(NM_PORT) $$(($(subst storageServer,,$@) * 2 + $(SS_PORTS))) $$(($(subst storageServer,,$@) * 2 + 1  + $(SS_PORTS)))
	rm $@

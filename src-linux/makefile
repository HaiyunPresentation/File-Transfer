all:Server Client

Server:Server.c
	gcc Server.c -o Server

Client:Client.c
	gcc Client.c -o Client

clear:
	if [ -f Server ];then rm Server; fi
	if [ -f Client ];then rm Client; fi
	
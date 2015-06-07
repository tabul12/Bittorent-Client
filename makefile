btclient: bencode.c construct.c btclient.c tracker.c
	gcc -o btclient -pthread bencode.c construct.c btclient.c tracker.c -lssl -lcrypto -lcurl
clean: 
	rm -f btclient

 


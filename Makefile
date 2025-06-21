# all: builds both server and client
all: server client

# server compilation
server: server.c tinyexpr.c
	gcc server.c tinyexpr.c -o server -lpthread -lm

#client compilation
client: client.c
	gcc client.c -o client -lpthread -lm

# clean up
clean:
	rm -f server client *.o
	@echo "Clean successful"

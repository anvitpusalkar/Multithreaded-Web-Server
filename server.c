// Server Implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tinyexpr.h"

#define MAX_THREADS 5
#define PORT 8080
#define IP_ADDR "127.0.0.1"
#define BUFFER 1024
#define CONNECTIONS 5

// Queue for Client Sockets
struct Node {
				int client_socket;
				struct Node* next;
};

typedef struct {
				struct Node *front, *rear;
				int count;
} Queue;

Queue queue;

// Mutex and Conditional Variable
pthread_mutex_t mutex;

pthread_cond_t cond;

int server_running = 1;

// Queue Operations
void initialize(Queue *q){
				q->front = NULL;
    		q->rear = NULL;
    		q->count = 0;
}

void enqueue(Queue *q, int socket){
				struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    		new_node->client_socket = socket;
    		new_node->next = NULL;

    		if (q->rear == NULL) {
        				q->front = new_node;
        				q->rear = new_node;
    		} else {
        				q->rear->next = new_node;
        				q->rear = new_node;
    		}
    		q->count++;
}

int dequeue(Queue *q){
				if (q->front == NULL) {
								printf("Can't remove from empty queue\n");
    						return -1;
				}
				struct Node* temp = q->front;
				int socket = temp->client_socket;
				q->front = q->front->next;
				if (q->front == NULL) {
   				 			q->rear = NULL;
				}
				free(temp);
				q->count--;
				return socket;
}

int isEmpty(Queue *q){
				return (q->count == 0);
}

// Worker Function
void *worker_function(void *arg){
				int thread_id = (int)(long)arg;

				// Loop to server clients
				while (server_running){
								int client_socket;

								pthread_mutex_lock(&mutex);

								while (isEmpty(&queue) && server_running){
												pthread_cond_wait(&cond, &mutex);
								}

								// Server is not running and queue is empty
								if (!server_running && isEmpty(&queue)){
												pthread_mutex_unlock(&mutex);
												break;
								}

								client_socket = dequeue(&queue);

								pthread_mutex_unlock(&mutex);

								// Print which thread is handling which socket
								printf("Worker thread %d handling client socket %d\n", thread_id, client_socket);

								// Declare the buffer
								char buffer[BUFFER];
								memset(buffer, 0, BUFFER);

								char response[BUFFER];
								memset(response, 0, BUFFER);

								// Receive the mathematical expression from client
								ssize_t bytes_received = recv(client_socket, buffer, BUFFER-1, 0);
								if (bytes_received <= 0){
												if (bytes_received == 0){
																printf("Client disconnected.\n");
												}
												else{
																printf("Error receiving data\n");
												}
												close(client_socket);
												continue;
								}
								buffer[bytes_received] = '\0';

								// Print the received expression
								printf("Worker thread %d received expression: '%s' from socket %d.\n", thread_id, buffer, client_socket);

								// Evaluate the result using tinexpr
								int error_pos;
								double result = te_interp(buffer, &error_pos);

								// Format the result as a string
								if (error_pos == 0){
												snprintf(response, BUFFER, "Result: %.10g\n", result);
								}

								// Send data(result) to client
								ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);
								if (bytes_sent == -1){
												printf("Error sending data\n");
								}

								printf("Worker thread %d sent response: '%s' to socket %d.\n", thread_id, response, client_socket);
								// Close the client_socket
								close(client_socket);
				}
				pthread_exit(NULL);
}

int main(){
				// Initialize mutex,conditional variable and client connections queue
				pthread_mutex_init(&mutex, NULL);
				pthread_cond_init(&cond, NULL);
				initialize(&queue);

				// Setting up connection
				int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (server_socket == -1){
								printf("Error creating socket...\n");
								pthread_mutex_destroy(&mutex);
								pthread_cond_destroy(&cond);
								return 1;
				}

				// Binding socket to IP address and Port
				struct sockaddr_in server_address;
				server_address.sin_family = AF_INET;
				server_address.sin_addr.s_addr = inet_addr(IP_ADDR);
				server_address.sin_port = htons(PORT);

				if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1){
								printf("Error binding socket...\n");
								pthread_mutex_destroy(&mutex);
								pthread_cond_destroy(&cond);
								return 1;
				}

				// Listen for connections
				if (listen(server_socket, CONNECTIONS) == -1){
								printf("Error listening on socket...\n");
								pthread_mutex_destroy(&mutex);
								pthread_cond_destroy(&cond);
								return 1;
				}

				printf("Server listening on IP: %s and Port: %d...\n", IP_ADDR, PORT);

				// Create worker threads
				pthread_t worker_threads[MAX_THREADS];
				for (int i = 0; i<MAX_THREADS; i++){
								pthread_create(&worker_threads[i],NULL,worker_function,(void*)(long)(i+1));
				}

				// Loop for accepting connections
				while (server_running){
								struct sockaddr_in client_address;
								socklen_t client_address_len = sizeof(client_address);

								int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
								if (client_socket == -1){
												printf("Error accepting connections...\n");
												if (!server_running) {
																break;
												}
												continue;
								}

								char client_ip[INET_ADDRSTRLEN];
								inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
								printf("Accepted connection from %s:%hu (socket: %d)\n", client_ip, ntohs(client_address.sin_port), client_socket);

								// Add the connection to the client connection queue
								pthread_mutex_lock(&mutex);
								enqueue(&queue, client_socket);
								pthread_mutex_unlock(&mutex);

								// Notify worker thread
								pthread_cond_signal(&cond);
				}

				printf("Server shutting down...\n");

				pthread_cond_broadcast(&cond);

				// Join all worker threads
    		for (int i = 0; i < MAX_THREADS; ++i) {
        		pthread_join(worker_threads[i], NULL);
    		}

				// Destroy mutex and conditional variable
				pthread_mutex_destroy(&mutex);
    		pthread_cond_destroy(&cond);

				// Close the socket
				close(server_socket);

				printf("Server shutdown!");

				return 0;
}

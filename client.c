// Client Implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_THREADS 5
#define BUFFER 1024
#define EXPRESSIONS "expressions.txt"

struct Node {
    char* expression;
    struct Node* next;
};

typedef struct {
    struct Node *front, *rear;
    int count;
} Queue;

Queue queue;

// Declare mutex and conditional variable
pthread_mutex_t mutex;
pthread_cond_t cond;
int expressions_loaded = 0;

void initialize(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

void enqueue(Queue *q, const char* expr) {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    new_node->expression = strdup(expr);
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

char* dequeue(Queue *q) {
    if (q->front == NULL) {
						printf("Error: queue is empty\n");
						return NULL;
    }
    struct Node* temp = q->front;
    char* expr = temp->expression;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->count--;
    return expr;
}

int isEmpty(Queue *q) {
    return (q->count == 0);
}

// Worker function
void* worker_function(void* arg){
				int thread_id = (int)(long)arg;
				while(1){
								char* expr = NULL;

								pthread_mutex_lock(&mutex);

								while (isEmpty(&queue) && !expressions_loaded){
												pthread_cond_wait(&cond, &mutex);
								}

								// Exit if there is no more work
								if (isEmpty(&queue) && expressions_loaded){
												pthread_mutex_unlock(&mutex);
												break;
								}

								// Get an expression
								expr = dequeue(&queue);

								pthread_mutex_unlock(&mutex);

								// Create socket
								int client_socket = socket(AF_INET, SOCK_STREAM, 0);
								if (client_socket == -1){
												printf("Error creating socket\n");
												free(expr);
												continue;
								}

								// Bind the socket
								struct sockaddr_in server_address;
        				server_address.sin_family = AF_INET;
        				server_address.sin_port = htons(SERVER_PORT);

								// Convert IP to network format
								if (inet_aton(SERVER_IP, &server_address.sin_addr) == 0) {
            						fprintf(stderr, "Client: Invalid server IP address: %s\n", SERVER_IP);
            						close(client_socket);
            						free(expr);
            						continue;
        				}

								// Connect to server
								if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1){
												fprintf(stderr, "Client thread %d: Error connecting to server: %s (errno: %d) for expression '%s'\n", thread_id, strerror(errno), errno, expr);
												close(client_socket);
												free(expr);
												continue;
								}

								// Send the expression to the server
								ssize_t bytes_sent = send(client_socket, expr, strlen(expr), 0);
								if (bytes_sent == 1){
												printf("Error sending data\n");
								}

								char response[BUFFER];
								memset(response, 0, BUFFER);

								// Handled received data
								ssize_t bytes_received = recv(client_socket, response, BUFFER-1, 0);
								if (bytes_received < 0){
												printf("Error receiving data");
								}

								response[bytes_received] = '\0';
								printf("Expression: %s -> %s\n",expr, response);

								// Close the connection
								close(client_socket);
								free(expr);
			}
			pthread_exit(NULL);
}

int main(){
				// Initialize mutex,conditional variable and queue
				pthread_mutex_init(&mutex, NULL);
				pthread_cond_init(&cond, NULL);

				initialize(&queue);

				FILE* file = fopen(EXPRESSIONS, "r");
				if (file == NULL){
								printf("Error opening file\n");
								return 1;
				}

				char buffer[BUFFER];

				// Load the expressions
				while (fgets(buffer,BUFFER,file) != NULL){
								if (strlen(buffer) > 0){
												pthread_mutex_lock(&mutex);
            						enqueue(&queue, buffer);
            						pthread_mutex_unlock(&mutex);
            						pthread_cond_signal(&cond);
								}
				}

				fclose(file);

				// Signal that expressions have been loaded
				pthread_mutex_lock(&mutex);
				expressions_loaded = 1;
				pthread_mutex_unlock(&mutex);
				pthread_cond_broadcast(&cond);

				// Create worker threads
				pthread_t client_threads[MAX_THREADS];
				for (int i = 0; i<MAX_THREADS; i++){
								pthread_create(&client_threads[i], NULL, worker_function, (void*)(long)(i+1));
				}

				// Join all worker threads
				for (int i = 0; i<MAX_THREADS; i++){
								pthread_join(client_threads[i], NULL);
				}

				// Destroy mutex and conditional variable
				pthread_mutex_destroy(&mutex);
    		pthread_cond_destroy(&cond);

				printf("Client shutdown..\n");

				return 0;
}

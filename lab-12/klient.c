#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#define BUFFER_SIZE 256

int server_socket = -1;
bool running = true;
char client_name[32];
struct sockaddr_in server_addr;
socklen_t server_len;

// Obsługa sygnału Ctrl+C
void handle_sigint(int sig) {
    printf("\nRozłączanie...\n");
    if (server_socket != -1) {
        char stop_msg[10] = "STOP";
        sendto(server_socket, stop_msg, strlen(stop_msg), 0, 
              (struct sockaddr*)&server_addr, server_len);
        close(server_socket);
    }
    running = false;
    exit(0);
}

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    while (running) {
        bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE-1, 0, 
                                 (struct sockaddr*)&from_addr, &from_len);
        
        if (bytes_received <= 0) {
            continue;
        }
        
        buffer[bytes_received] = '\0';
        
        if (strcmp(buffer, "PING") == 0) {
            sendto(server_socket, "PING", 4, 0, (struct sockaddr*)&server_addr, server_len);
        } else {
            printf("%s\n", buffer);
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Użycie: %s <nazwa_klienta> <adres_serwera> <port>\n", argv[0]);
        return 1;
    }
    
    strncpy(client_name, argv[1], sizeof(client_name)-1);
    client_name[sizeof(client_name)-1] = '\0';
    
    char* server_address = argv[2];
    int port = atoi(argv[3]);
    
    signal(SIGINT, handle_sigint);
    
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_address, &server_addr.sin_addr);
    server_len = sizeof(server_addr);
    
    printf("Łączenie z serwerem UDP. Rejestracja jako %s...\n", client_name);
    
    // Wysłanie wiadomości rejestrującej
    char register_msg[BUFFER_SIZE];
    sprintf(register_msg, "REGISTER:%s", client_name);
    sendto(server_socket, register_msg, strlen(register_msg), 0, 
          (struct sockaddr*)&server_addr, server_len);
    
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, NULL);

    char input[BUFFER_SIZE];
    while (running) {
        printf("> ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            continue;
        }
        
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
            len--;
        }
        
        if (sendto(server_socket, input, strlen(input), 0, 
                  (struct sockaddr*)&server_addr, server_len) <= 0) {
            printf("Błąd wysyłania wiadomości\n");
            continue;
        }
        
        if (strcmp(input, "STOP") == 0) {
            printf("Rozłączanie...\n");
            running = false;
            break;
        }
    }
    
    if (running) {
        pthread_cancel(receive_thread);
    }
    
    if (server_socket != -1) {
        close(server_socket);
    }
    
    return 0;
}
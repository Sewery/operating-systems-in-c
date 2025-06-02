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

// Zmienne globalne
int server_socket = -1;
bool running = true;
char client_name[32];

// Obsługa sygnału Ctrl+C
void handle_sigint(int sig) {
    printf("\nRozłączanie...\n");
    if (server_socket != -1) {
        // Wysłanie komunikatu o rozłączeniu
        char stop_msg[10] = "STOP";
        write(server_socket, stop_msg, strlen(stop_msg));
        close(server_socket);
    }
    running = false;
    exit(0);
}

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    while (running) {
        bytes_received = read(server_socket, buffer, BUFFER_SIZE-1);
        
        if (bytes_received <= 0) {
            printf("\nUtracono połączenie z serwerem\n");
            running = false;
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        if (strcmp(buffer, "PING") == 0) {
            write(server_socket, "PING", 4);
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
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Błąd tworzenia gniazda");
        return 1;
    }
    
    // Konfiguracja adresu serwera
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Nieprawidłowy adres serwera");
        close(server_socket);
        return 1;
    }
    
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Nie udało się połączyć z serwerem");
        close(server_socket);
        return 1;
    }
    
    printf("Połączono z serwerem. Wysyłanie nazwy klienta...\n");
    
    write(server_socket, client_name, strlen(client_name));
    
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Błąd tworzenia wątku");
        close(server_socket);
        return 1;
    }

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
        
        if (len == 0) {
            continue;
        }
        
        // Wysłanie wiadomości do serwera
        if (write(server_socket, input, len) <= 0) {
            printf("Błąd wysyłania wiadomości\n");
            break;
        }
        
        // Sprawdzenie czy to komenda STOP
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
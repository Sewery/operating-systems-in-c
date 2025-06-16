#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256
#define CLIENT_NAME_SIZE 32
#define PING_INTERVAL 30 

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
    char name[CLIENT_NAME_SIZE];
    bool active;
    time_t last_response;
} Client;

static Client clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool server_running = true;
static int server_socket;

pthread_t IO_manager;
pthread_t clients_manager;
pthread_t ping_manager;

// Funkcja wysyłająca wiadomość do klienta
void send_to_client(int client_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id].active) {
        sendto(server_socket, message, strlen(message), 0,
               (struct sockaddr*)&clients[client_id].addr, clients[client_id].addr_len);
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Funkcja wysyłająca wiadomość do wszystkich klientów
void send_to_all(int sender_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != sender_id) {
            sendto(server_socket, message, strlen(message), 0,
                  (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Znajduje klienta po adresie
int find_client_by_addr(struct sockaddr_in *addr) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr && 
            clients[i].addr.sin_port == addr->sin_port) {
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

// Rejestruje nowego klienta
int register_client(struct sockaddr_in *addr, socklen_t addr_len, char *name) {
    pthread_mutex_lock(&clients_mutex);
    int client_id = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            client_id = i;
            memcpy(&clients[i].addr, addr, sizeof(struct sockaddr_in));
            clients[i].addr_len = addr_len;
            strncpy(clients[i].name, name, CLIENT_NAME_SIZE-1);
            clients[i].name[CLIENT_NAME_SIZE-1] = '\0';
            clients[i].active = true;
            clients[i].last_response = time(NULL);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return client_id;
}

// Wątek obsługujący wiadomości od klientów
void *fun_clients(void *arg) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (server_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recvfrom(server_socket, buffer, BUFFER_SIZE-1, 0, 
                            (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes <= 0) continue;
        
        buffer[bytes] = '\0';
        
        // Sprawdzenie czy to rejestracja nowego klienta
        if (strncmp(buffer, "REGISTER:", 9) == 0) {
            char name[CLIENT_NAME_SIZE];
            strncpy(name, buffer + 9, CLIENT_NAME_SIZE-1);
            name[CLIENT_NAME_SIZE-1] = '\0';
            
            int client_id = register_client(&client_addr, client_len, name);
            
            if (client_id >= 0) {
                char welcome[BUFFER_SIZE];
                sprintf(welcome, "Zarejestrowano jako: %s (ID: %d)", name, client_id);
                sendto(server_socket, welcome, strlen(welcome), 0, 
                      (struct sockaddr*)&client_addr, client_len);
                
                printf("Nowy klient %s (ID: %d) połączony\n", name, client_id);
            } else {
                sendto(server_socket, "Serwer jest pełny. Spróbuj później.", 35, 0,
                      (struct sockaddr*)&client_addr, client_len);
            }
            continue;
        }
        
        // Znajdź klienta po adresie
        int client_id = find_client_by_addr(&client_addr);
        if (client_id < 0) {
            continue;
        }
        
        // Aktualizacja czasu ostatniej odpowiedzi
        pthread_mutex_lock(&clients_mutex);
        clients[client_id].last_response = time(NULL);
        pthread_mutex_unlock(&clients_mutex);
        
        // Obsługa komend od klienta
        if (strncmp(buffer, "PING", 4) == 0) {
            sendto(server_socket, "PONG", 4, 0, (struct sockaddr*)&client_addr, client_len);
        } 
        else if (strncmp(buffer, "STOP", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            if (clients[client_id].active) {
                clients[client_id].active = false;
                printf("Klient %s (ID: %d) rozłączony\n", clients[client_id].name, client_id);
            }
            pthread_mutex_unlock(&clients_mutex);
        }
        else if (strncmp(buffer, "LIST", 4) == 0) {
            // Wysłanie listy aktywnych klientów
            char list_buffer[BUFFER_SIZE] = "Aktywni klienci:\n";
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    char client_info[100];
                    sprintf(client_info, "ID %d: %s\n", i, clients[i].name);
                    strcat(list_buffer, client_info);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            sendto(server_socket, list_buffer, strlen(list_buffer), 0, 
                  (struct sockaddr*)&client_addr, client_len);
        }
        else if (strncmp(buffer, "2ALL ", 5) == 0) {
            // Wysłanie wiadomości do wszystkich
            time_t current_time = time(NULL);
            struct tm *time_info = localtime(&current_time);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
            
            pthread_mutex_lock(&clients_mutex);
            char sender_name[CLIENT_NAME_SIZE];
            strcpy(sender_name, clients[client_id].name);
            pthread_mutex_unlock(&clients_mutex);
            
            char message[BUFFER_SIZE];
            sprintf(message, "[%s] %s do wszystkich: %s", time_str, sender_name, buffer + 5);
            send_to_all(client_id, message);
        }
        else if (strncmp(buffer, "2ONE ", 5) == 0) {
            // Wysłanie wiadomości do konkretnego klienta
            int target_id;
            char msg[BUFFER_SIZE];
            sscanf(buffer + 5, "%d %[^\n]", &target_id, msg);
            
            if (target_id >= 0 && target_id < MAX_CLIENTS) {
                pthread_mutex_lock(&clients_mutex);
                bool target_active = clients[target_id].active;
                char sender_name[CLIENT_NAME_SIZE];
                strcpy(sender_name, clients[client_id].name);
                pthread_mutex_unlock(&clients_mutex);
                
                if (target_active) {
                    time_t current_time = time(NULL);
                    struct tm *time_info = localtime(&current_time);
                    char time_str[20];
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
                    
                    char message[BUFFER_SIZE];
                    sprintf(message, "[%s] Prywatna wiadomość od %s: %s", time_str, sender_name, msg);
                    send_to_client(target_id, message);
                    
                    sendto(server_socket, "Wiadomość wysłana", 18, 0, 
                          (struct sockaddr*)&client_addr, client_len);
                } 
            } else {
                sendto(server_socket, "Nieprawidłowe ID klienta", 25, 0, 
                      (struct sockaddr*)&client_addr, client_len);
            }
        }
    }
    
    return NULL;
}

void *fun_IO(void *arg) {
    while (server_running) {
        char line[BUFFER_SIZE];
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }
    
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strcmp(line, "LIST") == 0) {
            // Wyświetlenie listy aktywnych klientów
            pthread_mutex_lock(&clients_mutex);
            printf("Aktywni klienci:\n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(clients[i].addr.sin_addr), ip, INET_ADDRSTRLEN);
                    printf("ID %d: %s (%s:%d)\n", i, clients[i].name, ip, ntohs(clients[i].addr.sin_port));
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        } 
        else if (strncmp(line, "2ALL ", 5) == 0) {
            // Wysłanie wiadomości do wszystkich klientów
            time_t current_time = time(NULL);
            struct tm *time_info = localtime(&current_time);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
            
            char message[BUFFER_SIZE];
            sprintf(message, "[%s] SERWER do wszystkich: %s", time_str, line + 5);
            send_to_all(-1, message);
            printf("Wysłano wiadomość do wszystkich klientów\n");
        } 
        else if (strncmp(line, "2ONE ", 5) == 0) {
            int client_id;
            char message[BUFFER_SIZE];
            sscanf(line + 5, "%d %[^\n]", &client_id, message);
            
            if (client_id >= 0 && client_id < MAX_CLIENTS) {
                pthread_mutex_lock(&clients_mutex);
                bool is_active = clients[client_id].active;
                char client_name[CLIENT_NAME_SIZE];
                strcpy(client_name, clients[client_id].name);
                pthread_mutex_unlock(&clients_mutex);
                
                if (is_active) {
                    time_t current_time = time(NULL);
                    struct tm *time_info = localtime(&current_time);
                    char time_str[20];
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
                    
                    char formatted_message[BUFFER_SIZE];
                    sprintf(formatted_message, "[%s] SERWER: %s", time_str, message);
                    send_to_client(client_id, formatted_message);
                    printf("Wysłano wiadomość do klienta %s (ID: %d)\n", client_name, client_id);
                } else {
                    printf("Klient o ID %d nie jest aktywny\n", client_id);
                }
            } else {
                printf("Nieprawidłowy identyfikator klienta\n");
            }
        } 
        else if (strcmp(line, "ALIVE") == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    printf("Klient %d (%s) jest aktywny\n", i, clients[i].name);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        } 
        else if (strcmp(line, "STOP") == 0) {
            printf("Zatrzymywanie serwera...\n");
            server_running = false;
            break;
        }
        else {
            printf("Nieznana komenda. Dostępne komendy: LIST, 2ALL, 2ONE, ALIVE, STOP\n");
        }
    }
    
    return NULL;
}

// Wątek pingujący klientów
void *ping_clients(void *arg) {
    while (server_running) {
        sleep(PING_INTERVAL);
        
        time_t current_time = time(NULL);
        
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                if (current_time - clients[i].last_response > PING_INTERVAL * 2) {
                    printf("Klient %s (ID: %d) nie odpowiada, rozłączam\n", clients[i].name, i);
                    clients[i].active = false;
                } else {
                    if (sendto(server_socket, "PING", 4, 0, 
                              (struct sockaddr*)&clients[i].addr, clients[i].addr_len) <= 0) {
                        printf("Klient %s (ID: %d) rozłączony (błąd wysyłania)\n", clients[i].name, i);
                        clients[i].active = false;
                    }
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Użycie: %s <port>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = false;
    }
    
    // Tworzenie gniazda serwera UDP
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    printf("Serwer UDP uruchomiony na porcie %d\n", port);
    
    pthread_create(&clients_manager, NULL, fun_clients, NULL);
    pthread_create(&IO_manager, NULL, fun_IO, NULL);
    pthread_create(&ping_manager, NULL, ping_clients, NULL);
    
    pthread_join(IO_manager, NULL);
    

    pthread_cancel(clients_manager);
    pthread_cancel(ping_manager);
    

    close(server_socket);
    printf("Serwer zakończył pracę\n");
    
    return 0;
}
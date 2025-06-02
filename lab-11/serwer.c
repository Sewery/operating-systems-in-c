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
#define PING_INTERVAL 30 // 30 sekund między pingami

typedef struct {
    int socket;
    char name[CLIENT_NAME_SIZE];
    bool active;
    time_t last_response;
} Client;

static Client clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool server_running = true;

pthread_t IO_manager;
pthread_t clients_manager;
pthread_t ping_manager;

// Funkcja wysyłająca wiadomość do klienta
void send_to_client(int client_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id].active) {
        write(clients[client_id].socket, message, strlen(message));
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Funkcja wysyłająca wiadomość do wszystkich klientów
void send_to_all(int sender_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != sender_id) {
            write(clients[i].socket, message, strlen(message));
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *fun_client(void *arg) {
    int client_id = *((int*)arg);
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    // Potwierdzenie rejestracji
    char welcome[BUFFER_SIZE];
    sprintf(welcome, "Zarejestrowano jako: %s (ID: %d)\n", clients[client_id].name, client_id);
    write(clients[client_id].socket, welcome, strlen(welcome));
    
    while (server_running) {
        bytes_received = read(clients[client_id].socket, buffer, BUFFER_SIZE-1);
        if (bytes_received <= 0) {
            break;  // Klient rozłączony
        }
        
        buffer[bytes_received] = '\0';
        
        // Aktualizacja czasu ostatniej odpowiedzi
        pthread_mutex_lock(&clients_mutex);
        clients[client_id].last_response = time(NULL);
        pthread_mutex_unlock(&clients_mutex);
        
        // Obsługa komend od klienta
        if (strncmp(buffer, "PING", 4) == 0) {
            write(clients[client_id].socket, "PONG", 4);
        } 
        else if (strncmp(buffer, "STOP", 4) == 0) {
            break; 
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
            write(clients[client_id].socket, list_buffer, strlen(list_buffer));
        }
        else if (strncmp(buffer, "2ALL ", 5) == 0) {
            // Wysłanie wiadomości do wszystkich
            time_t current_time = time(NULL);
            struct tm *time_info = localtime(&current_time);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
            
            char message[BUFFER_SIZE];
            sprintf(message, "[%s] %s do wszystkich: %s", time_str, clients[client_id].name, buffer + 5);
            send_to_all(client_id, message);
        }
        else if (strncmp(buffer, "2ONE ", 5) == 0) {
            // Wysłanie wiadomości do konkretnego klienta
            int target_id;
            char msg[BUFFER_SIZE];
            
            if (target_id >= 0 && target_id < MAX_CLIENTS) {
                pthread_mutex_lock(&clients_mutex);
                if (clients[target_id].active) {
                    time_t current_time = time(NULL);
                    struct tm *time_info = localtime(&current_time);
                    char time_str[20];
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
                    
                    char message[BUFFER_SIZE];
                    sprintf(message, "[%s] Prywatna wiadomość od %s: %s", time_str, clients[client_id].name, msg);
                    write(clients[target_id].socket, message, strlen(message));
                    
                    write(clients[client_id].socket, "Wiadomość wysłana\n", 19);
                } else {
                    write(clients[client_id].socket, "Klient o podanym ID nie jest aktywny\n", 38);
                }
                pthread_mutex_unlock(&clients_mutex);
            } else {
                write(clients[client_id].socket, "Nieprawidłowe ID klienta\n", 26);
            }
    }
    }
    
    // Zakończenie pracy klienta
    pthread_mutex_lock(&clients_mutex);
    if (clients[client_id].active) {
        close(clients[client_id].socket);
        clients[client_id].active = false;
        printf("Klient %s (ID: %d) rozłączony\n", clients[client_id].name, client_id);
    }
    pthread_mutex_unlock(&clients_mutex);
    
    return NULL;
}

// Wątek obsługujący nowe połączenia klientów
void *fun_clients(void *arg) {
    int server_socket = *((int*)arg);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (server_running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Błąd accept");
            continue;
        }
        char name_buffer[CLIENT_NAME_SIZE];
        int bytes = read(client_socket, name_buffer, CLIENT_NAME_SIZE-1);
        if (bytes <= 0) {
            close(client_socket);
            continue;
        }
        name_buffer[bytes] = '\0';
        

        pthread_mutex_lock(&clients_mutex);
        int client_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                client_id = i;
                clients[i].socket = client_socket;
                strcpy(clients[i].name, name_buffer);
                clients[i].active = true;
                clients[i].last_response = time(NULL);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        
        if (client_id == -1) {
            // Brak wolnych miejsc
            char *msg = "Serwer jest pełny. Spróbuj później.\n";
            write(client_socket, msg, strlen(msg));
            close(client_socket);
        } else {
            printf("Nowy klient %s (ID: %d) połączony\n", name_buffer, client_id);
            
            // Utworzenie wątku obsługującego klienta
            pthread_t client_thread;
            int *client_id_ptr = malloc(sizeof(int));
            *client_id_ptr = client_id;
            pthread_create(&client_thread, NULL, fun_client, client_id_ptr);
            pthread_detach(client_thread);
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
                    printf("ID %d: %s (socket: %d)\n", i, clients[i].name, clients[i].socket);
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
            sprintf(message, "[%s] SERWER do wszystkich: %s\n", time_str, line + 5);
            send_to_all(-1, message);
            printf("Wysłano wiadomość do wszystkich klientów\n");
        } 
        else if (strncmp(line, "2ONE ", 5) == 0) {
            int client_id;
            char message[BUFFER_SIZE];
           
                if (client_id >= 0 && client_id < MAX_CLIENTS) {
                    pthread_mutex_lock(&clients_mutex);
                    if (clients[client_id].active) {
                        time_t current_time = time(NULL);
                        struct tm *time_info = localtime(&current_time);
                        char time_str[20];
                        strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
                        
                        char formatted_message[BUFFER_SIZE];
                        sprintf(formatted_message, "[%s] SERWER: %s\n", time_str, message);
                        write(clients[client_id].socket, formatted_message, strlen(formatted_message));
                        printf("Wysłano wiadomość do klienta %s (ID: %d)\n", clients[client_id].name, client_id);
                    } else {
                        printf("Klient o ID %d nie jest aktywny\n", client_id);
                    }
                    pthread_mutex_unlock(&clients_mutex);
                } else {
                    printf("Nieprawidłowy identyfikator klienta\n");
                }
        
        } 
        else if (strcmp(line, "ALIVE") == 0) {
            // Sprawdzenie, czy klienci są aktywni
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    printf("Klient %d (%s) jest aktywny\n", i, clients[i].name);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        } 
        else if (strcmp(line, "STOP") == 0) {
            // Zakończenie działania serwera
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
                // Sprawdzenie czy klient odpowiedział w ostatnim interwale
                if (current_time - clients[i].last_response > PING_INTERVAL * 2) {
                    // Klient nie odpowiedział, rozłączamy go
                    printf("Klient %s (ID: %d) nie odpowiada, rozłączam\n", clients[i].name, i);
                    close(clients[i].socket);
                    clients[i].active = false;
                } else {
                    // Wysłanie pinga
                    if (write(clients[i].socket, "PING", 4) <= 0) {
                        // Błąd wysyłania, klient prawdopodobnie odłączony
                        printf("Klient %s (ID: %d) rozłączony (błąd wysyłania)\n", clients[i].name, i);
                        close(clients[i].socket);
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
    
    // Inicjalizacja tablicy klientów
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = false;
    }
    
    // Tworzenie gniazda serwera
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Błąd tworzenia gniazda");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Błąd bind");
        close(server_socket);
        return 1;
    }
    
    if (listen(server_socket, 5) == -1) {
        perror("Błąd listen");
        close(server_socket);
        return 1;
    }
    
    printf("Serwer uruchomiony na porcie %d\n", port);
    
    // Utworzenie wątków
    pthread_create(&clients_manager, NULL, fun_clients, &server_socket);
    pthread_create(&IO_manager, NULL, fun_IO, NULL);
    pthread_create(&ping_manager, NULL, ping_clients, NULL);
    
    // Oczekiwanie na zakończenie wątku IO
    pthread_join(IO_manager, NULL);
    
    // Zatrzymanie pozostałych wątków
    pthread_cancel(clients_manager);
    pthread_cancel(ping_manager);
    
    // Zamknięcie gniazda serwera
    close(server_socket);
    printf("Serwer zakończył pracę\n");
    
    return 0;
}
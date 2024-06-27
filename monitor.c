#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#define PORT 8081

void login_user(const char *username, const char *password) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024] = {0};
    char current_username[256];

    strncpy(current_username, username, sizeof(current_username) - 1);
    current_username[sizeof(current_username) - 1] = '\0';

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    snprintf(message, sizeof(message), "MONITOR LOGIN %s %s", username, password);
    send(sock, message, strlen(message), 0);
    valread = read(sock, buffer, 1024);
    buffer[valread] = '\0';
    printf("%s\n", buffer);

    if (strstr(buffer, "berhasil login")) {
        while (1) {
            // Read message from server
            valread = read(sock, buffer, 1024);
            buffer[valread] = '\0';
            //printf("%s\n", buffer);
            if (strstr(buffer, "exit")) {
                break;
            }

            if (strstr(buffer, "-channel") && strstr(buffer, "-room")) {
                // Send the input to the server
                send(sock, buffer, strlen(buffer), 0);
                valread = read(sock, buffer, 1024);
                buffer[valread] = '\0';
                // memset(buffer, 0, sizeof(buffer));
                //printf("%s\n", buffer);
                // Extract channel name and room name
                char channel_name[256], room_name[256];
                sscanf(buffer, "%*s %255s %*s %255s", channel_name, room_name);
                printf("Channel name: %s, Room name: %s\n", channel_name, room_name);
                
                // Read chat.csv file
                char file_path[512];
                snprintf(file_path, sizeof(file_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s/chat.csv", channel_name, room_name);
                FILE *file = fopen(file_path, "r");
                if (file == NULL) {
                    printf("Failed to open chat.csv file\n");
                    continue;
                }

                struct stat st;
                off_t last_size = 0;
                while (1) {
                    valread = read(sock, buffer, 1024);
                    if (valread > 0) {
                        buffer[valread] = '\0';
                        if (strstr(buffer, "EXIT")) {
                            break;
                        }
                    }

                    if (stat(file_path, &st) == 0) {
                        if (st.st_size > last_size) {
                            fseek(file, last_size, SEEK_SET); 
                            char line[1024];
                            while (fgets(line, sizeof(line), file)) {
                                printf("%s", line);
                            }
                            last_size = st.st_size;
                        }
                    }
                    
                    sleep(1); 
                }
                fclose(file);
                close(sock); 
                break; 
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 5 || (strcmp(argv[1], "LOGIN") != 0) || strcmp(argv[3], "-p") != 0) {
        printf("Usage:\n");
        printf("Login: %s LOGIN username -p password\n", argv[0]);
        return 1;
    }

    const char *command = argv[1];
    const char *username = argv[2];
    const char *password = argv[4];

    if (strcmp(command, "LOGIN") == 0) {
        login_user(username, password);
    }

    return 0;
}

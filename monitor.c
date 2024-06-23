#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define PORT 8081

void login_user(const char *username, const char *password) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024] = {0};
    char current_username[256];

    strncpy(current_username, username, sizeof(current_username) - 1); // 
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
        char current_channel[256] = {0};
        char current_room[256] = {0};
        while (1) {
            // ubah nama
            if(strstr(buffer, "Profil diupdate menjadi")){
                char *new_username = strstr(buffer, "Profil diupdate menjadi ") + strlen("Profil diupdate menjadi ");
                strcpy(current_username, new_username);
            } 
            
            // format path
            if (strlen(current_channel) == 0 && strlen(current_room) == 0) {
                printf("[%s]> ", current_username);
            } else if (strlen(current_channel) != 0 && strlen(current_room) == 0) {
                printf("[%s/%s]> ", current_username, current_channel);
            } else {
                printf("[%s/%s/%s]> ", current_username, current_channel, current_room);
            }

            // baca inout user
            char input[1024];
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0; // Remove newline character

            if(strstr(input, "-channel") && strstr(input, "-room")){
                
                // Send the input to the server
                send(sock, input, strlen(input), 0);
                valread = read(sock, buffer, 1024);
                buffer[valread] = '\0';
                printf("%s\n", buffer);
                // ambil nama channel dan room
                char channel_name[256], room_name[256];
                sscanf(input, "%*s %255s %*s %255s", channel_name, room_name);
                
                strcpy(current_channel, channel_name);
                strcpy(current_room, room_name);
                continue;
            } else if(strstr(input, "EXIT")){
                send(sock, input, strlen(input), 0);
                valread = read(sock, buffer, 1024);
                buffer[valread] = '\0';
                //printf("%s\n", buffer);
                if (strstr(buffer, "exit room")){
                    strcpy(current_room, "");
                    continue;
                } else if (strstr(buffer, "exit channel")){
                    strcpy(current_channel, "");
                    continue;
                } else {
                    break;
                }
            } else {
                printf("Invalid command. Try on discorit\n");
                continue;
            }

        }
        return;
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

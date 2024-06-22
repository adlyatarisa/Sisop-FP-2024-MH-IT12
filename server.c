#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#define PORT 8081

#define USERS_FILE "/Users/tarisa/smt-2/sisop/FP/discorit/users.csv"
#define CHANNELS_FILE "/Users/tarisa/smt-2/sisop/FP/discorit/channels.csv"
#define USER_LOG "/Users/tarisa/smt-2/sisop/FP/discorit/users.log"
#define ROLE_ROOT "ROOT"
#define ROLE_ADMIN "ADMIN"
#define ROLE_USER "USER"

typedef struct {
    char username[256];
    char password[256];
    char role[256];
    int id;
    int in_channel;
    char cur_channel[256];
    int in_room;
} User;

typedef struct {
    int id;
    char name[256];
    char key[256];
    char admin[256];
} Channel;

int user_exists(const char *username) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {

        char file_username[256];
        sscanf(line, "%*[^,], %255[^,]", file_username);
        if (strcmp(file_username, username) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int getIDuser() { 
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        return 1;  // Mulai dari 1 jika file tidak ada
    }

    char line[1024];
    int max_id = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        sscanf(line, "%d", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(file);
    return max_id + 1;
}

int getIDuser_channel(const char *channel_name) {
    char channel_path[256];
    snprintf(channel_path, sizeof(channel_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(channel_path, "r");
    if (!file) {
        return 1;
    }

    char line[1024];
    int max_id = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        sscanf(line, "%d", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(file);
    return max_id + 1;
}

int register_user(const char *username, const char *password) {
    if (user_exists(username)) {
        return 0;
    }

    FILE *file = fopen(USERS_FILE, "a");
    if (!file) {
        perror("Could not open users file");
        return -1;
    }

    const char *role = ROLE_USER;
    // kalo file masih kosong, user pertama jadi root
    if (ftell(file) == 0) {
        role = ROLE_ROOT;
    }
    int user_id = getIDuser();
    fprintf(file, "%d,%s,%s,%s\n", user_id, username, password, role);
    fclose(file);

    return 1;
}

int verify_user(const char *username, const char *password, User *user) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int user_id;
        char file_username[256], file_password[256], file_role[256];
        sscanf(line, "%d, %255[^,],%255[^,],%255[^,]", &user_id, file_username, file_password, file_role);
        if (strcmp(file_username, username) == 0 && strcmp(file_password, password) == 0) {
            user->id = user_id;
            strcpy(user->username, file_username);
            strcpy(user->password, file_password);
            strcpy(user->role, file_role);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

void list_channels(int socket) {
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) {
        send(socket, "No channels.csv available\n", strlen("No channels.csv available\n"), 0);
        return;
    }

    char line[1024];
    char response[4096] = {0};
    int empty = 1; // flag to check if the file is empty
    while (fgets(line, sizeof(line), file)) {
        empty = 0; // file is not empty
        char channel_id[256], channel_name[256], channel_key[256];
        sscanf(line, "%255[^,],%255[^,],%255[^,]", channel_id, channel_name, channel_key);
        strcat(response, channel_name);
        strcat(response, " ");
    }

    if (empty) {
        send(socket, "No channels available\n", strlen("No channels available\n"), 0);
    } else {
        send(socket, response, strlen(response), 0);
    }

    fclose(file);
}

void list_rooms(int socket, const char *channel_name){
    char channel_path[256];
    snprintf(channel_path, sizeof(channel_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", channel_name);

    DIR *dir = opendir(channel_path);
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *ent;
    char response[4096] = {0};
    int found = 0;
    
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            if (strcmp(ent->d_name, "admin") == 0){
                continue;
            }
            strcat(response, ent->d_name);
            strcat(response, " ");
            found = 1;
        }
    } 
    if (!found) {
        strcat(response, "Tidak ada room yang tersedia\n");
    }
    closedir(dir);
    send(socket, response, strlen(response), 0);
}

//list all user
void list_users(int socket){
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        send(socket, "No users available\n", strlen("No users available\n"), 0);
        return;
    }

    char line[1024];
    char response[4096] = {0};
    while (fgets(line, sizeof(line), file)) {
        char username[256];
        sscanf(line, "%*[^,],%255[^,],%*[^,],%*[^,]", username);
        strcat(response, username);
        strcat(response, " ");
    }

    send(socket, response, strlen(response), 0);
    fclose(file);
}

//list user channel
void list_channel_users(int socket, const char *channel_name){
    char channel_path[256];
    snprintf(channel_path, sizeof(channel_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(channel_path, "r");
    if (!file) {
        send(socket, "No users available\n", strlen("No users available\n"), 0);
        return;
    }

    char line[1024];
    char response[4096] = {0};
    while (fgets(line, sizeof(line), file)) {
        char user[256];
        sscanf(line, "%*[^,],%255[^,],%*[^,]", user);
        strcat(response, user);
        strcat(response, " ");
    }

    send(socket, response, strlen(response), 0);
    fclose(file);
}

void log_user_activity(const char *username, const char *activity, const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Could not open user log directory");
        return;
    }

    time_t now = time(NULL);
    char time_str[256];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char log_file[512];
    snprintf(log_file, sizeof(log_file), "%s/users.log", path);
    FILE *file = fopen(log_file, "a");
    if (!file) {
        perror("Could not open user log file");
        closedir(dir);
        return;
    }

    fprintf(file, "[%s] %s\n", time_str, activity);
    fclose(file);
    closedir(dir);
}

int getIDchannel() {
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) {
        return 1;  // Mulai dari 1 jika file tidak ada
    }

    char line[1024];
    int max_id = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        sscanf(line, "%d", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(file);
    return max_id + 1;
}

int create_channel(const char *channel_name, const char *username, const char *key) {
    int channel_id = getIDchannel();

    FILE *file = fopen(CHANNELS_FILE, "a");
    if (!file) {
        perror("Could not open channels file");
        return 0;
    }

    fprintf(file, "%d,%s,%s\n", channel_id, channel_name, key);
    fclose(file);

    // Create a directory for the channel
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", channel_name);

    if (mkdir(dir_path, 0777) == -1) {
        perror("Could not create directory for channel");
        return 0;
    } else{
        printf("Channel created successfully\n");
    }

    //buat folder admin sama log
    char admin_path[512];
    snprintf(admin_path, sizeof(admin_path), "%s/admin",dir_path);
    if (mkdir(admin_path, 0777) == -1) {
        perror("Could not create directory for admin");
        return 0;
    }

    char activity[256];
    snprintf(activity, sizeof(activity), "%s buat %s", username, channel_name);
    log_user_activity(username, activity, admin_path);

    return 1;
}

int create_room(const char *channel_name, char *room_name, const char *username) {
    char dir_path[512], chat_path[256];
    snprintf(dir_path, sizeof(dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, room_name);
    snprintf(chat_path, sizeof(chat_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s/chat.csv", channel_name, room_name);

    if (mkdir(dir_path, 0777) == -1) {
        perror("Could not create directory for room");
        return 0;
    } else{
    
            printf("Room created successfully\n");
    }

    // catet log
    char admin_path[512], activity[256];
    snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
    snprintf(activity, sizeof(activity), "%s buat %s", username, room_name);
    log_user_activity(username, activity, admin_path);

    return 1;
}

int edit_channel(const char *username, char *old_channel_name, const char *new_channel_name) {
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) {
        perror("Could not open channels file");
        return 0;
    }

    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", CHANNELS_FILE);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char channel_name[256], channel_key[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, channel_name, channel_key);
        if (strcmp(channel_name, old_channel_name) == 0) {
            fprintf(temp_file, "%d,%s,%s", id, new_channel_name, channel_key);
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        // Overwrite the original file with the temporary file
        if (rename(temp_filename, CHANNELS_FILE) != 0) {
            perror("Could not rename temporary file to channels file");
            return 0;
        }
        // Rename the channel directory
        char old_dir_path[512], new_dir_path[512];
        snprintf(old_dir_path, sizeof(old_dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", old_channel_name);
        snprintf(new_dir_path, sizeof(new_dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", new_channel_name);
        if (rename(old_dir_path, new_dir_path) != 0) {
            perror("Could not rename channel directory");
            return 0;
        }

        char activity[256];
        snprintf(activity, sizeof(activity), "Channel %s diubah menjadi %s", old_channel_name, new_channel_name);
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s/admin", new_dir_path);
        log_user_activity(username, activity, log_file);

        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}

int edit_room(const char *username, const char *channel_name, char *old_room_name, const char *new_room_name) {
    char room_path[256];
    snprintf(room_path, sizeof(room_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, old_room_name);
    int found = 0;

    // cek room nya udah ada apa belum?
    DIR *dir = opendir(room_path);
    if (dir) {
        found = 1;
    }

    if (found) {
        // Rename the channel directory
        char old_dir_path[512], new_dir_path[512];
        snprintf(old_dir_path, sizeof(old_dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, old_room_name);
        snprintf(new_dir_path, sizeof(new_dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, new_room_name);
        if (rename(old_dir_path, new_dir_path) != 0) {
            perror("Could not rename room directory");
            return 0;
        }

        char activity[256];
        snprintf(activity, sizeof(activity), "Room %s diubah menjadi %s", old_room_name, new_room_name);
        char channel_path[256];
        snprintf(channel_path, sizeof(channel_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s/admin", channel_path);
        log_user_activity(username, activity, log_file);

        return 1;
    } 

    return 0;
}

int remove_directory(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            remove_directory(filepath);
        } else {
            remove(filepath);
        }
    }
    closedir(dir);
    return rmdir(path);
}

int delete_channel(const char *channel_name) {
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", channel_name);

    // Check if directory exists and remove it
    if (remove_directory(dir_path) == -1) {
        perror("Could not delete channel directory");
        return 0;
    }

    // After successful deletion of the directory, update the CSV file
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) {
        perror("Could not open channels file");
        return 0;
    }

    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", CHANNELS_FILE);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char channel_name_csv[256], channel_key[256], admin_name[256];
        sscanf(line, "%d,%255[^,],%255[^,],%255[^,]", &id, channel_name_csv, channel_key, admin_name);
        if (strcmp(channel_name_csv, channel_name) == 0) {
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, CHANNELS_FILE) != 0) {
            perror("Could not rename temporary file");
            return 0;
        }
        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}

int delete_all_room(const char *channel_name){
    char channel_path[256];
    snprintf(channel_path, sizeof(channel_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", channel_name);

    DIR *dir = opendir(channel_path);
    if (dir == NULL) {
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            if (strcmp(ent->d_name, "admin") == 0){
                continue;
            }
            char room_path[256];
            snprintf(room_path, sizeof(room_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, ent->d_name);
            if(remove_directory(room_path)== -1){
                perror("Could not delete room directory");
                return 0;
            }
            
        }
    }
    closedir(dir);
    return 0;
}


int channel_exists(const char *channel_name) {
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int id;
        char file_channel_name[256], file_channel_key[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, file_channel_name, file_channel_key);
        if (strcmp(file_channel_name, channel_name) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int room_exists(const char *channel_name, char *room_name) {
    char room_path[256];
    snprintf(room_path, sizeof(room_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", channel_name, room_name);
    
    DIR *dir = opendir(room_path);
    if (dir) {
        // room nya ada
        closedir(dir);
        return 1;
    }

    return 0;
}

int is_admin(char *channel_name, int id) {
    char auth_file[512];
    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(auth_file, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int file_id;
        char file_username[256], file_role[256];
        sscanf(line, "%d,%*[^,],%255[^,]", &file_id, file_role);
        file_role[strcspn(file_role, "\r\n")]=0; // remove newline
        if (file_id == id && strcmp(file_role, ROLE_ADMIN) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int is_root(const char *username) {
    FILE *file = fopen("/Users/tarisa/smt-2/sisop/FP/discorit/users.csv", "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char file_username[256], file_role[256];
        sscanf(line, "%*[^,],%255[^,],%*[^,],%255[^,]", file_username, file_role);
        file_role[strcspn(file_role, "\r\n")]=0; // remove newline
        if (strcmp(file_username, username) == 0 && strcmp(file_role, ROLE_ROOT) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int is_member(const char *username, const char *channel_name){
    char auth_file[512];
    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(auth_file, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int file_id;
        char file_username[256], file_role[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &file_id, file_username, file_role);
        file_role[strcspn(file_role, "\r\n")]=0; // remove newline
        printf("%s\n", file_username);
        // kalo ada, maka member
        if (strcmp(username, file_username) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;

}

int edit_username(const char *username, const char *new_username) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        printf("ga ada file nya");
        perror("Could not open users.csv");
        return 0;
    }

    // bikin temp file
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", USERS_FILE);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        printf("gabisa buat temp file");
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char old_username[256], password[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,],%255[^,]", &id, old_username, password, user_role);
        user_role[strcspn(user_role, "\r\n")]=0; // remove newline
        // cek dulu nama baru nya sama kek nama user lain ga
        if (user_exists(new_username)) {
            fclose(file);
            fclose(temp_file);
            return 0;
        } else {
            if (strcmp(username, old_username) == 0) {
                fprintf(temp_file, "%d,%s,%s,%s\n", id, new_username, password, user_role);
                found = 1;
            } else {
                fputs(line, temp_file);
            }
        }
        
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, USERS_FILE) != 0) {
            perror("Could not rename temporary file to user.csv");
            return 0;
        }

        // edit username di setiap file auth.csv di setiap channel
        DIR *dir = opendir("/Users/tarisa/smt-2/sisop/FP/discorit");
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char channel_name[256];
                    snprintf(channel_name, sizeof(channel_name), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", entry->d_name);
                    char auth_file[512];
                    snprintf(auth_file, sizeof(auth_file), "%s/admin/auth.csv", channel_name);
                    FILE *auth = fopen(auth_file, "r");
                    if (auth) {
                        char auth_line[1024];
                        char temp_auth_filename[512];
                        snprintf(temp_auth_filename, sizeof(temp_auth_filename), "%s.tmp", auth_file);
                        FILE *temp_auth = fopen(temp_auth_filename, "w");
                        if (temp_auth) {
                            int auth_found = 0;
                            while (fgets(auth_line, sizeof(auth_line), auth)) {
                                int id;
                                char auth_username[256], user_role[256];
                                sscanf(auth_line, "%d,%255[^,],%255[^,]", &id, auth_username, user_role);
                                user_role[strcspn(user_role, "\r\n")]=0; // remove newline
                                if (strcmp(username, auth_username) == 0) {
                                    fprintf(temp_auth, "%d,%s,%s\n", id, new_username, user_role);
                                    auth_found = 1;
                                } else {
                                    fputs(auth_line, temp_auth);
                                }
                            }
                            fclose(auth);
                            fclose(temp_auth);
                            if (auth_found) {
                                if (rename(temp_auth_filename, auth_file) != 0) {
                                    perror("Could not rename temporary file to auth.csv");
                                    return 0;
                                }
                            } else {
                                remove(temp_auth_filename);
                            }
                        } else {
                            perror("Could not open temporary auth file");
                            fclose(auth);
                            return 0;
                        }
                    }
                }
            }
            closedir(dir);
        } else {
            perror("Could not open directory");
            return 0;
        }
        
    } else {
        remove(temp_filename);
        return 0;
    }
    
    return 1;
}

// edit password user
int edit_password(const char *username, const char *new_password) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        perror("Could not open users.csv");
        return 0;
    }

    // bikin temp file
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", USERS_FILE);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char old_username[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, old_username, user_role);
        if (strcmp(username, old_username) == 0) {
            fprintf(temp_file, "%d,%s,%s", id, old_username, new_password);
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, USERS_FILE) != 0) {
            perror("Could not rename temporary file to user.csv");
            return 0;
        }

        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}

// remove user (cuma root yang bisa)
int remove_user(const char *username) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        perror("Could not open users.csv");
        return 0;
    }

    // bikin temp file
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", USERS_FILE);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char old_username[256], password[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,],%255[^,]", &id, old_username, password, user_role);
        if (strcmp(username, old_username) == 0) {
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, USERS_FILE) != 0) {
            perror("Could not rename temporary file to user.csv");
            return 0;
        }

        // Remove user dari setiap auth.csv di setiap channel
        DIR *dir = opendir("/Users/tarisa/smt-2/sisop/FP/discorit");
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char channel_name[256];
                    snprintf(channel_name, sizeof(channel_name), "/Users/tarisa/smt-2/sisop/FP/discorit/%s", entry->d_name);
                    char auth_file[512];
                    snprintf(auth_file, sizeof(auth_file), "%s/admin/auth.csv", channel_name);
                    FILE *auth = fopen(auth_file, "r");
                    if (auth) {
                        char auth_line[1024];
                        char temp_auth_filename[512];
                        snprintf(temp_auth_filename, sizeof(temp_auth_filename), "%s.tmp", auth_file);
                        FILE *temp_auth = fopen(temp_auth_filename, "w");
                        if (temp_auth) {
                            int auth_found = 0;
                            while (fgets(auth_line, sizeof(auth_line), auth)) {
                                char auth_username[256];
                                sscanf(auth_line, "%*[^,],%255[^,],%*[^,],%*[^,]", auth_username);
                                if (strcmp(username, auth_username) == 0) {
                                    auth_found = 1;
                                } else {
                                    fputs(auth_line, temp_auth);
                                }
                            }
                            fclose(auth);
                            fclose(temp_auth);
                            if (auth_found) {
                                if (rename(temp_auth_filename, auth_file) != 0) {
                                    perror("Could not rename temporary file to auth.csv");
                                    return 0;
                                }
                            } else {
                                remove(temp_auth_filename);
                            }
                        } else {
                            perror("Could not open temporary auth file");
                            fclose(auth);
                            return 0;
                        }
                    }
                }
            }
            closedir(dir);
        } else {
            perror("Could not open directory");
            return 0;
        }

        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}

//ban user
int ban_user(const char *channel_name, const char *username) {
    char auth_file[512];
    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(auth_file, "r");
    if (!file) return 0;

    // bikin temp file
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", auth_file);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char auth_username[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, auth_username, user_role);
        if (strcmp(username, auth_username) == 0) {
            fprintf(temp_file, "%d,%s,%s\n", id, auth_username, "BANNED");
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, auth_file) != 0) {
            perror("Could not rename temporary file to auth.csv");
            return 0;
        }

        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}

// is ban
int is_banned(const char *channel_name, int user_id) {
    char auth_file[512];
    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(auth_file, "r");
    if (!file) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int id;
        char auth_username[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, auth_username, user_role);
        user_role[strcspn(user_role, "\r\n")]=0; // remove newline


        if (id == user_id && strcmp(user_role, "BANNED") == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// unban user
int unban_user(const char *channel_name, const char *username) {
    char auth_file[512];
    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);

    FILE *file = fopen(auth_file, "r");
    if (!file) return 0;

    // bikin temp file
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", auth_file);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        perror("Could not open temporary file");
        fclose(file);
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        char auth_username[256], user_role[256];
        sscanf(line, "%d,%255[^,],%255[^,]", &id, auth_username, user_role);
        user_role[strcspn(user_role, "\r\n")]=0; // remove newline
        if (strcmp(username, auth_username) == 0) {
            fprintf(temp_file, "%d,%s,%s\n", id, auth_username, ROLE_USER);
            found = 1;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        if (rename(temp_filename, auth_file) != 0) {
            perror("Could not rename temporary file to auth.csv");
            return 0;
        }

        return 1;
    } else {
        remove(temp_filename);
        return 0;
    }
}
void handle_client(int new_socket) {
    char buffer[1024] = {0};
    int valread;

    // Read initial command
    valread = read(new_socket, buffer, 1024);
    buffer[valread] = '\0';

    char command[256], username[256], password[256];
    sscanf(buffer, "%255s %255s %255s", command, username, password);

    if (strcmp(command, "REGISTER") == 0) {
        if (register_user(username, password) == 1) {
            send(new_socket, "User registered successfully\n", strlen("User registered successfully\n"), 0);
        } else {
            send(new_socket, "Username already exists\n", strlen("Username already exists\n"), 0);
        }
    } else if (strcmp(command, "LOGIN") == 0) {
        User user;
        Channel channel;
        if (verify_user(username, password, &user)) {
            char welcome_message[256];
            snprintf(welcome_message, sizeof(welcome_message), "%s berhasil login\n", username);
            send(new_socket, welcome_message, strlen(welcome_message), 0);

            while ((valread = read(new_socket, buffer, 1024)) > 0) {
                buffer[valread] = '\0';

                if (strncmp(buffer, "LIST CHANNELS", 13) == 0) { 
                    list_channels(new_socket);
                } else if (strncmp(buffer, "CREATE CHANNEL", 14) == 0) {
                    char channel_name[256], key[256];
                    sscanf(buffer, "%*s %*s %255s -k %255s", channel_name, key);
                    if (create_channel(channel_name, username, key)) {
                        // catet di auth.csv sesuai dengan id di struct
                        // buat auth.c buat nyatet informasi user (admin)
                        char auth_path[512];
                        snprintf(auth_path, sizeof(auth_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);
                        FILE *auth = fopen(auth_path, "a");
                        if (auth) {
                            fprintf(auth, "%d,%s,%s\n", user.id, username, ROLE_ADMIN);
                            fclose(auth);
                        }
                        
                        char response[256];
                        snprintf(response, sizeof(response), "Channel %s dibuat\n", channel_name);
                        send(new_socket, response, strlen(response), 0);
                    } else {
                        send(new_socket, "Gagal membuat channel\n", strlen("Gagal membuat channel\n"), 0);
                    }
                } else if (strncmp(buffer, "EDIT CHANNEL", 12) == 0) {
                    char old_channel_name[256], new_channel_name[256];
                    sscanf(buffer, "%*s %*s %255s %*s %255s", old_channel_name, new_channel_name);
                    printf("%s %s\n", username, old_channel_name);
                    //cek admin bukan?
                    if (is_admin(old_channel_name, user.id)||is_root(user.username)) {
                        if (edit_channel(username, old_channel_name, new_channel_name)) {
                            char response[256];
                            snprintf(response, sizeof(response), "Channel %s diubah menjadi %s\n", old_channel_name, new_channel_name);
                            send(new_socket, response, strlen(response), 0);
                        } else {
                            send(new_socket, "Gagal mengubah channel\n", strlen("Gagal mengubah channel\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda bukan admin channel\n", strlen("Anda bukan admin channel\n"), 0);
                    }

                    
                } else if (strncmp(buffer, "DEL CHANNEL", 11) == 0) {
                    char channel_name[256];
                    sscanf(buffer, "%*s %*s %255s", channel_name);
                    if(is_admin(channel_name, user.id)||is_root(user.username)){
                        if (delete_channel(channel_name)) {
                            char response[256];
                            snprintf(response, sizeof(response), "%s berhasil dihapus\n", channel_name);
                            send(new_socket, response, strlen(response), 0);
                            
                        } else {
                            send(new_socket, "Gagal menghapus channel\n", strlen("Gagal menghapus channel\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda bukan admin channel\n", strlen("Anda bukan admin channel\n"), 0);
                    }
                    
                } else if (strncmp(buffer, "CREATE ROOM", 11) == 0) {
                    char channel_name[256], room_name[256];
                    sscanf(buffer, "%*s %*s %255s", room_name);
                    if (create_room(user.cur_channel, room_name, username)) {
                        char response[256];
                        snprintf(response, sizeof(response), "Room %s dibuat\n", room_name);
                        send(new_socket, response, strlen(response), 0);
                    } else {
                        send(new_socket, "Gagal membuat room\n", strlen("Gagal membuat room\n"), 0);
                    }
                } else if (strncmp(buffer, "EDIT ROOM", 9) == 0) {
                    char channel_name[256], old_room_name[256], new_room_name[256];
                    sscanf(buffer, "%*s %*s %255s %255s %255s", channel_name, old_room_name, new_room_name);
                    if (edit_room(username, channel_name, old_room_name, new_room_name)) {
                        char response[256];
                        snprintf(response, sizeof(response), "Room %s diubah menjadi %s\n", old_room_name, new_room_name);
                        send(new_socket, response, strlen(response), 0);
                    } else {
                        send(new_socket, "Gagal mengubah room\n", strlen("Gagal mengubah room\n"), 0);
                    }
                } else if (strncmp(buffer, "JOIN", 4) == 0) {
                    char channel_name[256];
                    // belum pernah join channel
                    // join channel
                    if(user.in_channel){
                        sscanf(buffer, "%*s %255s", channel_name);
                        if (channel_exists(channel_name)) {
                            // admin/root 
                            if (is_admin(channel_name, user.id)) {
                                printf("ada admin\n");
                                // catet di auth.csv
                                char auth_file[512];
                                snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);
                                // kalo belum member, tulis namanya di auth.csv
                                if (!is_member(user.username, channel_name)){
                                    printf("bukan member\n");
                                    FILE *auth_fp = fopen(auth_file, "a");
                                    if (auth_fp) {
                                        fprintf(auth_fp, "%d,%s,%s\n", user.id, username, ROLE_ROOT);
                                        fclose(auth_fp);
                                    }
                                }
                                // ubah nilai in_channel
                                user.in_channel = 0;
                                strcpy(user.cur_channel, channel_name);
                                // catet log
                                char admin_path[256];
                                snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                char activity[256];
                                snprintf(activity, sizeof(activity), "%s masuk ke channel 'say hi'", username);
                                log_user_activity(username, activity, admin_path);
                                send(new_socket, " ", strlen(" "), 0);
                            } else if (is_root(user.username)){
                                printf("ada root\n");
                                // catet di auth.csv
                                char auth_file[512];
                                snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);
                            
                                // kalo belum member, tulis namanya di auth.csv
                                if (!is_member(user.username, channel_name)){
                                    printf("bukan member\n");
                                    FILE *auth_fp = fopen(auth_file, "a");
                                    if (auth_fp) {
                                        fprintf(auth_fp, "%d,%s,%s\n", user.id, username, ROLE_ROOT);
                                        fclose(auth_fp);
                                    }
                                }

                                // ubah nilai in_channel
                                user.in_channel = 0;
                                strcpy(user.cur_channel, channel_name);
                                // catet log
                                char admin_path[256];
                                snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                char activity[256];
                                snprintf(activity, sizeof(activity), "%s masuk ke channel 'say hi'", username);
                                log_user_activity(username, activity, admin_path);
                                send(new_socket, " ", strlen(" "), 0);
                            } else { //bukan admin/root
                                // user udah pernah join sebelumnya (member)
                                if (is_banned(channel_name, user.id)){
                                    send(new_socket, "Anda dibanned\n", strlen("Anda dibanned\n"), 0);
                                } else {
                                    if(is_member(user.username, channel_name)){
                                        printf("ada member\n");
                                        // ubah nilai in_channel
                                        user.in_channel = 0;
                                        strcpy(user.cur_channel, channel_name);
                                        // catet log
                                        char admin_path[256];
                                        snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                        char activity[256];
                                        snprintf(activity, sizeof(activity), "%s masuk ke channel 'say hi'", username);
                                        log_user_activity(username, activity, admin_path);
                                        send(new_socket, " ", strlen(" "), 0);
                                    } else {
                                        printf("ada orang baru\n");
                                        // user belum pernah join sebelumnya (bukan member)
                                        char response[256];
                                        snprintf(response, sizeof(response), "Key: ");
                                        send(new_socket, response, strlen(response), 0);
                                        
                                        // baca key dari client
                                        valread = read(new_socket, buffer, 1024);
                                        buffer[valread] = '\0';
                                        char key[256];
                                        sscanf(buffer, "%255s", key);

                                        // Verify key
                                        FILE *file = fopen(CHANNELS_FILE, "r");
                                        int key_valid = 0;
                                        if (file) {
                                            char line[1024];
                                            while (fgets(line, sizeof(line), file)) {
                                                int id;
                                                char file_channel_name[256], file_channel_key[256];
                                                sscanf(line, "%d,%255[^,],%255[^,]", &id, file_channel_name, file_channel_key);
                                                file_channel_key[strcspn(file_channel_key, "\r\n")]=0; // remove newline
                                                //printf("ini key csv: %s\n", file_channel_key);
                                                //printf("ini key input: %s\n", key);
                                                if (strcmp(file_channel_key, key) == 0) {
                                                    // catat di auth.csv
                                                    key_valid = 1;
                                                    char auth_file[512];
                                                    snprintf(auth_file, sizeof(auth_file), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin/auth.csv", channel_name);
                                                    FILE *auth_fp = fopen(auth_file, "a");
                                                    if (auth_fp) {
                                                        int user_id = getIDuser_channel(channel_name);
                                                        fprintf(auth_fp, "%d,%s,%s\n", user_id, username, ROLE_USER);
                                                        fclose(auth_fp);
                                                    }
                                                    // ubah nilai in_channel
                                                    user.in_channel = 0;
                                                    strcpy(user.cur_channel, channel_name);
                                                    // catet log
                                                    char admin_path[256];
                                                    snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                                    char activity[256];
                                                    snprintf(activity, sizeof(activity), "%s masuk ke channel 'say hi'", username);
                                                    log_user_activity(username, activity, admin_path);
                                                    send(new_socket, " ", strlen(" "), 0);
                                                    break;
                                                }
                                                //break;
                                            }
                                            fclose(file);
                                        }

                                        if (!key_valid) {
                                            send(new_socket, "Invalid key\n", strlen("Invalid key\n"), 0);
                                        } 
                                    }
                                }
                            }  
                        } else {
                            send(new_socket, "Channel tidak tersedia\n", strlen("Channel tidak tersedia\n"), 0);
                        }
                    } else {
                        // sudah pernah masuk channel
                        // JOIN ROOM
                        char room_name[256];
                        sscanf(buffer, "%*s %255s", room_name);
                        room_name[strcspn(room_name, "\r\n")]=0; // remove newline
                        printf("%s\n", channel_name);
                        printf("%s\n", room_name);

                        if (room_exists(channel_name, room_name) && strcmp(room_name, "admin") != 0) {
                            // admin/root 
                            if (is_admin(room_name, user.id) || is_root(user.username)) {
                                // catet log
                                char admin_path[256];
                                snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                char activity[256];
                                snprintf(activity, sizeof(activity), "%s masuk ke room %s", username, room_name);
                                log_user_activity(username, activity, admin_path);
                                user.in_room = 0;
                                send(new_socket, "masuk room", strlen("masuk room"), 0);
                            } else { //bukan admin/root (user)
                                // catet log
                                char admin_path[256];
                                snprintf(admin_path, sizeof(admin_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", channel_name);
                                char activity[256];
                                snprintf(activity, sizeof(activity), "%s masuk ke room %s", username, room_name);
                                log_user_activity(username, activity, admin_path);
                                user.in_room = 0;
                                send(new_socket, "masuk room", strlen("masuk room"), 0);
                            }  
                        } else {
                            send(new_socket, "Room tidak tersedia\n", strlen("Room tidak tersedia\n"), 0);
                        }
                    }   
                    
                } else if (strncmp(buffer, "EDIT ROOM", 9) == 0){ // hanya buat admin dan root
                    // sudah pernah masuk channel
                    if (user.in_channel == 0){
                        if (is_admin(user.cur_channel, user.id)||is_root(user.username)) {
                            char room_name[256], new_room_name[256];
                            sscanf(buffer, "%*s %*s %255s %*s %255s", room_name, new_room_name);
                            room_name[strcspn(room_name, "\r\n")]=0; // remove newline
                            new_room_name[strcspn(new_room_name, "\r\n")]=0; // remove newline

                            // cek admin apa bukan
                            if(is_admin(user.cur_channel, user.id)||is_root(user.username)){
                            // printf("%s %s\n", user.cur_channel, )
                                if (room_exists(user.cur_channel, room_name)) {
                                    if (edit_room(username, user.cur_channel, room_name, new_room_name)) {
                                        char response[256];
                                        snprintf(response, sizeof(response), "Room %s diubah menjadi %s\n", room_name, new_room_name);
                                        char act[256];
                                        snprintf(act, sizeof(act), "%s mengubah room %s menjadi %s", username, room_name, new_room_name);
                                        char log_path[256];
                                        snprintf(log_path, sizeof(log_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/admin", user.cur_channel);
                                        log_user_activity(username, act, log_path);
                                        send(new_socket, response, strlen(response), 0);
                                    } else {
                                        send(new_socket, "Gagal mengubah room\n", strlen("Gagal mengubah room\n"), 0);
                                    }
                                } else {
                                    send(new_socket, "Room tidak ditemukan\n", strlen("Room tidak ditemukan\n"), 0);
                                }
                            } else {
                                send(new_socket, "Anda bukan admin\n", strlen("Anda bukan admin\n"), 0);
                            }
                        }
                    } else {
                        send(new_socket, "Anda belum masuk ke channel manapun\n", strlen("Anda belum masuk ke channel manapun\n"), 0);
                    }
                } else if (strncmp(buffer, "DEL ROOM", 8) == 0){ // hanya buat admin
                    // sudah pernah masuk channel
                    if (user.in_channel == 0){
                        char room_name[256];
                        sscanf(buffer, "%*s %*s %255s", room_name);

                        if(strcmp(room_name, "ALL") == 0){
                            delete_all_room(user.cur_channel);
                        }
                        // cek admin apa bukan
                        if(is_admin(user.cur_channel, user.id)||is_root(user.username)){
                            // cek roomnya ada apa ngga
                            if (room_exists(user.cur_channel, room_name)) {
                                char room_path[256];
                                snprintf(room_path, sizeof(room_path), "/Users/tarisa/smt-2/sisop/FP/discorit/%s/%s", user.cur_channel, room_name);
                                if (remove_directory(room_path) == -1) {
                                    send(new_socket, "Gagal menghapus room\n", strlen("Gagal menghapus room\n"), 0);
                                } else {
                                    char response[256];
                                    snprintf(response, sizeof(response), "Room %s berhasil dihapus", room_name);
                                    send(new_socket, response, strlen(response), 0);
                                }
                            } else {
                                send(new_socket, "Room tidak ditemukan\n", strlen("Room tidak ditemukan\n"), 0);
                            }
                        } else {
                            send(new_socket, "Anda bukan admin\n", strlen("Anda bukan admin\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda belum masuk ke channel manapun\n", strlen("Anda belum masuk ke channel manapun\n"), 0);
                    }
                } else if (strncmp(buffer, "LIST ROOM", 9) == 0){ // all user
                    // sudah pernah masuk channel
                    if (user.in_channel == 0){
                        list_rooms(new_socket, user.cur_channel);
                    } else {
                        send(new_socket, "Anda belum masuk ke channel manapun\n", strlen("Anda belum masuk ke channel manapun\n"), 0);
                    }
                } else if(strncmp(buffer,"EDIT", 4) == 0){
                    if (strstr(buffer, "PROFILE SELF -u") != NULL){
                        char new_username[256];
                        sscanf(buffer, "%*s %*s %*s %*s %255s", new_username); //EDIT PROFILE SELF -u new_username
                        if (edit_username(user.username, new_username)) {
                            char response[256];
                            snprintf(response, sizeof(response), "Profil diupdate menjadi %s", new_username);
                            send(new_socket, response, strlen(response), 0);
                            strcpy(user.username, new_username); //update username di struct
                        } else {
                            send(new_socket, "Profile gagal diupdate\n", strlen("Profile gagal diupdate\n"), 0);
                        }
                    } else if (strstr(buffer, "PROFILE SELF -p") != NULL){
                        char new_password[256];
                        sscanf(buffer, "%*s %*s %*s %*s %255s", new_password); //EDIT PROFILE SELF -p new_password
                        if (edit_password(username, new_password)) {
                            strcpy(user.password, new_password); //update pass di struct
                            send(new_socket, "Password diupdate\n", strlen("Password diupdate\n"), 0);
                        } else {
                            send(new_socket, "Password gagal diupdate\n", strlen("Password gagal diupdate\n"), 0);
                        }
                    } else if (strstr(buffer, "WHERE") != NULL){
                        if (strstr(buffer, "-u") != NULL){ //
                            if(is_root(user.username)){
                                char new_username[256];
                                char users_name[256];
                                sscanf(buffer, "%*s %*s %255s %*s %255s", users_name, new_username); //EDIT WHERE user1 -u user01
                                // cek nama yang mau diubah ni nama rootnya apa bukan?
                                if(is_root(users_name)){
                                    if (edit_username(users_name, new_username)) {
                                        char response[256];
                                        snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s", users_name, new_username);
                                        send(new_socket, response, strlen(response), 0);
                                        strcpy(user.username, new_username); //ubah username yang di struct
                                    } else {
                                        send(new_socket, "username gagal diubah\n", strlen("username gagal diubah\n"), 0);
                                    }
                                } else {
                                    if (edit_username(users_name, new_username)) {
                                        char response[256];
                                        snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s", users_name, new_username);
                                        send(new_socket, response, strlen(response), 0);
                                    } else {
                                        send(new_socket, "username gagal diubah\n", strlen("username gagal diubah\n"), 0);
                                    }
                                }
                            } else {
                                send(new_socket, "Anda bukan root\n", strlen("Anda bukan root\n"), 0);
                            }
                        } else if (strstr(buffer, "-p") != NULL){
                            if(is_root(user.username)){
                                char new_password[256];
                                char users_name[256];
                                sscanf(buffer, "%*s %*s %255s %*s %255s", users_name, new_password); //EDIT WHERE user1 -p password01
                                if(is_root(users_name)){
                                    if (edit_password(users_name, new_password)) {
                                        char response[256];
                                        snprintf(response, sizeof(response), "password %s berhasil diubah", users_name);
                                        send(new_socket, response, strlen(response), 0);
                                        strcpy(user.password, new_password);
                                    } else {
                                        send(new_socket, "password gagal diubah\n", strlen("password gagal diubah\n"), 0);
                                    }
                                } else { // bukan root gausah ganti struct
                                    if (edit_password(users_name, new_password)) {
                                        char response[256];
                                        snprintf(response, sizeof(response), "password %s berhasil diubah", users_name);
                                        send(new_socket, response, strlen(response), 0);
                                    } else {
                                        send(new_socket, "password gagal diubah\n", strlen("password gagal diubah\n"), 0);
                                    }
                                }
                            } else {
                                send(new_socket, "Anda bukan root\n", strlen("Anda bukan root\n"), 0);
                            }
                        } else {
                            send(new_socket, "Invalid command\n", strlen("Invalid command\n"), 0);
                        }
                    } else {
                        send(new_socket, "Invalid command\n", strlen("Invalid command\n"), 0);
                    }
                } else if(strcmp(buffer, "LIST USER") == 0){
                    // cek di dalem channel apa bukan
                    if(user.in_channel == 0){ 
                        // list user keseluruhan (cuma bisa root)
                        list_channel_users(new_socket, user.cur_channel);
                    } else {
                        if(is_root(user.username)){
                            list_users(new_socket);
                        } else {
                            send(new_socket, "Anda bukan root\n", strlen("Anda bukan root\n"), 0);
                        }
                    }
                } else if(strncmp(buffer, "BAN", 3) == 0){
                    char users_name[256];
                    sscanf(buffer, "%*s %255s", users_name); //BAN user1
                    // user udah dalem channel
                    if(user.in_channel == 0){
                        if(is_member(users_name, user.cur_channel)){
                        // ban user (cuma bisa admin)
                            if(is_admin(user.cur_channel, user.id)||is_root(user.username)){
                                
                                if(strcmp(user.username, users_name) != 0){ // user bukan admin atau root
                                    if (ban_user(user.cur_channel, users_name)) {
                                        send(new_socket, "User dibanned\n", strlen("User dibanned\n"), 0);
                                    } else {
                                        send(new_socket, "User gagal dibanned\n", strlen("User gagal dibanned\n"), 0);
                                    }
                                } else {
                                    send(new_socket, "Anda tidak bisa ban diri sendiri\n", strlen("Anda tidak bisa ban diri sendiri\n"), 0);
                                }
                            } else {
                                send(new_socket, "Anda bukan admin atau root\n", strlen("Anda bukan admin atau root\n"), 0);
                            }
                        } else {
                            send(new_socket, "User tidak ditemukan\n", strlen("User tidak ditemukan\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda belum masuk ke channel manapun\n", strlen("Anda belum masuk ke channel manapun\n"), 0);
                    }
                } else if(strncmp(buffer, "UNBAN", 5) == 0){
                    char users_name[256];
                    sscanf(buffer, "%*s %255s", users_name); //UNBAN user1
                    // user udah dalem channel
                    if(user.in_channel == 0){
                        // ada ga usernya?
                        if(is_member(users_name, user.cur_channel)){    
                            // unban user (cuma bisa admin sama root)
                            if(is_admin(user.cur_channel, user.id)||is_root(user.username)){
                                if(strcmp(user.username, users_name) != 0){ // user bukan admin atau root
                                    if (unban_user(user.cur_channel, users_name)) {
                                        send(new_socket, "User diunbanned\n", strlen("User diunbanned\n"), 0);
                                    } else {
                                        send(new_socket, "User gagal diunbanned\n", strlen("User gagal diunbanned\n"), 0);
                                    }
                                } else {
                                    send(new_socket, "Anda tidak bisa unban diri sendiri\n", strlen("Anda tidak bisa unban diri sendiri\n"), 0);
                                }
                            } else {
                                send(new_socket, "Anda bukan admin atau root\n", strlen("Anda bukan admin atau root\n"), 0);
                            }
                        } else {
                            send(new_socket, "User tidak ditemukan\n", strlen("User tidak ditemukan\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda belum masuk ke channel manapun\n", strlen("Anda belum masuk ke channel manapun\n"), 0);
                    }
                } else if(strncmp(buffer, "EXIT", 4)==0){
                    // lagi masuk room
                    if(user.in_room == 0){
                        user.in_room = 1;
                        send(new_socket, "exit room\n", strlen("exit room\n"), 0);
                    } else if(user.in_channel == 0) {
                        user.in_channel = 1;
                        send(new_socket, "exit channel\n", strlen("exit channel\n"), 0);
                    } else {
                        close(new_socket);
                        pthread_exit(0);
                    }
                } else if (strncmp(buffer, "REMOVE", 6) == 0){
                    if(is_root(user.username)){
                        char users_name[256];
                        sscanf(buffer, "%*s %255s", users_name); //REMOVE user1
                        if (remove_user(users_name)) {
                            send(new_socket, "User dihapus\n", strlen("User dihapus\n"), 0);
                        } else {
                            send(new_socket, "User gagal dihapus\n", strlen("User gagal dihapus\n"), 0);
                        }
                    } else {
                        send(new_socket, "Anda bukan root\n", strlen("Anda bukan root\n"), 0);
                    }
                } else {
                    send(new_socket, "Unknown command\n", strlen("Unknown command\n"), 0);
                }
            }
        } else {
            send(new_socket, "Invalid username or password\n", strlen("Invalid username or password\n"), 0);
        }
    } else {
        send(new_socket, "Invalid command\n", strlen("Invalid command\n"), 0);
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setting SO_REUSEADDR separately
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    // Setting SO_REUSEPORT separately
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        if (fork() == 0) {  // Create a new process for each client
            close(server_fd);
            handle_client(new_socket);
            close(new_socket);
            exit(0);
        }
        close(new_socket);
    }

    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return 0;
}

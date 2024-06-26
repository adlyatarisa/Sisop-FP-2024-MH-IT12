# Laporan Resmi Praktikum Sistem Operasi 2024 Final Project

## Anggota Kelompok IT 12
| Nama Lengkap          | NRP        |
| --------------------- | ---------- |
| Muhammad Dzaky Ahnaf  | 5027231039 |
| Adlya Isriena Aftarisya  | 5027231066 |
| Nisrina Atiqah Dwiputri Ridzki  | 5027231075 |

# Daftar Isi
- [Deskripsi Soal](#deskripsi-soal)
- [Penjelasan Kode](#penjelasan-kode)
- [Dokumentasi Program](#dokumentasi-program) 

## Deskripsi Soal
Intinya pada tugas Final Project ini, kita diharapkan membuat sebuah program apikasi chatting DiscorIT. DiscorIT terdiri dari 3 program yaitu discorit, server, dan monitor. Program discorit dan monitor berperan sebagai client, sedangkan program server berperan sebagai server yang dimana client dan server berinteraksi melalui socket.
- Program discorit bekerja sebagai client yang mengirimkan request user kepada server.
- Program server berjalan sebagai server yang menerima semua request dari client dan mengembalikan response kepada client sesuai ketentuan pada soal. Program server berjalan sebagai daemon.
- Untuk hanya menampilkan chat, user perlu membuka program client (monitor)

## Penjelasan Kode
### discorit.c

1. Fungsi ```register_user```
	  ```c
	void register_user(const char *username, const char *password) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024] = {0};


	
	```
	Fungsi ```register_user``` untuk melakukan registrasi username dan password baru. Dengan mendeklarasikan variabel untuk soket dan jumlah byte, struktur alamat server, dan buffer nya.

2.  Inisialisasi soket dan alamat server
    ```c
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }
   
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

	```
	Bagian ini untuk membuat soket menggunakan IPv4 dan protokol TCP. Jika gagal, akan mencetak pesan kesalahan dan keluar dari fungsi. Lalu menginisialisasi serv_addr dengan nol, kemudian mengatur keluarga alamat ke AF_INET, dan port ke nilai yang sudah terdefinisi dikonversi ke format jaringan.

3. Menghubungkan ke server dan mengirim pesan
   ```c
	    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }
   
    snprintf(message, sizeof(message), "DISCORIT REGISTER %s %s", username, password);
    send(sock, message, strlen(message), 0);
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
	}


	```
	
	Mengkonversi alamat IP ke format biner dan menyimpannya di serv_addr.sin_addr. Setelah itu, menghubungkan soket ke alamat server. Kemudian, membentuk pesan pendaftaran dengan username dan password, mengirim pesan ke server, membaca respons dari server, dan mencetaknya.

4. Fungsi ```login_user```
   ```c
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

    snprintf(message, sizeof(message), "DISCORIT LOGIN %s %s", username, password);
    send(sock, message, strlen(message), 0);
    valread = read(sock, buffer, 1024);
    buffer[valread] = '\0';
    printf("%s\n", buffer);

    if (strstr(buffer, "berhasil login")) {
        char current_channel[256] = {0};
        char current_room[256] = {0};
        while (1) {
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

            char input[1024];
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0; // Remove newline character


            if(strstr(input, "-channel") && strstr(input, "-room")){
                // jangan di send
                printf("Invalid command. Try on monitor\n");
                continue;
            }            

            // Send the input to the server
            send(sock, input, strlen(input), 0);

            // Receive response from server
            valread = read(sock, buffer, 1024);
            buffer[valread] = '\0';

	```

    Fungsi ```login_user``` digunakan untuk melakukan login dengan username dan password yang sudah di registrasi sebelumnya di fungsi register_user. Setelah mengirimkan pesan login, fungsi membaca respons dari server. Jika login berhasil, fungsi memasuki loop yang terus-menerus untuk menerima dan mengirim pesan antara pengguna dan server, serta menangani berbagai perintah seperti "JOIN", "CREATE ROOM", dan "EXIT", serta memperbarui status pengguna, channel, dan room sesuai dengan respons server.
    
5.  Pengelolaan perintah dan respons server
    ```c
   	           char temp_channel[256];
            if (strncmp(input, "JOIN", 4) == 0) {
                
                sscanf(input, "JOIN %255s", temp_channel);
                char temp_room[256];
                sscanf(input, "JOIN %255s", temp_room);
                if (strstr(buffer, "Key:")) {
                    // Server requests key
                    printf("%s", buffer);
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0; // Remove newline character
                    send(sock, input, strlen(input), 0);
                    valread = read(sock, buffer, 1024);
                    buffer[valread] = '\0';
                    if (strstr(buffer, "Invalid key")) {
                        // If key is incorrect, don't change current_channel
                        printf("%s", buffer);
                        continue;
                    } else {
                        strcpy(current_channel, temp_channel);
                        continue;
                    }
                } else if(strstr(buffer, "Anda dibanned")){
                    printf("%s", buffer);
                    continue;
                } else if (strstr(buffer, "Channel tidak tersedia")) {
                    // If channel is not available, don't change current_channel
                    printf("%s", buffer);
                    continue;
                } else if(strstr(buffer, "Room tidak tersedia")){
                    printf("%s", buffer);
                    continue;
                } else if (strstr(buffer, "masuk room")){
                    strcpy(current_room, temp_room);
                    continue;
                } else {
                    // If admin/root/already joined
                    strcpy(current_channel, temp_channel);
                    continue;
                } 
            } else if((strncmp(buffer, "Channel", 7) == 0 && strstr(buffer, "diubah menjadi")) && strcmp(current_channel,"")){
                    printf("%s", buffer);
                    char *new_channel = strstr(buffer, "diubah menjadi ") + strlen("diubah menjadi ");
                    new_channel[strcspn(new_channel, "\n")] = 0; // Remove newline
                    strcpy(current_channel, new_channel);
                    continue;
            } else if((strncmp(buffer, "Room", 4) == 0 && strstr(buffer, "diubah menjadi")) && strcmp(current_room,"")){
                    printf("%s", buffer);
                    char *new_room = strstr(buffer, "diubah menjadi ") + strlen("diubah menjadi ");
                    new_room[strcspn(new_room, "\n")] = 0; // Remove newline
                    strcpy(current_room, new_room);
                    continue;
            } else if (strncmp(input, "CREATE ROOM", 11) == 0){
                char temp_room[256];
                sscanf(input, "CREATE ROOM %255s", temp_room);
                if (strstr(buffer, "Room sudah ada")) { 
                    printf("%s", buffer); //print to terminal
                    continue;
                } else {
                    if (strstr(buffer, "dibuat\n")){
                        printf("%s", buffer);
                        continue;
                    } else {
                        printf("%s", buffer);
                        continue;
                    }
                }
            } else if(strncmp(input, "EXIT", 4) == 0){
                if (strstr(buffer, "exit room")){
                    strcpy(current_room, "");
                    continue;
                } else if (strstr(buffer, "exit channel")){
                    strcpy(current_channel, "");
                    continue;
                } else {
                    break;
                }
            }

            printf("%s\n", buffer);
        }
        return;
      }
    }
	```
 
    Fungsi ini untuk mengecek apakah channel dan room tersedia serta apakah pengguna di-ban dari channel/room tersebut. Jika channel atau room tidak tersedia, maka akan keluar pesan bahwa channel atau room tersebut tidak tersedia, dan pengguna tidak bisa join. Selain itu, jika pengguna di-ban dari channel/room tersebut, maka mereka tidak akan bisa bergabung. Status pengguna, channel, dan room diperbarui sesuai dengan respons server.
   
6. Fungsi ```main```
   	```c
   	int main(int argc, char const *argv[]) {
    if (argc != 5 || (strcmp(argv[1], "REGISTER") != 0 && strcmp(argv[1], "LOGIN") != 0) || strcmp(argv[3], "-p") != 0) {
        printf("Usage:\n");
        printf("Register: %s REGISTER username -p password\n", argv[0]);
        printf("Login: %s LOGIN username -p password\n", argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    const char *username = argv[2];
    const char *password = argv[4];

    if (strcmp(command, "REGISTER") == 0) {
        register_user(username, password);
    } else if (strcmp(command, "LOGIN") == 0) {
        login_user(username, password);
    }

        return 0;
     }
	```
    
    Fungsi ```main``` digunakan untuk mengecek apakah argumen perintah sesuai dengan format yang seharusnya. Jika tidak sesuai, maka akan mengeluarkan pesan format argumen yang benar.


  
### monitor.c

1. Fungsi ```login_user```
	  ```c
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

	
	```
	Fungsi ```login_user``` akan menerima username dan password,  lalu akan mendeklarasikan variabel untuk soket, buffer, dan alamat server, serta menyalin username ke current_username. Soket dibuat menggunakan IPv4 dan TCP, dengan pengecekan kesalahan. Alamat server diinisialisasi dan dikonversi ke format biner, kemudian soket dihubungkan ke server. Jika koneksi berhasil, pesan login dibentuk dan dikirim ke server. Respons dari server dibaca dan dicetak. Jika login berhasil, fungsi memasuki loop untuk menerima dan memproses pesan dari server, mengelola file chat.csv berdasarkan informasi channel dan room, hingga menerima perintah "exit" dari server, lalu koneksi ditutup.

2. Inisialisasi dan Koneksi 
   ```c
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

	```
   
   Bagian ini ntuk membuat soket menggunakan IPv4 dan protokol TCP. Jika gagal, akan mencetak pesan kesalahan dan keluar dari fungsi. Lalu menginisialisasi serv_addr dengan nol, kemudian mengatur keluarga alamat ke AF_INET, dan port ke nilai yang sudah terdefinisi dikonversi ke format jaringan.

3. Memproses Respon Login
   ```c
	if (strstr(buffer, "berhasil login")) {
    while (1) {
        valread = read(sock, buffer, 1024);
        buffer[valread] = '\0';
        if (strstr(buffer, "exit")) {
            break;
        }

        if (strstr(buffer, "-channel") && strstr(buffer, "-room")) {
            send(sock, buffer, strlen(buffer), 0);
            valread = read(sock, buffer, 1024);
            buffer[valread] = '\0';

            char channel_name[256], room_name[256];
            sscanf(buffer, "%*s %255s %*s %255s", channel_name, room_name);
            printf("Channel name: %s, Room name: %s\n", channel_name, room_name);

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
	```
	
	Setelah berhasil login, fungsi ini memasuki loop untuk mengelola pesan yang diterima dari server. Jika server mengirimkan pesan yang berisi kata "exit", proses loop akan berhenti. Jika pesan yang diterima mengandung kata "-channel" dan "-room", pesan tersebut dikirim kembali ke server dan responsnya dibaca. Dari respons tersebut, nama channel dan room diekstraksi dan dicetak. Selanjutnya, fungsi membaca file chat.csv yang terletak di direktori sesuai dengan channel dan room yang diterima. Jika file chat.csv berhasil dibuka, fungsi memeriksa perubahan ukuran file untuk mencetak baris baru yang ditambahkan sejak bacaan terakhir. Setelah selesai, file dan soket ditutup sebelum keluar dari loop.

4. Fungsi ```main```
   ```c
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

	```

    Fungsi ```main``` digunakan untuk mengecek apakah argumen perintah sesuai dengan format yang seharusnya. Jika tidak sesuai, maka akan mengeluarkan pesan format argumen yang benar.

### server.c
### 1. Inisialisasi Library dan Struktur Data
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <bcrypt.h>

#define PORT 8081
#define USERS_FILE "/home/jeky/sisop/praktikum/FP/users.csv"
#define CHANNELS_FILE "/home/jeky/sisop/praktikum/FP/channels.csv"
#define USER_LOG "/home/jeky/sisop/praktikum/FP/users.log"
#define ROLE_ROOT "ROOT"
#define ROLE_ADMIN "ADMIN"
#define ROLE_USER "USER"
#define MAX_CLIENTS 5

typedef struct {
    char username[256];
    char password[256];
    char role[256];
    int id;
    int in_channel;
    char cur_channel[256];
    int in_room;
    char cur_room[256];
    int login_monitor;
} User;

typedef struct {
    int id;
    char name[256];
    char key[256];
    char admin[256];
} Channel;

typedef struct {
    int socket;
    bool is_monitor;
    bool logged_in;
    char username[256];
} Client;

typedef struct {
    int logged_in;
} Monitor;

Client clients[MAX_CLIENTS] = {0};
int client_count = 0;
```
Penjelasan:

- Kode dimulai dengan meng-include berbagai library yang diperlukan untuk fungsi-fungsi jaringan, file I/O, threading, dan enkripsi.
- Beberapa konstanta penting didefinisikan, termasuk ```PORT```, path file, dan role pengguna.
- Struktur data utama (User, Channel, Client, Monitor) didefinisikan untuk menyimpan informasi pengguna, channel, client, dan monitor.
- Array ```clients``` diinisialisasi untuk menangani multiple client, dengan maksimum 5 client (```MAX_CLIENTS```).

### 2. Fungsi-fungsi Utilitas
```c
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

char *bcrypt(const char *password) {
    char salt[] = "$2b$12$XXXXXXXXXXXXXXXXXXXXXX"; // Generate a random salt
    char *encrypted_password = crypt(password, salt);
    return strdup(encrypted_password);
}

int check_password(const char *password, const char *encrypted_password) {
    return strcmp(encrypted_password, crypt(password, encrypted_password)) == 0;
}
```
Penjelasan:

- ```user_exists```: Memeriksa apakah username sudah ada di file ```users.csv```.
- ```getIDuser```: Mencari ID user tertinggi di ```users.csv``` dan mengembalikan ID baru (tertinggi + 1).
- ```bcrypt```: Mengenkripsi password menggunakan bcrypt dengan salt statis.
- ```check_password```: Memverifikasi password dengan membandingkan hasil enkripsi.

### 3. Manajemen User
```c
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
    if (ftell(file) == 0) {
        role = ROLE_ROOT;
    }
    int user_id = getIDuser();
    char *encrypted_password = bcrypt(password);
    fprintf(file, "%d,%s,%s,%s\n", user_id, username, encrypted_password, role);
    free(encrypted_password);
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
        if (strcmp(file_username, username) == 0 && check_password(password,file_password)) {
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
```
Penjelasan:

- ```register_user```: Mendaftarkan user baru ke ```users.csv```. Jika file kosong, user pertama menjadi ```ROOT```.
- ```verify_user```: Memverifikasi kredensial user saat login, membandingkan dengan data di ```users.csv```.

### 4. Manajemen Channel dan Room
```c
void list_channels(int socket) {
    FILE *file = fopen(CHANNELS_FILE, "r");
    if (!file) {
        send(socket, "No channels.csv available\n", strlen("No channels.csv available\n"), 0);
        return;
    }

    char line[1024];
    char response[4096] = {0};
    int empty = 1;
    while (fgets(line, sizeof(line), file)) {
        empty = 0;
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
    snprintf(channel_path, sizeof(channel_path), "/home/jeky/sisop/praktikum/FP/%s", channel_name);

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

// Fungsi list_users dan list_channel_users dihilangkan untuk menghemat ruang

int create_channel(const char *channel_name, const char *username, const char *key) {
    int channel_id = getIDchannel();

    FILE *file = fopen(CHANNELS_FILE, "a");
    if (!file) {
        perror("Could not open channels file");
        return 0;
    }

    char *encrypted_key = bcrypt(key);
    fprintf(file, "%d,%s,%s\n", channel_id, channel_name, encrypted_key);
    fclose(file);

    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "/home/jeky/sisop/praktikum/FP/%s", channel_name);

    if (mkdir(dir_path, 0777) == -1) {
        perror("Could not create directory for channel");
        return 0;
    } else{
        printf("Channel created successfully\n");
    }

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

// Fungsi create_room, edit_channel, dan edit_room dihilangkan untuk menghemat ruang
```
Penjelasan:
- ```list_channels```: Menampilkan daftar channel dari ```channels.csv```.
- ```list_rooms```: Menampilkan daftar room dalam sebuah channel.
- ```create_channel```: Membuat channel baru, termasuk direktori dan file yang diperlukan.

## Dokumentasi Program
### 1. Autentikasi User
- Melakukan register dan login user

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/055b8439-17ea-48da-9beb-f5e19ed5b668)

- Akan tercatat informasi nama user, password yang terenkripsi dengan bcrypt, dan role nya di ```users.csv```

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/a08a33d3-bf81-449f-8c41-232458da8f63)


### 2. Fitur Channel
- Membuat channel

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/81032d2e-9dc9-418f-8a0d-fd1bcdcb0aaa)

- Akan tercatat informasi nama channel dan key yang terenkripsi dengan bcrypt di ```channels.csv```

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/93da0491-52e2-4df7-9456-599dc45720f9)

- Akan terbuat sebuah file baru bernama channel yang dibuat sebelumnya dan ```admin``` didalamnya yang berisi ```auth.csv``` dan ```users.log```. Semua perubahan dan aktivitas user pada channel dicatat dalam file ```users.log``` dan informasi mengenai user dan rolenya pada channel tersebut dicatat dalam file ```auth.csv```

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/33ffdc4f-c4d4-4841-8fce-6ecd6e9119c6)

  - Isi ```auth.csv```
    
    ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/9b3d3f29-398e-49f6-8b98-ea731429b75e)

  - Isi ```users.log```
    
    ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/dda82a49-5c53-4135-bbe2-5ebca2e73a56)

- User Join channel
  - Ketika user memiliki role ```ROOT``` akan langsung join tanpa memasukkan key
    
    ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/ef314744-3b30-4c7e-b339-54867b50579d)
    
  - Ketika user tidak memiliki role ```ROOT``` akan join dengan memasukkan key
    
    ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/cda4c5c0-5297-40a1-bee7-99d6fdf39ddd)

- User ROOT mengedit dan menghapus channel.
 
    ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/c8950163-b8f2-4502-bac0-a3b58b93199e)

    
### 3. Fitur Room
- Membuat dan join room

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/4f92f53c-1f4d-4e9e-bd6a-fc0f51ed6120)

- Chat didalam room, melihat, mengedit, dan menghapus chat di room.
  
  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/a7d0c4d4-8416-4f60-8950-d06d8f128e71)

- Admin channel mengedit room, menghapus room, dan menghapus semua room.

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/7d3fda4c-f393-4e41-ad5b-0e8e7798146c)


### 4. Fitur pada user ROOT
- User ROOT dapat melakukan edit dan remove user lain.

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/c980d375-ff21-48b7-960b-937839bcf261)

- Admin dapat melakukan ban, unban, dan remove user lain

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/e2791bf3-5641-4edf-adf3-0565316795df)
### 5. Fitur pada user masing-masing
- User bisa mengedit profile masing-masing seperti username dan password.

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/127a384a-ff5b-40f3-a041-79d6aa242f4e)

- User ketika dirinya dibanned karena nakal.

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/8f7bf972-a3c8-499f-9dc5-70d5b1494c3f)

### 6. Hasil ```auth.csv``` dan ```users.log``` setelah eksperimen

- ```auth.csv```

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/7d996320-123e-400b-96cd-ef552ce7f910)

- ```users.log```

  ![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/6a06ac96-d6e8-4dd6-a3dd-e2a896bed317)

### 7. Fitur Monitor
Monitor dapat menampilkan chat dari command yang dikirim melalui discorit

<img width="799" alt="image" src="https://github.com/adlyatarisa/Sisop-FP-2024-MH-IT12/assets/164857172/f03abdfd-f0b8-49d2-a358-a4714c948915">


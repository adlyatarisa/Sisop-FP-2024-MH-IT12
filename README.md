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
## Dokumentasi Program
### 1. Autentikasi User
- Melakukan register dan login user

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/b6a589fa-aa29-428c-83a8-c87ced583630)

- Akan tercatat informasi nama user, password yang terenkripsi dengan bcrypt, dan role nya di ```users.csv```

![image](https://github.com/DzakyAhnaf/Sisop-FP-2024-MH-IT12/assets/110287409/f9d61d38-62f2-4c66-b5b1-4bc9899e4957)


### 2. Channel
- Membuat channel
- Akan tercatat informasi nama channel

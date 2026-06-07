# UAS KELOMPOK 5
---
## 🍽 Sistem Manajemen Restoran
Aplikasi Sistem Manajemen untuk Restoran berbasis C++ dan Qt6 yang mengimplementasi dari materi materi Algoritma dan Pemrograman dan Struktur Data, dibangun sebagai proyek UAS Terintegrasi.

---

## 🛠️Cara Menjalankan Aplikasi
1. Donwload dan Install `SistemRestoran_Setup`
2. Aplikasi siap dijalankan

---

## Implementasi Materi Algoritma dan Pemrograman
1. Struct
2. Reference
3. Pointer
4. Namespace
5. Callback Functiom
6. Default Argument / Inline Function
7. Function Overloading / Function Template
8. Exception Handling
9. STL Vector or List, Iterator, Sort, Find, Count
10. File Handling
11. Lambda Expression

---

## Implementasi Materi Struktur Data
1. Linked List
2. Circular Linked list
3. Stack
4. Queue
5. Binary Tree / AVL Tree
6. Graph BFS DFS
7. Hashing dan Hash Table
8. Sorting

---

## 📱 Fitur Aplikasi
### 📊 Dashboard
- Ringkasan statistik real-time: total menu, total pesanan, pesanan pending, meja aktif
- Log aktivitas sistem
- Jam digital yang diperbarui setiap detik

### 🍜 Manajemen Menu
- Tambah, hapus, dan cari item menu
- Filter menu berdasarkan kategori (menggunakan callback)
- Pengurutan multi-kriteria (nama, harga, kategori)
- Simpan & muat data menu dari file
- Pencarian cepat via AVL Tree dan Hash Table

### 🧾 Manajemen Pesanan
- Buat pesanan baru dengan memilih meja dan item menu
- Sistem prioritas pesanan: **VIP** dan **Normal**
- Proses pesanan berikutnya dari antrian (Priority Queue)
- Update status pesanan: `pending → preparing → served → paid`
- Perhitungan total harga otomatis

### 🪑 Manajemen Meja
- Navigasi meja dengan Circular Linked List
- Tandai meja sebagai terisi atau kosong
- Visualisasi grafik hubungan antar meja
- Jalankan algoritma **BFS** dan **DFS** pada graf meja

### 👥 Manajemen Staf & Shift
- Tambah dan hapus data staf
- Rotasi shift menggunakan Circular Linked List
- Tampilkan staf yang sedang bertugas

### 📜 Riwayat & Laporan
- Riwayat semua aksi dengan fitur **Undo** (Stack)
- Generate laporan pesanan dengan berbagai opsi pengurutan
- Demo STL (`std::find`, `std::count`) untuk analisis data
- Ekspor laporan ke file teks

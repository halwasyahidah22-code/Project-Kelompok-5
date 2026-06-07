# UAS KELOMPOK 5
---
## 🍽 Sistem Manajemen Restoran
Sistem Manajemen untuk Restoran mengimplementasi dari materi materi Algoritma dan Pemrograman dan Struktur Data

---

## Cara Menjalankan Aplikasi
1. Donwload dan Install SistemRestoran_Setup
2. Aplikasi siap dijalankan

---

## Implementasi Materi Algoritma dan Pemrograman

| No | Elemen              | Implementasi                                       | File               |
|----|---------------------|----------------------------------------------------|--------------------|
| 1  | `struct`            | `MenuItem`, `Order`, `Table`, `Staff`, `Action`   | datastructures.h   |
| 2  | References `&`      | Parameter fungsi `insert(const MenuItem&)`        | datastructures.h   |
| 3  | Pointer `*`         | Node Linked List, AVL Tree (`MenuItem*`, `AVLNode*`) | datastructures.h |
| 4  | `namespace`         | `namespace RestaurantSystem`, `namespace Sorting`, `namespace FileIO` | datastructures.h |
| 5  | Callback Function   | `forEachMenu(menus, MenuCallback cb)`             | datastructures.h   |
| 6  | Default Arg / Inline| Constructor default args, `inline` fungsi sort   | datastructures.h   |
| 7  | Template            | `template<typename T> T* linearSearch(...)`       | datastructures.h   |
| 8  | Exception Handling  | `try/catch/throw RestaurantException`             | mainwindow.cpp     |
| 9  | STL (vector, list, iterator, sort, find, count) | `std::vector`, `std::list`, `std::find_if`, `std::sort`, `std::count_if` | datastructures.h |
| 10 | File Handling       | `FileIO::saveMenuToFile`, `loadMenuFromFile`, `saveOrdersToFile` | datastructures.h |
| 11 | Lambda Expression   | `sort` with lambda, `find_if` with lambda, `for_each` | datastructures.h / mainwindow.cpp |

---

## Implementasi Materi Struktur Data

| No | Struktur Data         | Digunakan Untuk                          | Kelas                    |
|----|-----------------------|------------------------------------------|--------------------------|
| 1  | Linked List (Single)  | Master data menu restoran                | `MenuLinkedList`          |
| 2  | Circular Linked List  | Rotasi meja aktif + rotasi shift staf   | `TableCircularList`, `StaffCircularList` |
| 3  | Stack (LIFO)          | Riwayat aksi + fitur Undo               | `ActionStack`             |
| 4  | Priority Queue        | Antrian order (VIP lebih diutamakan)    | `OrderQueue`              |
| 5  | AVL Tree              | Pencarian menu berdasarkan ID           | `AVLTree`                 |
| 6  | Graph + BFS + DFS     | Layout & relasi antar meja restoran     | `TableGraph`              |
| 7  | Hash Table            | Lookup cepat O(1) item menu             | `MenuHashTable`           |
| 8  | Sorting               | Bubble Sort (harga), Merge Sort (order), STL sort (nama) | `namespace Sorting` |

---

K

## 📱 Fitur Aplikasi (Tab)

### 📊 Dashboard
### 🍜 Menu Management
### 📋 Order Management
### 🪑 Table Management
### 👤 Staf & Shift
### 📁 Riwayat & Laporan

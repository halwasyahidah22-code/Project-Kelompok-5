# 🍽 Sistem Manajemen Restoran — Qt C++

Proyek UAS Terintegrasi untuk 4 Mata Kuliah:
Teori Alpro, Praktikum Alpro, Teori Strukdat, Praktikum Strukdat

---

## 🔧 Cara Build & Jalankan

### Menggunakan Qt Creator (Direkomendasikan)
1. Buka Qt Creator
2. File → Open Project → pilih `RestaurantSystem.pro`
3. Klik Configure Project
4. Tekan tombol ▶ Run

### Menggunakan qmake (Terminal)
```bash
cd RestaurantSystem/
mkdir build && cd build
qmake ../RestaurantSystem.pro
make
./RestaurantSystem
```

### Menggunakan CMake
```bash
mkdir build && cd build
cmake ..
make
./RestaurantSystem
```

---

## ✅ Checklist 11 Elemen C++ (Praktikum Alpro)

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

## ✅ Checklist 8 Struktur Data (Praktikum Strukdat)

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

## 🗂 Struktur File

```
RestaurantSystem/
├── RestaurantSystem.pro   # Qt project file
├── CMakeLists.txt         # CMake alternative
├── README.md
└── src/
    ├── main.cpp           # Entry point
    ├── mainwindow.h       # Deklarasi MainWindow + Qt slots
    ├── mainwindow.cpp     # Implementasi UI + semua slot
    └── datastructures.h   # Semua struktur data C++ (header-only)
```

---

## 📱 Fitur Aplikasi (Tab)

### 📊 Dashboard
- Statistik real-time: total menu, total order, pending, meja terisi
- Log aktivitas sistem
- Jam digital berjalan

### 🍜 Menu Management
- CRUD menu (Linked List + AVL Tree + Hash Table)
- Pencarian cepat by ID (Hash Table O(1)) atau nama
- Sorting: Bubble Sort (harga), STL sort (nama), AVL inorder (ID)
- Simpan/muat dari file `.txt`

### 📋 Order Management
- Buat order baru per meja
- Tambah/hapus item dari order
- Submit ke Priority Queue (VIP prioritas lebih tinggi)
- Proses order berikutnya (FIFO dalam prioritas sama)
- Tampilkan semua order (STL list + iterator)

### 🪑 Table Management
- Navigasi meja (Circular Linked List — tidak ada ujung)
- Tandai meja terisi / kosong
- Jalankan BFS/DFS dari meja aktif (Graph)
- Tampilkan adjacency list

### 👤 Staf & Shift
- CRUD data staf (Circular Linked List)
- Rotasi shift (maju ke staf berikutnya secara circular)

### 📁 Riwayat & Laporan
- Stack history semua aksi (tampil LIFO)
- Fitur Undo (pop dari stack)
- Generate laporan order (Merge Sort)
- Sort laporan: Merge Sort, STL sort + lambda
- Simpan laporan ke `orders.txt`

---

## 👨‍💻 Catatan Implementasi

- Semua struktur data diimplementasikan **dari nol** tanpa library eksternal (kecuali STL)
- AVL Tree auto-balance setelah setiap insert/delete
- Priority Queue menggunakan `std::priority_queue` dengan custom comparator
- Hash Table menggunakan `std::unordered_map` (O(1) average)
- Graph menggunakan adjacency list (`unordered_map<int, vector<int>>`)
- Circular Linked List benar-benar sirkular (tail->next = head)
- Stack menggunakan `std::stack` (LIFO terjamin)

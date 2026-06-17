# Panduan Lengkap Penggunaan Restaurant System 🍽️

Selamat datang di aplikasi **Sistem Manajemen Restoran**! Aplikasi ini dirancang untuk memudahkan seluruh kegiatan operasional restoran, mulai dari pendaftaran menu, penataan meja, manajemen staf, pencatatan order, hingga rekapitulasi keuangan.

Berikut adalah panduan langkah demi langkah untuk menggunakan semua fitur yang ada.

---

## 1. Halaman Utama (Dashboard)
Saat pertama kali program dijalankan, Anda akan melihat halaman **Dashboard**.
- **Status Operasional:** Menampilkan ringkasan total order, status ketersediaan menu, dan status penggunaan meja secara real-time.
- Pastikan Anda selalu mengecek dashboard ini untuk mengetahui kondisi restoran secara keseluruhan.

---

## 2. Tab Menu (Master Data Makanan & Minuman)
Di tab ini, Anda dapat mengelola seluruh daftar menu yang dijual.

- **Menambah Menu Baru:**
  1. Isi form di sebelah kiri (Nama Menu, Kategori, Harga).
  2. Klik tombol **Tambah Menu**. Data akan langsung tersimpan.
- **Menghapus Menu:**
  1. Klik salah satu menu di dalam tabel di sebelah kanan.
  2. Klik tombol **Hapus Menu**.
- **Mencari Menu:**
  Gunakan kolom pencarian di pojok kanan atas, ketik nama atau ID, lalu klik **Cari**.
- **Filter & Sorting:**
  Pilih kategori dari *dropdown* untuk memfilter, atau gunakan tombol **Sortir (Nama)** dan **Sortir (Harga)** untuk merapikan urutan menu.

---

## 3. Tab Order (Pemesanan)
Ini adalah menu utama untuk melayani pelanggan yang datang.

### Langkah-langkah Membuat Pesanan:
1. **Pilih Meja:** Di bagian atas, pilih meja yang kosong dari *dropdown*. *(Catatan: Anda tidak bisa memilih meja yang sudah terisi)*.
2. **Pilih Prioritas:** Pilih apakah pelanggan adalah tamu **Normal** atau **VIP**.
3. **Mulai Order:** Klik tombol biru **Buat Order Baru**.
4. **Tambah Item:** Pilih makanan/minuman dari *dropdown*, tentukan jumlah (Qty), lalu klik **Tambah Item**.
5. **Submit Order:** Jika pesanan sudah lengkap, klik **Submit Order**. Pesanan akan masuk ke dalam antrian dapur.

---

## 4. Tab Dapur (Status Pesanan)
Di bagian tengah Tab Order, terdapat **Tabel Daftar Order** yang mengatur alur masakan.

- Setiap pesanan yang baru disubmit akan memiliki status **`pending`**.
- Klik tombol **Proses Order Berikutnya** untuk menarik pesanan `pending` menjadi **`preparing`** (sedang dimasak). Prioritas VIP akan otomatis didahulukan oleh sistem.
- Setelah makanan selesai dimasak, cari pesanan berstatus `preparing` di tabel, dan klik tombol hijau **Sajikan** di kolom "Aksi". Status akan berubah menjadi **`served`** (telah dihidangkan ke meja).

---

## 5. Tab Pembayaran (Kasir)
Ketika pelanggan selesai makan dan ingin membayar tagihan, gunakan menu ini.

1. **Muat Tagihan:** Masukkan **Order ID** pelanggan dan klik **Muat Data Order**. Ringkasan pesanan akan muncul.
2. **Beri Diskon (Opsional):** Masukkan persentase diskon di kolom yang tersedia (misal: 10 untuk 10%), tekan Enter untuk melihat perubahan harga.
3. **Bayar:** Masukkan jumlah uang tunai yang diberikan pelanggan di kolom "Uang Bayar", lalu tekan Enter untuk melihat otomatis jumlah kembalian.
4. **Proses Pembayaran:** Klik tombol hijau **Proses Pembayaran**. Pesanan menjadi **`paid`** dan meja otomatis kembali kosong.
5. **Cetak Struk:** Klik tombol **Cetak Struk** untuk menampilkan struk digital di layar kecil sebelah kanan.

---

## 6. Tab Inventaris (Stok Bahan Baku)
Gunakan tab ini untuk mengecek ketersediaan bahan dapur.

- **Tambah Bahan:** Isi Nama, Kategori, Jumlah Stok, Satuan (kg/liter/pcs), Batas Minimum Stok, dan Harga Beli. Klik **Tambah Bahan**.
- **Update Stok:** Jika ada bahan yang dipakai, klik bahan di tabel, isi jumlah pemakaian di kolom angka (gunakan angka minus seperti `-5` untuk mengurangi, atau `10` untuk menambah), lalu klik **Update**.
- **Peringatan Stok:** Bahan yang stoknya berada di bawah batas minimum akan menyala merah muda dengan status **⚠ Menipis**. Klik **Cek Peringatan Stok** untuk memunculkan notifikasi pop-up bahan apa saja yang harus segera dibeli.

---

## 7. Tab Keuangan (Laporan)
Setiap transaksi yang berhasil dibayar akan masuk ke tab Keuangan.

- **Generate Laporan:** Klik tombol **Generate Laporan** untuk melihat ringkasan pendapatan hari ini, total diskon yang diberikan, dan rincian metode pembayaran yang paling sering digunakan pelanggan.
- Laporan juga menampilkan analisis lengkap riwayat status pesanan.

---

## 8. Fitur Pendukung Lainnya

- **Tab Staf:** Untuk menambahkan nama karyawan baru, menghapus, atau merotasi _Shift_ (mengubah siapa yang sedang bertugas hari ini).
- **Tab Layout Meja:** Menampilkan visualisasi jalur antar meja dalam restoran. Anda bisa mensimulasikan pencarian jalur terpendek (BFS) atau eksplorasi rute (DFS) antar meja.
- **Tab History (Undo):** Jika Anda salah menambahkan menu atau keliru mencatat stok, klik tab History dan tekan tombol **Undo Aksi Terakhir**.
- **Penyimpanan Data (Save/Load):** Di pojok kiri bawah masing-masing tab, sering kali terdapat tombol **Simpan**. Gunakan tombol ini untuk mengekspor data ke dalam bentuk file `.txt` di folder project Anda sebagai rekapitulasi harian.

---
*Semoga aplikasi ini dapat memperlancar bisnis restoran Anda! Selamat bekerja!* 🚀

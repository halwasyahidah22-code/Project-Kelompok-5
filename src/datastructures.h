#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================
//  NAMESPACE utama sistem restoran
// ============================================================
namespace RestaurantSystem {

// ============================================================
//  STRUCT definisi entitas (syarat 1: struct)
// ============================================================
struct MenuItem {
    int id;
    std::string name;
    std::string category;
    double price;
    bool available;
    MenuItem* next;   // untuk Linked List

    // Default argument (syarat 6)
    MenuItem(int i = 0, std::string n = "", std::string c = "Umum",
             double p = 0.0, bool a = true)
        : id(i), name(n), category(c), price(p), available(a), next(nullptr) {}
};

struct OrderItem {
    int menuItemId;
    std::string menuItemName;
    int quantity;
    double subtotal;
};

struct Order {
    int orderId;
    int tableNumber;
    std::vector<OrderItem> items;
    double totalPrice;
    std::string status; // "pending", "preparing", "served", "paid"
    int priority;       // 1=VIP, 2=Normal (untuk Priority Queue)
    std::string timestamp;

    Order* next; // untuk Linked List

    Order(int oid = 0, int tbl = 0, int prio = 2)
        : orderId(oid), tableNumber(tbl), totalPrice(0.0),
          status("pending"), priority(prio), next(nullptr) {}

    // operator < untuk Priority Queue (syarat 4)
    bool operator>(const Order& other) const {
        return priority > other.priority;
    }
};

struct Table {
    int tableNumber;
    int capacity;
    bool isOccupied;
    int currentOrderId;
    Table* next; // untuk Circular Linked List

    Table(int num = 0, int cap = 4)
        : tableNumber(num), capacity(cap),
          isOccupied(false), currentOrderId(-1), next(nullptr) {}
};

struct Staff {
    int staffId;
    std::string name;
    std::string role;
    bool onDuty;
    Staff* next; // untuk Circular Linked List

    Staff(int id = 0, std::string n = "", std::string r = "Waiter")
        : staffId(id), name(n), role(r), onDuty(false), next(nullptr) {}
};

// ============================================================
//  STRUCT: INVENTARIS / STOK BAHAN BAKU
// ============================================================
struct InventarisItem {
    int id;
    std::string nama;
    std::string kategori;
    int stok;
    std::string satuan;
    int minStok;
    double hargaBeli;
    InventarisItem* next; // untuk Linked List

    InventarisItem(int i = 0, std::string n = "", std::string k = "Bahan Pokok",
                   int s = 0, std::string sat = "kg", int minS = 10, double hb = 0.0)
        : id(i), nama(n), kategori(k), stok(s), satuan(sat),
          minStok(minS), hargaBeli(hb), next(nullptr) {}

    double nilaiStok() const { return stok * hargaBeli; }
    bool menipis()     const { return stok <= minStok; }
};

// ============================================================
//  STRUCT: TRANSAKSI PEMBAYARAN
// ============================================================
struct Transaksi {
    int transaksiId;
    int orderId;
    int tableNumber;
    double subtotal;
    double diskonPersen;
    double totalBayar;
    double jumlahBayar;  // uang yang diserahkan (untuk tunai)
    double kembalian;
    std::string metodePembayaran;
    std::string timestamp;
    Transaksi* next;

    Transaksi(int tid = 0, int oid = 0, int tbl = 0)
        : transaksiId(tid), orderId(oid), tableNumber(tbl),
          subtotal(0), diskonPersen(0), totalBayar(0),
          jumlahBayar(0), kembalian(0),
          metodePembayaran("Tunai"), next(nullptr) {}
};

// ============================================================
//  ACTION untuk Stack (History/Undo)
// ============================================================
struct Action {
    std::string type;  // "add_menu", "remove_menu", "add_order", etc.
    std::string description;
    int relatedId;

    Action(std::string t, std::string d, int id = -1)
        : type(t), description(d), relatedId(id) {}
};

// ============================================================
//  1. LINKED LIST (Single) — Master Data Menu
// ============================================================
class MenuLinkedList {
private:
    MenuItem* head;
    int size;

public:
    MenuLinkedList() : head(nullptr), size(0) {}

    ~MenuLinkedList() {
        MenuItem* curr = head;
        while (curr) {
            MenuItem* tmp = curr->next;
            delete curr;
            curr = tmp;
        }
    }

    // Insert dengan referensi (syarat 2: references &)
    void insert(const MenuItem& item) {
        MenuItem* node = new MenuItem(item.id, item.name,
                                      item.category, item.price,
                                      item.available);
        if (!head) {
            head = node;
        } else {
            MenuItem* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = node;
        }
        size++;
    }

    bool remove(int id) {
        if (!head) return false;
        if (head->id == id) {
            MenuItem* tmp = head;
            head = head->next;
            delete tmp;
            size--;
            return true;
        }
        MenuItem* curr = head;
        while (curr->next && curr->next->id != id)
            curr = curr->next;
        if (!curr->next) return false;
        MenuItem* tmp = curr->next;
        curr->next = tmp->next;
        delete tmp;
        size--;
        return true;
    }

    MenuItem* find(int id) const {
        MenuItem* curr = head;
        while (curr) {
            if (curr->id == id) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    // Menggunakan pointer (syarat 3)
    std::vector<MenuItem*> getAll() const {
        std::vector<MenuItem*> result;
        MenuItem* curr = head;
        while (curr) {
            result.push_back(curr);
            curr = curr->next;
        }
        return result;
    }

    int getSize() const { return size; }
    MenuItem* getHead() const { return head; }
};

// ============================================================
//  2. CIRCULAR LINKED LIST — Rotasi Meja & Shift Staf
// ============================================================
class TableCircularList {
private:
    Table* head;
    Table* tail;
    Table* current; // pointer meja aktif saat ini
    int size;

public:
    TableCircularList() : head(nullptr), tail(nullptr), current(nullptr), size(0) {}

    ~TableCircularList() {
        if (!head) return;
        Table* curr = head;
        for (int i = 0; i < size; i++) {
            Table* tmp = curr->next;
            delete curr;
            curr = tmp;
        }
    }

    void insert(const Table& t) {
        Table* node = new Table(t.tableNumber, t.capacity);
        if (!head) {
            head = tail = node;
            node->next = head;
            current = head;
        } else {
            tail->next = node;
            tail = node;
            tail->next = head;
        }
        size++;
    }

    // Putar ke meja berikutnya (sifat circular)
    Table* next() {
        if (!current) return nullptr;
        current = current->next;
        return current;
    }

    Table* findTable(int num) const {
        if (!head) return nullptr;
        Table* curr = head;
        do {
            if (curr->tableNumber == num) return curr;
            curr = curr->next;
        } while (curr != head);
        return nullptr;
    }

    bool occupyTable(int num, int orderId) {
        Table* t = findTable(num);
        if (!t || t->isOccupied) return false;
        t->isOccupied = true;
        t->currentOrderId = orderId;
        return true;
    }

    bool freeTable(int num) {
        Table* t = findTable(num);
        if (!t || !t->isOccupied) return false;
        t->isOccupied = false;
        t->currentOrderId = -1;
        return true;
    }

    std::vector<Table*> getAll() const {
        std::vector<Table*> result;
        if (!head) return result;
        Table* curr = head;
        do {
            result.push_back(curr);
            curr = curr->next;
        } while (curr != head);
        return result;
    }

    Table* getCurrent() const { return current; }
    int getSize() const { return size; }
};

// Circular Linked List untuk rotasi shift staf
class StaffCircularList {
private:
    Staff* head;
    Staff* tail;
    Staff* onDuty;
    int size;

public:
    StaffCircularList() : head(nullptr), tail(nullptr), onDuty(nullptr), size(0) {}

    ~StaffCircularList() {
        if (!head) return;
        Staff* curr = head;
        for (int i = 0; i < size; i++) {
            Staff* tmp = curr->next;
            delete curr;
            curr = tmp;
        }
    }

    void insert(const Staff& s) {
        Staff* node = new Staff(s.staffId, s.name, s.role);
        if (!head) {
            head = tail = node;
            node->next = head;
            onDuty = head;
        } else {
            tail->next = node;
            tail = node;
            tail->next = head;
        }
        size++;
    }

    bool remove(int staffId) {
        if (!head) return false;

        // Kasus: hanya 1 node
        if (head == tail && head->staffId == staffId) {
            if (onDuty == head) onDuty = nullptr;
            delete head;
            head = tail = nullptr;
            size--;
            return true;
        }

        // Cari node dengan traversal circular
        Staff* prev = tail;
        Staff* curr = head;
        do {
            if (curr->staffId == staffId) {
                // Jika yang dihapus sedang bertugas, geser onDuty ke berikutnya
                if (onDuty == curr) {
                    onDuty = curr->next;
                    if (onDuty) onDuty->onDuty = true;
                }
                if (curr == head) head = curr->next;
                if (curr == tail) tail = prev;
                prev->next = curr->next;
                delete curr;
                size--;
                return true;
            }
            prev = curr;
            curr = curr->next;
        } while (curr != head);

        return false;
    }

    Staff* rotateShift() {
        if (!onDuty) return nullptr;
        onDuty->onDuty = false;
        onDuty = onDuty->next;
        onDuty->onDuty = true;
        return onDuty;
    }

    std::vector<Staff*> getAll() const {
        std::vector<Staff*> result;
        if (!head) return result;
        Staff* curr = head;
        do {
            result.push_back(curr);
            curr = curr->next;
        } while (curr != head);
        return result;
    }

    Staff* getOnDuty() const { return onDuty; }
    int getSize() const { return size; }
};

// ============================================================
//  3. STACK (LIFO) — History & Undo
// ============================================================
class ActionStack {
private:
    std::stack<Action> history;
    static const int MAX_HISTORY = 50;

public:
    void push(const Action& a) {
        if (history.size() >= MAX_HISTORY) {
            // Trim tumpukan lama (copy to temp)
            std::stack<Action> tmp;
            int cnt = 0;
            while (!history.empty() && cnt < MAX_HISTORY - 1) {
                tmp.push(history.top());
                history.pop();
                cnt++;
            }
            history = tmp;
        }
        history.push(a);
    }

    Action pop() {
        if (history.empty())
            throw std::runtime_error("Tidak ada aksi untuk di-undo!");
        Action a = history.top();
        history.pop();
        return a;
    }

    const Action& peek() const {
        if (history.empty())
            throw std::runtime_error("Stack kosong!");
        return history.top();
    }

    bool isEmpty() const { return history.empty(); }
    int getSize() const { return (int)history.size(); }

    std::vector<Action> getAll() const {
        std::vector<Action> result;
        std::stack<Action> tmp = history;
        while (!tmp.empty()) {
            result.push_back(tmp.top());
            tmp.pop();
        }
        return result;
    }
};

// ============================================================
//  4. PRIORITY QUEUE (FIFO + Prioritas) — Antrian Order
// ============================================================
struct OrderComparator {
    bool operator()(const Order* a, const Order* b) const {
        if (a->priority != b->priority)
            return a->priority > b->priority; // priority kecil = lebih penting
        return a->orderId > b->orderId;       // FIFO dalam prioritas sama
    }
};

class OrderQueue {
private:
    std::priority_queue<Order*, std::vector<Order*>, OrderComparator> pq;
    std::list<Order*> allOrders; // STL list (syarat 9)
    int nextId;

public:
    OrderQueue() : nextId(1) {}

    Order* enqueue(int tableNum, int priority = 2) {
        Order* order = new Order(nextId++, tableNum, priority);
        pq.push(order);
        allOrders.push_back(order);
        return order;
    }

    Order* dequeue() {
        if (pq.empty())
            throw std::runtime_error("Antrian order kosong!");
        Order* order = pq.top();
        pq.pop();
        return order;
    }

    Order* front() const {
        if (pq.empty()) return nullptr;
        return pq.top();
    }

    bool isEmpty() const { return pq.empty(); }

    // Menggunakan STL list + iterator (syarat 9)
    std::list<Order*>& getAllOrders() { return allOrders; }

    // Lambda expression untuk filter (syarat 11)
    std::vector<Order*> getOrdersByStatus(const std::string& status) const {
        std::vector<Order*> result;
        auto it = allOrders.begin();
        while (it != allOrders.end()) {
            if ((*it)->status == status)
                result.push_back(*it);
            ++it;
        }
        return result;
    }

    std::vector<Order*> getPendingOrders() const {
        return getOrdersByStatus("pending");
    }

    int queueSize() const { return (int)pq.size(); }
    int totalOrders() const { return (int)allOrders.size(); }

    ~OrderQueue() {
        for (Order* o : allOrders)
            delete o;
        allOrders.clear();
    }
};

// ============================================================
//  5. AVL TREE — Pencarian Menu Cepat
// ============================================================
struct AVLNode {
    MenuItem data;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const MenuItem& m)
        : data(m), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    static int nodeHeight(AVLNode* n) { return n ? n->height : 0; }

    static int balanceFactor(AVLNode* n) {
        return n ? nodeHeight(n->left) - nodeHeight(n->right) : 0;
    }

    static void updateHeight(AVLNode* n) {
        if (n) n->height = 1 + std::max(nodeHeight(n->left), nodeHeight(n->right));
    }

    static AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    static AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    static AVLNode* balance(AVLNode* n) {
        updateHeight(n);
        int bf = balanceFactor(n);
        if (bf > 1) {
            if (balanceFactor(n->left) < 0)
                n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (bf < -1) {
            if (balanceFactor(n->right) > 0)
                n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }

    static AVLNode* insert(AVLNode* node, const MenuItem& item) {
        if (!node) return new AVLNode(item);
        if (item.id < node->data.id)
            node->left = insert(node->left, item);
        else if (item.id > node->data.id)
            node->right = insert(node->right, item);
        else
            node->data = item;
        return balance(node);
    }

    static AVLNode* findMin(AVLNode* n) {
        while (n->left) n = n->left;
        return n;
    }

    static AVLNode* removeNode(AVLNode* node, int id) {
        if (!node) return nullptr;
        if (id < node->data.id)
            node->left = removeNode(node->left, id);
        else if (id > node->data.id)
            node->right = removeNode(node->right, id);
        else {
            if (!node->left || !node->right) {
                AVLNode* tmp = node->left ? node->left : node->right;
                delete node;
                return tmp;
            }
            AVLNode* minRight = findMin(node->right);
            node->data = minRight->data;
            node->right = removeNode(node->right, minRight->data.id);
        }
        return balance(node);
    }

    static void inorder(AVLNode* node, std::vector<MenuItem>& result) {
        if (!node) return;
        inorder(node->left, result);
        result.push_back(node->data);
        inorder(node->right, result);
    }

    void destroy(AVLNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    AVLTree() : root(nullptr) {}
    ~AVLTree() { destroy(root); }

    void insert(const MenuItem& item) { root = insert(root, item); }
    void remove(int id) { root = removeNode(root, id); }

    MenuItem* search(int id) const {
        AVLNode* curr = root;
        while (curr) {
            if (id == curr->data.id) return &curr->data;
            curr = (id < curr->data.id) ? curr->left : curr->right;
        }
        return nullptr;
    }

    std::vector<MenuItem> inorderTraversal() const {
        std::vector<MenuItem> result;
        inorder(root, result);
        return result;
    }

    int getHeight() const { return nodeHeight(root); }
};

// ============================================================
//  6. GRAPH (BFS & DFS) — Relasi / Layout Meja Restoran
// ============================================================
class TableGraph {
private:
    std::unordered_map<int, std::vector<int>> adjList;

public:
    void addTable(int tableNum) {
        if (adjList.find(tableNum) == adjList.end())
            adjList[tableNum] = {};
    }

    void addEdge(int from, int to) {
        adjList[from].push_back(to);
        adjList[to].push_back(from);
    }

    // BFS — temukan jalur terpendek antara dua meja
    std::vector<int> bfs(int start) {
        std::vector<int> visited;
        std::unordered_map<int, bool> seen;
        std::queue<int> q;
        q.push(start);
        seen[start] = true;
        while (!q.empty()) {
            int node = q.front(); q.pop();
            visited.push_back(node);
            for (int neighbor : adjList[node]) {
                if (!seen[neighbor]) {
                    seen[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        return visited;
    }

    // DFS — eksplorasi seluruh layout dari satu titik
    std::vector<int> dfs(int start) {
        std::vector<int> visited;
        std::unordered_map<int, bool> seen;
        std::stack<int> s;
        s.push(start);
        while (!s.empty()) {
            int node = s.top(); s.pop();
            if (seen[node]) continue;
            seen[node] = true;
            visited.push_back(node);
            for (int neighbor : adjList[node])
                if (!seen[neighbor]) s.push(neighbor);
        }
        return visited;
    }

    std::vector<int> getNeighbors(int tableNum) const {
        auto it = adjList.find(tableNum);
        if (it == adjList.end()) return {};
        return it->second;
    }

    std::vector<int> getAllTables() const {
        std::vector<int> result;
        for (auto& pair : adjList)
            result.push_back(pair.first);
        std::sort(result.begin(), result.end());
        return result;
    }
};

// ============================================================
//  7. HASH TABLE — Lookup Cepat Item Menu
// ============================================================
class MenuHashTable {
private:
    std::unordered_map<int, MenuItem> table;    // key: id
    std::unordered_map<std::string, int> nameIndex; // key: nama lowercase

    std::string toLower(std::string s) const {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

public:
    void insert(const MenuItem& item) {
        table[item.id] = item;
        nameIndex[toLower(item.name)] = item.id;
    }

    void remove(int id) {
        auto it = table.find(id);
        if (it != table.end()) {
            nameIndex.erase(toLower(it->second.name));
            table.erase(it);
        }
    }

    MenuItem* findById(int id) {
        auto it = table.find(id);
        return (it != table.end()) ? &it->second : nullptr;
    }

    MenuItem* findByName(const std::string& name) {
        auto it = nameIndex.find(toLower(name));
        if (it == nameIndex.end()) return nullptr;
        return findById(it->second);
    }

    // STL count (syarat 9)
    int countByCategory(const std::string& cat) const {
        return (int)std::count_if(table.begin(), table.end(),
            [&cat](const auto& pair) {
                return pair.second.category == cat;
            });
    }

    std::vector<MenuItem> getAll() const {
        std::vector<MenuItem> result;
        for (auto& pair : table)
            result.push_back(pair.second);
        return result;
    }

    int size() const { return (int)table.size(); }
};

// ============================================================
//  8. SORTING — Laporan & Ranking
// ============================================================
namespace Sorting {
    // Bubble Sort untuk menu berdasarkan harga
    inline void bubbleSortByPrice(std::vector<MenuItem>& vec, bool ascending = true) {
        int n = vec.size();
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if ((ascending && vec[j].price > vec[j+1].price) ||
                    (!ascending && vec[j].price < vec[j+1].price))
                    std::swap(vec[j], vec[j+1]);
    }

    // STL sort dengan lambda (syarat 9 + 11)
    inline void sortByName(std::vector<MenuItem>& vec) {
        std::sort(vec.begin(), vec.end(),
            [](const MenuItem& a, const MenuItem& b) {
                return a.name < b.name;
            });
    }

    // Merge Sort untuk order berdasarkan total
    inline void merge(std::vector<Order*>& vec, int l, int m, int r) {
        std::vector<Order*> left(vec.begin()+l, vec.begin()+m+1);
        std::vector<Order*> right(vec.begin()+m+1, vec.begin()+r+1);
        int i=0, j=0, k=l;
        while (i<(int)left.size() && j<(int)right.size())
            vec[k++] = (left[i]->totalPrice >= right[j]->totalPrice) ? left[i++] : right[j++];
        while (i<(int)left.size()) vec[k++] = left[i++];
        while (j<(int)right.size()) vec[k++] = right[j++];
    }

    inline void mergeSort(std::vector<Order*>& vec, int l, int r) {
        if (l >= r) return;
        int m = (l + r) / 2;
        mergeSort(vec, l, m);
        mergeSort(vec, m+1, r);
        merge(vec, l, m, r);
    }
}

// ============================================================
//  FUNCTION TEMPLATE — Generic search & utility (syarat 7)
// ============================================================

// [TEMPLATE 1] linearSearch — mencari satu elemen pertama yang cocok
template<typename T>
T* linearSearch(std::vector<T*>& vec, std::function<bool(T*)> predicate) {
    // STL find_if dengan lambda (syarat 9 + 11)
    auto it = std::find_if(vec.begin(), vec.end(), predicate);
    return (it != vec.end()) ? *it : nullptr;
}

// [TEMPLATE 2] linearSearchAll — mencari SEMUA elemen yang cocok
template<typename T>
std::vector<T*> linearSearchAll(std::vector<T*>& vec, std::function<bool(T*)> predicate) {
    std::vector<T*> result;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(result), predicate);
    return result;
}

// [TEMPLATE 3] countIf — menghitung elemen yang memenuhi kondisi
template<typename T>
int countIf(const std::vector<T*>& vec, std::function<bool(const T*)> predicate) {
    return (int)std::count_if(vec.begin(), vec.end(), predicate);
}

// ============================================================
//  FUNCTION OVERLOADING — displayItem (syarat 7)
//  Fungsi dengan nama sama, parameter berbeda (overloading)
// ============================================================

// Overload 1: menampilkan info MenuItem ke string
inline std::string displayItem(const MenuItem& m) {
    return "[MENU #" + std::to_string(m.id) + "] "
         + m.name + " | " + m.category
         + " | Rp" + std::to_string((int)m.price)
         + (m.available ? " | TERSEDIA" : " | HABIS");
}

// Overload 2: menampilkan info Order ke string
inline std::string displayItem(const Order& o) {
    return "[ORDER #" + std::to_string(o.orderId) + "] "
         + "Meja " + std::to_string(o.tableNumber)
         + " | Total Rp" + std::to_string((int)o.totalPrice)
         + " | Status: " + o.status
         + (o.priority == 1 ? " | VIP" : " | Normal");
}

// Overload 3: menampilkan info Staff ke string
inline std::string displayItem(const Staff& s) {
    return "[STAF #" + std::to_string(s.staffId) + "] "
         + s.name + " | " + s.role
         + (s.onDuty ? " | BERTUGAS" : " | ISTIRAHAT");
}

// ============================================================
//  CALLBACK FUNCTION — Syarat 5
// ============================================================
using MenuCallback  = std::function<void(const MenuItem&)>;
using OrderCallback = std::function<void(const Order&)>;
using StaffCallback = std::function<void(const Staff&)>;

// [CALLBACK 1] forEachMenu — iterasi semua menu dengan callback
inline void forEachMenu(const std::vector<MenuItem*>& menus, MenuCallback cb) {
    for (const auto* m : menus) {
        if (m) cb(*m);
    }
}

// [CALLBACK 2] applyToAllOrders — iterasi semua order dengan callback
// Menerima std::list<Order*> langsung dari OrderQueue::getAllOrders()
inline void applyToAllOrders(std::list<Order*>& orders, OrderCallback cb) {
    for (auto* o : orders) {
        if (o) cb(*o);
    }
}

// [CALLBACK 3] filterMenuByCategory — filter menu berdasarkan kategori,
// lalu jalankan callback untuk setiap item yang lolos filter
inline void filterMenuByCategory(const std::vector<MenuItem*>& menus,
                                  const std::string& category,
                                  MenuCallback cb) {
    for (const auto* m : menus) {
        if (m && m->category == category) cb(*m);
    }
}

// [CALLBACK 4] processMenusWithCondition — callback generik dengan predikat
// Menerima predikat (predicate) + callback, hanya jalankan cb jika predikat terpenuhi
inline void processMenusWithCondition(const std::vector<MenuItem*>& menus,
                                       std::function<bool(const MenuItem&)> predicate,
                                       MenuCallback cb) {
    for (const auto* m : menus) {
        if (m && predicate(*m)) cb(*m);
    }
}

// ============================================================
//  STL UTILITIES — std::find dan std::count (syarat 9)
// ============================================================
namespace STLUtils {

    // [STL FIND] Cari order berdasarkan orderId menggunakan std::find
    // Mengembalikan iterator, atau allOrders.end() jika tidak ditemukan
    inline std::list<Order*>::iterator findOrderById(std::list<Order*>& allOrders, int targetId) {
        // std::find_if dengan lambda — mencari order dengan id tertentu
        return std::find_if(allOrders.begin(), allOrders.end(),
            [targetId](Order* o) { return o->orderId == targetId; });
    }

    // [STL FIND] Cari ID menu dalam vector<int> menggunakan std::find (exact match)
    inline bool menuIdExists(const std::vector<int>& idList, int targetId) {
        // std::find — pencarian exact match pada tipe primitif
        auto it = std::find(idList.begin(), idList.end(), targetId);
        return it != idList.end();
    }

    // [STL COUNT] Hitung berapa order dengan status tertentu menggunakan std::count_if
    inline int countOrdersByStatus(const std::list<Order*>& orders,
                                    const std::string& status) {
        return (int)std::count_if(orders.begin(), orders.end(),
            [&status](Order* o) { return o->status == status; });
    }

    // [STL COUNT] Hitung berapa menu yang tersedia (available == true) menggunakan std::count_if
    inline int countAvailableMenus(const std::vector<MenuItem*>& menus) {
        return (int)std::count_if(menus.begin(), menus.end(),
            [](MenuItem* m) { return m && m->available; });
    }

    // [STL COUNT] Hitung meja yang sedang terisi menggunakan std::count_if
    inline int countOccupiedTables(const std::vector<Table*>& tables) {
        return (int)std::count_if(tables.begin(), tables.end(),
            [](Table* t) { return t && t->isOccupied; });
    }

    // [STL FIND + COUNT gabungan] Rangkuman statistik order dalam satu fungsi
    // Mengembalikan map status -> jumlah
    inline std::unordered_map<std::string, int> getOrderStatusSummary(
            const std::list<Order*>& orders) {
        std::unordered_map<std::string, int> summary;
        for (const auto& s : std::vector<std::string>{"pending", "preparing", "served", "paid"}) {
            summary[s] = (int)std::count_if(orders.begin(), orders.end(),
                [&s](Order* o) { return o->status == s; });
        }
        return summary;
    }

} // namespace STLUtils

// ============================================================
//  INVENTARIS LINKED LIST — Manajemen Stok Bahan Baku
// ============================================================
class InventarisLinkedList {
private:
    InventarisItem* head;
    int size;
    int nextId;

public:
    InventarisLinkedList() : head(nullptr), size(0), nextId(1) {}

    ~InventarisLinkedList() {
        InventarisItem* curr = head;
        while (curr) {
            InventarisItem* tmp = curr->next;
            delete curr;
            curr = tmp;
        }
    }

    InventarisItem* insert(const InventarisItem& item) {
        InventarisItem* node = new InventarisItem(
            nextId++, item.nama, item.kategori,
            item.stok, item.satuan, item.minStok, item.hargaBeli);
        if (!head) {
            head = node;
        } else {
            InventarisItem* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = node;
        }
        size++;
        return node;
    }

    bool remove(int id) {
        if (!head) return false;
        if (head->id == id) {
            InventarisItem* tmp = head;
            head = head->next;
            delete tmp; size--;
            return true;
        }
        InventarisItem* curr = head;
        while (curr->next && curr->next->id != id)
            curr = curr->next;
        if (!curr->next) return false;
        InventarisItem* tmp = curr->next;
        curr->next = tmp->next;
        delete tmp; size--;
        return true;
    }

    InventarisItem* findById(int id) const {
        InventarisItem* curr = head;
        while (curr) {
            if (curr->id == id) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    bool updateStok(int id, int delta) {
        InventarisItem* item = findById(id);
        if (!item) return false;
        int newStok = item->stok + delta;
        if (newStok < 0) newStok = 0;
        item->stok = newStok;
        return true;
    }

    std::vector<InventarisItem*> getAll() const {
        std::vector<InventarisItem*> result;
        InventarisItem* curr = head;
        while (curr) { result.push_back(curr); curr = curr->next; }
        return result;
    }

    std::vector<InventarisItem*> getMenipis() const {
        std::vector<InventarisItem*> result;
        InventarisItem* curr = head;
        while (curr) {
            if (curr->menipis()) result.push_back(curr);
            curr = curr->next;
        }
        return result;
    }

    double getTotalNilaiStok() const {
        double total = 0.0;
        InventarisItem* curr = head;
        while (curr) { total += curr->nilaiStok(); curr = curr->next; }
        return total;
    }

    int getSize() const { return size; }
};

// ============================================================
//  TRANSAKSI LINKED LIST — Riwayat Pembayaran
// ============================================================
class TransaksiList {
private:
    std::vector<Transaksi*> transaksiVec;
    int nextId;

public:
    TransaksiList() : nextId(1) {}

    ~TransaksiList() {
        for (auto* t : transaksiVec) delete t;
    }

    Transaksi* tambah(int orderId, int tableNum, double subtotal,
                       double diskonPersen, double jumlahBayar,
                       const std::string& metode,
                       const std::string& timestamp) {
        double diskonNominal = subtotal * (diskonPersen / 100.0);
        double total         = subtotal - diskonNominal;
        double kembalian     = (metode == "Tunai" || metode == "💵 Tunai")
                               ? jumlahBayar - total : 0.0;

        Transaksi* t = new Transaksi(nextId++, orderId, tableNum);
        t->subtotal          = subtotal;
        t->diskonPersen      = diskonPersen;
        t->totalBayar        = total;
        t->jumlahBayar       = jumlahBayar;
        t->kembalian         = kembalian;
        t->metodePembayaran  = metode;
        t->timestamp         = timestamp;
        transaksiVec.push_back(t);
        return t;
    }

    std::vector<Transaksi*>& getAll() { return transaksiVec; }

    double getTotalPendapatan() const {
        double total = 0.0;
        for (auto* t : transaksiVec) total += t->totalBayar;
        return total;
    }

    double getTotalDiskon() const {
        double total = 0.0;
        for (auto* t : transaksiVec)
            total += t->subtotal * (t->diskonPersen / 100.0);
        return total;
    }

    double getRataRata() const {
        if (transaksiVec.empty()) return 0.0;
        return getTotalPendapatan() / transaksiVec.size();
    }

    double getTerbesar() const {
        double max = 0.0;
        for (auto* t : transaksiVec)
            if (t->totalBayar > max) max = t->totalBayar;
        return max;
    }

    Transaksi* findByOrderId(int orderId) const {
        for (auto* t : transaksiVec)
            if (t->orderId == orderId) return t;
        return nullptr;
    }

    int getSize() const { return (int)transaksiVec.size(); }
};


namespace FileIO {
    inline bool saveMenuToFile(const std::vector<MenuItem>& menus,
                                const std::string& filename = "menu.txt") {
        std::ofstream ofs(filename);
        if (!ofs.is_open()) return false;
        ofs << "=== DATA MENU RESTORAN ===\n";
        for (const auto& m : menus) {
            ofs << m.id << "|" << m.name << "|" << m.category
                << "|" << m.price << "|" << (m.available ? "1" : "0") << "\n";
        }
        ofs.close();
        return true;
    }

    inline std::vector<MenuItem> loadMenuFromFile(const std::string& filename = "menu.txt") {
        std::vector<MenuItem> result;
        std::ifstream ifs(filename);
        if (!ifs.is_open()) return result;
        std::string line;
        std::getline(ifs, line); // skip header
        while (std::getline(ifs, line)) {
            std::istringstream ss(line);
            std::string tok;
            std::vector<std::string> parts;
            while (std::getline(ss, tok, '|')) parts.push_back(tok);
            if (parts.size() >= 5) {
                MenuItem m(std::stoi(parts[0]), parts[1], parts[2],
                           std::stod(parts[3]), parts[4] == "1");
                result.push_back(m);
            }
        }
        return result;
    }

    inline bool saveInventarisToFile(const std::vector<InventarisItem*>& items,
                                      const std::string& filename = "inventaris.txt") {
        std::ofstream ofs(filename);
        if (!ofs.is_open()) return false;
        ofs << "=== DATA INVENTARIS BAHAN BAKU ===\n";
        for (const auto* i : items)
            ofs << i->id << "|" << i->nama << "|" << i->kategori << "|"
                << i->stok << "|" << i->satuan << "|" << i->minStok
                << "|" << i->hargaBeli << "\n";
        ofs.close();
        return true;
    }

    inline bool saveLaporanKeuanganToFile(const std::vector<Transaksi*>& transaksi,
                                           double totalPendapatan, double totalDiskon,
                                           const std::string& filename = "laporan_keuangan.txt") {
        std::ofstream ofs(filename);
        if (!ofs.is_open()) return false;
        ofs << "=== LAPORAN KEUANGAN ===\n";
        ofs << "Total Pendapatan: Rp" << (long long)totalPendapatan << "\n";
        ofs << "Total Diskon   : Rp" << (long long)totalDiskon << "\n";
        ofs << "Jumlah Transaksi: " << transaksi.size() << "\n";
        ofs << "---------------------------\n";
        for (const auto* t : transaksi)
            ofs << "TID:" << t->transaksiId
                << " | OID:" << t->orderId
                << " | Meja:" << t->tableNumber
                << " | Subtotal:Rp" << (long long)t->subtotal
                << " | Diskon:" << t->diskonPersen << "%"
                << " | Total:Rp" << (long long)t->totalBayar
                << " | Metode:" << t->metodePembayaran
                << " | " << t->timestamp << "\n";
        ofs.close();
        return true;
    }

                                  const std::string& filename = "orders.txt") {
        std::ofstream ofs(filename);
        if (!ofs.is_open()) return false;
        ofs << "=== LAPORAN ORDER ===\n";
        for (const auto* o : orders) {
            ofs << "OrderID:" << o->orderId
                << " | Meja:" << o->tableNumber
                << " | Total:Rp" << o->totalPrice
                << " | Status:" << o->status << "\n";
            for (const auto& item : o->items)
                ofs << "  - " << item.menuItemName
                    << " x" << item.quantity
                    << " = Rp" << item.subtotal << "\n";
        }
        ofs.close();
        return true;
    }
}

// ============================================================
//  EXCEPTION HANDLING — Syarat 8
// ============================================================
class RestaurantException : public std::exception {
    std::string msg;
public:
    RestaurantException(const std::string& m) : msg(m) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

} // namespace RestaurantSystem

#endif // DATASTRUCTURES_H

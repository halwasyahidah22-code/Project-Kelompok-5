#include "mainwindow.h"
#include <QApplication>
#include <ctime>

// ============================================================
//  KONSTRUKTOR & SETUP
// ============================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      nextMenuId(1), nextStaffId(1),
      currentOrder(nullptr), currentOrderTotal(0.0)
{
    // Inisialisasi semua struktur data
    menuList   = new MenuLinkedList();
    tableList  = new TableCircularList();
    staffList  = new StaffCircularList();
    orderQueue = new OrderQueue();
    menuAVL    = new AVLTree();
    menuHash   = new MenuHashTable();
    tableGraph = new TableGraph();
    actionStack= new ActionStack();

    currentOrderItems.clear();

    // Init pointer UI tambahan (akan diset di setupDashboard)
    lblAvailableMenus = nullptr;
    lblMenuByCategory = nullptr;
    statsDisplay      = nullptr;

    setupUI();
    setupInitialData();
    applyStyleSheet();

    // Timer untuk update waktu setiap detik
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    timer->start(1000);

    setWindowTitle("🍽 Sistem Manajemen Restoran - Qt");
    resize(1200, 750);
    refreshDashboard();
}

MainWindow::~MainWindow() {
    delete menuList;
    delete tableList;
    delete staffList;
    delete orderQueue;
    delete menuAVL;
    delete menuHash;
    delete tableGraph;
    delete actionStack;
    if (currentOrder) delete currentOrder;
}

// ============================================================
//  SETUP UI UTAMA
// ============================================================
void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setupDashboard();
    setupMenuTab();
    setupOrderTab();
    setupTableTab();
    setupStaffTab();
    setupHistoryTab();

    tabWidget->addTab(dashboardTab,  "📊 Dashboard");
    tabWidget->addTab(menuTab,       "🍜 Menu");
    tabWidget->addTab(orderTab,      "📋 Order");
    tabWidget->addTab(tableTab,      "🪑 Meja");
    tabWidget->addTab(staffTab,      "👤 Staf");
    tabWidget->addTab(historyTab,    "📁 Riwayat");

    statusBar()->showMessage("Sistem Manajemen Restoran siap digunakan.");
}

// ============================================================
//  DASHBOARD
// ============================================================
void MainWindow::setupDashboard() {
    dashboardTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(dashboardTab);

    QLabel* title = new QLabel("📊 Dashboard Restoran");
    title->setAlignment(Qt::AlignCenter);
    QFont f = title->font();
    f.setPointSize(16);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    lblCurrentTime = new QLabel("--:--:--");
    lblCurrentTime->setAlignment(Qt::AlignCenter);
    layout->addWidget(lblCurrentTime);

    // Stat cards
    QHBoxLayout* cards = new QHBoxLayout();

    auto makeCard = [&](const QString& title, QLabel*& lbl, const QString& color) {
        QGroupBox* box = new QGroupBox(title);
        box->setStyleSheet(QString("QGroupBox { border: 2px solid %1; border-radius: 8px; "
                                   "font-weight: bold; padding-top: 10px; }").arg(color));
        QVBoxLayout* bl = new QVBoxLayout(box);
        lbl = new QLabel("0");
        lbl->setAlignment(Qt::AlignCenter);
        QFont lf = lbl->font();
        lf.setPointSize(28);
        lf.setBold(true);
        lbl->setFont(lf);
        lbl->setStyleSheet(QString("color: %1;").arg(color));
        bl->addWidget(lbl);
        return box;
    };

    cards->addWidget(makeCard("Total Menu",      lblTotalMenu,      "#2196F3"));
    cards->addWidget(makeCard("Total Order",     lblTotalOrders,    "#4CAF50"));
    cards->addWidget(makeCard("Order Pending",   lblPendingOrders,  "#FF9800"));
    cards->addWidget(makeCard("Meja Terpakai",   lblActiveTables,   "#F44336"));
    layout->addLayout(cards);

    // Baris kedua: stat cards STL count (syarat 9 — std::count_if)
    QHBoxLayout* cards2 = new QHBoxLayout();
    cards2->addWidget(makeCard("Menu Tersedia",  lblAvailableMenus, "#00897B"));

    // lblMenuByCategory dipakai untuk menampilkan jumlah menu per kategori
    lblMenuByCategory = new QLabel("Makanan: 0 | Minuman: 0 | Snack: 0");
    lblMenuByCategory->setAlignment(Qt::AlignCenter);
    lblMenuByCategory->setStyleSheet("font-size: 12px; color: #616161; padding: 4px;");
    cards2->addWidget(lblMenuByCategory);
    layout->addLayout(cards2);

    QLabel* logTitle = new QLabel("📝 Log Aktivitas:");
    layout->addWidget(logTitle);
    logDisplay = new QTextEdit();
    logDisplay->setReadOnly(true);
    logDisplay->setMaximumHeight(160);
    layout->addWidget(logDisplay);

    // Panel statistik STL (std::count + std::find) — syarat 9
    QLabel* statsTitle = new QLabel("📊 Statistik STL (std::count + std::find):");
    statsTitle->setStyleSheet("font-weight: bold; margin-top: 4px;");
    layout->addWidget(statsTitle);
    statsDisplay = new QTextEdit();
    statsDisplay->setReadOnly(true);
    statsDisplay->setMaximumHeight(100);
    statsDisplay->setStyleSheet("font-family: Courier New; font-size: 12px; background: #FAFAFA;");
    layout->addWidget(statsDisplay);

    QPushButton* btnRefresh = new QPushButton("🔄 Refresh Dashboard");
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refreshDashboard);
    layout->addWidget(btnRefresh);
    layout->addStretch();
}

// ============================================================
//  MENU TAB
// ============================================================
void MainWindow::setupMenuTab() {
    menuTab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(menuTab);

    // Left: form input
    QGroupBox* formBox = new QGroupBox("Tambah / Edit Menu");
    formBox->setMaximumWidth(280);
    QVBoxLayout* fl = new QVBoxLayout(formBox);

    fl->addWidget(new QLabel("Nama Menu:"));
    inputMenuName = new QLineEdit();
    inputMenuName->setPlaceholderText("Contoh: Nasi Goreng");
    fl->addWidget(inputMenuName);

    fl->addWidget(new QLabel("Kategori:"));
    inputMenuCategory = new QComboBox();
    inputMenuCategory->addItems({"Makanan Utama", "Minuman", "Snack", "Dessert", "Vegetarian"});
    fl->addWidget(inputMenuCategory);

    fl->addWidget(new QLabel("Harga (Rp):"));
    inputMenuPrice = new QDoubleSpinBox();
    inputMenuPrice->setRange(0, 999999);
    inputMenuPrice->setSingleStep(1000);
    inputMenuPrice->setPrefix("Rp ");
    fl->addWidget(inputMenuPrice);

    inputMenuAvailable = new QCheckBox("Tersedia");
    inputMenuAvailable->setChecked(true);
    fl->addWidget(inputMenuAvailable);

    QPushButton* btnAdd = new QPushButton("➕ Tambah Menu");
    btnAdd->setObjectName("btnPrimary");
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddMenuItem);
    fl->addWidget(btnAdd);

    QPushButton* btnRemove = new QPushButton("🗑 Hapus Menu Terpilih");
    btnRemove->setObjectName("btnDanger");
    connect(btnRemove, &QPushButton::clicked, this, &MainWindow::onRemoveMenuItem);
    fl->addWidget(btnRemove);

    fl->addWidget(new QLabel("──────────────"));

    fl->addWidget(new QLabel("Cari Menu:"));
    searchMenuInput = new QLineEdit();
    searchMenuInput->setPlaceholderText("Nama atau ID...");
    connect(searchMenuInput, &QLineEdit::textChanged, this, &MainWindow::onSearchMenu);
    fl->addWidget(searchMenuInput);

    fl->addWidget(new QLabel("Urutkan:"));
    sortMenuCombo = new QComboBox();
    sortMenuCombo->addItems({"ID (default)", "Harga ↑", "Harga ↓", "Nama A-Z"});
    connect(sortMenuCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortMenu);
    fl->addWidget(sortMenuCombo);

    QPushButton* btnSave = new QPushButton("💾 Simpan ke File");
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveMenu);
    fl->addWidget(btnSave);

    QPushButton* btnLoad = new QPushButton("📂 Muat dari File");
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadMenu);
    fl->addWidget(btnLoad);

    // Tombol callback filterMenuByCategory (syarat 5)
    fl->addWidget(new QLabel("──────────────"));
    fl->addWidget(new QLabel("Filter & Callback:"));

    filterCategoryCombo = new QComboBox();
    filterCategoryCombo->addItems({"Makanan Utama","Minuman","Snack","Dessert","Vegetarian"});
    fl->addWidget(filterCategoryCombo);

    QPushButton* btnFilter = new QPushButton("🔎 Filter Kategori (Callback)");
    btnFilter->setObjectName("btnPrimary");
    connect(btnFilter, &QPushButton::clicked, this, &MainWindow::onFilterByCategory);
    fl->addWidget(btnFilter);

    QPushButton* btnResetFilter = new QPushButton("↩ Reset Filter");
    connect(btnResetFilter, &QPushButton::clicked, this, &MainWindow::refreshMenuTable);
    fl->addWidget(btnResetFilter);

    fl->addStretch();
    layout->addWidget(formBox);

    // Right: table display
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* menuTitle = new QLabel("📋 Daftar Menu (Linked List + AVL Tree + Hash Table)");
    menuTitle->setStyleSheet("font-weight: bold; font-size: 13px;");
    rightLayout->addWidget(menuTitle);

    menuTable = new QTableWidget();
    menuTable->setColumnCount(5);
    menuTable->setHorizontalHeaderLabels({"ID", "Nama", "Kategori", "Harga", "Tersedia"});
    menuTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    menuTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    menuTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    menuTable->setAlternatingRowColors(true);
    rightLayout->addWidget(menuTable);

    layout->addLayout(rightLayout);
}

// ============================================================
//  ORDER TAB
// ============================================================
void MainWindow::setupOrderTab() {
    orderTab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(orderTab);

    // Left: Buat Order Baru
    QGroupBox* createBox = new QGroupBox("Buat Order Baru");
    createBox->setMaximumWidth(300);
    QVBoxLayout* cl = new QVBoxLayout(createBox);

    cl->addWidget(new QLabel("Nomor Meja:"));
    orderTableSelect = new QComboBox();
    cl->addWidget(orderTableSelect);

    cl->addWidget(new QLabel("Prioritas:"));
    orderPrioritySelect = new QComboBox();
    orderPrioritySelect->addItems({"Normal (2)", "VIP (1)"});
    cl->addWidget(orderPrioritySelect);

    QPushButton* btnCreate = new QPushButton("🆕 Mulai Order Baru");
    btnCreate->setObjectName("btnPrimary");
    connect(btnCreate, &QPushButton::clicked, this, &MainWindow::onCreateOrder);
    cl->addWidget(btnCreate);

    cl->addWidget(new QLabel("──────────────"));
    cl->addWidget(new QLabel("Tambah Item ke Order:"));

    orderMenuSelect = new QComboBox();
    cl->addWidget(orderMenuSelect);

    cl->addWidget(new QLabel("Jumlah:"));
    orderQtyInput = new QSpinBox();
    orderQtyInput->setRange(1, 50);
    cl->addWidget(orderQtyInput);

    QPushButton* btnAddItem = new QPushButton("➕ Tambah Item");
    connect(btnAddItem, &QPushButton::clicked, this, &MainWindow::onAddItemToOrder);
    cl->addWidget(btnAddItem);

    QPushButton* btnRemItem = new QPushButton("➖ Hapus Item");
    connect(btnRemItem, &QPushButton::clicked, this, &MainWindow::onRemoveItemFromOrder);
    cl->addWidget(btnRemItem);

    cl->addWidget(new QLabel("Item dalam Order Saat Ini:"));
    orderItemList = new QListWidget();
    orderItemList->setMaximumHeight(120);
    cl->addWidget(orderItemList);

    lblOrderTotal = new QLabel("Total: Rp 0");
    lblOrderTotal->setStyleSheet("font-weight: bold; font-size: 14px; color: #4CAF50;");
    cl->addWidget(lblOrderTotal);

    QPushButton* btnSubmit = new QPushButton("✅ Submit Order");
    btnSubmit->setObjectName("btnSuccess");
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onSubmitOrder);
    cl->addWidget(btnSubmit);

    cl->addWidget(new QLabel("──────────────"));

    QPushButton* btnProcess = new QPushButton("⚙ Proses Order Berikutnya");
    btnProcess->setObjectName("btnWarning");
    connect(btnProcess, &QPushButton::clicked, this, &MainWindow::onProcessNextOrder);
    cl->addWidget(btnProcess);

    cl->addStretch();
    layout->addWidget(createBox);

    // Right: tabel order
    QVBoxLayout* rightL = new QVBoxLayout();
    QLabel* orderTitle = new QLabel("📋 Semua Order (Priority Queue + STL List)");
    orderTitle->setStyleSheet("font-weight: bold; font-size: 13px;");
    rightL->addWidget(orderTitle);

    orderTable = new QTableWidget();
    orderTable->setColumnCount(6);
    orderTable->setHorizontalHeaderLabels({"ID", "Meja", "Total", "Status", "Prioritas", "Aksi"});
    orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable->setAlternatingRowColors(true);
    rightL->addWidget(orderTable);

    QLabel* pendingTitle = new QLabel("⏳ Antrian Order (Priority Queue - FIFO + VIP):");
    rightL->addWidget(pendingTitle);
    pendingOrdersTable = new QTableWidget();
    pendingOrdersTable->setColumnCount(4);
    pendingOrdersTable->setHorizontalHeaderLabels({"ID", "Meja", "Prioritas", "Total"});
    pendingOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    pendingOrdersTable->setMaximumHeight(160);
    pendingOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rightL->addWidget(pendingOrdersTable);

    layout->addLayout(rightL);
}

// ============================================================
//  TABLE TAB
// ============================================================
void MainWindow::setupTableTab() {
    tableTab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(tableTab);

    // Left: controls
    QGroupBox* ctrl = new QGroupBox("Manajemen Meja");
    ctrl->setMaximumWidth(280);
    QVBoxLayout* cl = new QVBoxLayout(ctrl);

    lblCurrentTable = new QLabel("Meja Aktif: -");
    lblCurrentTable->setStyleSheet("font-weight: bold; font-size: 16px;");
    cl->addWidget(lblCurrentTable);

    QPushButton* btnNext = new QPushButton("▶ Meja Berikutnya (Circular)");
    connect(btnNext, &QPushButton::clicked, this, &MainWindow::onNextTable);
    cl->addWidget(btnNext);

    QPushButton* btnOccupy = new QPushButton("🔴 Tandai Terisi");
    btnOccupy->setObjectName("btnDanger");
    connect(btnOccupy, &QPushButton::clicked, this, &MainWindow::onOccupyTable);
    cl->addWidget(btnOccupy);

    QPushButton* btnFree = new QPushButton("🟢 Tandai Kosong");
    btnFree->setObjectName("btnSuccess");
    connect(btnFree, &QPushButton::clicked, this, &MainWindow::onFreeTable);
    cl->addWidget(btnFree);

    cl->addWidget(new QLabel("──────────────"));
    cl->addWidget(new QLabel("Graph BFS / DFS:"));

    QPushButton* btnBFS = new QPushButton("🔍 Jalankan BFS");
    connect(btnBFS, &QPushButton::clicked, this, &MainWindow::onRunBFS);
    cl->addWidget(btnBFS);

    QPushButton* btnDFS = new QPushButton("🔍 Jalankan DFS");
    connect(btnDFS, &QPushButton::clicked, this, &MainWindow::onRunDFS);
    cl->addWidget(btnDFS);

    cl->addWidget(new QLabel("──────────────"));
    cl->addWidget(new QLabel("Output Graph:"));
    graphDisplay = new QTextEdit();
    graphDisplay->setReadOnly(true);
    graphDisplay->setMaximumHeight(200);
    cl->addWidget(graphDisplay);

    cl->addStretch();
    layout->addWidget(ctrl);

    // Right: table grid
    QVBoxLayout* rightL = new QVBoxLayout();
    QLabel* tblTitle = new QLabel("🪑 Status Meja (Circular Linked List + Graph)");
    tblTitle->setStyleSheet("font-weight: bold; font-size: 13px;");
    rightL->addWidget(tblTitle);

    tableDisplay = new QTableWidget();
    tableDisplay->setColumnCount(4);
    tableDisplay->setHorizontalHeaderLabels({"No. Meja", "Kapasitas", "Status", "Order ID"});
    tableDisplay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableDisplay->setAlternatingRowColors(true);
    rightL->addWidget(tableDisplay);

    layout->addLayout(rightL);
}

// ============================================================
//  STAFF TAB
// ============================================================
void MainWindow::setupStaffTab() {
    staffTab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(staffTab);

    QGroupBox* formBox = new QGroupBox("Manajemen Staf");
    formBox->setMaximumWidth(280);
    QVBoxLayout* fl = new QVBoxLayout(formBox);

    fl->addWidget(new QLabel("Nama Staf:"));
    inputStaffName = new QLineEdit();
    inputStaffName->setPlaceholderText("Nama lengkap");
    fl->addWidget(inputStaffName);

    fl->addWidget(new QLabel("Jabatan:"));
    inputStaffRole = new QComboBox();
    inputStaffRole->addItems({"Waiter", "Kasir", "Koki", "Manager", "Cleaning"});
    fl->addWidget(inputStaffRole);

    QPushButton* btnAdd = new QPushButton("➕ Tambah Staf");
    btnAdd->setObjectName("btnPrimary");
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddStaff);
    fl->addWidget(btnAdd);

    QPushButton* btnRemove = new QPushButton("🗑 Hapus Staf");
    btnRemove->setObjectName("btnDanger");
    connect(btnRemove, &QPushButton::clicked, this, &MainWindow::onRemoveStaff);
    fl->addWidget(btnRemove);

    fl->addWidget(new QLabel("──────────────"));

    lblOnDuty = new QLabel("Sedang Bertugas: -");
    lblOnDuty->setStyleSheet("font-weight: bold; font-size: 14px; color: #4CAF50;");
    fl->addWidget(lblOnDuty);

    QPushButton* btnRotate = new QPushButton("🔄 Rotasi Shift (Circular)");
    btnRotate->setObjectName("btnWarning");
    connect(btnRotate, &QPushButton::clicked, this, &MainWindow::onRotateShift);
    fl->addWidget(btnRotate);

    fl->addStretch();
    layout->addWidget(formBox);

    QVBoxLayout* rightL = new QVBoxLayout();
    QLabel* staffTitle = new QLabel("👤 Daftar Staf (Circular Linked List - Rotasi Shift)");
    staffTitle->setStyleSheet("font-weight: bold; font-size: 13px;");
    rightL->addWidget(staffTitle);

    staffTable = new QTableWidget();
    staffTable->setColumnCount(4);
    staffTable->setHorizontalHeaderLabels({"ID", "Nama", "Jabatan", "Status"});
    staffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staffTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    staffTable->setAlternatingRowColors(true);
    rightL->addWidget(staffTable);

    layout->addLayout(rightL);
}

// ============================================================
//  HISTORY & REPORT TAB
// ============================================================
void MainWindow::setupHistoryTab() {
    historyTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(historyTab);

    QHBoxLayout* topRow = new QHBoxLayout();

    QGroupBox* histBox = new QGroupBox("📚 Riwayat Aksi (Stack - LIFO)");
    QVBoxLayout* hl = new QVBoxLayout(histBox);
    historyList = new QListWidget();
    hl->addWidget(historyList);
    QPushButton* btnUndo = new QPushButton("↩ Undo Aksi Terakhir");
    btnUndo->setObjectName("btnWarning");
    connect(btnUndo, &QPushButton::clicked, this, &MainWindow::onUndoAction);
    hl->addWidget(btnUndo);

    // Tombol STL Demo (syarat 9 — std::find + std::count)
    QPushButton* btnSTLDemo = new QPushButton("🔬 Jalankan STL Demo (find + count)");
    btnSTLDemo->setObjectName("btnPrimary");
    connect(btnSTLDemo, &QPushButton::clicked, this, &MainWindow::onRunSTLDemo);
    hl->addWidget(btnSTLDemo);

    topRow->addWidget(histBox);

    layout->addLayout(topRow);

    QGroupBox* repBox = new QGroupBox("📈 Laporan Order (Sorting + STL)");
    QVBoxLayout* rl = new QVBoxLayout(repBox);

    QHBoxLayout* repRow = new QHBoxLayout();
    QPushButton* btnGenReport = new QPushButton("📊 Generate Laporan");
    btnGenReport->setObjectName("btnPrimary");
    connect(btnGenReport, &QPushButton::clicked, this, &MainWindow::onGenerateReport);
    repRow->addWidget(btnGenReport);

    repRow->addWidget(new QLabel("Urutkan:"));
    reportSortCombo = new QComboBox();
    reportSortCombo->addItems({"Total ↓ (Merge Sort)", "ID ↑", "Meja ↑"});
    connect(reportSortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortReport);
    repRow->addWidget(reportSortCombo);

    QPushButton* btnSaveRep = new QPushButton("💾 Simpan Laporan");
    connect(btnSaveRep, &QPushButton::clicked, this, &MainWindow::onSaveReport);
    repRow->addWidget(btnSaveRep);
    rl->addLayout(repRow);

    reportTable = new QTableWidget();
    reportTable->setColumnCount(5);
    reportTable->setHorizontalHeaderLabels({"ID Order", "Meja", "Total", "Status", "Prioritas"});
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reportTable->setAlternatingRowColors(true);
    rl->addWidget(reportTable);

    layout->addWidget(repBox);
}

// ============================================================
//  DATA AWAL (SEED DATA)
// ============================================================
void MainWindow::setupInitialData() {
    // Tambah menu awal ke Linked List + AVL + Hash Table
    std::vector<std::pair<std::string, std::pair<std::string, double>>> initialMenus = {
        {"Nasi Goreng Spesial",  {"Makanan Utama", 35000}},
        {"Mie Ayam Bakso",       {"Makanan Utama", 28000}},
        {"Soto Ayam",            {"Makanan Utama", 25000}},
        {"Es Teh Manis",         {"Minuman",       8000}},
        {"Jus Alpukat",          {"Minuman",       18000}},
        {"Kopi Hitam",           {"Minuman",       12000}},
        {"Pisang Goreng",        {"Snack",         15000}},
        {"Lumpia Sayur",         {"Snack",         12000}},
        {"Es Krim Vanilla",      {"Dessert",       20000}},
        {"Gado-Gado",            {"Vegetarian",    22000}},
    };

    for (auto& m : initialMenus) {
        MenuItem item(nextMenuId++, m.first, m.second.first, m.second.second, true);
        menuList->insert(item);
        menuAVL->insert(item);
        menuHash->insert(item);
    }

    // Tambah meja (Circular Linked List + Graph)
    for (int i = 1; i <= 8; i++) {
        Table t(i, (i <= 4) ? 4 : 6);
        tableList->insert(t);
        tableGraph->addTable(i);
    }
    // Hubungkan meja dalam graph (layout restoran)
    tableGraph->addEdge(1, 2); tableGraph->addEdge(2, 3); tableGraph->addEdge(3, 4);
    tableGraph->addEdge(5, 6); tableGraph->addEdge(6, 7); tableGraph->addEdge(7, 8);
    tableGraph->addEdge(1, 5); tableGraph->addEdge(2, 6); tableGraph->addEdge(3, 7);
    tableGraph->addEdge(4, 8);

    // Tambah staf (Circular Linked List)
    std::vector<std::pair<std::string, std::string>> initialStaff = {
        {"Budi Santoso", "Waiter"},
        {"Siti Rahayu",  "Kasir"},
        {"Ahmad Fauzi",  "Koki"},
        {"Dewi Lestari", "Waiter"},
    };
    for (auto& s : initialStaff) {
        Staff st(nextStaffId++, s.first, s.second);
        staffList->insert(st);
    }

    // Update combo box nomor meja untuk order
    auto tables = tableList->getAll();
    for (auto* t : tables) {
        orderTableSelect->addItem(QString("Meja %1 (Kap: %2)").arg(t->tableNumber).arg(t->capacity),
                                  t->tableNumber);
    }

    // Update combo menu untuk order
    auto menus = menuList->getAll();
    for (auto* m : menus) {
        orderMenuSelect->addItem(QString("%1 - %2").arg(QString::fromStdString(m->name), formatPrice(m->price)), m->id);
    }

    refreshMenuTable();
    refreshTableDisplay();
    refreshStaffTable();
    refreshHistoryList();

    // Set staf pertama on duty
    if (staffList->getSize() > 0) {
        Staff* s = staffList->getOnDuty();
        if (s) {
            s->onDuty = true;
            lblOnDuty->setText(QString("Sedang Bertugas: %1 (%2)")
                               .arg(QString::fromStdString(s->name))
                               .arg(QString::fromStdString(s->role)));
        }
    }

    addLog("✅ Sistem berhasil diinisialisasi dengan data awal.");
    addLog(QString("📦 %1 item menu dimuat.").arg(menuList->getSize()));
    addLog(QString("🪑 %1 meja dikonfigurasi.").arg(tableList->getSize()));
    addLog(QString("👤 %1 staf terdaftar.").arg(staffList->getSize()));
}

// ============================================================
//  STYLESHEET
// ============================================================
void MainWindow::applyStyleSheet() {
    setStyleSheet(R"(

        /* ── GLOBAL ─────────────────────────────── */
        QWidget {
            background-color: #F5F5F5;
            color: #212121;
            font-family: Arial;
            font-size: 13px;
        }

        /* ── MAIN WINDOW ────────────────────────── */
        QMainWindow {
            background-color: #F5F5F5;
        }
        QStatusBar {
            background-color: #E0E0E0;
            color: #424242;
        }

        /* ── TAB WIDGET ─────────────────────────── */
        QTabWidget::pane {
            border: 1px solid #BDBDBD;
            border-radius: 0 6px 6px 6px;
            background-color: #F5F5F5;
        }
        QTabBar::tab {
            background-color: #E0E0E0;
            color: #424242;
            padding: 8px 18px;
            border-radius: 4px 4px 0 0;
            margin-right: 2px;
            font-weight: bold;
        }
        QTabBar::tab:selected {
            background-color: #1976D2;
            color: #FFFFFF;
        }
        QTabBar::tab:hover:!selected {
            background-color: #BDBDBD;
            color: #212121;
        }

        /* ── BUTTONS ────────────────────────────── */
        QPushButton {
            padding: 8px 14px;
            border-radius: 6px;
            border: none;
            background-color: #E0E0E0;
            color: #212121;
            font-weight: bold;
        }
        QPushButton:hover    { background-color: #BDBDBD; }
        QPushButton:pressed  { background-color: #9E9E9E; }
        QPushButton:disabled { background-color: #EEEEEE; color: #9E9E9E; }

        QPushButton#btnPrimary         { background-color: #1976D2; color: #FFFFFF; }
        QPushButton#btnPrimary:hover   { background-color: #1565C0; }
        QPushButton#btnPrimary:pressed { background-color: #0D47A1; }

        QPushButton#btnDanger          { background-color: #D32F2F; color: #FFFFFF; }
        QPushButton#btnDanger:hover    { background-color: #B71C1C; }
        QPushButton#btnDanger:pressed  { background-color: #7F0000; }

        QPushButton#btnSuccess         { background-color: #388E3C; color: #FFFFFF; }
        QPushButton#btnSuccess:hover   { background-color: #2E7D32; }
        QPushButton#btnSuccess:pressed { background-color: #1B5E20; }

        QPushButton#btnWarning         { background-color: #F57C00; color: #FFFFFF; }
        QPushButton#btnWarning:hover   { background-color: #E65100; }
        QPushButton#btnWarning:pressed { background-color: #BF360C; }

        /* ── GROUP BOX ──────────────────────────── */
        QGroupBox {
            background-color: #F5F5F5;
            color: #212121;
            font-weight: bold;
            border: 1px solid #BDBDBD;
            border-radius: 6px;
            margin-top: 10px;
            padding: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            padding: 0 6px;
            color: #1976D2;
        }

        /* ── INPUT WIDGETS ──────────────────────── */
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #FFFFFF;
            color: #212121;
            border: 1px solid #BDBDBD;
            border-radius: 4px;
            padding: 5px;
            selection-background-color: #1976D2;
            selection-color: #FFFFFF;
        }
        QLineEdit:focus, QTextEdit:focus {
            border: 1px solid #1976D2;
        }
        QLineEdit:disabled {
            background-color: #EEEEEE;
            color: #9E9E9E;
        }

        QComboBox {
            background-color: #FFFFFF;
            color: #212121;
            border: 1px solid #BDBDBD;
            border-radius: 4px;
            padding: 5px 8px;
        }
        QComboBox:focus  { border: 1px solid #1976D2; }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            color: #212121;
            selection-background-color: #1976D2;
            selection-color: #FFFFFF;
            border: 1px solid #BDBDBD;
        }

        QSpinBox, QDoubleSpinBox {
            background-color: #FFFFFF;
            color: #212121;
            border: 1px solid #BDBDBD;
            border-radius: 4px;
            padding: 5px;
        }
        QSpinBox:focus, QDoubleSpinBox:focus {
            border: 1px solid #1976D2;
        }

        QCheckBox {
            color: #212121;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #BDBDBD;
            border-radius: 3px;
            background-color: #FFFFFF;
        }
        QCheckBox::indicator:checked {
            background-color: #1976D2;
            border-color: #1976D2;
        }

        /* ── TABLE WIDGET ───────────────────────── */
        QTableWidget, QTableView {
            background-color: #FFFFFF;
            color: #212121;
            gridline-color: #E0E0E0;
            border: 1px solid #BDBDBD;
            alternate-background-color: #F0F4FF;
            selection-background-color: #BBDEFB;
            selection-color: #212121;
        }
        QHeaderView::section {
            background-color: #1976D2;
            color: #FFFFFF;
            padding: 6px;
            font-weight: bold;
            border: none;
            border-right: 1px solid #1565C0;
        }
        QHeaderView::section:last {
            border-right: none;
        }
        QTableWidget::item:selected {
            background-color: #BBDEFB;
            color: #212121;
        }
        QTableCornerButton::section {
            background-color: #1976D2;
            border: none;
        }

        /* ── LIST WIDGET ────────────────────────── */
        QListWidget {
            background-color: #FFFFFF;
            color: #212121;
            border: 1px solid #BDBDBD;
            border-radius: 4px;
            alternate-background-color: #F0F4FF;
        }
        QListWidget::item {
            padding: 4px;
            color: #212121;
        }
        QListWidget::item:selected {
            background-color: #1976D2;
            color: #FFFFFF;
        }
        QListWidget::item:hover:!selected {
            background-color: #E3F2FD;
        }

        /* ── SCROLL BAR ─────────────────────────── */
        QScrollBar:vertical {
            background-color: #F5F5F5;
            width: 10px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background-color: #BDBDBD;
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background-color: #9E9E9E; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }

        QScrollBar:horizontal {
            background-color: #F5F5F5;
            height: 10px;
            border-radius: 5px;
        }
        QScrollBar::handle:horizontal {
            background-color: #BDBDBD;
            border-radius: 5px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background-color: #9E9E9E; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }

        /* ── LABEL ──────────────────────────────── */
        QLabel {
            background-color: transparent;
            color: #212121;
        }

        /* ── TOOLTIP ────────────────────────────── */
        QToolTip {
            background-color: #FFFDE7;
            color: #212121;
            border: 1px solid #F9A825;
            border-radius: 4px;
            padding: 4px;
        }

        /* ── MESSAGE BOX ────────────────────────── */
        QMessageBox {
            background-color: #F5F5F5;
            color: #212121;
        }
        QMessageBox QLabel {
            color: #212121;
        }

        /* ── MENU BAR ───────────────────────────── */
        QMenuBar {
            background-color: #E0E0E0;
            color: #212121;
        }
        QMenuBar::item:selected {
            background-color: #1976D2;
            color: #FFFFFF;
        }
        QMenu {
            background-color: #FFFFFF;
            color: #212121;
            border: 1px solid #BDBDBD;
        }
        QMenu::item:selected {
            background-color: #1976D2;
            color: #FFFFFF;
        }

        /* ── SPLITTER ───────────────────────────── */
        QSplitter::handle {
            background-color: #BDBDBD;
        }

    )");
}

// ============================================================
//  HELPER METHODS
// ============================================================
void MainWindow::addLog(const QString& msg) {
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    logDisplay->append(timestamp + msg);
}

QString MainWindow::formatPrice(double price) {
    long long val = (long long)price;
    QString s = QString::number(val);
    // Insert thousand separators manually (no QRegularExpression needed)
    int insertPos = s.length() - 3;
    while (insertPos > 0) {
        s.insert(insertPos, '.');
        insertPos -= 3;
    }
    return QString("Rp %1").arg(s);
}

std::string MainWindow::getCurrentTimestamp() {
    auto t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

void MainWindow::updateClock() {
    lblCurrentTime->setText(QDateTime::currentDateTime().toString("dddd, dd MMMM yyyy - hh:mm:ss"));
}

// ============================================================
//  REFRESH METHODS
// ============================================================
void MainWindow::refreshMenuTable() {
    auto items = menuList->getAll();
    menuTable->setRowCount((int)items.size());
    for (int i = 0; i < (int)items.size(); i++) {
        MenuItem* m = items[i];
        menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        QTableWidgetItem* avail = new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak");
        avail->setForeground(m->available ? Qt::darkGreen : Qt::red);
        menuTable->setItem(i, 4, avail);
    }
}

void MainWindow::refreshOrderTable() {
    auto& orders = orderQueue->getAllOrders();
    orderTable->setRowCount((int)orders.size());
    int row = 0;
    for (auto it = orders.begin(); it != orders.end(); ++it, ++row) {
        Order* o = *it;
        orderTable->setItem(row, 0, new QTableWidgetItem(QString::number(o->orderId)));
        orderTable->setItem(row, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        orderTable->setItem(row, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        QTableWidgetItem* status = new QTableWidgetItem(QString::fromStdString(o->status));
        if (o->status == "pending")   status->setForeground(QColor(255,152,  0)); // #FF9800
        if (o->status == "preparing") status->setForeground(QColor( 25,118,210)); // #1976D2
        if (o->status == "served")    status->setForeground(QColor( 76,175, 80)); // #4CAF50
        if (o->status == "paid")      status->setForeground(Qt::gray);
        orderTable->setItem(row, 3, status);
        orderTable->setItem(row, 4, new QTableWidgetItem(
            o->priority == 1 ? "⭐ VIP" : "Normal"));
    }
    refreshPendingOrders();
}

void MainWindow::refreshPendingOrders() {
    auto pending = orderQueue->getPendingOrders();
    pendingOrdersTable->setRowCount((int)pending.size());
    for (int i = 0; i < (int)pending.size(); i++) {
        Order* o = pending[i];
        pendingOrdersTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        pendingOrdersTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        pendingOrdersTable->setItem(i, 2, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
        pendingOrdersTable->setItem(i, 3, new QTableWidgetItem(formatPrice(o->totalPrice)));
    }
}

void MainWindow::refreshTableDisplay() {
    auto tables = tableList->getAll();
    tableDisplay->setRowCount((int)tables.size());
    for (int i = 0; i < (int)tables.size(); i++) {
        Table* t = tables[i];
        tableDisplay->setItem(i, 0, new QTableWidgetItem(QString("Meja %1").arg(t->tableNumber)));
        tableDisplay->setItem(i, 1, new QTableWidgetItem(QString("%1 orang").arg(t->capacity)));
        QTableWidgetItem* status = new QTableWidgetItem(t->isOccupied ? "🔴 Terisi" : "🟢 Kosong");
        status->setForeground(t->isOccupied ? Qt::red : Qt::darkGreen);
        tableDisplay->setItem(i, 2, status);
        tableDisplay->setItem(i, 3, new QTableWidgetItem(
            t->currentOrderId >= 0 ? QString("#%1").arg(t->currentOrderId) : "-"));
    }

    Table* curr = tableList->getCurrent();
    if (curr)
        lblCurrentTable->setText(QString("Meja Aktif: %1 (Kap: %2)")
                                 .arg(curr->tableNumber).arg(curr->capacity));
}

void MainWindow::refreshStaffTable() {
    auto staffs = staffList->getAll();
    staffTable->setRowCount((int)staffs.size());
    for (int i = 0; i < (int)staffs.size(); i++) {
        Staff* s = staffs[i];
        staffTable->setItem(i, 0, new QTableWidgetItem(QString::number(s->staffId)));
        staffTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s->name)));
        staffTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s->role)));
        QTableWidgetItem* status = new QTableWidgetItem(s->onDuty ? "✅ Bertugas" : "💤 Istirahat");
        status->setForeground(s->onDuty ? Qt::darkGreen : Qt::gray);
        staffTable->setItem(i, 3, status);
    }
}

void MainWindow::refreshHistoryList() {
    historyList->clear();
    auto actions = actionStack->getAll();
    for (auto& a : actions) {
        historyList->addItem(QString("[%1] %2").arg(QString::fromStdString(a.type), QString::fromStdString(a.description)));
    }
}

void MainWindow::refreshDashboard() {
    lblTotalMenu->setText(QString::number(menuList->getSize()));
    lblTotalOrders->setText(QString::number(orderQueue->totalOrders()));
    lblPendingOrders->setText(QString::number(orderQueue->getPendingOrders().size()));

    // [STL COUNT] Hitung meja terisi menggunakan STLUtils::countOccupiedTables
    auto tables = tableList->getAll();
    int occupied = STLUtils::countOccupiedTables(tables);
    lblActiveTables->setText(QString::number(occupied));

    // [STL COUNT] Hitung menu tersedia menggunakan STLUtils::countAvailableMenus (syarat 9)
    if (lblAvailableMenus) {
        auto menus = menuList->getAll();
        int avail = STLUtils::countAvailableMenus(menus);
        lblAvailableMenus->setText(QString::number(avail));
    }

    // [STL COUNT] Hitung menu per kategori menggunakan MenuHashTable::countByCategory (syarat 9)
    if (lblMenuByCategory) {
        int makanan  = menuHash->countByCategory("Makanan Utama");
        int minuman  = menuHash->countByCategory("Minuman");
        int snack    = menuHash->countByCategory("Snack");
        int dessert  = menuHash->countByCategory("Dessert");
        int veggie   = menuHash->countByCategory("Vegetarian");
        lblMenuByCategory->setText(
            QString("Makanan: %1 | Minuman: %2 | Snack: %3 | Dessert: %4 | Veggie: %5")
            .arg(makanan).arg(minuman).arg(snack).arg(dessert).arg(veggie));
    }

    // Update stats panel menggunakan std::count_if order status (syarat 9)
    refreshStatisticsPanel();
}

void MainWindow::updateGraphDisplay() {
    QString output;
    auto all = tableGraph->getAllTables();
    for (int tbl : all) {
        output += QString("Meja %1 → ").arg(tbl);
        auto neighbors = tableGraph->getNeighbors(tbl);
        for (int i = 0; i < (int)neighbors.size(); i++) {
            output += QString("Meja %1").arg(neighbors[i]);
            if (i < (int)neighbors.size() - 1) output += ", ";
        }
        output += "\n";
    }
    graphDisplay->setText(output);
}

// ============================================================
//  SLOT: MENU
// ============================================================
void MainWindow::onAddMenuItem() {
    try {
        QString name = inputMenuName->text().trimmed();
        if (name.isEmpty())
            throw RestaurantException("Nama menu tidak boleh kosong!");

        // Exception handling (syarat 8)
        double price = inputMenuPrice->value();
        if (price <= 0)
            throw RestaurantException("Harga harus lebih dari 0!");

        MenuItem item(nextMenuId++,
                      name.toStdString(),
                      inputMenuCategory->currentText().toStdString(),
                      price,
                      inputMenuAvailable->isChecked());

        // Insert ke semua struktur data sekaligus
        menuList->insert(item);   // Linked List
        menuAVL->insert(item);    // AVL Tree
        menuHash->insert(item);   // Hash Table

        // [CALLBACK 1] forEachMenu — callback untuk logging item yang baru ditambah (syarat 5)
        auto newItems = std::vector<MenuItem*>();
        MenuItem* inserted = menuList->find(item.id);
        if (inserted) newItems.push_back(inserted);
        forEachMenu(newItems, [this](const MenuItem& m) {
            addLog(QString("➕ Menu ditambah: %1 (%2)")
                   .arg(QString::fromStdString(m.name), formatPrice(m.price)));
        });

        // [CALLBACK 3] filterMenuByCategory — log semua menu sekategori setelah tambah (syarat 5)
        // Menunjukkan callback dengan predikat kategori
        auto allMenus = menuList->getAll();
        int sameCategory = 0;
        filterMenuByCategory(allMenus, item.category,
            [&sameCategory](const MenuItem& /*m*/) {
                sameCategory++;  // hitung berapa menu sekategori
            });
        addLog(QString("   ↳ Total menu kategori '%1' sekarang: %2 item")
               .arg(QString::fromStdString(item.category)).arg(sameCategory));

        // Push ke Stack history (syarat 3: Stack)
        actionStack->push(Action("add_menu",
            "Tambah menu: " + item.name, item.id));

        // Update combo order
        orderMenuSelect->addItem(QString("%1 - %2").arg(name, formatPrice(price)), item.id);

        inputMenuName->clear();
        inputMenuPrice->setValue(0);
        refreshMenuTable();
        refreshDashboard();
        refreshHistoryList();

    } catch (const RestaurantException& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onRemoveMenuItem() {
    int row = menuTable->currentRow();
    if (row < 0) { QMessageBox::information(this, "Info", "Pilih menu yang akan dihapus!"); return; }
    int id = menuTable->item(row, 0)->text().toInt();
    QString name = menuTable->item(row, 1)->text();

    menuList->remove(id);
    menuAVL->remove(id);
    menuHash->remove(id);

    actionStack->push(Action("remove_menu", "Hapus menu: " + name.toStdString(), id));
    addLog(QString("🗑 Menu dihapus: %1").arg(name));
    refreshMenuTable();
    refreshDashboard();
    refreshHistoryList();
}

void MainWindow::onSearchMenu() {
    QString query = searchMenuInput->text().trimmed();
    if (query.isEmpty()) { refreshMenuTable(); return; }

    // Coba cari by ID dulu (Hash Table O(1))
    bool ok;
    int id = query.toInt(&ok);
    if (ok) {
        MenuItem* found = menuHash->findById(id);
        if (found) {
            menuTable->setRowCount(1);
            menuTable->setItem(0, 0, new QTableWidgetItem(QString::number(found->id)));
            menuTable->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(found->name)));
            menuTable->setItem(0, 2, new QTableWidgetItem(QString::fromStdString(found->category)));
            menuTable->setItem(0, 3, new QTableWidgetItem(formatPrice(found->price)));
            menuTable->setItem(0, 4, new QTableWidgetItem(found->available ? "✅ Ya" : "❌ Tidak"));
            // [OVERLOAD 1] displayItem(MenuItem) — function overloading (syarat 7)
            addLog(QString("🔍 [Hash O(1)] Ditemukan: %1")
                   .arg(QString::fromStdString(displayItem(*found))));
            return;
        }
    }

    // [TEMPLATE linearSearch] Cari by nama partial match menggunakan function template (syarat 7)
    // linearSearch<MenuItem> — memanggil template dengan tipe MenuItem
    auto allPtrs = menuList->getAll();
    MenuItem* foundByTemplate = linearSearch<MenuItem>(allPtrs,
        [&query](MenuItem* m) {
            return QString::fromStdString(m->name)
                       .contains(query, Qt::CaseInsensitive);
        });

    if (foundByTemplate) {
        // Ditemukan satu hasil exact/prefix — tampilkan via overload displayItem
        addLog(QString("🔍 [Template linearSearch] Ditemukan: %1")
               .arg(QString::fromStdString(displayItem(*foundByTemplate))));
    }

    // [TEMPLATE linearSearchAll] Cari SEMUA yang cocok untuk ditampilkan di tabel (syarat 7)
    auto filtered = linearSearchAll<MenuItem>(allPtrs,
        [&query](MenuItem* m) {
            return QString::fromStdString(m->name)
                       .contains(query, Qt::CaseInsensitive) ||
                   QString::fromStdString(m->category)
                       .contains(query, Qt::CaseInsensitive);
        });

    // [STL COUNT] Hitung hasil yang ditemukan (syarat 9)
    int found_count = STLUtils::countAvailableMenus(
        std::vector<MenuItem*>(filtered.begin(), filtered.end()));
    addLog(QString("   ↳ Ditemukan %1 menu, %2 di antaranya tersedia")
           .arg(filtered.size()).arg(found_count));

    menuTable->setRowCount((int)filtered.size());
    for (int i = 0; i < (int)filtered.size(); i++) {
        MenuItem* m = filtered[i];
        menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        menuTable->setItem(i, 4, new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak"));
    }
}

void MainWindow::onSortMenu() {
    // Ambil semua dari AVL Tree (inorder = sudah terurut by ID)
    auto items = menuAVL->inorderTraversal();

    int idx = sortMenuCombo->currentIndex();
    if (idx == 1) Sorting::bubbleSortByPrice(items, true);   // Bubble Sort ↑
    else if (idx == 2) Sorting::bubbleSortByPrice(items, false); // Bubble Sort ↓
    else if (idx == 3) Sorting::sortByName(items);            // STL sort + lambda

    menuTable->setRowCount((int)items.size());
    for (int i = 0; i < (int)items.size(); i++) {
        menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(items[i].id)));
        menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(items[i].name)));
        menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(items[i].category)));
        menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(items[i].price)));
        menuTable->setItem(i, 4, new QTableWidgetItem(items[i].available ? "✅ Ya" : "❌ Tidak"));
    }
    addLog(QString("🔃 Menu diurutkan: %1").arg(sortMenuCombo->currentText()));
}

void MainWindow::onSaveMenu() {
    auto items = menuAVL->inorderTraversal();
    // Lambda untuk transform ke format save (syarat 11)
    bool ok = FileIO::saveMenuToFile(items);
    if (ok) {
        addLog("💾 Data menu berhasil disimpan ke menu.txt");
        QMessageBox::information(this, "Berhasil", "Menu berhasil disimpan ke menu.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan file!");
    }
}

void MainWindow::onLoadMenu() {
    auto loaded = FileIO::loadMenuFromFile();
    if (loaded.empty()) {
        QMessageBox::warning(this, "Peringatan", "File menu.txt tidak ditemukan atau kosong!");
        return;
    }
    for (auto& m : loaded) {
        m.id = nextMenuId++;
        menuList->insert(m);
        menuAVL->insert(m);
        menuHash->insert(m);
        orderMenuSelect->addItem(QString::fromStdString(m.name), m.id);
    }
    refreshMenuTable();
    refreshDashboard();
    addLog(QString("📂 %1 item menu dimuat dari file.").arg(loaded.size()));
}

// ============================================================
//  SLOT: ORDER
// ============================================================
void MainWindow::onCreateOrder() {
    if (currentOrder) {
        QMessageBox::warning(this, "Peringatan", "Selesaikan atau batalkan order saat ini terlebih dahulu!");
        return;
    }
    int tableNum = orderTableSelect->currentData().toInt();
    int priority = (orderPrioritySelect->currentIndex() == 0) ? 2 : 1;

    currentOrder = new Order(0, tableNum, priority);
    currentOrderItems.clear();
    currentOrderTotal = 0.0;
    orderItemList->clear();
    lblOrderTotal->setText("Total: Rp 0");
    addLog(QString("🆕 Order baru dibuat untuk Meja %1 (Prioritas: %2)")
           .arg(tableNum).arg(priority == 1 ? "VIP" : "Normal"));
}

void MainWindow::onAddItemToOrder() {
    if (!currentOrder) {
        QMessageBox::warning(this, "Peringatan", "Buat order baru terlebih dahulu!");
        return;
    }
    int menuId = orderMenuSelect->currentData().toInt();
    int qty = orderQtyInput->value();

    // Cari di Hash Table (O(1))
    MenuItem* m = menuHash->findById(menuId);
    if (!m) { QMessageBox::warning(this, "Error", "Menu tidak ditemukan!"); return; }

    OrderItem item;
    item.menuItemId   = m->id;
    item.menuItemName = m->name;
    item.quantity     = qty;
    item.subtotal     = m->price * qty;

    currentOrderItems.push_back(item);
    currentOrderTotal += item.subtotal;

    orderItemList->addItem(QString("%1 x%2 = %3")
                           .arg(QString::fromStdString(m->name))
                           .arg(qty)
                           .arg(formatPrice(item.subtotal)));
    lblOrderTotal->setText(QString("Total: %1").arg(formatPrice(currentOrderTotal)));
}

void MainWindow::onRemoveItemFromOrder() {
    int row = orderItemList->currentRow();
    if (row < 0 || !currentOrder) return;
    currentOrderTotal -= currentOrderItems[row].subtotal;
    currentOrderItems.erase(currentOrderItems.begin() + row);
    delete orderItemList->takeItem(row);
    lblOrderTotal->setText(QString("Total: %1").arg(formatPrice(currentOrderTotal)));
}

void MainWindow::onSubmitOrder() {
    if (!currentOrder || currentOrderItems.empty()) {
        QMessageBox::warning(this, "Peringatan", "Tambahkan item ke order terlebih dahulu!");
        return;
    }

    // Enqueue ke Priority Queue
    Order* submitted = orderQueue->enqueue(currentOrder->tableNumber, currentOrder->priority);
    submitted->items = currentOrderItems;
    submitted->totalPrice = currentOrderTotal;
    submitted->timestamp = getCurrentTimestamp();

    // Tandai meja terpakai
    tableList->occupyTable(currentOrder->tableNumber, submitted->orderId);

    // [CALLBACK 2] applyToAllOrders — callback ke semua order untuk update statistik (syarat 5)
    // Dijalankan setelah submit agar log mencerminkan state terkini
    double totalAllOrders = 0.0;
    applyToAllOrders(orderQueue->getAllOrders(), [&totalAllOrders](const Order& o) {
        if (o.status != "paid") totalAllOrders += o.totalPrice;
    });
    addLog(QString("💰 Total nilai order aktif (via callback): %1").arg(formatPrice(totalAllOrders)));

    actionStack->push(Action("add_order",
        "Submit order #" + std::to_string(submitted->orderId) +
        " Meja " + std::to_string(submitted->tableNumber),
        submitted->orderId));

    addLog(QString("✅ Order #%1 disubmit (Meja %2, Total: %3, Prioritas: %4)")
           .arg(submitted->orderId)
           .arg(submitted->tableNumber)
           .arg(formatPrice(submitted->totalPrice))
           .arg(submitted->priority == 1 ? "VIP" : "Normal"));

    delete currentOrder;
    currentOrder = nullptr;
    currentOrderItems.clear();
    currentOrderTotal = 0.0;
    orderItemList->clear();
    lblOrderTotal->setText("Total: Rp 0");

    refreshOrderTable();
    refreshTableDisplay();
    refreshDashboard();
    refreshHistoryList();
}

void MainWindow::onProcessNextOrder() {
    try {
        if (orderQueue->isEmpty())
            throw RestaurantException("Tidak ada order dalam antrian!");

        Order* next = orderQueue->dequeue();
        next->status = "preparing";

        addLog(QString("⚙ Order #%1 (Meja %2) mulai diproses.")
               .arg(next->orderId).arg(next->tableNumber));

        actionStack->push(Action("process_order",
            "Proses order #" + std::to_string(next->orderId)));

        refreshOrderTable();
        refreshDashboard();
        refreshHistoryList();

    } catch (const RestaurantException& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onUpdateOrderStatus() {}

// ============================================================
//  SLOT: TABLE
// ============================================================
void MainWindow::onNextTable() {
    Table* t = tableList->next();
    if (t) {
        lblCurrentTable->setText(QString("Meja Aktif: %1 (Kap: %2, Status: %3)")
                                 .arg(t->tableNumber)
                                 .arg(t->capacity)
                                 .arg(t->isOccupied ? "Terisi" : "Kosong"));
        addLog(QString("▶ Navigasi ke Meja %1 (Circular)").arg(t->tableNumber));
    }
}

void MainWindow::onOccupyTable() {
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    if (curr->isOccupied) {
        QMessageBox::warning(this, "Peringatan", "Meja sudah terisi!"); return;
    }
    tableList->occupyTable(curr->tableNumber, -1);
    addLog(QString("🔴 Meja %1 ditandai terisi.").arg(curr->tableNumber));
    refreshTableDisplay();
    refreshDashboard();
}

void MainWindow::onFreeTable() {
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    tableList->freeTable(curr->tableNumber);
    addLog(QString("🟢 Meja %1 dikosongkan.").arg(curr->tableNumber));
    refreshTableDisplay();
    refreshDashboard();
}

void MainWindow::onRunBFS() {
    Table* curr = tableList->getCurrent();
    if (!curr) return;

    std::vector<int> result = tableGraph->bfs(curr->tableNumber);
    QString output = "=== BFS dari Meja " + QString::number(curr->tableNumber) + " ===\n";
    output += "Urutan kunjungan: ";
    for (int i = 0; i < (int)result.size(); i++) {
        output += "Meja " + QString::number(result[i]);
        if (i < (int)result.size()-1) output += " → ";
    }
    graphDisplay->setText(output);
    addLog(QString("🔍 BFS dari Meja %1 selesai.").arg(curr->tableNumber));
    updateGraphDisplay();
    graphDisplay->setText(output + "\n\n📋 Adjacency List:\n" + graphDisplay->toPlainText());
}

void MainWindow::onRunDFS() {
    Table* curr = tableList->getCurrent();
    if (!curr) return;

    std::vector<int> result = tableGraph->dfs(curr->tableNumber);
    QString output = "=== DFS dari Meja " + QString::number(curr->tableNumber) + " ===\n";
    output += "Urutan kunjungan: ";
    for (int i = 0; i < (int)result.size(); i++) {
        output += "Meja " + QString::number(result[i]);
        if (i < (int)result.size()-1) output += " → ";
    }
    graphDisplay->setText(output);
    addLog(QString("🔍 DFS dari Meja %1 selesai.").arg(curr->tableNumber));
}

// ============================================================
//  SLOT: STAFF
// ============================================================
void MainWindow::onAddStaff() {
    QString name = inputStaffName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Peringatan", "Nama staf tidak boleh kosong!"); return;
    }
    Staff s(nextStaffId++, name.toStdString(), inputStaffRole->currentText().toStdString());
    staffList->insert(s);

    actionStack->push(Action("add_staff", "Tambah staf: " + s.name, s.staffId));
    addLog(QString("👤 Staf ditambah: %1 (%2)").arg(name, inputStaffRole->currentText()));
    inputStaffName->clear();
    refreshStaffTable();
    refreshHistoryList();
}

void MainWindow::onRemoveStaff() {
    int row = staffTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Pilih staf di tabel yang ingin dihapus!");
        return;
    }
    int staffId  = staffTable->item(row, 0)->text().toInt();
    QString name = staffTable->item(row, 1)->text();

    // [TEMPLATE linearSearch<Staff>] Cari staf di vector sebelum hapus (syarat 7)
    // Membuktikan template bekerja untuk tipe Staff, bukan hanya MenuItem
    auto allStaffs = staffList->getAll();
    Staff* toDelete = linearSearch<Staff>(allStaffs,
        [staffId](Staff* s) { return s->staffId == staffId; });
    if (toDelete) {
        // [OVERLOAD 3] displayItem(Staff) — function overloading (syarat 7)
        addLog(QString("🔍 [Template linearSearch<Staff>] Target: %1")
               .arg(QString::fromStdString(displayItem(*toDelete))));
    }

    auto reply = QMessageBox::question(this, "Konfirmasi Hapus",
        QString("Yakin ingin menghapus staf:\n%1?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    bool ok = staffList->remove(staffId);
    if (ok) {
        actionStack->push(Action("remove_staff", "Hapus staf: " + name.toStdString(), staffId));
        addLog(QString("🗑 Staf dihapus: %1").arg(name));
        Staff* duty = staffList->getOnDuty();
        if (duty) {
            duty->onDuty = true;
            lblOnDuty->setText(QString("Sedang Bertugas: %1 (%2)")
                .arg(QString::fromStdString(duty->name), QString::fromStdString(duty->role)));
        } else {
            lblOnDuty->setText("Sedang Bertugas: -");
        }
        refreshStaffTable();
        refreshHistoryList();
    } else {
        QMessageBox::warning(this, "Error", "Gagal menghapus staf!");
    }
}

void MainWindow::onRotateShift() {
    try {
        if (staffList->getSize() == 0)
            throw RestaurantException("Tidak ada staf terdaftar!");

        Staff* next = staffList->rotateShift();
        if (next) {
            lblOnDuty->setText(QString("Sedang Bertugas: %1 (%2)")
                               .arg(QString::fromStdString(next->name), QString::fromStdString(next->role)));
            addLog(QString("🔄 Shift dirotasi → %1 (%2) bertugas.")
                   .arg(QString::fromStdString(next->name), QString::fromStdString(next->role)));
            actionStack->push(Action("rotate_shift", "Rotasi shift ke: " + next->name));
        }
        refreshStaffTable();
        refreshHistoryList();
    } catch (const RestaurantException& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

// ============================================================
//  SLOT: HISTORY & REPORT
// ============================================================
void MainWindow::onUndoAction() {
    try {
        Action a = actionStack->pop();
        addLog(QString("↩ UNDO: [%1] %2").arg(QString::fromStdString(a.type), QString::fromStdString(a.description)));
        QMessageBox::information(this, "Undo",
            QString("Aksi dibatalkan:\n%1").arg(QString::fromStdString(a.description)));
        refreshHistoryList();
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onGenerateReport() {
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());

    // Merge Sort default (by total)
    if (!allOrders.empty())
        Sorting::mergeSort(allOrders, 0, (int)allOrders.size() - 1);

    reportTable->setRowCount((int)allOrders.size());
    for (int i = 0; i < (int)allOrders.size(); i++) {
        Order* o = allOrders[i];
        reportTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        reportTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        reportTable->setItem(i, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        reportTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(o->status)));
        reportTable->setItem(i, 4, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
    }

    // Hitung total pendapatan menggunakan STL + lambda
    double totalRevenue = 0.0;
    std::for_each(allOrders.begin(), allOrders.end(),
        [&totalRevenue](Order* o) { totalRevenue += o->totalPrice; });

    // [CALLBACK 2] applyToAllOrders — log semua order lewat callback (syarat 5)
    // Mendemonstrasikan OrderCallback digunakan secara nyata
    logOrdersViaCallback("paid");

    // [OVERLOAD 2] displayItem(Order) — cetak info order terakhir via overloading (syarat 7)
    if (!allOrders.empty()) {
        addLog(QString("📋 [Overload displayItem(Order)] Order pertama: %1")
               .arg(QString::fromStdString(displayItem(*allOrders.front()))));
    }

    addLog(QString("📈 Laporan digenerate. Total order: %1, Pendapatan: %2")
           .arg(allOrders.size()).arg(formatPrice(totalRevenue)));
}

void MainWindow::onSortReport() {
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());

    int idx = reportSortCombo->currentIndex();
    if (idx == 0) {
        // Merge Sort by total
        if (!allOrders.empty())
            Sorting::mergeSort(allOrders, 0, (int)allOrders.size() - 1);
    } else if (idx == 1) {
        // STL sort by ID (lambda)
        std::sort(allOrders.begin(), allOrders.end(),
            [](Order* a, Order* b) { return a->orderId < b->orderId; });
    } else {
        // STL sort by table (lambda)
        std::sort(allOrders.begin(), allOrders.end(),
            [](Order* a, Order* b) { return a->tableNumber < b->tableNumber; });
    }

    reportTable->setRowCount((int)allOrders.size());
    for (int i = 0; i < (int)allOrders.size(); i++) {
        Order* o = allOrders[i];
        reportTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        reportTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        reportTable->setItem(i, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        reportTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(o->status)));
        reportTable->setItem(i, 4, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
    }
}

void MainWindow::onSaveReport() {
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());
    bool ok = FileIO::saveOrdersToFile(allOrders);
    if (ok) {
        addLog("💾 Laporan order disimpan ke orders.txt");
        QMessageBox::information(this, "Berhasil", "Laporan berhasil disimpan ke orders.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan laporan!");
    }
}

// ============================================================
//  SLOT BARU: STL DEMO — std::find + std::count (syarat 9)
// ============================================================
void MainWindow::onRunSTLDemo() {
    auto& allOrders = orderQueue->getAllOrders();
    auto allMenus   = menuList->getAll();
    auto allTables  = tableList->getAll();

    QString result;
    result += "=== DEMO STL find + count ===\n\n";

    // [STL COUNT 1] Hitung order per status menggunakan std::count_if
    result += "--- std::count_if: Order per Status ---\n";
    for (const auto& s : std::vector<std::string>{"pending","preparing","served","paid"}) {
        int n = STLUtils::countOrdersByStatus(allOrders, s);
        result += QString("  %1: %2 order\n").arg(QString::fromStdString(s)).arg(n);
    }

    // [STL COUNT 2] Hitung menu tersedia
    int avail = STLUtils::countAvailableMenus(allMenus);
    result += QString("\n--- std::count_if: Menu Tersedia ---\n");
    result += QString("  Menu TERSEDIA : %1\n").arg(avail);
    result += QString("  Menu HABIS    : %1\n").arg((int)allMenus.size() - avail);

    // [STL COUNT 3] Hitung meja terisi
    int occ = STLUtils::countOccupiedTables(allTables);
    result += QString("\n--- std::count_if: Status Meja ---\n");
    result += QString("  Meja TERISI : %1 dari %2\n").arg(occ).arg(allTables.size());

    // [STL FIND 1] Cari order ID = 1 menggunakan std::find_if via STLUtils
    result += "\n--- std::find_if: Cari Order #1 ---\n";
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), 1);
    if (it != allOrders.end()) {
        // [OVERLOAD 2] displayItem(Order) — overloading (syarat 7)
        result += QString("  Ditemukan: %1\n")
                  .arg(QString::fromStdString(displayItem(**it)));
    } else {
        result += "  Order #1 tidak ditemukan (belum ada order)\n";
    }

    // [STL FIND 2] std::find exact match pada vector<int> ID menu
    result += "\n--- std::find: Cek ID Menu ---\n";
    auto allMenuPtrs = menuList->getAll();
    std::vector<int> idList;
    for (auto* m : allMenuPtrs) idList.push_back(m->id);

    for (int testId : {1, 5, 999}) {
        bool exists = STLUtils::menuIdExists(idList, testId);
        result += QString("  ID %1: %2\n")
                  .arg(testId).arg(exists ? "ADA ✅" : "TIDAK ADA ❌");
    }

    // [CALLBACK + STL] Kombinasi: filter by category lalu count
    result += "\n--- Callback + std::count_if: Menu per Kategori ---\n";
    for (const auto& cat : std::vector<std::string>{"Makanan Utama","Minuman","Snack","Dessert","Vegetarian"}) {
        int n = menuHash->countByCategory(cat);
        result += QString("  %1: %2 item\n").arg(QString::fromStdString(cat)).arg(n);
    }

    if (statsDisplay) statsDisplay->setText(result);

    // Tampilkan di log juga (ringkas)
    addLog("🔬 [STL Demo] std::find + std::count dijalankan — lihat panel statistik.");

    // Juga tampilkan di message box
    QMessageBox::information(this, "STL Demo — find + count", result);
}

// ============================================================
//  SLOT BARU: FILTER BY CATEGORY (Callback filterMenuByCategory)
// ============================================================
void MainWindow::onFilterByCategory() {
    QString cat = filterCategoryCombo->currentText();
    std::string catStr = cat.toStdString();

    // [CALLBACK 3] filterMenuByCategory — filter + callback (syarat 5)
    // Mengumpulkan hasil lewat callback, bukan return value
    std::vector<MenuItem*> filtered;
    auto allMenus = menuList->getAll();

    filterMenuByCategory(allMenus, catStr,
        [&filtered](const MenuItem& m) {
            // Karena callback menerima const ref, kita perlu cari pointer-nya
            // Ini demonstrasi bahwa callback bisa mengumpulkan data
            filtered.push_back(const_cast<MenuItem*>(&m));
        });

    // [CALLBACK 4] processMenusWithCondition — callback dengan predikat harga > 15000 (syarat 5)
    int expensive = 0;
    processMenusWithCondition(allMenus,
        [&catStr](const MenuItem& m) { return m.category == catStr && m.price > 15000; },
        [&expensive](const MenuItem& /*m*/) { expensive++; });

    addLog(QString("🔎 [Callback filterMenuByCategory] Kategori '%1': %2 item, %3 di antaranya > Rp15.000")
           .arg(cat).arg(filtered.size()).arg(expensive));

    // Tampilkan hasil filter di tabel menu
    menuTable->setRowCount((int)filtered.size());
    for (int i = 0; i < (int)filtered.size(); i++) {
        MenuItem* m = filtered[i];
        menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        menuTable->setItem(i, 4, new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak"));
    }

    if (filtered.empty()) {
        QMessageBox::information(this, "Filter Kategori",
            QString("Tidak ada menu dalam kategori '%1'.").arg(cat));
    }
}

// ============================================================
//  METHOD BARU: Callback helper — logMenusViaCallback (syarat 5)
// ============================================================
void MainWindow::logMenusViaCallback(const std::string& category) {
    // [CALLBACK 3] filterMenuByCategory digunakan sebagai helper logging
    auto allMenus = menuList->getAll();
    int count = 0;
    filterMenuByCategory(allMenus, category, [&count](const MenuItem& /*m*/) {
        count++;
    });
    if (count > 0)
        addLog(QString("   [Callback] Kategori '%1' memiliki %2 item.")
               .arg(QString::fromStdString(category)).arg(count));
}

// ============================================================
//  METHOD BARU: Callback helper — logOrdersViaCallback (syarat 5)
// ============================================================
void MainWindow::logOrdersViaCallback(const std::string& status) {
    // [CALLBACK 2] applyToAllOrders digunakan sebagai helper logging (syarat 5)
    int count = 0;
    double total = 0.0;
    applyToAllOrders(orderQueue->getAllOrders(), [&](const Order& o) {
        if (o.status == status) { count++; total += o.totalPrice; }
    });
    addLog(QString("   [Callback applyToAllOrders] Status '%1': %2 order, total Rp%3")
           .arg(QString::fromStdString(status)).arg(count).arg((int)total));
}

// ============================================================
//  METHOD BARU: Function overloading formatDisplayItem (syarat 7)
// ============================================================
QString MainWindow::formatDisplayItem(const MenuItem& m) {
    // [OVERLOAD 1] — memanggil free function displayItem(MenuItem) dari datastructures.h
    return QString::fromStdString(displayItem(m));
}
QString MainWindow::formatDisplayItem(const Order& o) {
    // [OVERLOAD 2] — memanggil free function displayItem(Order)
    return QString::fromStdString(displayItem(o));
}
QString MainWindow::formatDisplayItem(const Staff& s) {
    // [OVERLOAD 3] — memanggil free function displayItem(Staff)
    return QString::fromStdString(displayItem(s));
}

// ============================================================
//  METHOD BARU: refreshStatisticsPanel — STL count update (syarat 9)
// ============================================================
void MainWindow::refreshStatisticsPanel() {
    if (!statsDisplay) return;

    auto& allOrders = orderQueue->getAllOrders();
    auto allMenus   = menuList->getAll();
    auto allTables  = tableList->getAll();

    // [STL COUNT] Ringkasan status order via getOrderStatusSummary (syarat 9)
    auto summary = STLUtils::getOrderStatusSummary(allOrders);

    QString stats;
    stats += QString("Order  — pending:%1 preparing:%2 served:%3 paid:%4\n")
             .arg(summary["pending"]).arg(summary["preparing"])
             .arg(summary["served"]).arg(summary["paid"]);

    // [STL COUNT] Menu tersedia vs habis
    int avail = STLUtils::countAvailableMenus(allMenus);
    stats += QString("Menu   — tersedia:%1  habis:%2  total:%3\n")
             .arg(avail).arg((int)allMenus.size() - avail).arg(allMenus.size());

    // [STL COUNT] Meja terisi vs kosong
    int occ = STLUtils::countOccupiedTables(allTables);
    stats += QString("Meja   — terisi:%1  kosong:%2  total:%3\n")
             .arg(occ).arg((int)allTables.size() - occ).arg(allTables.size());

    statsDisplay->setText(stats);
}

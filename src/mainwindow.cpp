#include "mainwindow.h"
#include "ui_mainwindow.h"   // ← di-generate otomatis dari mainwindow.ui oleh uic
#include <QApplication>
#include <ctime>
#include <QPainter>
#include <QFont>

// ============================================================
//  KONSTRUKTOR & DESTRUKTOR
// ============================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
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
    inventarisList = new InventarisLinkedList();
    transaksiList  = new TransaksiList();

    currentOrderItems.clear();

    // Setup UI dari file .ui (ganti semua setupXxxTab())
    ui->setupUi(this);

    ui->tabWidget->tabBar()->setExpanding(false);
    ui->tabWidget->tabBar()->setStyleSheet("QTabBar { alignment: center; }");

    // Sambungkan semua signal ke slot
    setupConnections();

    // Isi data awal
    setupInitialData();

    // Terapkan stylesheet kustom
    applyStyleSheet();

    // Timer jam
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    timer->start(1000);

    setWindowTitle("🍽 Sistem Manajemen Restoran - Qt");
    statusBar()->showMessage("Sistem Manajemen Restoran siap digunakan.");
    refreshDashboard();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete menuList;
    delete tableList;
    delete staffList;
    delete orderQueue;
    delete menuAVL;
    delete menuHash;
    delete tableGraph;
    delete actionStack;
    delete inventarisList;
    delete transaksiList;
    if (currentOrder) delete currentOrder;
}

// ============================================================
//  SETUP CONNECTIONS  (pengganti setupUI() lama)
//  Semua widget diakses via ui->namaWidget sesuai .ui file
// ============================================================
void MainWindow::setupConnections()
{
    // ── Dashboard ────────────────────────────────────────────
    connect(ui->btnRefreshDashboard, &QPushButton::clicked,
            this, &MainWindow::refreshDashboard);

    // ── Menu ─────────────────────────────────────────────────
    connect(ui->btnAddMenuItem,   &QPushButton::clicked, this, &MainWindow::onAddMenuItem);
    connect(ui->btnRemoveMenuItem,&QPushButton::clicked, this, &MainWindow::onRemoveMenuItem);
    connect(ui->btnSaveMenu,      &QPushButton::clicked, this, &MainWindow::onSaveMenu);
    connect(ui->btnLoadMenu,      &QPushButton::clicked, this, &MainWindow::onLoadMenu);
    connect(ui->btnFilterCategory,&QPushButton::clicked, this, &MainWindow::onFilterByCategory);
    connect(ui->btnResetFilter,   &QPushButton::clicked, this, &MainWindow::refreshMenuTable);

    connect(ui->searchMenuInput, &QLineEdit::textChanged,
            this, &MainWindow::onSearchMenu);
    connect(ui->sortMenuCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortMenu);

    // ── Order ────────────────────────────────────────────────
    connect(ui->btnCreateOrder,       &QPushButton::clicked, this, &MainWindow::onCreateOrder);
    connect(ui->btnAddItemToOrder,    &QPushButton::clicked, this, &MainWindow::onAddItemToOrder);
    connect(ui->btnRemoveItemFromOrder,&QPushButton::clicked,this, &MainWindow::onRemoveItemFromOrder);
    connect(ui->btnSubmitOrder,       &QPushButton::clicked, this, &MainWindow::onSubmitOrder);
    connect(ui->btnProcessNextOrder,  &QPushButton::clicked, this, &MainWindow::onProcessNextOrder);

    // ── Meja ─────────────────────────────────────────────────
    connect(ui->btnNextTable,   &QPushButton::clicked, this, &MainWindow::onNextTable);
    connect(ui->btnOccupyTable, &QPushButton::clicked, this, &MainWindow::onOccupyTable);
    connect(ui->btnFreeTable,   &QPushButton::clicked, this, &MainWindow::onFreeTable);
    connect(ui->btnRunBFS,      &QPushButton::clicked, this, &MainWindow::onRunBFS);
    connect(ui->btnRunDFS,      &QPushButton::clicked, this, &MainWindow::onRunDFS);

    // ── Staf ─────────────────────────────────────────────────
    connect(ui->btnAddStaff,    &QPushButton::clicked, this, &MainWindow::onAddStaff);
    connect(ui->btnRemoveStaff, &QPushButton::clicked, this, &MainWindow::onRemoveStaff);
    connect(ui->btnRotateShift, &QPushButton::clicked, this, &MainWindow::onRotateShift);

    // ── Riwayat & Laporan ────────────────────────────────────
    connect(ui->btnUndoAction,    &QPushButton::clicked, this, &MainWindow::onUndoAction);
    connect(ui->btnGenerateReport,&QPushButton::clicked, this, &MainWindow::onGenerateReport);
    connect(ui->btnSortReport,    &QPushButton::clicked, this, &MainWindow::onSortReport);
    connect(ui->btnSaveReport,    &QPushButton::clicked, this, &MainWindow::onSaveReport);
    connect(ui->btnRunSTLDemo,    &QPushButton::clicked, this, &MainWindow::onRunSTLDemo);

    // ── Inventaris ───────────────────────────────────────────
    connect(ui->btnTambahStok,      &QPushButton::clicked, this, &MainWindow::onTambahStok);
    connect(ui->btnUpdateStok,      &QPushButton::clicked, this, &MainWindow::onUpdateStok);
    connect(ui->btnHapusStok,       &QPushButton::clicked, this, &MainWindow::onHapusStok);
    connect(ui->btnCekStokMinim,    &QPushButton::clicked, this, &MainWindow::onCekStokMinim);
    connect(ui->btnSimpanInventaris,&QPushButton::clicked, this, &MainWindow::onSimpanInventaris);

    // ── Pembayaran ───────────────────────────────────────────
    connect(ui->btnMuatOrder,         &QPushButton::clicked, this, &MainWindow::onMuatOrder);
    connect(ui->btnHitungKembalian,   &QPushButton::clicked, this, &MainWindow::onHitungKembalian);
    connect(ui->btnProsesPembayaran,  &QPushButton::clicked, this, &MainWindow::onProsesPembayaran);
    connect(ui->btnCetakStruk,        &QPushButton::clicked, this, &MainWindow::onCetakStruk);

    // ── Laporan Keuangan ─────────────────────────────────────
    connect(ui->btnGenerateLapKeu, &QPushButton::clicked, this, &MainWindow::onGenerateLapKeu);
    connect(ui->btnSimpanLapKeu,   &QPushButton::clicked, this, &MainWindow::onSimpanLapKeu);

    // Tab bar Center
    ui->tabWidget->tabBar()->setExpanding(false);

    // ── Tabel Header ─────────────────────────────────────────
    ui->menuTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pendingOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableDisplay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->staffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->inventarisTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->transaksiTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->laporanKeuTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// ============================================================
//  DATA AWAL (SEED DATA)
// ============================================================
void MainWindow::setupInitialData()
{
    // Menu awal
    std::vector<std::pair<std::string, std::pair<std::string, double>>> initialMenus = {
                                                                                        {"Nasi Goreng Spesial",  {"Makanan Utama", 35000}},
                                                                                        {"Mie Ayam Bakso",       {"Makanan Utama", 28000}},
                                                                                        {"Soto Ayam",            {"Makanan Utama", 25000}},
                                                                                        {"Es Teh Manis",         {"Minuman",        8000}},
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

    // Meja (Circular Linked List + Graph)
    for (int i = 1; i <= 8; i++) {
        Table t(i, (i <= 4) ? 4 : 6);
        tableList->insert(t);
        tableGraph->addTable(i);
    }
    tableGraph->addEdge(1,2); tableGraph->addEdge(2,3); tableGraph->addEdge(3,4);
    tableGraph->addEdge(5,6); tableGraph->addEdge(6,7); tableGraph->addEdge(7,8);
    tableGraph->addEdge(1,5); tableGraph->addEdge(2,6); tableGraph->addEdge(3,7);
    tableGraph->addEdge(4,8);

    // Staf (Circular Linked List)
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

    // Populate combo box nomor meja
    auto tables = tableList->getAll();
    for (auto* t : tables)
        ui->orderTableSelect->addItem(
            QString("Meja %1 (Kap: %2)").arg(t->tableNumber).arg(t->capacity),
            t->tableNumber);

    // Populate combo menu order
    auto menus = menuList->getAll();
    for (auto* m : menus)
        ui->orderMenuSelect->addItem(
            QString("%1 - %2").arg(QString::fromStdString(m->name), formatPrice(m->price)),
            m->id);

    refreshMenuTable();
    refreshTableDisplay();
    refreshStaffTable();
    refreshHistoryList();

    // Staf pertama on-duty
    if (staffList->getSize() > 0) {
        Staff* s = staffList->getOnDuty();
        if (s) {
            s->onDuty = true;
            ui->lblOnDuty->setText(QString("Sedang Bertugas: %1 (%2)")
                                       .arg(QString::fromStdString(s->name))
                                       .arg(QString::fromStdString(s->role)));
        }
    }

    addLog("✅ Sistem berhasil diinisialisasi dengan data awal.");
    addLog(QString("📦 %1 item menu dimuat.").arg(menuList->getSize()));
    addLog(QString("🪑 %1 meja dikonfigurasi.").arg(tableList->getSize()));
    addLog(QString("👤 %1 staf terdaftar.").arg(staffList->getSize()));

    // ── Inventaris Seed Data ──
    std::vector<std::tuple<std::string,std::string,int,std::string,int,double>> initInv = {
        {"Beras",         "Bahan Pokok",     50, "kg",  10, 15000},
        {"Minyak Goreng", "Bahan Pokok",     20, "liter", 5, 20000},
        {"Ayam",          "Protein",         15, "kg",    5, 35000},
        {"Telur",         "Protein",        100, "pcs",  20, 2500},
        {"Bawang Merah",  "Bumbu & Rempah",  10, "kg",   3, 25000},
        {"Bawang Putih",  "Bumbu & Rempah",   8, "kg",   3, 30000},
        {"Sayur Bayam",   "Sayuran",         12, "kg",   4, 8000},
        {"Tomat",         "Sayuran",          8, "kg",   3, 12000},
        {"Air Mineral",   "Minuman",         48, "dus",  10, 25000},
        {"Gelas Plastik", "Kemasan",        200, "pcs",  50, 500},
    };
    for (auto& inv : initInv) {
        InventarisItem item;
        item.nama      = std::get<0>(inv);
        item.kategori  = std::get<1>(inv);
        item.stok      = std::get<2>(inv);
        item.satuan    = std::get<3>(inv);
        item.minStok   = std::get<4>(inv);
        item.hargaBeli = std::get<5>(inv);
        inventarisList->insert(item);
    }
    addLog(QString("📦 %1 item inventaris dimuat.").arg(inventarisList->getSize()));
    refreshInventarisTable();

    // Setup tab Credits
    setupCreditTab();
}

// ============================================================
//  STYLESHEET
// ============================================================
void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #F5F5F5;
            color: #212121;
            font-family: Arial;
        }
        QTabBar, QStatusBar, QMenuBar, QGroupBox,
        QPushButton, QLineEdit, QTextEdit, QPlainTextEdit,
        QComboBox, QSpinBox, QDoubleSpinBox, QCheckBox,
        QTableWidget, QTableView, QListWidget, QLabel,
        QHeaderView::section {
            font-size: 13px;
        }
        QMainWindow { background-color: #F5F5F5; }
        QStatusBar  { background-color: #E0E0E0; color: #424242; }

        QTabWidget::pane {
            border: 1px solid #BDBDBD;
            border-radius: 0 6px 6px 6px;
            background-color: #F5F5F5;
        }
        
        QTabWidget::tab-bar {
            alignment: center;
        }

        QTabBar::tab {
            background-color: #E0E0E0; color: #424242;
            padding: 8px 18px; border-radius: 4px 4px 0 0;
            margin-right: 2px; font-weight: bold;
        }
        QTabBar::tab:selected           { background-color: #1976D2; color: #FFFFFF; }
        QTabBar::tab:hover:!selected    { background-color: #BDBDBD; color: #212121; }

        QPushButton {
            padding: 8px 14px; border-radius: 6px; border: none;
            background-color: #E0E0E0; color: #212121; font-weight: bold;
        }
        QPushButton:hover    { background-color: #BDBDBD; }
        QPushButton:pressed  { background-color: #9E9E9E; }
        QPushButton:disabled { background-color: #EEEEEE; color: #9E9E9E; }

        QPushButton#btnAddMenuItem, QPushButton#btnCreateOrder,
        QPushButton#btnAddStaff,    QPushButton#btnGenerateReport,
        QPushButton#btnFilterCategory, QPushButton#btnRunSTLDemo
            { background-color: #1976D2; color: #FFFFFF; }
        QPushButton#btnAddMenuItem:hover,   QPushButton#btnCreateOrder:hover,
        QPushButton#btnAddStaff:hover,      QPushButton#btnGenerateReport:hover,
        QPushButton#btnFilterCategory:hover,QPushButton#btnRunSTLDemo:hover
            { background-color: #1565C0; }

        QPushButton#btnRemoveMenuItem, QPushButton#btnRemoveStaff
            { background-color: #D32F2F; color: #FFFFFF; }
        QPushButton#btnRemoveMenuItem:hover, QPushButton#btnRemoveStaff:hover
            { background-color: #B71C1C; }

        QPushButton#btnSubmitOrder, QPushButton#btnFreeTable
            { background-color: #388E3C; color: #FFFFFF; }
        QPushButton#btnSubmitOrder:hover, QPushButton#btnFreeTable:hover
            { background-color: #2E7D32; }

        QPushButton#btnProcessNextOrder, QPushButton#btnUndoAction,
        QPushButton#btnRotateShift
            { background-color: #F57C00; color: #FFFFFF; }
        QPushButton#btnProcessNextOrder:hover, QPushButton#btnUndoAction:hover,
        QPushButton#btnRotateShift:hover
            { background-color: #E65100; }

        QPushButton#btnOccupyTable
            { background-color: #D32F2F; color: #FFFFFF; }
        QPushButton#btnOccupyTable:hover { background-color: #B71C1C; }

        QGroupBox {
            background-color: #F5F5F5; color: #212121; font-weight: bold;
            border: 1px solid #BDBDBD; border-radius: 6px;
            margin-top: 10px; padding: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin; padding: 0 6px; color: #1976D2;
        }
        
        QLabel#lblDashboardTitle { margin-top: 20px; }
        
        QLabel#lblCurrentTime {
            margin-top: 5px;
            margin-bottom: 15px;
        }
        
        QGroupBox#cardTotalMenu,
        QGroupBox#cardTotalOrders,
        QGroupBox#cardPendingOrders {
            margin-bottom: 8px;
        }
        
        QLabel#lblTotalMenu,
        QLabel#lblTotalOrders,
        QLabel#lblPendingOrders,
        QLabel#lblActiveTables,
        QLabel#lblAvailableMenus {
            padding-top: 0px;
            margin-top: 0px;
            qproperty-alignment: AlignCenter;
        }
        
        QLabel#lblMenuByCategory {
            margin-top: 18px;
            margin-bottom: 20px;
        }
        
        QLabel { background-color: transparent; color: #212121; }

        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #FFFFFF; color: #212121;
            border: 1px solid #BDBDBD; border-radius: 4px;
            padding: 5px; selection-background-color: #1976D2; selection-color: #FFFFFF;
        }
        QLineEdit:focus, QTextEdit:focus { border: 1px solid #1976D2; }
        QLineEdit:disabled { background-color: #EEEEEE; color: #9E9E9E; }

        QComboBox {
            background-color: #FFFFFF; color: #212121;
            border: 1px solid #BDBDBD; border-radius: 4px; padding: 5px 8px;
        }
        QComboBox:focus { border: 1px solid #1976D2; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox QAbstractItemView {
            background-color: #FFFFFF; color: #212121;
            selection-background-color: #1976D2; selection-color: #FFFFFF;
            border: 1px solid #BDBDBD;
        }

        QSpinBox, QDoubleSpinBox {
            background-color: #FFFFFF; color: #212121;
            border: 1px solid #BDBDBD; border-radius: 4px; padding: 5px;
        }
        QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #1976D2; }

        QCheckBox { color: #212121; spacing: 6px; }
        QCheckBox::indicator {
            width: 16px; height: 16px;
            border: 1px solid #BDBDBD; border-radius: 3px; background-color: #FFFFFF;
        }
        QCheckBox::indicator:checked { background-color: #1976D2; border-color: #1976D2; }

        QTableWidget, QTableView {
            background-color: #FFFFFF; color: #212121;
            gridline-color: #E0E0E0; border: 1px solid #BDBDBD;
            alternate-background-color: #F0F4FF;
            selection-background-color: #BBDEFB; selection-color: #212121;
        }
        QHeaderView::section {
            background-color: #1976D2; color: #FFFFFF;
            padding: 6px; font-weight: bold;
            border: none; border-right: 1px solid #1565C0;
        }
        QHeaderView::section:last { border-right: none; }
        QTableWidget::item:selected { background-color: #BBDEFB; color: #212121; }
        QTableCornerButton::section { background-color: #1976D2; border: none; }

        QListWidget {
            background-color: #FFFFFF; color: #212121;
            border: 1px solid #BDBDBD; border-radius: 4px;
            alternate-background-color: #F0F4FF;
        }
        QListWidget::item { padding: 4px; color: #212121; }
        QListWidget::item:selected            { background-color: #1976D2; color: #FFFFFF; }
        QListWidget::item:hover:!selected     { background-color: #E3F2FD; }

        QScrollBar:vertical {
            background-color: #F5F5F5; width: 10px; border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background-color: #BDBDBD; border-radius: 5px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background-color: #9E9E9E; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }

        QScrollBar:horizontal {
            background-color: #F5F5F5; height: 10px; border-radius: 5px;
        }
        QScrollBar::handle:horizontal {
            background-color: #BDBDBD; border-radius: 5px; min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background-color: #9E9E9E; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }

        QToolTip {
            background-color: #FFFDE7; color: #212121;
            border: 1px solid #F9A825; border-radius: 4px; padding: 4px;
        }
        QMessageBox { background-color: #F5F5F5; color: #212121; }
        QMessageBox QLabel { color: #212121; }

        QMenuBar { background-color: #E0E0E0; color: #212121; }
        QMenuBar::item:selected { background-color: #1976D2; color: #FFFFFF; }
        QMenu {
            background-color: #FFFFFF; color: #212121; border: 1px solid #BDBDBD;
        }
        QMenu::item:selected { background-color: #1976D2; color: #FFFFFF; }

        QPushButton#btnTambahStok, QPushButton#btnProsesPembayaran,
        QPushButton#btnGenerateLapKeu
            { background-color: #1976D2; color: #FFFFFF; }
        QPushButton#btnTambahStok:hover, QPushButton#btnProsesPembayaran:hover,
        QPushButton#btnGenerateLapKeu:hover
            { background-color: #1565C0; }

        QPushButton#btnHapusStok
            { background-color: #D32F2F; color: #FFFFFF; }
        QPushButton#btnHapusStok:hover { background-color: #B71C1C; }

        QPushButton#btnCetakStruk, QPushButton#btnSimpanInventaris,
        QPushButton#btnSimpanLapKeu
            { background-color: #388E3C; color: #FFFFFF; }
        QPushButton#btnCetakStruk:hover, QPushButton#btnSimpanInventaris:hover,
        QPushButton#btnSimpanLapKeu:hover
            { background-color: #2E7D32; }

        QPushButton#btnUpdateStok, QPushButton#btnHitungKembalian,
        QPushButton#btnMuatOrder, QPushButton#btnCekStokMinim
            { background-color: #F57C00; color: #FFFFFF; }
        QPushButton#btnUpdateStok:hover, QPushButton#btnHitungKembalian:hover,
        QPushButton#btnMuatOrder:hover, QPushButton#btnCekStokMinim:hover
            { background-color: #E65100; }

        QSplitter::handle { background-color: #BDBDBD; }
    )");
}

// ============================================================
//  HELPERS
// ============================================================
void MainWindow::addLog(const QString& msg)
{
    QString ts = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    ui->logDisplay->append(ts + msg);
}

QString MainWindow::formatPrice(double price)
{
    long long val = (long long)price;
    QString s = QString::number(val);
    int pos = s.length() - 3;
    while (pos > 0) { s.insert(pos, '.'); pos -= 3; }
    return QString("Rp %1").arg(s);
}

std::string MainWindow::getCurrentTimestamp()
{
    auto t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

void MainWindow::updateClock()
{
    ui->lblCurrentTime->setText(
        QDateTime::currentDateTime().toString("dddd, dd MMMM yyyy - hh:mm:ss"));
}

// ============================================================
//  REFRESH METHODS
// ============================================================
void MainWindow::refreshMenuTable()
{
    auto items = menuList->getAll();
    ui->menuTable->setRowCount((int)items.size());
    for (int i = 0; i < (int)items.size(); i++) {
        MenuItem* m = items[i];
        ui->menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        ui->menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        ui->menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        ui->menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        auto* avail = new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak");
        avail->setForeground(m->available ? Qt::darkGreen : Qt::red);
        ui->menuTable->setItem(i, 4, avail);
    }
}

void MainWindow::refreshOrderTable()
{
    auto& orders = orderQueue->getAllOrders();
    ui->orderTable->setRowCount((int)orders.size());
    int row = 0;
    for (auto it = orders.begin(); it != orders.end(); ++it, ++row) {
        Order* o = *it;
        ui->orderTable->setItem(row, 0, new QTableWidgetItem(QString::number(o->orderId)));
        ui->orderTable->setItem(row, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        ui->orderTable->setItem(row, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        auto* status = new QTableWidgetItem(QString::fromStdString(o->status));
        if (o->status == "pending")   status->setForeground(QColor(255, 152,  0));
        if (o->status == "preparing") status->setForeground(QColor( 25, 118, 210));
        if (o->status == "served")    status->setForeground(QColor( 76, 175,  80));
        if (o->status == "paid")      status->setForeground(Qt::gray);
        ui->orderTable->setItem(row, 3, status);
        ui->orderTable->setItem(row, 4, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
    }
    refreshPendingOrders();
}

void MainWindow::refreshPendingOrders()
{
    auto pending = orderQueue->getPendingOrders();
    ui->pendingOrdersTable->setRowCount((int)pending.size());
    for (int i = 0; i < (int)pending.size(); i++) {
        Order* o = pending[i];
        ui->pendingOrdersTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        ui->pendingOrdersTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        ui->pendingOrdersTable->setItem(i, 2, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
        ui->pendingOrdersTable->setItem(i, 3, new QTableWidgetItem(formatPrice(o->totalPrice)));
    }
}

void MainWindow::refreshTableDisplay()
{
    auto tables = tableList->getAll();
    ui->tableDisplay->setRowCount((int)tables.size());
    for (int i = 0; i < (int)tables.size(); i++) {
        Table* t = tables[i];
        ui->tableDisplay->setItem(i, 0, new QTableWidgetItem(QString("Meja %1").arg(t->tableNumber)));
        ui->tableDisplay->setItem(i, 1, new QTableWidgetItem(QString("%1 orang").arg(t->capacity)));
        auto* status = new QTableWidgetItem(t->isOccupied ? "🔴 Terisi" : "🟢 Kosong");
        status->setForeground(t->isOccupied ? Qt::red : Qt::darkGreen);
        ui->tableDisplay->setItem(i, 2, status);
        ui->tableDisplay->setItem(i, 3, new QTableWidgetItem(
                                            t->currentOrderId >= 0 ? QString("#%1").arg(t->currentOrderId) : "-"));
    }
    Table* curr = tableList->getCurrent();
    if (curr)
        ui->lblCurrentTable->setText(
            QString("Meja Aktif: %1 (Kap: %2)").arg(curr->tableNumber).arg(curr->capacity));
}

void MainWindow::refreshStaffTable()
{
    auto staffs = staffList->getAll();
    ui->staffTable->setRowCount((int)staffs.size());
    for (int i = 0; i < (int)staffs.size(); i++) {
        Staff* s = staffs[i];
        ui->staffTable->setItem(i, 0, new QTableWidgetItem(QString::number(s->staffId)));
        ui->staffTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s->name)));
        ui->staffTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s->role)));
        auto* status = new QTableWidgetItem(s->onDuty ? "✅ Bertugas" : "💤 Istirahat");
        status->setForeground(s->onDuty ? Qt::darkGreen : Qt::gray);
        ui->staffTable->setItem(i, 3, status);
    }
}

void MainWindow::refreshHistoryList()
{
    ui->historyList->clear();
    auto actions = actionStack->getAll();
    for (auto& a : actions)
        ui->historyList->addItem(
            QString("[%1] %2")
                .arg(QString::fromStdString(a.type), QString::fromStdString(a.description)));
}

void MainWindow::refreshDashboard()
{
    ui->lblTotalMenu->setText(QString::number(menuList->getSize()));
    ui->lblTotalOrders->setText(QString::number(orderQueue->totalOrders()));
    ui->lblPendingOrders->setText(QString::number(orderQueue->getPendingOrders().size()));

    // STL count: meja terisi
    auto tables = tableList->getAll();
    ui->lblActiveTables->setText(QString::number(STLUtils::countOccupiedTables(tables)));

    // STL count: menu tersedia
    auto menus = menuList->getAll();
    ui->lblAvailableMenus->setText(QString::number(STLUtils::countAvailableMenus(menus)));

    // Breakdown kategori
    int makanan = menuHash->countByCategory("Makanan Utama");
    int minuman  = menuHash->countByCategory("Minuman");
    int snack    = menuHash->countByCategory("Snack");
    int dessert  = menuHash->countByCategory("Dessert");
    int veggie   = menuHash->countByCategory("Vegetarian");
    ui->lblMenuByCategory->setText(
        QString("Makanan: %1 | Minuman: %2 | Snack: %3 | Dessert: %4 | Veggie: %5")
            .arg(makanan).arg(minuman).arg(snack).arg(dessert).arg(veggie));

    refreshStatisticsPanel();
}

void MainWindow::updateGraphDisplay()
{
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
    ui->graphDisplay->setText(output);
}

// ============================================================
//  SLOT: MENU
// ============================================================
void MainWindow::onAddMenuItem()
{
    try {
        QString name = ui->inputMenuName->text().trimmed();
        if (name.isEmpty())
            throw RestaurantException("Nama menu tidak boleh kosong!");

        double price = ui->inputMenuPrice->value();
        if (price <= 0)
            throw RestaurantException("Harga harus lebih dari 0!");

        MenuItem item(nextMenuId++,
                      name.toStdString(),
                      ui->inputMenuCategory->currentText().toStdString(),
                      price,
                      ui->inputMenuAvailable->isChecked());

        menuList->insert(item);
        menuAVL->insert(item);
        menuHash->insert(item);

        // [CALLBACK 1] forEachMenu
        auto newItems = std::vector<MenuItem*>();
        MenuItem* inserted = menuList->find(item.id);
        if (inserted) newItems.push_back(inserted);
        forEachMenu(newItems, [this](const MenuItem& m) {
            addLog(QString("➕ Menu ditambah: %1 (%2)")
                       .arg(QString::fromStdString(m.name), formatPrice(m.price)));
        });

        // [CALLBACK 3] filterMenuByCategory
        auto allMenus = menuList->getAll();
        int sameCategory = 0;
        filterMenuByCategory(allMenus, item.category,
                             [&sameCategory](const MenuItem&) { sameCategory++; });
        addLog(QString("   ↳ Total menu kategori '%1' sekarang: %2 item")
                   .arg(QString::fromStdString(item.category)).arg(sameCategory));

        actionStack->push(Action("add_menu", "Tambah menu: " + item.name, item.id));

        ui->orderMenuSelect->addItem(
            QString("%1 - %2").arg(name, formatPrice(price)), item.id);

        ui->inputMenuName->clear();
        ui->inputMenuPrice->setValue(0);
        refreshMenuTable();
        refreshDashboard();
        refreshHistoryList();

    } catch (const RestaurantException& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onRemoveMenuItem()
{
    int row = ui->menuTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Pilih menu yang akan dihapus!"); return;
    }
    int id       = ui->menuTable->item(row, 0)->text().toInt();
    QString name = ui->menuTable->item(row, 1)->text();

    menuList->remove(id);
    menuAVL->remove(id);
    menuHash->remove(id);

    actionStack->push(Action("remove_menu", "Hapus menu: " + name.toStdString(), id));
    addLog(QString("🗑 Menu dihapus: %1").arg(name));
    refreshMenuTable();
    refreshDashboard();
    refreshHistoryList();
}

void MainWindow::onSearchMenu()
{
    QString query = ui->searchMenuInput->text().trimmed();
    if (query.isEmpty()) { refreshMenuTable(); return; }

    bool ok;
    int id = query.toInt(&ok);
    if (ok) {
        MenuItem* found = menuHash->findById(id);
        if (found) {
            ui->menuTable->setRowCount(1);
            ui->menuTable->setItem(0, 0, new QTableWidgetItem(QString::number(found->id)));
            ui->menuTable->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(found->name)));
            ui->menuTable->setItem(0, 2, new QTableWidgetItem(QString::fromStdString(found->category)));
            ui->menuTable->setItem(0, 3, new QTableWidgetItem(formatPrice(found->price)));
            ui->menuTable->setItem(0, 4, new QTableWidgetItem(found->available ? "✅ Ya" : "❌ Tidak"));
            addLog(QString("🔍 [Hash O(1)] Ditemukan: %1")
                       .arg(QString::fromStdString(displayItem(*found))));
            return;
        }
    }

    auto allPtrs = menuList->getAll();
    MenuItem* foundByTemplate = linearSearch<MenuItem>(allPtrs,
                                                       [&query](MenuItem* m) {
                                                           return QString::fromStdString(m->name).contains(query, Qt::CaseInsensitive);
                                                       });
    if (foundByTemplate)
        addLog(QString("🔍 [Template linearSearch] Ditemukan: %1")
                   .arg(QString::fromStdString(displayItem(*foundByTemplate))));

    auto filtered = linearSearchAll<MenuItem>(allPtrs,
                                              [&query](MenuItem* m) {
                                                  return QString::fromStdString(m->name).contains(query, Qt::CaseInsensitive) ||
                                                         QString::fromStdString(m->category).contains(query, Qt::CaseInsensitive);
                                              });

    int found_count = STLUtils::countAvailableMenus(
        std::vector<MenuItem*>(filtered.begin(), filtered.end()));
    addLog(QString("   ↳ Ditemukan %1 menu, %2 di antaranya tersedia")
               .arg(filtered.size()).arg(found_count));

    ui->menuTable->setRowCount((int)filtered.size());
    for (int i = 0; i < (int)filtered.size(); i++) {
        MenuItem* m = filtered[i];
        ui->menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        ui->menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        ui->menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        ui->menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        ui->menuTable->setItem(i, 4, new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak"));
    }
}

void MainWindow::onSortMenu()
{
    auto items = menuAVL->inorderTraversal();
    int idx = ui->sortMenuCombo->currentIndex();
    if      (idx == 1) Sorting::bubbleSortByPrice(items, true);
    else if (idx == 2) Sorting::bubbleSortByPrice(items, false);
    else if (idx == 3) Sorting::sortByName(items);

    ui->menuTable->setRowCount((int)items.size());
    for (int i = 0; i < (int)items.size(); i++) {
        ui->menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(items[i].id)));
        ui->menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(items[i].name)));
        ui->menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(items[i].category)));
        ui->menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(items[i].price)));
        ui->menuTable->setItem(i, 4, new QTableWidgetItem(items[i].available ? "✅ Ya" : "❌ Tidak"));
    }
    addLog(QString("🔃 Menu diurutkan: %1").arg(ui->sortMenuCombo->currentText()));
}

void MainWindow::onSaveMenu()
{
    auto items = menuAVL->inorderTraversal();
    if (FileIO::saveMenuToFile(items)) {
        addLog("💾 Data menu berhasil disimpan ke menu.txt");
        QMessageBox::information(this, "Berhasil", "Menu berhasil disimpan ke menu.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan file!");
    }
}

void MainWindow::onLoadMenu()
{
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
        ui->orderMenuSelect->addItem(QString::fromStdString(m.name), m.id);
    }
    refreshMenuTable();
    refreshDashboard();
    addLog(QString("📂 %1 item menu dimuat dari file.").arg(loaded.size()));
}

void MainWindow::onFilterByCategory()
{
    QString cat    = ui->filterCategoryCombo->currentText();
    std::string cs = cat.toStdString();

    std::vector<MenuItem*> filtered;
    auto allMenus = menuList->getAll();

    // [CALLBACK 3] filterMenuByCategory
    filterMenuByCategory(allMenus, cs,
                         [&filtered](const MenuItem& m) {
                             filtered.push_back(const_cast<MenuItem*>(&m));
                         });

    // [CALLBACK 4] processMenusWithCondition
    int expensive = 0;
    processMenusWithCondition(allMenus,
                              [&cs](const MenuItem& m) { return m.category == cs && m.price > 15000; },
                              [&expensive](const MenuItem&) { expensive++; });

    addLog(QString("🔎 [Callback] Kategori '%1': %2 item, %3 di antaranya > Rp15.000")
               .arg(cat).arg(filtered.size()).arg(expensive));

    ui->menuTable->setRowCount((int)filtered.size());
    for (int i = 0; i < (int)filtered.size(); i++) {
        MenuItem* m = filtered[i];
        ui->menuTable->setItem(i, 0, new QTableWidgetItem(QString::number(m->id)));
        ui->menuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m->name)));
        ui->menuTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m->category)));
        ui->menuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(m->price)));
        ui->menuTable->setItem(i, 4, new QTableWidgetItem(m->available ? "✅ Ya" : "❌ Tidak"));
    }

    if (filtered.empty())
        QMessageBox::information(this, "Filter Kategori",
                                 QString("Tidak ada menu dalam kategori '%1'.").arg(cat));
}

// ============================================================
//  SLOT: ORDER
// ============================================================
void MainWindow::onCreateOrder()
{
    if (currentOrder) {
        QMessageBox::warning(this, "Peringatan",
                             "Selesaikan atau batalkan order saat ini terlebih dahulu!");
        return;
    }
    int tableNum = ui->orderTableSelect->currentData().toInt();
    int priority = (ui->orderPrioritySelect->currentIndex() == 0) ? 2 : 1;

    currentOrder = new Order(0, tableNum, priority);
    currentOrderItems.clear();
    currentOrderTotal = 0.0;
    ui->orderItemList->clear();
    ui->lblOrderTotal->setText("Total: Rp 0");
    addLog(QString("🆕 Order baru untuk Meja %1 (Prioritas: %2)")
               .arg(tableNum).arg(priority == 1 ? "VIP" : "Normal"));
}

void MainWindow::onAddItemToOrder()
{
    if (!currentOrder) {
        QMessageBox::warning(this, "Peringatan", "Buat order baru terlebih dahulu!"); return;
    }
    int menuId = ui->orderMenuSelect->currentData().toInt();
    int qty    = ui->orderQtyInput->value();

    MenuItem* m = menuHash->findById(menuId);
    if (!m) { QMessageBox::warning(this, "Error", "Menu tidak ditemukan!"); return; }

    OrderItem item;
    item.menuItemId   = m->id;
    item.menuItemName = m->name;
    item.quantity     = qty;
    item.subtotal     = m->price * qty;

    currentOrderItems.push_back(item);
    currentOrderTotal += item.subtotal;

    ui->orderItemList->addItem(
        QString("%1 x%2 = %3")
            .arg(QString::fromStdString(m->name))
            .arg(qty)
            .arg(formatPrice(item.subtotal)));
    ui->lblOrderTotal->setText(QString("Total: %1").arg(formatPrice(currentOrderTotal)));
}

void MainWindow::onRemoveItemFromOrder()
{
    int row = ui->orderItemList->currentRow();
    if (row < 0 || !currentOrder) return;
    currentOrderTotal -= currentOrderItems[row].subtotal;
    currentOrderItems.erase(currentOrderItems.begin() + row);
    delete ui->orderItemList->takeItem(row);
    ui->lblOrderTotal->setText(QString("Total: %1").arg(formatPrice(currentOrderTotal)));
}

void MainWindow::onSubmitOrder()
{
    if (!currentOrder || currentOrderItems.empty()) {
        QMessageBox::warning(this, "Peringatan", "Tambahkan item ke order terlebih dahulu!");
        return;
    }

    Order* submitted = orderQueue->enqueue(currentOrder->tableNumber, currentOrder->priority);
    submitted->items      = currentOrderItems;
    submitted->totalPrice = currentOrderTotal;
    submitted->timestamp  = getCurrentTimestamp();

    tableList->occupyTable(currentOrder->tableNumber, submitted->orderId);

    // [CALLBACK 2] applyToAllOrders
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
    ui->orderItemList->clear();
    ui->lblOrderTotal->setText("Total: Rp 0");

    refreshOrderTable();
    refreshTableDisplay();
    refreshDashboard();
    refreshHistoryList();
}

void MainWindow::onProcessNextOrder()
{
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
//  SLOT: MEJA
// ============================================================
void MainWindow::onNextTable()
{
    Table* t = tableList->next();
    if (t) {
        ui->lblCurrentTable->setText(
            QString("Meja Aktif: %1 (Kap: %2, Status: %3)")
                .arg(t->tableNumber).arg(t->capacity)
                .arg(t->isOccupied ? "Terisi" : "Kosong"));
        addLog(QString("▶ Navigasi ke Meja %1 (Circular)").arg(t->tableNumber));
    }
}

void MainWindow::onOccupyTable()
{
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    if (curr->isOccupied) { QMessageBox::warning(this, "Peringatan", "Meja sudah terisi!"); return; }
    tableList->occupyTable(curr->tableNumber, -1);
    addLog(QString("🔴 Meja %1 ditandai terisi.").arg(curr->tableNumber));
    refreshTableDisplay();
    refreshDashboard();
}

void MainWindow::onFreeTable()
{
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    tableList->freeTable(curr->tableNumber);
    addLog(QString("🟢 Meja %1 dikosongkan.").arg(curr->tableNumber));
    refreshTableDisplay();
    refreshDashboard();
}

void MainWindow::onRunBFS()
{
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    auto result = tableGraph->bfs(curr->tableNumber);
    QString output = "=== BFS dari Meja " + QString::number(curr->tableNumber) + " ===\n";
    output += "Urutan kunjungan: ";
    for (int i = 0; i < (int)result.size(); i++) {
        output += "Meja " + QString::number(result[i]);
        if (i < (int)result.size()-1) output += " → ";
    }
    updateGraphDisplay();
    ui->graphDisplay->setText(output + "\n\n📋 Adjacency List:\n" + ui->graphDisplay->toPlainText());
    addLog(QString("🔍 BFS dari Meja %1 selesai.").arg(curr->tableNumber));
}

void MainWindow::onRunDFS()
{
    Table* curr = tableList->getCurrent();
    if (!curr) return;
    auto result = tableGraph->dfs(curr->tableNumber);
    QString output = "=== DFS dari Meja " + QString::number(curr->tableNumber) + " ===\n";
    output += "Urutan kunjungan: ";
    for (int i = 0; i < (int)result.size(); i++) {
        output += "Meja " + QString::number(result[i]);
        if (i < (int)result.size()-1) output += " → ";
    }
    ui->graphDisplay->setText(output);
    addLog(QString("🔍 DFS dari Meja %1 selesai.").arg(curr->tableNumber));
}

// ============================================================
//  SLOT: STAF
// ============================================================
void MainWindow::onAddStaff()
{
    QString name = ui->inputStaffName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Peringatan", "Nama staf tidak boleh kosong!"); return;
    }
    Staff s(nextStaffId++, name.toStdString(), ui->inputStaffRole->currentText().toStdString());
    staffList->insert(s);
    actionStack->push(Action("add_staff", "Tambah staf: " + s.name, s.staffId));
    addLog(QString("👤 Staf ditambah: %1 (%2)").arg(name, ui->inputStaffRole->currentText()));
    ui->inputStaffName->clear();
    refreshStaffTable();
    refreshHistoryList();
}

void MainWindow::onRemoveStaff()
{
    int row = ui->staffTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Pilih staf di tabel yang ingin dihapus!"); return;
    }
    int staffId  = ui->staffTable->item(row, 0)->text().toInt();
    QString name = ui->staffTable->item(row, 1)->text();

    // [TEMPLATE linearSearch<Staff>]
    auto allStaffs = staffList->getAll();
    Staff* toDelete = linearSearch<Staff>(allStaffs,
                                          [staffId](Staff* s) { return s->staffId == staffId; });
    if (toDelete)
        addLog(QString("🔍 [Template linearSearch<Staff>] Target: %1")
                   .arg(QString::fromStdString(displayItem(*toDelete))));

    auto reply = QMessageBox::question(this, "Konfirmasi Hapus",
                                       QString("Yakin ingin menghapus staf:\n%1?").arg(name),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (staffList->remove(staffId)) {
        actionStack->push(Action("remove_staff", "Hapus staf: " + name.toStdString(), staffId));
        addLog(QString("🗑 Staf dihapus: %1").arg(name));
        Staff* duty = staffList->getOnDuty();
        ui->lblOnDuty->setText(duty
                                   ? QString("Sedang Bertugas: %1 (%2)")
                                         .arg(QString::fromStdString(duty->name), QString::fromStdString(duty->role))
                                   : "Sedang Bertugas: -");
        refreshStaffTable();
        refreshHistoryList();
    } else {
        QMessageBox::warning(this, "Error", "Gagal menghapus staf!");
    }
}

void MainWindow::onRotateShift()
{
    try {
        if (staffList->getSize() == 0)
            throw RestaurantException("Tidak ada staf terdaftar!");

        Staff* next = staffList->rotateShift();
        if (next) {
            ui->lblOnDuty->setText(
                QString("Sedang Bertugas: %1 (%2)")
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
//  SLOT: RIWAYAT & LAPORAN
// ============================================================
void MainWindow::onUndoAction()
{
    try {
        Action a = actionStack->pop();
        addLog(QString("↩ UNDO: [%1] %2")
                   .arg(QString::fromStdString(a.type), QString::fromStdString(a.description)));
        QMessageBox::information(this, "Undo",
                                 QString("Aksi dibatalkan:\n%1").arg(QString::fromStdString(a.description)));
        refreshHistoryList();
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onGenerateReport()
{
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());
    if (!allOrders.empty())
        Sorting::mergeSort(allOrders, 0, (int)allOrders.size() - 1);

    ui->reportTable->setRowCount((int)allOrders.size());
    for (int i = 0; i < (int)allOrders.size(); i++) {
        Order* o = allOrders[i];
        ui->reportTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        ui->reportTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        ui->reportTable->setItem(i, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        ui->reportTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(o->status)));
        ui->reportTable->setItem(i, 4, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
    }

    double totalRevenue = 0.0;
    std::for_each(allOrders.begin(), allOrders.end(),
                  [&totalRevenue](Order* o) { totalRevenue += o->totalPrice; });

    logOrdersViaCallback("paid");
    if (!allOrders.empty())
        addLog(QString("📋 [Overload displayItem(Order)] Order pertama: %1")
                   .arg(QString::fromStdString(displayItem(*allOrders.front()))));

    addLog(QString("📈 Laporan digenerate. Total order: %1, Pendapatan: %2")
               .arg(allOrders.size()).arg(formatPrice(totalRevenue)));
}

void MainWindow::onSortReport()
{
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());

    int idx = ui->reportSortCombo->currentIndex();
    if (idx == 0) {
        if (!allOrders.empty())
            Sorting::mergeSort(allOrders, 0, (int)allOrders.size() - 1);
    } else if (idx == 1) {
        std::sort(allOrders.begin(), allOrders.end(),
                  [](Order* a, Order* b) { return a->orderId < b->orderId; });
    } else {
        std::sort(allOrders.begin(), allOrders.end(),
                  [](Order* a, Order* b) { return a->tableNumber < b->tableNumber; });
    }

    ui->reportTable->setRowCount((int)allOrders.size());
    for (int i = 0; i < (int)allOrders.size(); i++) {
        Order* o = allOrders[i];
        ui->reportTable->setItem(i, 0, new QTableWidgetItem(QString::number(o->orderId)));
        ui->reportTable->setItem(i, 1, new QTableWidgetItem(QString("Meja %1").arg(o->tableNumber)));
        ui->reportTable->setItem(i, 2, new QTableWidgetItem(formatPrice(o->totalPrice)));
        ui->reportTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(o->status)));
        ui->reportTable->setItem(i, 4, new QTableWidgetItem(o->priority == 1 ? "⭐ VIP" : "Normal"));
    }
}

void MainWindow::onSaveReport()
{
    auto& orders = orderQueue->getAllOrders();
    std::vector<Order*> allOrders(orders.begin(), orders.end());
    if (FileIO::saveOrdersToFile(allOrders)) {
        addLog("💾 Laporan order disimpan ke orders.txt");
        QMessageBox::information(this, "Berhasil", "Laporan berhasil disimpan ke orders.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan laporan!");
    }
}

void MainWindow::onRunSTLDemo()
{
    auto& allOrders = orderQueue->getAllOrders();
    auto allMenus   = menuList->getAll();
    auto allTables  = tableList->getAll();

    QString result;
    result += "=== DEMO STL find + count ===\n\n";

    result += "--- std::count_if: Order per Status ---\n";
    for (const auto& s : std::vector<std::string>{"pending","preparing","served","paid"}) {
        int n = STLUtils::countOrdersByStatus(allOrders, s);
        result += QString("  %1: %2 order\n").arg(QString::fromStdString(s)).arg(n);
    }

    int avail = STLUtils::countAvailableMenus(allMenus);
    result += QString("\n--- std::count_if: Menu Tersedia ---\n");
    result += QString("  Menu TERSEDIA : %1\n").arg(avail);
    result += QString("  Menu HABIS    : %1\n").arg((int)allMenus.size() - avail);

    int occ = STLUtils::countOccupiedTables(allTables);
    result += QString("\n--- std::count_if: Status Meja ---\n");
    result += QString("  Meja TERISI : %1 dari %2\n").arg(occ).arg(allTables.size());

    result += "\n--- std::find_if: Cari Order #1 ---\n";
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), 1);
    if (it != allOrders.end())
        result += QString("  Ditemukan: %1\n").arg(QString::fromStdString(displayItem(**it)));
    else
        result += "  Order #1 tidak ditemukan (belum ada order)\n";

    result += "\n--- std::find: Cek ID Menu ---\n";
    std::vector<int> idList;
    for (auto* m : allMenus) idList.push_back(m->id);
    for (int testId : {1, 5, 999}) {
        bool exists = STLUtils::menuIdExists(idList, testId);
        result += QString("  ID %1: %2\n").arg(testId).arg(exists ? "ADA ✅" : "TIDAK ADA ❌");
    }

    result += "\n--- Callback + std::count_if: Menu per Kategori ---\n";
    for (const auto& cat : std::vector<std::string>{"Makanan Utama","Minuman","Snack","Dessert","Vegetarian"}) {
        int n = menuHash->countByCategory(cat);
        result += QString("  %1: %2 item\n").arg(QString::fromStdString(cat)).arg(n);
    }

    ui->statsDisplay->setText(result);
    addLog("🔬 [STL Demo] std::find + std::count dijalankan — lihat panel statistik.");
    QMessageBox::information(this, "STL Demo — find + count", result);
}

// ============================================================
//  CALLBACK HELPERS
// ============================================================
void MainWindow::logMenusViaCallback(const std::string& category)
{
    auto allMenus = menuList->getAll();
    int count = 0;
    filterMenuByCategory(allMenus, category, [&count](const MenuItem&) { count++; });
    if (count > 0)
        addLog(QString("   [Callback] Kategori '%1' memiliki %2 item.")
                   .arg(QString::fromStdString(category)).arg(count));
}

void MainWindow::logOrdersViaCallback(const std::string& status)
{
    int count = 0; double total = 0.0;
    applyToAllOrders(orderQueue->getAllOrders(), [&](const Order& o) {
        if (o.status == status) { count++; total += o.totalPrice; }
    });
    addLog(QString("   [Callback applyToAllOrders] Status '%1': %2 order, total Rp%3")
               .arg(QString::fromStdString(status)).arg(count).arg((int)total));
}

// ============================================================
//  FUNCTION OVERLOADING (syarat 7)
// ============================================================
QString MainWindow::formatDisplayItem(const MenuItem& m) { return QString::fromStdString(displayItem(m)); }
QString MainWindow::formatDisplayItem(const Order& o)    { return QString::fromStdString(displayItem(o)); }
QString MainWindow::formatDisplayItem(const Staff& s)    { return QString::fromStdString(displayItem(s)); }

// ============================================================
//  STL STATISTICS PANEL (syarat 9)
// ============================================================
void MainWindow::refreshStatisticsPanel()
{
    if (!ui->statsDisplay) return;

    auto& allOrders = orderQueue->getAllOrders();
    auto allMenus   = menuList->getAll();
    auto allTables  = tableList->getAll();

    auto summary = STLUtils::getOrderStatusSummary(allOrders);

    QString stats;
    stats += QString("Order  — pending:%1 preparing:%2 served:%3 paid:%4\n")
                 .arg(summary["pending"]).arg(summary["preparing"])
                 .arg(summary["served"]).arg(summary["paid"]);

    int avail = STLUtils::countAvailableMenus(allMenus);
    stats += QString("Menu   — tersedia:%1  habis:%2  total:%3\n")
                 .arg(avail).arg((int)allMenus.size() - avail).arg(allMenus.size());

    int occ = STLUtils::countOccupiedTables(allTables);
    stats += QString("Meja   — terisi:%1  kosong:%2  total:%3\n")
                 .arg(occ).arg((int)allTables.size() - occ).arg(allTables.size());

    ui->statsDisplay->setText(stats);
}

// ============================================================
//  REFRESH: INVENTARIS
// ============================================================
void MainWindow::refreshInventarisTable()
{
    auto items = inventarisList->getAll();
    ui->inventarisTable->setRowCount((int)items.size());
    int menipis = 0;
    for (int i = 0; i < (int)items.size(); i++) {
        InventarisItem* inv = items[i];
        if (inv->menipis()) menipis++;
        ui->inventarisTable->setItem(i, 0, new QTableWidgetItem(QString::number(inv->id)));
        ui->inventarisTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(inv->nama)));
        ui->inventarisTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(inv->kategori)));
        ui->inventarisTable->setItem(i, 3, new QTableWidgetItem(QString::number(inv->stok)));
        ui->inventarisTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(inv->satuan)));
        ui->inventarisTable->setItem(i, 5, new QTableWidgetItem(QString::number(inv->minStok)));
        ui->inventarisTable->setItem(i, 6, new QTableWidgetItem(formatPrice(inv->hargaBeli)));
        ui->inventarisTable->setItem(i, 7, new QTableWidgetItem(formatPrice(inv->nilaiStok())));

        auto* statusItem = new QTableWidgetItem(inv->menipis() ? "⚠ Menipis" : "✅ Aman");
        statusItem->setForeground(inv->menipis() ? QColor(211, 47, 47) : QColor(56, 142, 60));
        ui->inventarisTable->setItem(i, 8, statusItem);

        if (inv->menipis()) {
            for (int col = 0; col < 9; col++) {
                if (ui->inventarisTable->item(i, col))
                    ui->inventarisTable->item(i, col)->setBackground(QColor(255, 243, 224));
            }
        }
    }
    ui->lblInvTotalBahan->setText(QString::number(items.size()));
    ui->lblInvStokMenipis->setText(QString("<font color='%1'>%2</font>")
        .arg(menipis > 0 ? "#D32F2F" : "#388E3C").arg(menipis));
    ui->lblInvNilaiTotal->setText(formatPrice(inventarisList->getTotalNilaiStok()));
}

// ============================================================
//  REFRESH: TRANSAKSI
// ============================================================
void MainWindow::refreshTransaksiTable()
{
    auto& transaksiVec = transaksiList->getAll();
    ui->transaksiTable->setRowCount((int)transaksiVec.size());
    for (int i = 0; i < (int)transaksiVec.size(); i++) {
        Transaksi* t = transaksiVec[i];
        ui->transaksiTable->setItem(i, 0, new QTableWidgetItem(QString::number(t->transaksiId)));
        ui->transaksiTable->setItem(i, 1, new QTableWidgetItem(QString::number(t->orderId)));
        ui->transaksiTable->setItem(i, 2, new QTableWidgetItem(QString("Meja %1").arg(t->tableNumber)));
        ui->transaksiTable->setItem(i, 3, new QTableWidgetItem(formatPrice(t->subtotal)));
        ui->transaksiTable->setItem(i, 4, new QTableWidgetItem(QString("%1%").arg(t->diskonPersen)));
        ui->transaksiTable->setItem(i, 5, new QTableWidgetItem(formatPrice(t->totalBayar)));
        ui->transaksiTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(t->metodePembayaran)));
        ui->transaksiTable->setItem(i, 7, new QTableWidgetItem(formatPrice(t->kembalian)));
        ui->transaksiTable->setItem(i, 8, new QTableWidgetItem(QString::fromStdString(t->timestamp)));
    }
    refreshPembayaranStats();
}

void MainWindow::refreshPembayaranStats()
{
    int paid = (int)transaksiList->getAll().size();
    auto& allOrders = orderQueue->getAllOrders();
    int belumLunas = (int)std::count_if(allOrders.begin(), allOrders.end(),
        [](Order* o){ return o->status != "paid"; });

    ui->lblPembTotalTranx->setText(QString::number(paid));
    ui->lblPembPendapatan->setText(formatPrice(transaksiList->getTotalPendapatan()));
    ui->lblPembBelumLunas->setText(QString("<font color='%1'>%2</font>")
        .arg(belumLunas > 0 ? "#D32F2F" : "#388E3C").arg(belumLunas));
}

// ============================================================
//  REFRESH: LAPORAN KEUANGAN
// ============================================================
void MainWindow::refreshLaporanKeuangan()
{
    auto& transaksiVec = transaksiList->getAll();
    ui->laporanKeuTable->setRowCount((int)transaksiVec.size());
    for (int i = 0; i < (int)transaksiVec.size(); i++) {
        Transaksi* t = transaksiVec[i];
        ui->laporanKeuTable->setItem(i, 0, new QTableWidgetItem(QString::number(t->transaksiId)));
        ui->laporanKeuTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(t->timestamp)));
        ui->laporanKeuTable->setItem(i, 2, new QTableWidgetItem(QString("Meja %1").arg(t->tableNumber)));
        ui->laporanKeuTable->setItem(i, 3, new QTableWidgetItem(formatPrice(t->subtotal)));
        ui->laporanKeuTable->setItem(i, 4, new QTableWidgetItem(
            t->diskonPersen > 0 ? QString("-%1% (%2)")
                .arg(t->diskonPersen).arg(formatPrice(t->subtotal * t->diskonPersen / 100.0))
            : "-"));
        ui->laporanKeuTable->setItem(i, 5, new QTableWidgetItem(formatPrice(t->totalBayar)));
        ui->laporanKeuTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(t->metodePembayaran)));
    }
}

// ============================================================
//  SLOT: INVENTARIS
// ============================================================
void MainWindow::onTambahStok()
{
    try {
        QString nama = ui->inputInvNama->text().trimmed();
        if (nama.isEmpty())
            throw RestaurantException("Nama bahan tidak boleh kosong!");

        InventarisItem item;
        item.nama      = nama.toStdString();
        item.kategori  = ui->inputInvKategori->currentText().toStdString();
        item.stok      = ui->inputInvStok->value();
        item.satuan    = ui->inputInvSatuan->currentText().toStdString();
        item.minStok   = ui->inputInvMinStok->value();
        item.hargaBeli = ui->inputInvHargaBeli->value();

        InventarisItem* inserted = inventarisList->insert(item);
        actionStack->push(Action("add_inventaris",
            "Tambah bahan: " + item.nama, inserted->id));

        QString logMsg = QString("📦 Bahan ditambah: %1 | %2 %3 | Harga: %4")
            .arg(nama, QString::number(item.stok),
                 QString::fromStdString(item.satuan), formatPrice(item.hargaBeli));
        ui->invLogDisplay->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + logMsg);
        addLog(logMsg);

        ui->inputInvNama->clear();
        ui->inputInvStok->setValue(0);
        ui->inputInvHargaBeli->setValue(0);
        refreshInventarisTable();
        refreshHistoryList();

    } catch (const RestaurantException& e) {
        QMessageBox::warning(this, "Peringatan", QString::fromStdString(e.what()));
    }
}

void MainWindow::onUpdateStok()
{
    int row = ui->inventarisTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Pilih bahan yang stoknya akan diupdate!"); return;
    }
    int id    = ui->inventarisTable->item(row, 0)->text().toInt();
    QString nama = ui->inventarisTable->item(row, 1)->text();
    int delta = ui->inputInvUpdateJumlah->value();

    InventarisItem* item = inventarisList->findById(id);
    if (!item) { QMessageBox::warning(this, "Error", "Bahan tidak ditemukan!"); return; }

    int oldStok = item->stok;
    inventarisList->updateStok(id, delta);

    QString logMsg = QString("🔄 Update stok: %1 | %2 → %3 (%4%5)")
        .arg(nama).arg(oldStok).arg(item->stok)
        .arg(delta >= 0 ? "+" : "").arg(delta);
    ui->invLogDisplay->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + logMsg);
    addLog(logMsg);

    actionStack->push(Action("update_stok", "Update stok: " + nama.toStdString(), id));
    refreshInventarisTable();
    refreshHistoryList();
}

void MainWindow::onHapusStok()
{
    int row = ui->inventarisTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Pilih bahan yang akan dihapus!"); return;
    }
    int id    = ui->inventarisTable->item(row, 0)->text().toInt();
    QString nama = ui->inventarisTable->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "Konfirmasi Hapus",
        QString("Hapus bahan:\n%1?").arg(nama), QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    inventarisList->remove(id);
    actionStack->push(Action("remove_inventaris", "Hapus bahan: " + nama.toStdString(), id));

    QString logMsg = QString("🗑 Bahan dihapus: %1").arg(nama);
    ui->invLogDisplay->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + logMsg);
    addLog(logMsg);
    refreshInventarisTable();
    refreshHistoryList();
}

void MainWindow::onCekStokMinim()
{
    auto menipis = inventarisList->getMenipis();
    if (menipis.empty()) {
        QMessageBox::information(this, "Stok Aman", "✅ Semua stok bahan dalam kondisi aman!");
        addLog("✅ Cek stok: semua bahan aman.");
        return;
    }

    QString msg = QString("⚠ PERINGATAN: %1 bahan stok menipis!\n\n").arg(menipis.size());
    for (auto* inv : menipis)
        msg += QString("• %1: %2 %3 (min: %4)\n")
            .arg(QString::fromStdString(inv->nama))
            .arg(inv->stok).arg(QString::fromStdString(inv->satuan))
            .arg(inv->minStok);

    QMessageBox::warning(this, "Stok Menipis!", msg);
    addLog(QString("⚠ Cek stok: %1 bahan menipis!").arg(menipis.size()));

    // Highlight baris menipis
    auto all = inventarisList->getAll();
    for (int i = 0; i < (int)all.size(); i++) {
        QColor bg = all[i]->menipis() ? QColor(255, 235, 238) : Qt::white;
        for (int c = 0; c < ui->inventarisTable->columnCount(); c++) {
            if (ui->inventarisTable->item(i, c))
                ui->inventarisTable->item(i, c)->setBackground(bg);
        }
    }
}

void MainWindow::onSimpanInventaris()
{
    auto items = inventarisList->getAll();
    if (FileIO::saveInventarisToFile(items)) {
        addLog("💾 Inventaris disimpan ke inventaris.txt");
        QMessageBox::information(this, "Berhasil", "Inventaris berhasil disimpan ke inventaris.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan file!");
    }
}

// ============================================================
//  SLOT: PEMBAYARAN
// ============================================================
void MainWindow::onMuatOrder()
{
    int orderId = ui->inputPembOrderId->value();
    auto& allOrders = orderQueue->getAllOrders();
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), orderId);

    if (it == allOrders.end()) {
        QMessageBox::warning(this, "Tidak Ditemukan",
            QString("Order #%1 tidak ditemukan!").arg(orderId));
        return;
    }

    Order* o = *it;
    if (o->status == "paid") {
        QMessageBox::information(this, "Sudah Lunas",
            QString("Order #%1 sudah dibayar.").arg(orderId));
        return;
    }

    // Tampilkan ringkasan
    QString ringkasan = QString("Order #%1 | Meja %2 | %3\n")
        .arg(o->orderId).arg(o->tableNumber)
        .arg(o->priority == 1 ? "⭐ VIP" : "Normal");
    ringkasan += QString("Status: %1\n\n").arg(QString::fromStdString(o->status));
    ringkasan += "Item:\n";
    for (auto& item : o->items)
        ringkasan += QString("  • %1 x%2 = %3\n")
            .arg(QString::fromStdString(item.menuItemName))
            .arg(item.quantity).arg(formatPrice(item.subtotal));
    ringkasan += QString("\nSubtotal: %1").arg(formatPrice(o->totalPrice));

    ui->pembRingkasanDisplay->setText(ringkasan);
    ui->lblPembSubtotal->setText(QString("Subtotal: %1").arg(formatPrice(o->totalPrice)));

    // Hitung total berdasarkan diskon saat ini
    double diskon = ui->inputPembDiskon->value();
    double total  = o->totalPrice * (1.0 - diskon / 100.0);
    ui->lblPembTotal->setText(QString("Total Bayar: %1").arg(formatPrice(total)));
    ui->inputPembBayar->setValue(total);

    addLog(QString("🔍 Order #%1 dimuat ke form pembayaran.").arg(orderId));
}

void MainWindow::onHitungKembalian()
{
    int orderId = ui->inputPembOrderId->value();
    auto& allOrders = orderQueue->getAllOrders();
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), orderId);
    if (it == allOrders.end()) {
        QMessageBox::warning(this, "Error", "Muat order terlebih dahulu!"); return;
    }

    double subtotal = (*it)->totalPrice;
    double diskon   = ui->inputPembDiskon->value();
    double total    = subtotal * (1.0 - diskon / 100.0);
    double bayar    = ui->inputPembBayar->value();
    double kembalian = bayar - total;

    ui->lblPembTotal->setText(QString("Total Bayar: %1").arg(formatPrice(total)));

    if (kembalian >= 0) {
        ui->lblPembKembalian->setText(QString("<b>Kembalian: <font color='#388E3C'>%1</font></b>")
            .arg(formatPrice(kembalian)));
    } else {
        ui->lblPembKembalian->setText(QString("<b>Kurang: <font color='#D32F2F'>%1</font></b>")
            .arg(formatPrice(-kembalian)));
    }
}

void MainWindow::onProsesPembayaran()
{
    int orderId = ui->inputPembOrderId->value();
    auto& allOrders = orderQueue->getAllOrders();
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), orderId);

    if (it == allOrders.end()) {
        QMessageBox::warning(this, "Error", "Order tidak ditemukan! Muat order terlebih dahulu.");
        return;
    }

    Order* o = *it;
    if (o->status == "paid") {
        QMessageBox::information(this, "Sudah Lunas", "Order ini sudah dibayar!"); return;
    }

    double subtotal = o->totalPrice;
    double diskon   = ui->inputPembDiskon->value();
    double total    = subtotal * (1.0 - diskon / 100.0);
    double bayar    = ui->inputPembBayar->value();
    QString metode  = ui->inputPembMetode->currentText();

    bool isTunai = metode.contains("Tunai");
    if (isTunai && bayar < total) {
        QMessageBox::warning(this, "Kurang Bayar",
            QString("Uang yang diberikan (%1) kurang dari total (%2)!")
                .arg(formatPrice(bayar), formatPrice(total)));
        return;
    }

    // Buat transaksi
    Transaksi* t = transaksiList->tambah(
        orderId, o->tableNumber, subtotal, diskon,
        bayar, metode.toStdString(), getCurrentTimestamp());

    // Update status order
    o->status = "paid";
    tableList->freeTable(o->tableNumber);

    actionStack->push(Action("payment",
        "Pembayaran order #" + std::to_string(orderId) + " via " + metode.toStdString(),
        orderId));

    addLog(QString("💰 Order #%1 LUNAS | %2 | Diskon: %3% | Total: %4 | Kembalian: %5")
        .arg(orderId).arg(metode)
        .arg(diskon).arg(formatPrice(total)).arg(formatPrice(t->kembalian)));

    // Reset form
    ui->pembRingkasanDisplay->clear();
    ui->lblPembSubtotal->setText("Subtotal: Rp 0");
    ui->lblPembTotal->setText("Total Bayar: Rp 0");
    ui->lblPembKembalian->setText("Kembalian: Rp 0");
    ui->inputPembDiskon->setValue(0);
    ui->inputPembBayar->setValue(0);

    refreshTransaksiTable();
    refreshOrderTable();
    refreshTableDisplay();
    refreshDashboard();
    refreshHistoryList();

    QMessageBox::information(this, "Pembayaran Berhasil!",
        QString("✅ Order #%1 berhasil dibayar!\n\nTotal: %2\nKembalian: %3")
            .arg(orderId).arg(formatPrice(total)).arg(formatPrice(t->kembalian)));
}

void MainWindow::onCetakStruk()
{
    int orderId = ui->inputPembOrderId->value();
    Transaksi* t = transaksiList->findByOrderId(orderId);

    if (!t) {
        // Coba cari dari order yang belum bayar
        auto& allOrders = orderQueue->getAllOrders();
        auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), orderId);
        if (it == allOrders.end()) {
            QMessageBox::warning(this, "Error", "Order tidak ditemukan!"); return;
        }
        Order* o = *it;
        // Preview struk sebelum bayar
        QString struk;
        struk += "============================\n";
        struk += "   RESTORAN KAMI\n";
        struk += "============================\n";
        struk += QString("Order #%1 | Meja %2\n").arg(o->orderId).arg(o->tableNumber);
        struk += QString("Status: %1\n").arg(QString::fromStdString(o->status));
        struk += "----------------------------\n";
        for (auto& item : o->items)
            struk += QString("%-16s x%2\n   = %3\n")
                .arg(QString::fromStdString(item.menuItemName)).arg(item.quantity)
                .arg(formatPrice(item.subtotal));
        struk += "----------------------------\n";
        struk += QString("TOTAL : %1\n").arg(formatPrice(o->totalPrice));
        struk += "============================\n";
        struk += "   Belum Lunas\n";
        struk += "============================\n";
        ui->strukDisplay->setText(struk);
        return;
    }

    // Struk lunas
    auto& allOrders = orderQueue->getAllOrders();
    auto it = STLUtils::findOrderById(const_cast<std::list<Order*>&>(allOrders), orderId);
    Order* o = (it != allOrders.end()) ? *it : nullptr;

    QString struk;
    struk += "============================\n";
    struk += "   RESTORAN KAMI\n";
    struk += "============================\n";
    struk += QString("Tanggal: %1\n").arg(QString::fromStdString(t->timestamp));
    struk += QString("No. Transaksi: %1\n").arg(t->transaksiId);
    struk += QString("Order #%1 | Meja %2\n").arg(t->orderId).arg(t->tableNumber);
    struk += "----------------------------\n";
    if (o) {
        for (auto& item : o->items)
            struk += QString("%-16s x%2\n   %3\n")
                .arg(QString::fromStdString(item.menuItemName)).arg(item.quantity)
                .arg(formatPrice(item.subtotal));
    }
    struk += "----------------------------\n";
    struk += QString("Subtotal : %1\n").arg(formatPrice(t->subtotal));
    if (t->diskonPersen > 0)
        struk += QString("Diskon   : -%1%  (%2)\n")
            .arg(t->diskonPersen)
            .arg(formatPrice(t->subtotal * t->diskonPersen / 100.0));
    struk += QString("TOTAL    : %1\n").arg(formatPrice(t->totalBayar));
    struk += QString("Bayar    : %1\n").arg(formatPrice(t->jumlahBayar));
    struk += QString("Kembali  : %1\n").arg(formatPrice(t->kembalian));
    struk += QString("Metode   : %1\n").arg(QString::fromStdString(t->metodePembayaran));
    struk += "============================\n";
    struk += "   Terima Kasih!\n";
    struk += "============================\n";

    ui->strukDisplay->setText(struk);
    addLog(QString("🧾 Struk Order #%1 dicetak.").arg(orderId));
}

// ============================================================
//  SLOT: LAPORAN KEUANGAN
// ============================================================
void MainWindow::onGenerateLapKeu()
{
    refreshLaporanKeuangan();

    double totalPend  = transaksiList->getTotalPendapatan();
    double totalDisk  = transaksiList->getTotalDiskon();
    int    jmlTranx   = transaksiList->getSize();
    double rataRata   = transaksiList->getRataRata();
    double terbesar   = transaksiList->getTerbesar();

    ui->lblKeuTotalPendapatan->setText(QString("<b>%1</b>").arg(formatPrice(totalPend)));
    ui->lblKeuTotalDiskon->setText(formatPrice(totalDisk));
    ui->lblKeuJmlTranx->setText(QString::number(jmlTranx));
    ui->lblKeuRataRata->setText(formatPrice(rataRata));
    ui->lblKeuTerbesar->setText(formatPrice(terbesar));

    // Analisis per metode pembayaran
    std::unordered_map<std::string, int> metodeCnt;
    std::unordered_map<std::string, double> metodeTotal;
    for (auto* t : transaksiList->getAll()) {
        metodeCnt[t->metodePembayaran]++;
        metodeTotal[t->metodePembayaran] += t->totalBayar;
    }

    QString analisis;
    analisis += "=== RINGKASAN LAPORAN KEUANGAN ===\n\n";
    analisis += QString("Total Pendapatan : %1\n").arg(formatPrice(totalPend));
    analisis += QString("Total Diskon     : %1\n").arg(formatPrice(totalDisk));
    analisis += QString("Jumlah Transaksi : %1\n").arg(jmlTranx);
    analisis += QString("Rata-rata/Tranx  : %1\n").arg(formatPrice(rataRata));
    analisis += QString("Transaksi Terbesar: %1\n\n").arg(formatPrice(terbesar));

    analisis += "=== BREAKDOWN PER METODE ===\n";
    for (auto& pair : metodeCnt) {
        analisis += QString("• %1: %2 transaksi = %3\n")
            .arg(QString::fromStdString(pair.first))
            .arg(pair.second)
            .arg(formatPrice(metodeTotal[pair.first]));
    }

    // Distribusi order status
    auto summary = STLUtils::getOrderStatusSummary(orderQueue->getAllOrders());
    analisis += "\n=== STATUS ORDER ===\n";
    for (auto& s : std::vector<std::string>{"pending","preparing","served","paid"})
        analisis += QString("• %1: %2\n")
            .arg(QString::fromStdString(s)).arg(summary[s]);

    // Stok menipis warning
    auto menipis = inventarisList->getMenipis();
    if (!menipis.empty()) {
        analisis += "\n⚠ PERHATIAN - Stok Menipis:\n";
        for (auto* inv : menipis)
            analisis += QString("• %1: %2 %3\n")
                .arg(QString::fromStdString(inv->nama))
                .arg(inv->stok).arg(QString::fromStdString(inv->satuan));
    }

    analisis += "\nLaporan di-generate: " +
        QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");

    ui->keuAnalysisDisplay->setText(analisis);
    addLog(QString("📈 Laporan keuangan di-generate. Total pendapatan: %1, %2 transaksi.")
        .arg(formatPrice(totalPend)).arg(jmlTranx));
}

void MainWindow::onSimpanLapKeu()
{
    auto& transaksiVec = transaksiList->getAll();
    if (FileIO::saveLaporanKeuanganToFile(
            transaksiVec,
            transaksiList->getTotalPendapatan(),
            transaksiList->getTotalDiskon())) {
        addLog("💾 Laporan keuangan disimpan ke laporan_keuangan.txt");
        QMessageBox::information(this, "Berhasil",
            "Laporan keuangan berhasil disimpan ke laporan_keuangan.txt!");
    } else {
        QMessageBox::critical(this, "Error", "Gagal menyimpan laporan keuangan!");
    }
}

// ============================================================
//  SETUP CREDIT TAB
// ============================================================
void MainWindow::setupCreditTab()
{
    // Semua tampilan diambil dari .ui (emoji, nama, NIM, peran, styling)
    // Scroll area transparan (tidak bisa diset dari .ui)
    ui->creditScrollArea->setStyleSheet(
        "QScrollArea{background:transparent;border:none;}"
        "QWidget#creditScrollContent{background:transparent;}");
}
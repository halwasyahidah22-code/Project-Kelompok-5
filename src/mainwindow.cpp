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

    currentOrderItems.clear();

    // Setup UI dari file .ui (ganti semua setupXxxTab())
    ui->setupUi(this);

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

    // ── Tabel Header ─────────────────────────────────────────
    ui->menuTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pendingOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableDisplay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->staffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

        QLabel { background-color: transparent; color: #212121; }

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
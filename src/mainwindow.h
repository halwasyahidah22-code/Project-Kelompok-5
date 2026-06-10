#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QHeaderView>
#include <QColor>

#include "datastructures.h"

// Forward-declare kelas Ui yang di-generate oleh uic dari mainwindow.ui
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace RestaurantSystem;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    // ---- Generated UI (dari mainwindow.ui) ----
    Ui::MainWindow* ui;

    // ---- Data Structures ----
    MenuLinkedList*    menuList;
    TableCircularList* tableList;
    StaffCircularList* staffList;
    OrderQueue*        orderQueue;
    AVLTree*           menuAVL;
    MenuHashTable*     menuHash;
    TableGraph*        tableGraph;
    ActionStack*       actionStack;

    int nextMenuId;
    int nextStaffId;

    // Current order being built
    Order*                 currentOrder;
    std::vector<OrderItem> currentOrderItems;
    double                 currentOrderTotal;

    // ---- Initialization ----
    void setupConnections();   // Semua signal-slot connect di sini (ganti setupUI)
    void setupInitialData();
    void applyStyleSheet();
    void setupCreditTab();

    // ---- Refresh / Display ----
    void refreshMenuTable();
    void refreshOrderTable();
    void refreshPendingOrders();
    void refreshTableDisplay();
    void refreshStaffTable();
    void refreshHistoryList();
    void refreshDashboard();
    void updateGraphDisplay();

    // ---- Helpers ----
    void    addLog(const QString& msg);
    QString formatPrice(double price);
    std::string getCurrentTimestamp();

    // ---- Callback helpers (syarat 5) ----
    void logMenusViaCallback(const std::string& category);
    void logOrdersViaCallback(const std::string& status);

    // ---- Function overloading display (syarat 7) ----
    QString formatDisplayItem(const MenuItem& m);
    QString formatDisplayItem(const Order& o);
    QString formatDisplayItem(const Staff& s);

    // ---- STL helpers (syarat 9) ----
    void refreshStatisticsPanel();

private slots:
    // Menu
    void onAddMenuItem();
    void onRemoveMenuItem();
    void onSearchMenu();
    void onSortMenu();
    void onSaveMenu();
    void onLoadMenu();
    void onFilterByCategory();

    // Order
    void onCreateOrder();
    void onAddItemToOrder();
    void onRemoveItemFromOrder();
    void onSubmitOrder();
    void onProcessNextOrder();
    void onUpdateOrderStatus();

    // Table
    void onNextTable();
    void onOccupyTable();
    void onFreeTable();
    void onRunBFS();
    void onRunDFS();

    // Staff
    void onAddStaff();
    void onRemoveStaff();
    void onRotateShift();

    // History & Report
    void onUndoAction();
    void onGenerateReport();
    void onSortReport();
    void onSaveReport();
    void onRunSTLDemo();

    // Timer
    void updateClock();
};

#endif // MAINWINDOW_H

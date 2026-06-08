#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QListWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QCheckBox>
#include <QFont>

#include "datastructures.h"

using namespace RestaurantSystem;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
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

    // ---- UI Components ----
    QTabWidget* tabWidget;

    // Tab: Dashboard
    QWidget*    dashboardTab;
    QLabel*     lblTotalMenu;
    QLabel*     lblTotalOrders;
    QLabel*     lblPendingOrders;
    QLabel*     lblActiveTables;
    QLabel*     lblCurrentTime;
    QTextEdit*  logDisplay;

    // Tab: Menu Management
    QWidget*     menuTab;
    QTableWidget* menuTable;
    QLineEdit*   inputMenuName;
    QComboBox*   inputMenuCategory;
    QDoubleSpinBox* inputMenuPrice;
    QCheckBox*   inputMenuAvailable;
    QLineEdit*   searchMenuInput;
    QComboBox*   sortMenuCombo;
    QComboBox*   filterCategoryCombo;  // untuk onFilterByCategory (callback syarat 5)

    // Tab: Order Management
    QWidget*     orderTab;
    QTableWidget* orderTable;
    QComboBox*   orderTableSelect;
    QListWidget* orderItemList;
    QComboBox*   orderMenuSelect;
    QSpinBox*    orderQtyInput;
    QComboBox*   orderPrioritySelect;
    QLabel*      lblOrderTotal;
    QTableWidget* pendingOrdersTable;

    // Tab: Table Management
    QWidget*     tableTab;
    QTableWidget* tableDisplay;
    QLabel*      lblCurrentTable;
    QTextEdit*   graphDisplay;

    // Tab: Staff & Shift
    QWidget*     staffTab;
    QTableWidget* staffTable;
    QLineEdit*   inputStaffName;
    QComboBox*   inputStaffRole;
    QLabel*      lblOnDuty;

    // Tab: History & Reports
    QWidget*     historyTab;
    QListWidget* historyList;
    QTableWidget* reportTable;
    QComboBox*   reportSortCombo;

    // ---- Helper Methods ----
    void setupUI();
    void setupDashboard();
    void setupMenuTab();
    void setupOrderTab();
    void setupTableTab();
    void setupStaffTab();
    void setupHistoryTab();
    void setupInitialData();
    void applyStyleSheet();

    void refreshMenuTable();
    void refreshOrderTable();
    void refreshTableDisplay();
    void refreshStaffTable();
    void refreshHistoryList();
    void refreshDashboard();
    void refreshPendingOrders();
    void updateGraphDisplay();

    void addLog(const QString& msg);
    QString formatPrice(double price);
    std::string getCurrentTimestamp();

    // ---- Callback helpers (syarat 5) ----
    // Dipanggil di berbagai slot untuk logging & statistik via callback
    void logMenusViaCallback(const std::string& category);
    void logOrdersViaCallback(const std::string& status);

    // ---- Function overloading display (syarat 7) ----
    // Nama sama, parameter berbeda → overloading
    QString formatDisplayItem(const MenuItem& m);
    QString formatDisplayItem(const Order& o);
    QString formatDisplayItem(const Staff& s);

    // ---- STL find & count helpers (syarat 9) ----
    void refreshStatisticsPanel();  // pakai STLUtils::count* untuk update stat cards

    // Current order being built
    Order* currentOrder;
    std::vector<OrderItem> currentOrderItems;
    double currentOrderTotal;

    // UI tambahan untuk panel statistik STL
    QLabel* lblAvailableMenus;
    QLabel* lblMenuByCategory;
    QTextEdit* statsDisplay;

private slots:
    void onAddMenuItem();
    void onRemoveMenuItem();
    void onSearchMenu();
    void onSortMenu();
    void onSaveMenu();
    void onLoadMenu();

    void onCreateOrder();
    void onAddItemToOrder();
    void onRemoveItemFromOrder();
    void onSubmitOrder();
    void onProcessNextOrder();
    void onUpdateOrderStatus();

    void onNextTable();
    void onOccupyTable();
    void onFreeTable();
    void onRunBFS();
    void onRunDFS();

    void onAddStaff();
    void onRemoveStaff();
    void onRotateShift();

    void onUndoAction();
    void onGenerateReport();
    void onSortReport();
    void onSaveReport();
    void onRunSTLDemo();         // Demo STL find + count dari Tab Riwayat
    void onFilterByCategory();  // Demo callback filterMenuByCategory dari Tab Menu

    void updateClock();
};

#endif // MAINWINDOW_H

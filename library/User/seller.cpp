#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <vector>
#include <map>

#include "./seller.h"
#include "../Bank/bank_customer.h"
#include "../Item/item.h"
#include "../Item/order.h"
#include "../Item/analytics.h"


extern vector<Order> orders; 
extern void loadOrders(std::vector<Order>&);
using namespace std;

Seller::Seller(const string& name, const string& password, const string& storeName)
    : User(name, password), storeName(storeName) {
        int newId = 1000 + (rand() % 9000);
        this->account = make_shared<BankCustomer>(newId, name, 0.0);
    }

Seller::Seller(const string& name, const string& password, const string& storeName, 
           shared_ptr<BankCustomer> existingAccount)
    : User(name, password, existingAccount), storeName(storeName) {}

string Seller::userToCSV() const {
    return getName() + "," + password + "," + getRole() + "," + storeName;
}

string Seller::inventoryToCSV() const {
    stringstream ss;
    for (const auto& item : items) {
        ss << storeName << "," << item.toCSV() << "\n";
    }
    return ss.str();
}

// Mengubah timepoint menjadi string
string formatTimePoint(chrono::system_clock::time_point tp) {
    time_t tt = chrono::system_clock::to_time_t(tp);
    tm tm = *localtime(&tt);
    
    stringstream ss;
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string Seller::getRole() const {
    return "Seller";
}

void Seller::showAccountInfo() const {
    cout << "\n-- Seller Information --\n";
    cout << "Store Name: " << storeName << "\n";
    cout << "Owner: " << getName() << "\n";
    
    if (account) {
        cout << "Bank ID: " << account->getId() << "\n";
        cout << "Balance: Rp" << fixed << setprecision(2) << getBalance() << "\n\n"; 
    } else {
        cout << "Bank account not linked.\n\n";
    }
}

double Seller::getBalance() const { 
    if (account) {
        return account->getBalance(); 
    }
    return 0.0;
}

void Seller::addItem(int id, const string& name, int qty, double price) {
    items.emplace_back(id, name, qty, price);
    cout << "Item " << name << " added to inventory.\n\n";
}

void Seller::showInventory() const {
    cout << "\n-- Store Inventory: --\n";
    if (items.empty()) {
        cout << "Inventory is empty.\n\n";
        return;
    }

    for (const auto& item : items) {
        cout << "ID: " << item.getId()
             << ", Name: " << item.getName()
             << ", Stock: " << item.getQuantity()
             << fixed << setprecision(0)
             << ", Price: Rp" << fixed << setprecision(0) << item.getPrice() << "\n\n";
    }
}

void Seller::removeItem(int id) {
    auto it = remove_if(items.begin(), items.end(), 
                             [id](const Item& item) {
                                 return item.getId() == id;
                             });
                             
    if (it != items.end()) {
        items.erase(it, items.end());
        cout << "Item with ID " << id << " removed.\n\n";
    } else {
        cout << "Item with ID " << id << " not found.\n\n";
    }
}

void Seller::viewMostFrequentItems(int mItems) const {
    map<int, int> itemFrequency;
    map<int, string> itemIdToName;

    for (const auto& order : orders) {
        if (order.getSellerStoreName() == this->storeName && order.getStatus() == "DONE") {
            for (const auto& item : order.getItems()) {
                itemFrequency[item.getId()] += item.getQuantity();
                itemIdToName[item.getId()] = item.getName();
            }
        }
    }

    if (itemFrequency.empty()) {
        cout << "\n-- TOP " << mItems << " MOST FREQUENT ITEMS SOLD by " << this->storeName << " --\n";
        cout << "No completed transactions found for the store.\n\n";
        return;
    }

    vector<pair<int,int>> itemsVec;
    itemsVec.reserve(itemFrequency.size());
    for (const auto& p : itemFrequency) itemsVec.emplace_back(p.first, p.second);

    sort(itemsVec.begin(), itemsVec.end(), [](const auto& a, const auto& b){
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;                             
    });

    int limit = (mItems > 0) ? min(static_cast<size_t>(mItems), itemsVec.size()) : itemsVec.size();
    cout << "\n-- TOP " << limit << " MOST FREQUENT ITEMS SOLD by " << this->storeName << " --\n";

    for (int i = 0; i < limit; ++i) {
        int itemId = itemsVec[i].first;
        int quantity = itemsVec[i].second;
        const string& itemName = itemIdToName[itemId];

        cout << left << setw(3) << (i + 1) << ". "
                << setw(30) << itemName
                << " (ID: " << setw(5) << itemId << ")"
                << " | Quantity Sold: " << fixed << setprecision(0) << quantity << "\n";
    }
    cout << "\n";
}

void Seller::viewOrders() const {
    cout << "-- Paid Status Order --" << " ==\n";
    bool foundPaidOrder = false;

    for(const auto& order : orders) {
        if(order.getSellerStoreName() == this-> storeName) {
            if (order.getStatus() == "DONE") {
                foundPaidOrder = true;
                
                cout << "Order ID: " << order.getOrderId() << "\n";
                cout << "Buyer Detail: " << order.getBuyerName() << "\n";
                cout << "Order Status: " << order.getStatus() << " (Paid)\n";
                cout << "Total transaction: Rp" << fixed << setprecision(2) << order.getTotalAmount() << "\n";
                
                cout << "-- List Items --\n";
                for (const auto& item : order.getItems()) { 
                    cout << "  - Product: " << item.getName() 
                         << " (ID: " << item.getId() << ")"
                         << "\n    Quantity: " << item.getQuantity() 
                         << " x Rp" << fixed << setprecision(0) << item.getPrice() 
                         << " = Subtotal: Rp" << fixed << setprecision(2) << item.getQuantity() * item.getPrice() << "\n\n";
                }
            }
        }
    }
    if (!foundPaidOrder) {
        cout << "There are currently no new orders with the status 'Paid'.\n\n";
    }
}

void Seller::handlePopularItemsReport() {
    loadOrders(orders);
    const vector<Order>& allOrders = orders;

    map<string, map<string, int>> monthlyItemSales;

    for (const auto& order : allOrders) {
        if (order.getStatus() == "DONE" && order.getSellerStoreName() == this->storeName) {
            string yearMonth = order.getYearMonthString();
            for (const auto& item : order.getItems()) {
                monthlyItemSales[yearMonth][item.getName()] += item.getQuantity();
            }
        }
    }

    if (monthlyItemSales.empty()) {
        cout << "\nNo sales data found for store " << this->storeName << ".\n\n";
        return;
    }

    int k;
    cout << "\n[POPULAR ITEMS REPORT - " << this->storeName << "] Enter the Top K item limit per month (e.g., 5): ";
    if (!(cin >> k) || k <= 0) {
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Using default K=5.\n";
        k = 5;
    }

    cout << "\n=== TOP " << k << " POPULAR ITEMS PER MONTH (" << this->storeName << ") ===\n";
    for (const auto& monthPair : monthlyItemSales) {
        const string& month = monthPair.first;
        const auto& itemSales = monthPair.second;

        vector<pair<string, int>> sortedItems;
        for (const auto& salesPair : itemSales) {
            sortedItems.push_back({salesPair.first, salesPair.second});
        }

        sort(sortedItems.begin(), sortedItems.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second > b.second;
            });

        cout << "\n--- Month: " << month << " ---\n";
        cout << setw(5) << left << "Rank" << setw(30) << "Item Name" << setw(15) << "Total Quantity" << "\n";
        cout << "------------------------------------------------\n";

        for (int i = 0; i < min(k, static_cast<int>(sortedItems.size())); ++i) {
            cout << setw(5) << left << (i + 1)
                 << setw(30) << sortedItems[i].first
                 << setw(15) << sortedItems[i].second << "\n";
        }
    }
    cout << "\n\n";
}

void Seller::handleLoyalCustomerReport() {
    loadOrders(orders);
    const vector<Order>& allOrders = orders;

    map<string, map<string, int>> monthlyCustomerLoyalty;

    for (const auto& order : allOrders) {
        if (order.getStatus() == "DONE" && order.getSellerStoreName() == this->storeName) {
            string yearMonth = order.getYearMonthString();
            monthlyCustomerLoyalty[yearMonth][order.getBuyerName()]++;
        }
    }

    if (monthlyCustomerLoyalty.empty()) {
        cout << "\n[LOYAL CUSTOMER REPORT] No 'DONE' sales data found for store " << this->storeName << ".\n\n";
        return;
    }

    cout << "\n=== MOST LOYAL CUSTOMERS PER MONTH (" << this->storeName << ") ===\n";
    cout << "(Based on number of DONE orders)\n";

    for (const auto& monthPair : monthlyCustomerLoyalty) {
        const string& month = monthPair.first;
        const auto& customerData = monthPair.second;

        string topCustomerName = "N/A";
        int maxOrders = 0;
        vector<string> loyalCustomers;

        for (const auto& customerPair : customerData) {
            if (customerPair.second > maxOrders) {
                maxOrders = customerPair.second;
                loyalCustomers.clear();
                loyalCustomers.push_back(customerPair.first);
            } else if (customerPair.second == maxOrders && maxOrders > 0) {
                loyalCustomers.push_back(customerPair.first);
            }
        }

        stringstream loyalList;
        for (size_t i = 0; i < loyalCustomers.size(); ++i) {
            loyalList << loyalCustomers[i];
            if (i < loyalCustomers.size() - 1) {
                loyalList << ", ";
            }
        }
        topCustomerName = loyalCustomers.empty() ? "N/A" : loyalList.str();

        cout << "\n--- Month: " << month << " ---\n";
        cout << setw(30) << left << "Most Loyal Customer:" 
             << topCustomerName << "\n";
        cout << setw(30) << left << "Number of DONE Orders:" 
             << maxOrders << "\n";
    }
    cout << "\n\n";
}


void Seller::handleStoreCapabilitiesMenu() {
    int choice;
    bool exitMenu = false;
    
    do {
        cout << "--- STORE CAPABILITIES MENU ---\n";
        cout << "1. View Paid Orders\n";
        cout << "2. View Most Frequent Completed Items\n";
        cout << "3. View Recent Transactions\n";
        cout << "4. List All Most Active Buyers\n";
        cout << "5. List All Most Active Sellers\n";
        cout << "6. Discover Top K Most Popular Items \n";
        cout << "7. Discover Loyal Customer\n";
        cout << "8. BACK to Main Menu\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid input. Returning to Main Menu.\n\n";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return; 
        }

        switch (choice) {
            case 1: {
                this->viewOrders();
                break;
            }
            case 2: {
                int mItems;
                cout << "Enter the number of top items to display : ";
                if (!(cin >> mItems)) {
                    cout << "Invalid input. Canceled.\n\n";
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                this->viewMostFrequentItems(mItems);
                break;
            }
            case 3: {
                int nDays;
                cout << "Enter the number of past days to review : ";
                if (!(cin >> nDays)) {
                    cout << "Invalid input. Canceled.\n\n";
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                // Call global analytics function
                showRecentTransactions(orders, nDays); 
                break;
            }
            case 4: {
                int nBuyers;
                cout << "Enter the number of top buyers to display : ";
                if (!(cin >> nBuyers)) {
                    cout << "Invalid input. Canceled.\n\n";
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                // Call global analytics function
                viewMostActiveBuyersPerDay(orders, nBuyers, 10); 
                break;
            }
            case 5:
                int nSellers;
                std::cout << "Enter the number of top sellers to display : ";
                if (!(std::cin >> nSellers)) {
                    std::cout << "Invalid input. Canceled.\n\n";
                    std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                viewMostActiveSellersPerDay(orders, nSellers, 10);
                break;
            case 6:
                handlePopularItemsReport();
                break;
            case 7:
                handleLoyalCustomerReport();
                break;
            case 8:
                cout << "Back to Main Menu. \n\n";
                return;
            default:
                cout << "Invalid choice. Please try again.\n\n";
                break;
        }
    } while (!exitMenu);
}
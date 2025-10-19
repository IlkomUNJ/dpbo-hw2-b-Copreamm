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

void Seller::handleStoreCapabilitiesMenu() {
    int choice;
    bool exitMenu = false;
    
    do {
        cout << "--- STORE CAPABILITIES MENU ---\n";
        cout << "1. View Paid Orders\n";
        cout << "2. View Most Frequent Completed Items\n";
        cout << "3. View Recent Transactions\n";
        cout << "4. List All Most Active Buyers\n";
        cout << "5. List All Most Active Sellers\n"; // Opsi Baru
        cout << "6. BACK to Main Menu\n";
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
                cout << "Back to Main Menu. \n\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n\n";
                break;
        }
    } while (!exitMenu);
}
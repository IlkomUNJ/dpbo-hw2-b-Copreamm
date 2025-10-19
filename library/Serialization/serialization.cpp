#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>

#include "serialization.h"
#include "../User/user.h"     
#include "../User/buyer.h"    
#include "../User/seller.h"   
#include "../Bank/bank_customer.h"
#include "../Item/item.h"
#include "../Item/order.h"
#include "../User/admin.h"
#include "../Bank/bank.h"

using namespace std;

// Path Folder CSV
const string DATA_FOLDER = "data/";
const string BANK_FILE = DATA_FOLDER + "bank_accounts.csv";
const string USERS_FILE = DATA_FOLDER + "users.csv";
const string INVENTORY_FILE = DATA_FOLDER + "inventory.csv";
const string ORDERS_FILE = DATA_FOLDER + "orders.csv";

extern Bank systemBank;

// Fungsi Helper CSV
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Deklarasi Fungsi Save
void saveBankAccounts(const vector<shared_ptr<User>>& users);
void saveUsers(const vector<shared_ptr<User>>& users);
void saveInventory(const vector<shared_ptr<User>>& users);
void saveOrders(const vector<Order>& orders);

// Fungsi Utama Save
void saveAllData(const vector<shared_ptr<User>>& users, 
                 const vector<Order>& orders) {
    saveBankAccounts(users);
    saveUsers(users);
    saveInventory(users);
    saveOrders(orders);
    cout << "All data was successfully saved to CSV file.\n\n";
}

// Menyimpan Account Bank
void saveBankAccounts(const vector<shared_ptr<User>>& users) {
    ofstream ofs(BANK_FILE);
    if (!ofs.is_open()) { return; }
    
    for (const auto& user : users) {
        if (auto account = user->getAccount()) {
            ofs << account->toCSV() << "\n";
        }
    }
    ofs.close();
}

// Menyimpan User
void saveUsers(const vector<shared_ptr<User>>& users) {
    ofstream ofs(USERS_FILE);
    if (!ofs.is_open()) { return; }
    
    for (const auto& user : users) {
        ofs << user->userToCSV() << "\n"; 
    }
    ofs.close();
}

// Menyimpan Inventory
void saveInventory(const vector<shared_ptr<User>>& users) {
    ofstream ofs(INVENTORY_FILE);
    if (!ofs.is_open()) { return; }
    
    for (const auto& user : users) {
        if (user->getRole() == "Seller") {
            if (auto seller = dynamic_pointer_cast<Seller>(user)) {
                ofs << seller->inventoryToCSV(); 
            }
        }
    }
    ofs.close();
}

// Menyimpan Order
void saveOrders(const vector<Order>& orders) {
    ofstream ofs(ORDERS_FILE);
    if (!ofs.is_open()) { return; }
    
    for (const auto& order : orders) {
        ofs << order.toCSV() << "\n";
    }
    ofs.close();
}

// Deklarasi Fungsi Load Internal
map<string, shared_ptr<BankCustomer>> loadBankAccounts();
void loadUsers(vector<shared_ptr<User>>& users, 
               const map<string, shared_ptr<BankCustomer>>& bankMap);
void loadInventory(vector<shared_ptr<User>>& users);
void loadOrders(vector<Order>& orders);


// Fungsi Load Utama
void loadAllData(vector<shared_ptr<User>>& users, 
                 vector<Order>& orders) {
    
    users.clear();
    orders.clear();

    auto bankMap = loadBankAccounts(); 

    loadUsers(users, bankMap);

    loadInventory(users);

    loadOrders(orders);
    
    cout << "All data was loaded successfully.\n\n";
}

// Memuat Bank Account
map<string, shared_ptr<BankCustomer>> loadBankAccounts() {
    map<string, shared_ptr<BankCustomer>> bankMap;
    ifstream ifs(BANK_FILE);
    if (!ifs.is_open()) { cout << "bank_accounts.csv not found/empty.\n"; return bankMap; }

    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        auto tokens = split(line, ',');
        auto account = BankCustomer::fromCSV(tokens); 
        if (account) {
            bankMap[account->getName()] = account; // Map kunci: Nama Pemilik
        }
    }
    ifs.close();
    return bankMap;
}

// Memuat User
void loadUsers(vector<shared_ptr<User>>& users, 
               const map<string, shared_ptr<BankCustomer>>& bankMap) {
    ifstream ifs(USERS_FILE);
    if (!ifs.is_open()) { cout << "users.csv not found/empty.\n"; return; }

    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        auto tokens = split(line, ',');
        if (tokens.size() < 3) continue; 

        string name = tokens[0];
        string password = tokens[1];
        string role = tokens[2];
        string storeName = (tokens.size() > 3) ? tokens[3] : "";

        shared_ptr<User> newUser = nullptr;
        shared_ptr<BankCustomer> account = nullptr;
        if (bankMap.count(name)) {
            account = bankMap.at(name);
        }

        if (role == "Buyer") {
            newUser = make_shared<Buyer>(name, password, account);
        } else if (role == "Seller") {
            newUser = make_shared<Seller>(name, password, storeName, account); 
        } else if (role == "Admin") {
            newUser = make_shared<Admin>(name, password);
        }
        
        if(newUser) {
            users.push_back(newUser);

            if (account) {
                systemBank.registerCustomer(account); 
            }
        }
    }
    ifs.close();
}

// Memuat Inventory
void loadInventory(vector<shared_ptr<User>>& users) {
    ifstream ifs(INVENTORY_FILE);
    if (!ifs.is_open()) { cout << "inventory.csv not found/empty.\n"; return; }

    map<string, shared_ptr<Seller>> sellerMap;
    for (const auto& user : users) {
        if (auto seller = dynamic_pointer_cast<Seller>(user)) {
            sellerMap[seller->getStoreName()] = seller;
        }
    }
    
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        auto tokens = split(line, ',');
        if (tokens.size() < 5) continue; 

        string storeName = tokens[0];
        
        vector<string> itemTokens(tokens.begin() + 1, tokens.begin() + 5);
        auto item = Item::fromCSV(itemTokens); 

        if (item && sellerMap.count(storeName)) {
            sellerMap.at(storeName)->addItemObject(*item);
        }
    }
    ifs.close();
}

// Orders
void loadOrders(std::vector<Order>& orders) {
    orders.clear();

    std::ifstream file(ORDERS_FILE); 

    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << ORDERS_FILE << " for reading. Orders list is empty.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> tokens = split(line, ',');
        
        if (!tokens.empty()) {
            orders.push_back(Order::fromCSV(tokens)); 
        }
    }
    file.close();
}
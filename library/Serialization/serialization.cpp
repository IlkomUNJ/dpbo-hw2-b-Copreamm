#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>

#if !defined(_WIN32) && !defined(__APPLE__)
    #define _XOPEN_SOURCE
#endif

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

string timePointToISOString(const chrono::system_clock::time_point& tp) {
    time_t timeT = chrono::system_clock::to_time_t(tp);
    tm* gmtm = gmtime(&timeT); 
    
    if (gmtm == nullptr) return "UnknownTime";

    stringstream ss;
    ss << put_time(gmtm, "%Y-%m-%d %H:%M:%S"); 
    return ss.str();
}

chrono::system_clock::time_point parseISOString(const string& isoString) {
    tm t{}; 
    istringstream ss(isoString);
    if (!(ss >> get_time(&t, "%Y-%m-%d %H:%M:%S"))) {
        return chrono::system_clock::time_point{}; 
    }
    
    time_t tt = static_cast<time_t>(-1);

    #if defined(_WIN32)
        t.tm_isdst = 0; 
        time_t local_time = mktime(&t);

        if (local_time != static_cast<time_t>(-1)) {
             extern long _timezone;
             tt = local_time - _timezone;
        } else {
             tt = static_cast<time_t>(-1);
        }
    #endif

    if (tt == static_cast<time_t>(-1)) { 
        return chrono::system_clock::time_point{};
    }

    return chrono::system_clock::from_time_t(tt);
}

string BankTransaction::toCSV() const {
    stringstream ss;
    ss << accountId << ",";
    ss << timePointToISOString(timestamp) << ",";
    ss << type << ",";
    ss << fixed << setprecision(2) << amount << ",";
    
    string safeDescription = description;
    replace(safeDescription.begin(), safeDescription.end(), ',', ';');
    ss << safeDescription;
    
    return ss.str();
}

BankTransaction BankTransaction::fromCSV(const vector<string>& tokens) {
    BankTransaction t;
    if (tokens.size() < 5) return t;
    
    try {
        t.accountId = stoi(tokens[0]);
        t.timestamp = parseISOString(tokens[1]);
        t.type = tokens[2];
        t.amount = stod(tokens[3]);
        t.description = tokens[4];
    } catch (...) {
    }
    return t;
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

void saveTransaction(const BankTransaction& t, const string& filename) {
    ofstream file(filename, ios::app); 
    if (file.is_open()) {
        file << t.toCSV() << "\n"; 
        file.close();
    } else {
        cerr << "ERROR: Could not open " << filename << " for saving transaction.\n";
    }
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
void loadOrders(vector<Order>& orders) {
    orders.clear();

    ifstream file(ORDERS_FILE); 

    if (!file.is_open()) {
        cerr << "Warning: Could not open " << ORDERS_FILE << " for reading. Orders list is empty.\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> tokens = split(line, ',');
        
        if (!tokens.empty()) {
            orders.push_back(Order::fromCSV(tokens)); 
        }
    }
    file.close();
}

std::vector<BankTransaction> BankTransaction::loadFromFile(const std::string& filename) {
    std::vector<BankTransaction> transactions;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        return transactions;
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Asumsi fungsi split() sudah didefinisikan dan terlihat di scope ini
        std::vector<std::string> tokens = split(line, ',');
        
        if (tokens.size() >= 5) {
            BankTransaction t = BankTransaction::fromCSV(tokens);
            if (t.accountId > 0) {
                 transactions.push_back(t);
            }
        }
    }

    file.close();
    return transactions;
}
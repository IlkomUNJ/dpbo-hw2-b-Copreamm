#include <iostream>
#include <algorithm>
#include <iomanip>
#include <map>
#include <chrono>
#include <limits>
#include <ctime>

#include "./bank.h"
#include "./bank_customer.h"

using namespace std;
using namespace chrono;

string Bank::getCustomerNameById(int id) const {
    auto customer = findAccount(id);
    return customer ? customer->getName() : "Unknown User";
}

Bank::Bank(const string& name) {
    this->name = name;
    this->customerCount = 0;
}

void Bank::addAccount(shared_ptr<BankCustomer> newCustomer) {
    if (!newCustomer) {
        cout << "Invalid customer. Cannot add account.\n\n";
        return;
    }

    if (findAccount(newCustomer->getId())) {
        cout << "Account with ID " << newCustomer->getId() << " already exists.\n\n";
        return;
    }

    accounts.push_back(newCustomer);
    customerCount++;
    cout << "Account for " << newCustomer->getName() << " added successfully.\n\n";
}

shared_ptr<BankCustomer> Bank::findAccount(int id) const {
    auto it = find_if(accounts.begin(), accounts.end(),
                      [id](const shared_ptr<BankCustomer>& customer) {
                          return customer->getId() == id;
                      });

    if (it != accounts.end()) {
        return *it;
    }
    return nullptr;
}

void Bank::listAccounts() const {
    cout << "\n-- Listing all bank accounts in " << name << " --\n";
    if (accounts.empty()) {
        cout << "No accounts available.\n\n";
        return;
    }

    for (const auto& customer : accounts) {
        customer->printInfo();
    }
}

// Bank Capabilities
void Bank::listRecentTransactions(int days) { 
    if (days <= 0) return;

    auto now = system_clock::now();
    auto timeLimit = now - hours(24 * days);

    cout << "\n-- RECENT TRANSACTIONS (LAST " << days << " DAYS) --\n";
    if (transactions.empty()) {
        cout << "No transaction history available.\n\n";
        return;
    }
    
    bool found = false;

    for (const auto& tx : transactions) {
        if (tx.transactionTime >= timeLimit) {
            found = true;
            cout << "Account ID: " << setw(10) << tx.accountId
                 << " | User: " << setw(20) << getCustomerNameById(tx.accountId)
                 << " | Amount: Rp" << fixed << setprecision(2) << tx.amount 
                 << " | Type: " << tx.type << "\n";
        }
    }

    if (!found) {
        cout << "No transactions found in the last " << days << " days.\n";
    }
    cout << "\n";
}

void Bank::listDormantAccounts() const { 
    auto now = system_clock::now();
    auto monthLimit = now - hours(24 * 30); 

    cout << "\n-- DORMANT ACCOUNTS (NO ACTIVITY IN 1 MONTH) --\n";
    bool foundDormant = false;

    if (accounts.empty()) {
        cout << "No accounts to check.\n\n";
        return;
    }

    for (const auto& customer : accounts) {
        if (customer->getLastTransactionTime() < monthLimit) {
            foundDormant = true;
            cout << "ID: " << setw(10) << customer->getId()
                 << " | Name: " << customer->getName() << "\n";
        }
    }

    if (!foundDormant) {
        cout << "No dormant accounts found.\n";
    }
    cout << "\n";
}

void Bank::listTopTransactingUsers(int n) const { 
    if (n <= 0) return;

    time_t rawtime = system_clock::to_time_t(system_clock::now());
    tm* localTime = localtime(&rawtime);
    
    map<int, int> userTxCount; 

    for (const auto& tx : transactions) {
        time_t tx_rawtime = system_clock::to_time_t(tx.transactionTime);
        tm* tx_localTime = localtime(&tx_rawtime);

        if (tx_localTime->tm_mday == localTime->tm_mday &&
            tx_localTime->tm_mon == localTime->tm_mon &&
            tx_localTime->tm_year == localTime->tm_year) 
        {
            userTxCount[tx.accountId]++;
        }
    }

    vector<pair<int, int>> sortedUsers; 
    for (const auto& pair : userTxCount) {
        sortedUsers.push_back({pair.second, pair.first});
    }

    sort(sortedUsers.rbegin(), sortedUsers.rend()); 

    cout << "\n-- TOP " << n << " TRANSACTING USERS (TODAY) --\n";
    if (sortedUsers.empty()) {
        cout << "No transactions recorded today.\n\n";
        return;
    }

    int count = 0;
    cout << left << setw(5) << "Rank"
         << setw(15) << "Account ID"
         << setw(30) << "Name"
         << "Tx Count\n";
    cout << string(60, '-') << "\n";

    for (const auto& pair : sortedUsers) {
        if (count >= n) break;
        
        int accountId = pair.second;
        string userName = getCustomerNameById(accountId); 

        cout << left << setw(5) << (count + 1) << ". "
             << setw(15) << accountId
             << setw(30) << userName
             << pair.first << "\n";
        count++;
    }
    cout << "\n";
}

// Admin Menu
void Bank::handleAdminMenu() {
    int choice;
    do {
        cout << "--" << name << " Admin Menu --\n";
        cout << "1. List All Transactions (Last 7 Days)\n";
        cout << "2. List All Bank Customers\n";
        cout << "3. List All Dormant Accounts (1 Month)\n";
        cout << "4. List Top Transacting Users (Today)\n";
        cout << "5. Back\n";
        cout << "Enter choice: ";
        
        if (!(cin >> choice)) { 
            cout << "Invalid input. Returning to Main Menu.\n\n";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return; 
        }

        switch (choice) {
            case 1: 
                this->listRecentTransactions(7); 
                break;
            case 2: 
                this->listAccounts(); // Reuses existing listAccounts
                break;
            case 3: 
                this->listDormantAccounts(); 
                break;
            case 4: {
                int n;
                cout << "Enter number of top users (N): ";
                if (!(cin >> n)) {
                    cout << "Invalid input N. Canceled.\n\n";
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                this->listTopTransactingUsers(n);
                break;
            }
            case 5: 
                cout << "Returning to Main Menu.\n\n";
                return;
            default: 
                cout << "Invalid choice. Please try again.\n\n";
        }
    } while (true);
}
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <ctime>

#include "./bank_customer.h"
#include "../User/user.h"
#include "../Item/order.h"
#include "../Serialization/serialization.h"

extern std::vector<Order> orders;
extern void loadOrders(std::vector<Order>&);
extern void saveTransaction(const BankTransaction& t, const std::string& filename);
using namespace std;

shared_ptr<BankCustomer> BankCustomer::fromCSV(const vector<string>& tokens) {
    if (tokens.size() < 3) return nullptr;
    try {
        int id = stoi(tokens[0]);
        string name = tokens[1];
        double balance = stod(tokens[2]);
        return make_shared<BankCustomer>(id, name, balance);
    } catch (...) {
        return nullptr;
    }
}

extern shared_ptr<User> currentUser;

enum BankingPrompt{
    BALANCE_CHECK, TRANSACTION_HISTORY, DEPOSIT, WITHDRAW, CASH_FLOW, BACK_BANKING
};

// Banking functions //
void BankCustomer::addBalance(double amount) {
    if (amount <= 0) {
        cout << "Invalid amount. Deposit failed.\n\n";
        return;
    }
    balance += amount;
    cout << "Deposited: Rp" << amount << "\n\n";
    
    BankTransaction t;
    t.timestamp = std::chrono::system_clock::now();
    t.accountId = this->id;
    t.type = "DEPOSIT";
    t.amount = amount;
    t.description = "Self-Deposit via ATM/Menu";

    saveTransaction(t, "transactions.csv");
}

bool BankCustomer::withdraw(double amount, const std::string& description) {
    if (amount <= 0 || balance < amount) {
        return false;
    }
    
    balance -= amount;

    BankTransaction t;
    t.timestamp = std::chrono::system_clock::now();
    t.accountId = this->id;
    t.type = "WITHDRAW";
    t.amount = amount;
    t.description = description;

    saveTransaction(t, "transactions.csv");
    
    return true;
}

void BankCustomer::showTransactionHistory() const {
    loadOrders(orders); 
    
    cout << "\n--- Transaction History for Account ID: " << id << " (" << name << ") ---\n";
    cout << "Type       | Date & Time          | Amount (Rp) | Description\n";
    cout << "-----------------------------------------------------------------------\n";

    bool found = false;
    
    for (const auto& order : orders) {
        if (order.getBuyerName() == name && order.getStatus() == "DONE") {
            found = true;
            
            time_t timeT = std::chrono::system_clock::to_time_t(order.getCreationTime());
            tm* ltm = localtime(&timeT);
            
            cout << left << setw(10) << "WITHDRAWAL" 
                 << " | " << put_time(ltm, "%Y-%m-%d %H:%M:%S") 
                 << " | " << fixed << setprecision(2) << setw(11) << order.getTotalAmount()
                 << " | Purchase from " << order.getSellerStoreName()
                 << " (Order ID: " << order.getOrderId() << ")\n";
        }
    }

    if (!found) {
        cout << "No completed purchase transactions found.\n";
    }
    cout << "\n";
}

void BankCustomer::printInfo() const {
    cout << "-- Bank Customer Info --\n" << id
            << ", Name: " << name << "\n"
            << ", Balance: Rp" << balance << "\n\n";
}

double BankCustomer::calculateCashFlow(int days) const {
    double netFlow = 0.0;

    auto timeLimitDuration = chrono::hours(days * 24); 
    auto now = chrono::system_clock::now();
    auto timeLimit = now - timeLimitDuration;

    std::vector<BankTransaction> allTransactions = 
        BankTransaction::loadFromFile("nama_file_transaksi.csv");

    int currentId = this->id;
    for (const auto& tx : allTransactions) {
        if (tx.accountId != currentId) {
            continue;
        }

        if (tx.timestamp >= timeLimit) {
            if (tx.type == "Deposit" || tx.type == "CREDIT") {
                netFlow += tx.amount;
            } else if (tx.type == "Withdrawal" || tx.type == "DEBIT") {
                netFlow -= tx.amount;
            }
        }
    }
    return netFlow;
}

void BankCustomer::handleBankingFunctions() {
    int choice;
    double amount = 0.0;

    do {
        cout << "-- Banking Account Menu --" << endl;
        cout << "1. Balance Checking" << endl;
        cout << "2. Transaction History" << endl;
        cout << "3. Deposit" << endl;
        cout << "4. Withdraw" << endl;
        cout << "5. List Cash Flow" << endl;
        cout << "6. Back" << endl;
        cout << "Select an option: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice < 1 || choice > 6) {
            cout << "Invalid Input\n";
            continue;
        }

        BankingPrompt bankingPrompt = static_cast<BankingPrompt>(choice - 1);

        switch (bankingPrompt) {
            case BALANCE_CHECK:
                this->printInfo();
                break;
            case TRANSACTION_HISTORY:
                this->showTransactionHistory();
                break;
            case DEPOSIT:
                cout << "Enter deposit amount: ";
                if (cin >> amount) {
                    this->addBalance(amount);
                } else {
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount input.\n\n";
                }
                break;
            case WITHDRAW:
                cout << "Enter withdraw amount: ";
                if (cin >> amount) {
                    if (this->withdraw(amount, "Manual Withdrawal from ATM/Menu")) { 
                        cout << "Withdrawal successful: Rp" << amount << "\n\n";
                    } else {
                        cout << "Withdrawal failed: Insufficient funds or invalid amount.\n\n";
                    }
                } else {
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount input.\n\n";
                }
                break;
            case CASH_FLOW: {
                cout << "\n--- CASH FLOW PERIOD ---\n";
                cout << "1. Today (Last 24 Hours)\n";
                cout << "2. Last 30 Days (A Month)\n";
                cout << "3. Back\n";
                cout << "Select period: ";
                int periodChoice;
                if (!(cin >> periodChoice)) break;

                int days = 0;
                if (periodChoice == 1) {
                    days = 1;
                } else if (periodChoice == 2) {
                    days = 30;
                } else {
                    break;
                }
                
                double flow = this->calculateCashFlow(days);
                
                cout << "\n--- REPORT: Last " << days << " Days ---\n";
                cout << "Net Cash Flow: ";
                cout << fixed << setprecision(2) << ((flow >= 0) ? "CREDIT (+)" : "DEBIT (-)") 
                     << " Rp" << abs(flow) << "\n\n";
                break;
            }
            case BACK_BANKING:
                cout << "Returning to Main Menu...\n\n";
                return;
            default:
                cout << "Invalid option. \n\n";
        }
    } while (true);
}

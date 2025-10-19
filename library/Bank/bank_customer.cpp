#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

#include "./bank_customer.h"
#include "../User/user.h"

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
}

bool BankCustomer::withdraw(double amount) { 
    if (amount <= 0) {
        return withdrawBalance(amount); 
    }
    return withdrawBalance(amount); 
}

bool BankCustomer::withdrawBalance(double amount) {
    if(amount <= 0) {
        cout << "Invalid amount. Withdrawal failed.\n\n";
        return false;
    }
    if(balance >= amount) {
        balance -= amount;
        cout << "Withdrawal Sucessful: - Rp" << amount << "\n\n";
        return true;
    } else {
        cout << "Insufficient balance. Withdrawal failed.\n\n";
        return false;
    }
}

void BankCustomer::showTransactionHistory() const {
    cout << "-> Checking transaction history for " << name << "...\n";
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

    for (const auto& tx : transactionHistory) {
        if (tx.timestamp >= timeLimit) {
            if (tx.type == "Credit") {
                netFlow += tx.amount;
            } else if (tx.type == "Debit") {
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

        if (choice < 1 || choice > 5) {
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
                    this->withdrawBalance(amount);
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

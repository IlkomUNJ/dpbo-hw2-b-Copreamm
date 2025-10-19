#ifndef BANK_CUSTOMER_H
#define BANK_CUSTOMER_H

#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <chrono>

using namespace std;

struct CustomerTransaction {
    double amount;
    string type;
    chrono::system_clock::time_point timestamp; 
};

class BankCustomer {
private:
    int id;
    string name;
    double balance;
    chrono::system_clock::time_point lastTransactionTime;
    vector<CustomerTransaction> transactionHistory;

public:
    BankCustomer(int id, const string& name, double balance)
        : id(id), name(name), balance(balance) {}

    virtual ~BankCustomer() = default;

    chrono::system_clock::time_point getLastTransactionTime() const {
        return lastTransactionTime;
    }

    string toCSV() const {
        stringstream ss;
        ss << id << "," << name << "," << balance;
        return ss.str();
    }

    static shared_ptr<BankCustomer> fromCSV(const vector<string>& tokens);

    void updateLastTransactionTime() {
        lastTransactionTime = chrono::system_clock::now();
    }

    bool withdraw(double amount);

    void recordTransaction(double amount, const string& type);

    double calculateCashFlow(int days) const;
    int getId() const { return id; }
    string getName() const { return name; }
    double getBalance() const { return balance; }

    void setName(const string& newName) { name = newName; }
    void setBalance(double newBalance) { balance = newBalance; }

    void addBalance(double amount);
    bool withdrawBalance(double amount);
    void showTransactionHistory() const;
    void printInfo() const;

    void handleBankingFunctions();
};


#endif // BANK_CUSTOMER_H
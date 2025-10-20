#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include "./bank_customer.h"

class BankCustomer;

using namespace std;

struct TransactionRecord {
    int accountId;
    double amount;
    string type;
    chrono::system_clock::time_point transactionTime;
};

class Bank {
private:
    string name;
    vector<shared_ptr<BankCustomer>> accounts;
    vector<TransactionRecord> transactions;
    int customerCount;
    string getCustomerNameById(int id) const;

public:
    Bank(const string& name);

    virtual ~Bank() = default;

    void registerCustomer(std::shared_ptr<BankCustomer> customer) {
        accounts.push_back(customer);
    }

    void addAccount(shared_ptr<BankCustomer> newCustomer);
    shared_ptr<BankCustomer> findAccount(int id) const;
    void listAccounts() const;

    string getName() const { return name; }
    int getCustomerCount() const { return customerCount; }

    void handleAdminMenu();
    void listRecentTransactions(int days);
    void listDormantAccounts() const;
    void listTopTransactingUsers(int n) const;
};

#endif // BANK_H
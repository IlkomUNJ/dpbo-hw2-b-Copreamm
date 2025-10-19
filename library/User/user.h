#ifndef USER_H
#define USER_H

#include <string>
#include <memory>

#include "../Bank/bank_customer.h"

using namespace std;

class BankCustomer;

class User {
private:
    string name;

protected:
    string password;
    shared_ptr<BankCustomer> account;

public:
    User(const string& uname, const string& upass) 
        : name(uname), password(upass), account(nullptr) {}

    User(const string& uname, const string& upass, shared_ptr<BankCustomer> acc) 
        : name(uname), password(upass), account(acc) {}

    virtual ~User() = default; 

    const string& getName() const { return name; }
    const string& getPassword() const { return password; }
    shared_ptr<BankCustomer> getAccount() const { return account; } 

    virtual bool isAdmin() const { return false; } 
    virtual bool isSeller() const { return false; }

    virtual string getRole() const = 0; 
    virtual void showAccountInfo() const = 0; 

    virtual string userToCSV() const = 0;

    virtual double getBalance() const {
        return 0.0;
    }
};

#endif

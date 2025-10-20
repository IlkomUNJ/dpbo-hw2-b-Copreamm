#ifndef ADMIN_H
#define ADMIN_H

#include "../User/user.h"

class Admin : public User {
public:
    Admin(const string& uname, const string& upass) 
        : User(uname, upass) {}

    string getRole() const override { return "Admin"; }
    bool isAdmin() const override { return true; } 

    void showAccountInfo() const override {
        cout << "Role: Admin\n";
        cout << "Name: " << getName() << "\n";
        cout << "Access: Full Bank Control\n";
    }

    string userToCSV() const override {
        return getName() + "," + getPassword() + "," + getRole() + ",N/A";
    }
    
    double getBalance() const override { return 0.0; }
};

#endif // ADMIN_H
#ifndef SELLER_H
#define SELLER_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <chrono>
#include <map>

#include "./user.h"
#include "../Bank/bank_customer.h"
#include "../Item/item.h"
#include "../Item/order.h"

class Item;
class BankCustomer;
class Order;

using namespace std;

class Seller : public User {
private:
    string storeName;
    vector<Item> items;

    vector<Order> loadAllOrders() const;

public:
    Seller(const string& name, const string& password, const string& storeName);
    
    Seller(const string& name, const string& password, const string& storeName, 
           shared_ptr<BankCustomer> existingAccount);

    string getRole() const override;
    bool isSeller() const override { return true; }
    void showAccountInfo() const override;

    double getBalance() const override;

    string userToCSV() const override;
    string inventoryToCSV() const;
    
    void addItem(int id, const string& name, int qty, double price);
    void addItemObject(const Item& item) { items.push_back(item); }
    void showInventory() const;
    void removeItem(int id);
    
    void viewMostFrequentItems(int mItems) const;
    void viewOrders() const;
    void handleStoreCapabilitiesMenu();

    const string& getStoreName() const { return storeName; }
};

#endif // SELLER_H
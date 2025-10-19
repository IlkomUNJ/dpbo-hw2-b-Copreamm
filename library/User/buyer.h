#ifndef BUYER_H
#define BUYER_H

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "./user.h"
#include "../Bank/bank_customer.h"
#include "../Item/order.h"
#include "../Item/item.h"

class BankCustomer;
class Order;
class Item;

using namespace std;

struct InventoryItem {
    int id;
    string name;
    int quantity;
    double price;
};

class Buyer : public User {
private:
    int id;

    static void updateInventoryCSV(const map<string, vector<InventoryItem>>& allStoreInventory, const string& filename = "inventory.csv");
	static void recordOrder(const Order& order, const string& filename = "orders.csv");

public:
    Buyer(const string& name, const string& password);

    Buyer(const string& name, const string& password, shared_ptr<BankCustomer> acc) 
        : User(name, password, acc) {}

    static void loadInventoryFromCSV(map<string, vector<InventoryItem>>& allStoreInventory, const string& filename = "inventory.csv");
    void handleBrowseStore();
    void handleOrderFunctionality();

    void purchaseItem(
		const string& storeName, 
		const InventoryItem& itemData,
		int purchaseQty,
		map<string, vector<InventoryItem>>& currentInventory
	);

    bool withdraw(double amount);

    string getRole() const override;
    void showAccountInfo() const override;

    double getBalance() const override;

    string userToCSV() const override;

    shared_ptr<BankCustomer> getBankCustomer() const;
    void deposit(double amount);

    void viewMyOrderHistory() const;
};

#endif // BUYER_H

#ifndef ORDER_H
#define ORDER_H

#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../Item/item.h" 

using namespace std;

class Order {
private:
    int orderId;
    string buyerName;
    string sellerStoreName;
    double totalAmount;
    string status;
    vector<Item> items;
    chrono::system_clock::time_point creationTime;   

public:
    Order(int id, const string& buyer, const string& sellerStore) 
        : orderId(id), buyerName(buyer), sellerStoreName(sellerStore), 
          totalAmount(0.0), status("Pending"),
          creationTime(chrono::system_clock::now()) {}
    
    Order(int id, const string& buyer, const string& sellerStore, 
          double total, const string& stat, const vector<Item>& itemList,
          chrono::system_clock::time_point time)
        : orderId(id), buyerName(buyer), sellerStoreName(sellerStore), 
          totalAmount(total), status(stat), items(itemList), creationTime(time) {}
    
    chrono::system_clock::time_point getCreationTime() const { return creationTime; }

    void setCreationTime(chrono::system_clock::time_point time);

    string getFormattedCreationTime() const;

    void addItem(const Item& item) {
        items.push_back(item);
        totalAmount += item.getQuantity() * item.getPrice();
    }

    string toCSV() const;

    int getOrderId() const { return orderId; }
    const string& getBuyerName() const { return buyerName; }
    const string& getSellerStoreName() const { return sellerStoreName; }
    const vector<Item>& getItems() const { return items; }
    void setTotalAmount(double amount); 
    double getTotalAmount() const { return totalAmount; }
    const string& getStatus() const { return status; }
    
    void setStatus(const string& newStatus) { status = newStatus; }

    static Order fromCSV(const vector<string>& tokens);
};

#endif // ORDER_H

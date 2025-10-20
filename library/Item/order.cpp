#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "./order.h"

using namespace std;

void Order::setTotalAmount(double totalAmount) {
    this->totalAmount = totalAmount; 
}

string Order::getFormattedCreationTime() const {
    time_t tt = chrono::system_clock::to_time_t(creationTime);
    tm tm_local = *localtime(&tt);
    stringstream ss;
    ss << put_time(&tm_local, "%Y-%m-%d %H:%M:%S");
    
    return ss.str();
}

string Order::getYearMonthString() const {
    time_t tt = chrono::system_clock::to_time_t(creationTime);
    tm tm = *localtime(&tt);
    stringstream ss;
    ss << put_time(&tm, "%Y-%m");
    return ss.str();
}

void Order::setCreationTime(chrono::system_clock::time_point time) {
    creationTime = time;
}

Order Order::fromCSV(const vector<string>& tokens) {
    int id = stoi(tokens[0]);
    string buyer = tokens[1];
    string seller = tokens[2];
    double total = stod(tokens[3]);
    string statusStr = tokens[4];

    chrono::system_clock::time_point loadedTime;

    if (tokens.size() > 5) {
        try {
            long long timestamp = stoll(tokens[5]);
            loadedTime = chrono::system_clock::from_time_t(timestamp);
        } catch (const exception& e) {
        }
    }
    
    Order loadedOrder(id, buyer, seller);
    
    loadedOrder.setTotalAmount(total);
    loadedOrder.setStatus(statusStr);
    loadedOrder.setCreationTime(loadedTime);

    for (size_t i = 6; i < tokens.size(); i += 3) {
    if (i + 2 < tokens.size()) {
        string itemName = tokens[i];
        int quantity = stoi(tokens[i+1]);
        double price = stod(tokens[i+2]);
        
        Item item(itemName, price, quantity, loadedOrder.getSellerStoreName());
        
        loadedOrder.addItem(item); 
    }
}

    return loadedOrder;
}

string Order::toCSV() const {
    auto timestamp = chrono::system_clock::to_time_t(creationTime); 
    
    stringstream ss;
    
    ss << orderId << "," 
       << buyerName << "," 
       << sellerStoreName << "," 
       << totalAmount << "," 
       << status 
       << "," << timestamp;
    
    for (const auto& item : items) {
        ss << ";" << item.getId() << "," << item.getName() << "," 
           << item.getQuantity() << "," << item.getPrice(); 
    }
    
    return ss.str();
}
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>

#include "./order.h"

using namespace std;

void Order::setTotalAmount(double totalAmount) {
    this->totalAmount = totalAmount; 
}

Order Order::fromCSV(const vector<string>& tokens) {
    if (tokens.size() < 7) {
        return Order(0, "INVALID_BUYER", "INVALID_STORE");
    }
    
    try {
        int id = stoi(tokens[0]);
        string buyer = tokens[1];
        string sellerStore = tokens[2];
        double total = stod(tokens[3]);
        string stat = tokens[4];
        string itemListString = tokens[5];
        long long timestamp_ll = stoll(tokens[6]);

        chrono::system_clock::time_point time;
        time += chrono::milliseconds(timestamp_ll);

        vector<Item> itemList;
        stringstream ss_items(itemListString);
        string item_segment;
        
        // Item segments are separated by '|'
        while (getline(ss_items, item_segment, '|')) {
            stringstream ss_item_data(item_segment);
            string item_token;
            vector<string> item_tokens;

            // Item data fields are separated by ';'
            while (getline(ss_item_data, item_token, ';')) {
                item_tokens.push_back(item_token);
            }

            // Item must have 4 tokens: id, name, quantity, price
            if (item_tokens.size() >= 4) {
                try {
                    int itemId = stoi(item_tokens[0]);
                    string itemName = item_tokens[1];
                    int itemQty = stoi(item_tokens[2]);
                    double itemPrice = stod(item_tokens[3]);
                    
                    itemList.emplace_back(itemId, itemName, itemQty, itemPrice);
                } catch (...) {
                    // Skip malformed item
                }
            }
        }
        
        return Order(id, buyer, sellerStore, total, stat, itemList, time);
    } catch (const exception& e) {
        return Order(0, "CRITICAL_ERROR", "CRITICAL_ERROR_STORE");
    }
}
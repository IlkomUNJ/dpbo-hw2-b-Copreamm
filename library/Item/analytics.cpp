#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <limits>

#include "../Item/order.h"
#include "../User/seller.h"
#include "../Item/analytics.h" 

using namespace std;

extern vector<Order> orders;

string formatTimePoint(chrono::system_clock::time_point tp);

pair<chrono::system_clock::time_point, chrono::system_clock::time_point> getMinMaxTime(const vector<Order>& orders) {
    if (orders.empty()) {
        return {chrono::system_clock::now(), chrono::system_clock::now()};
    }

    auto minTime = orders[0].getCreationTime();
    auto maxTime = orders[0].getCreationTime();

    for (const auto& order : orders) {
        if (order.getCreationTime() < minTime) {
            minTime = order.getCreationTime();
        }
        if (order.getCreationTime() > maxTime) {
            maxTime = order.getCreationTime();
        }
    }
    return {minTime, maxTime};
}

void showRecentTransactions(const vector<Order>& orders, int nDays) {
    if (nDays <= 0) {
        cout << "The number of days must be greater than zero.\n\n";
        return;
    }

    using namespace chrono;
    
    auto now = system_clock::now();
    auto nDaysAgo = now - hours(24 * nDays);

    cout << "\n-- TRANSACTIONS IN " << nDays << " LAST DAY --\n";

    bool found = false;
    
    for (const auto& order : orders) {
        if (order.getCreationTime() >= nDaysAgo) {
            found = true;
            cout << "ID: " << order.getOrderId() 
                 << " | Buyer: " << order.getBuyerName() 
                 << " | Store: " << order.getSellerStoreName()
                 << " | Total: Rp" << fixed << setprecision(2) << order.getTotalAmount()
                 << " | Status: " << order.getStatus()
                 << "\n";
        }
    }

    if (!found) {
        cout << "No transactions were found in " << nDays << " last days.\n";
    }
    cout << "\n\n";
}

void viewMostActiveBuyersPerDay(const vector<Order>& orders, int nTop, int days) {
    if (nTop <= 0) {
        cout << "The number of best buyers must be greater than zero.\n\n";
        return;
    }

    if (orders.empty()) {
        cout << "No transaction data is available for analysis.\n\n";
        return;
    }

    map<string, int> buyerTxCount;
    for (const auto& order : orders) {
        buyerTxCount[order.getBuyerName()]++;
    }
    
    double totalDays = static_cast<double>(days);
    
    if (totalDays < 1.0) {
        totalDays = 1.0; 
    }
    
    vector<pair<double, string>> buyerRatios;
    for (const auto& pair : buyerTxCount) {
        double ratio = static_cast<double>(pair.second) / totalDays;
        buyerRatios.push_back({ratio, pair.first});
    }

    sort(buyerRatios.rbegin(), buyerRatios.rend());

    cout << "\n-- TOP " << nTop << " MOST ACTIVE BUYERS --\n";
    cout << "Analysis Period: " << totalDays << " days.\n\n";
    cout << left << setw(5) << "Rank"
            << setw(30) << "Buyer Name"
            << setw(15) << "Total Tx"
            << "Tx/Day (Ratio)\n";
    cout << string(65, '-') << "\n";

    int count = 0;
    for (const auto& pair : buyerRatios) {
        if (count >= nTop) break;

        double ratio = pair.first;
        string buyerName = pair.second;
        int totalTx = buyerTxCount.at(buyerName);

        cout << left << setw(5) << (count + 1) << ". "
             << setw(30) << buyerName
             << setw(15) << totalTx
             << ratio << "\n";
        count++;
    }
    
    if (buyerRatios.empty()) {
        cout << "No buyers found.\n";
    }
    cout << "\n\n";
}

void viewMostActiveSellersPerDay(const vector<Order>& orders, int nTop, int days) {
    if (nTop <= 0) {
        cout << "The number of top sellers must be greater than zero.\n\n";
        return;
    }

    if (orders.empty()) {
        cout << "No transaction data is available for analysis.\n\n";
        return;
    }

    map<string, int> sellerTxCount;
    for (const auto& order : orders) {
        sellerTxCount[order.getSellerStoreName()]++;
    }
    
    double totalDays = static_cast<double>(days);
    
    if (totalDays < 1.0) {
        totalDays = 1.0; 
    }
    
    vector<pair<double, string>> sellerRatios;
    for (const auto& pair : sellerTxCount) {
        double ratio = static_cast<double>(pair.second) / totalDays;
        sellerRatios.push_back({ratio, pair.first});
    }

    sort(sellerRatios.rbegin(), sellerRatios.rend());

    cout << "\n-- TOP " << nTop << " MOST ACTIVE SELLERS --\n";
    cout << "Analysis Period: " << fixed << setprecision(2) << totalDays << " days.\n\n";
    cout << left << setw(5) << "Rank"
         << setw(30) << "Store Name"
         << setw(15) << "Total Tx"
         << "Tx/Day (Ratio)\n";
    cout << string(65, '-') << "\n";

    int count = 0;
    for (const auto& pair : sellerRatios) {
        if (count >= nTop) break;

        double ratio = pair.first;
        string storeName = pair.second;
        int totalTx = sellerTxCount.at(storeName);

        cout << left << setw(5) << (count + 1) << ". "
             << setw(30) << storeName
             << setw(15) << totalTx
             << fixed << setprecision(4) << ratio << "\n";
        count++;
    }
    
    if (sellerRatios.empty()) {
        cout << "No sellers were found.\n";
    }
    cout << "\n\n";
}
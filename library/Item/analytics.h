#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <vector>
#include <memory>
#include <chrono>

class Order; 

void showRecentTransactions(const std::vector<Order>& orders, int nDays);
void viewMostActiveBuyersPerDay(const std::vector<Order>& orders, int nTop, int nDays);
void viewMostActiveSellersPerDay(const std::vector<Order>& orders, int nTop, int nDays);

#endif // ANALYTICS_H
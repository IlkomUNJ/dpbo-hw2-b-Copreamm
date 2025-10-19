#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <memory>
#include <string>

#include "bank_transaction.h"

using namespace std;

class User;
class Order;

void saveAllData(const vector<shared_ptr<User>>& users, 
                 const vector<Order>& orders);

void loadAllData(vector<shared_ptr<User>>& users, 
                 vector<Order>& orders);

void saveTransaction(const BankTransaction& t, const string& filename);

#endif // SERIALIZATION_H
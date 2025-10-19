#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <memory>
#include <string>

using namespace std;

class User;
class Order;

void saveAllData(const vector<shared_ptr<User>>& users, 
                 const vector<Order>& orders);

void loadAllData(vector<shared_ptr<User>>& users, 
                 vector<Order>& orders);

#endif // SERIALIZATION_H
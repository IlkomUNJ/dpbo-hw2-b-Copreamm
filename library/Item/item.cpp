#include <vector>
#include <memory>
#include <string>

#include "./item.h"

using namespace std;

shared_ptr<Item> Item::fromCSV(const vector<string>& tokens) {
    if (tokens.size() < 4) return nullptr;
    try {
        int id = stoi(tokens[0]);
        string name = tokens[1];
        int quantity = stoi(tokens[2]);
        double price = stod(tokens[3]);
        return make_shared<Item>(id, name, quantity, price);
    } catch (...) {
        return nullptr;
    }
}
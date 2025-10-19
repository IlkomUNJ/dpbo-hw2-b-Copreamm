#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>

using namespace std;

class Item {
private:
    int id;
    string name;
    int quantity;
    double price;
    bool idDisplay;
    string sellerStoreName;

public:
    Item(int id, const std::string& name, int quantity, double price)
        : id(id), name(name), quantity(quantity), price(price) {
            idDisplay = false;
        }

    string toCSV() const {
        stringstream ss;
        ss << id << "," << name << "," << quantity << "," << fixed << setprecision(2) << price;
        return ss.str();
    }

    Item(const std::string& name, double price, int quantity, [[maybe_unused]] const std::string& sellerStoreName)
        : id(0), name(name), quantity(quantity), price(price), sellerStoreName(sellerStoreName) {
             idDisplay = false; 
        }

    static shared_ptr<Item> fromCSV(const vector<string>& tokens);

    int getId() const { return id; }
    const std::string& getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    Item *getItem() {
        return this;
    }

    void setId(int newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setQuantity(int newQuantity) { quantity = newQuantity; }
    void setPrice(double newPrice) { price = newPrice; }
    void setDisplay(bool display) { idDisplay = display; }

    void alterItemById(int itemId, const std::string& newName, int newQuantity, double newPrice) {
        if (id == itemId) {
            name = newName;
            quantity = newQuantity;
            price = newPrice;
        }
    }

    void updatePriceQuantity(int itemId, double newPrice, int newQuantity) {
        if (id == itemId) {
            price = newPrice;
            quantity = newQuantity;
        }
    }
};

#endif // ITEM_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <random>

#include "./buyer.h"
#include "../Bank/bank_customer.h"
#include "../Item/order.h"
#include "../Item/item.h"
#include "../User/user.h"

using namespace std;
extern std::vector<Order> orders;
extern void loadOrders(std::vector<Order>&);

Buyer::Buyer(const string& name, const string& password)
    : User(name, password) {
        int newId = 1000 + (rand() % 9000);
        this->account = make_shared<BankCustomer>(newId, name, 0.0);
    }

bool Buyer::withdraw(double amount) {
    if (this->account) {
        return this->account->withdraw(amount, "E-Commerce Purchase"); 
    }
    return false;
}

string Buyer::getRole() const {
    return "Buyer";
}

void Buyer::showAccountInfo() const {
    cout << "-- Buyer Account Info --\n";
    cout << "Name: " << getName() << "\n";

    if(account) {
        cout << "Bank Account ID: " << account->getId() << "\n";
        cout << "Balance: Rp" << fixed << setprecision(2) << getBalance() << "\n\n";
    } else {
        cout << "No bank account linked.\n\n";
    }
}

double Buyer::getBalance() const {
    if(account) {
        return account->getBalance();
    }
    return 0.0;
}

string Buyer::userToCSV() const {
    return getName() + "," + password + "," + getRole() + ",";
}

void Buyer::updateInventoryCSV(const map<string, vector<InventoryItem>>& allStoreInventory, const string& filename) {
	ofstream file("data/" + filename);

	if (!file.is_open()) {
		cerr << "Error: Could not open data/" << filename << " for writing. Inventory update failed.\n";
		return;
	}

	for (const auto& storePair : allStoreInventory) {
		const string& storeName = storePair.first;
		for (const auto& item : storePair.second) {
			if (item.quantity > 0) { 
				file << storeName << ","
					 << item.id << ","
					 << item.name << ","
					 << item.quantity << ","
					 << fixed << setprecision(2) << item.price << "\n";
			}
		}
	}
	file.close();
}

void Buyer::recordOrder(const Order& order, const string& filename) {
	ofstream file("data/" + filename, ios::app);
	
	if (!file.is_open()) {
		cerr << "Error: Could not open data/" << filename << " for writing. Order recording failed.\n";
		return;
	}

	file << order.toCSV() << "\n";
	
	file.close();
}

void Buyer::loadInventoryFromCSV(map<string, vector<InventoryItem>>& allStoreInventory, const string& filename) {
    allStoreInventory.clear();

    ifstream file("data/" + filename);

    if (!file.is_open()) {
        file.open("../data/" + filename);
    }
    
    if (!file.is_open()) {
        file.open(filename);
    }

    if (!file.is_open()) {
        cout << "Warning: Could not open " << filename << ". Checked multiple paths relative to 'data/' folder. Please verify file location.\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string segment;
        vector<string> tokens;
        
        while (getline(ss, segment, ',')) {
            tokens.push_back(segment);
        }

        if (tokens.size() >= 5) {
            try {
                string storeName = tokens[0];
                InventoryItem item;
                item.id = stoi(tokens[1]);
                item.name = tokens[2];
                item.quantity = stoi(tokens[3]);
                item.price = stod(tokens[4]);
                allStoreInventory[storeName].push_back(item);
            } catch (...) {
            }
        }
    }
    file.close();
}

void Buyer::purchaseItem(
	const string& storeName, 
	const InventoryItem& itemData, 
	int purchaseQty,
	map<string, vector<InventoryItem>>& currentInventory) {
	
	const string inventoryFile = "inventory.csv";
	const string ordersFile = "orders.csv";       

	if (!getAccount()) {
		cout << "\n[PURCHASE FAILED] Please create a bank account first to make a purchase.\n";
		return;
	}

	double totalCost = itemData.price * purchaseQty;
	
	int newOrderId = 10000 + (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() % 9999);
	
	Item purchasedItem(itemData.id, itemData.name, purchaseQty, itemData.price);

	if (purchaseQty > itemData.quantity) {
		cout << "\n[PURCHASE FAILED] Insufficient stock. Available: " << itemData.quantity << ".\n";

		Order incompleteOrder(newOrderId, this->getName(), storeName);
		incompleteOrder.addItem(purchasedItem);
		incompleteOrder.setTotalAmount(totalCost); 
		incompleteOrder.setStatus("INCOMPLETE");
		recordOrder(incompleteOrder, ordersFile);

		cout << "[ORDER INCOMPLETE] Transaction recorded (Insufficient Stock).\n";
		return;
	}

	if (getAccount()->getBalance() < totalCost) {
		cout << "\n[PURCHASE FAILED] Insufficient balance. Current: Rp" 
			 << fixed << setprecision(2) << getAccount()->getBalance()
			 << ". Required: Rp" << fixed << setprecision(2) << totalCost << ".\n";

		Order canceledOrder(newOrderId, this->getName(), storeName);
		canceledOrder.addItem(purchasedItem);
		canceledOrder.setTotalAmount(totalCost); 
		canceledOrder.setStatus("CANCELED");
		recordOrder(canceledOrder, ordersFile);

		cout << "[ORDER CANCELED] Transaction recorded with CANCELED status (Insufficient Balance).\n\n";
		return;
	}

	Order successOrder(newOrderId, this->getName(), storeName);
	successOrder.addItem(purchasedItem);
	successOrder.setTotalAmount(totalCost); 
	successOrder.setStatus("DONE"); 
	recordOrder(successOrder, ordersFile);

	withdraw(totalCost); 
	
	try {
		for (auto& itemRef : currentInventory.at(storeName)) {
			if (itemRef.id == itemData.id) {
				itemRef.quantity -= purchaseQty;
				cout << "\n[PURCHASE SUCCESS] Bought " << purchaseQty << "x " << itemData.name 
					 << " for Rp" << fixed << setprecision(2) << totalCost << ".\n";
				cout << "Remaining balance: Rp" << fixed << setprecision(2) << getBalance() << ".\n";
				cout << "Remaining stock: " << itemRef.quantity << ".\n";
				break;
			}
		}
	} catch (const out_of_range& e) {
		cerr << "[ERROR] Store name not found in inventory map: " << storeName << "\n";
	}
	
	updateInventoryCSV(currentInventory, inventoryFile);
	
	cout << "Transaction recorded with DONE status.\n\n";
}

void Buyer::handleBrowseStore() {
	map<string, vector<InventoryItem>> currentInventory;
	const string inventoryFile = "inventory.csv";
	loadInventoryFromCSV(currentInventory, inventoryFile);

	if (currentInventory.empty()) {
		cout << "No stores or items found in inventory.\n\n";
		return;
	}

	int choice = 0;
	vector<string> storeNames;
	for (const auto& pair : currentInventory) {
		storeNames.push_back(pair.first);
	}
	
	const int numStores = static_cast<int>(storeNames.size());
	const int backOption = numStores + 1;

	do {
		cout << "\n-- Browse Store Menu --\n";
		int i = 1;
		for (const string& name : storeNames) {
			cout << i++ << ". " << name << "\n";
		}
		cout << backOption << ". Back to Main Menu\n";
		cout << "Select a store number to view items: ";

		if (!(cin >> choice) || choice < 1 || choice > backOption) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Invalid selection. Please try again.\n";
			continue;
		}

		if (choice == backOption) { 
			break;
		}

		const string& selectedStoreName = storeNames[choice - 1]; 
		if (currentInventory.count(selectedStoreName) == 0) {
			cout << "Store not found. Please try again.\n";
			continue;
		}
		const auto& items = currentInventory.at(selectedStoreName);
		
		int itemChoice = 0;
		int purchaseQty = 0;
		const int numItems = static_cast<int>(items.size());
		const int itemBackOption = numItems + 1;

		do {
			cout << "\n--- ITEMS IN " << selectedStoreName << " ---\n";
			cout << setw(5) << "ID" << setw(5) << "#" << setw(30) << "Name" << setw(10) << "Qty" << setw(15) << "Price (Rp)\n";
			cout << "----------------------------------------------------------------\n";
			 
			int j = 1;
			for (const auto& item : items) {
				cout << setw(5) << item.id 
					 << setw(5) << j++
					 << setw(30) << item.name 
					 << setw(10) << item.quantity 
					 << setw(15) << fixed << setprecision(2) << item.price << "\n";
			}
			cout << itemBackOption << ". Back to Store List\n";
			cout << "----------------------------------------------------------------\n";
			cout << "Select item number to purchase, or " << itemBackOption << " to go back: ";

			if (!(cin >> itemChoice)) {
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "Invalid input type. Please try again.\n";
				continue;
			}
			
			if (itemChoice == itemBackOption) {
				break;
			}

			if (itemChoice < 1 || itemChoice > numItems) {
				cout << "Invalid item selection.\n";
				continue;
			}

			const InventoryItem& selectedItem = items[itemChoice - 1];
			
			cout << "Enter quantity for " << selectedItem.name << " (Available: " << selectedItem.quantity << "): ";
			if (!(cin >> purchaseQty) || purchaseQty <= 0) {
				cout << "Invalid quantity or input.\n";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				continue;
			}

			purchaseItem(selectedStoreName, selectedItem, purchaseQty, currentInventory);
			
			loadInventoryFromCSV(currentInventory, inventoryFile); 

		} while (true);
		
	} while (true);

	cout << "Returning to Main Menu...\n\n";
}

void Buyer::handleOrderFunctionality() {
    int choice;
    do {
        cout << "\n=== MENU ORDER FUNCTIONALITY ===\n";
        cout << "1. View My Order History\n";
        cout << "2. Back to Main Menu\n";
        cout << "Select option: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Returning to Main Menu.\n\n";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }

        switch (choice) {
            case 1:
                viewMyOrderHistory();
                break;
            case 2:
                cout << "Returning.\n";
                return;
            default:
                cout << "Invalid choice.\n";
                break;
        }
    } while (true);
}

void Buyer::viewMyOrderHistory() const {
    try {
        loadOrders(orders);
        const vector<Order>& allOrders = orders;

        cout << "\n-- MY ORDER HISTORY (" << this->getName() << ") --\n";
        bool found = false;

        for (const auto& order : allOrders) {
            if (order.getBuyerName() == this->getName()) {
                found = true;
                cout << "\nOrder ID: " << order.getOrderId() << "\n";
                cout << "Seller: " << order.getSellerStoreName() << "\n";
                cout << "Status: " << order.getStatus() << "\n";
                cout << "Total Cost: Rp" << fixed << setprecision(2) << order.getTotalAmount() << "\n";
                
                cout << "Item Details:\n";
                for (const auto& item : order.getItems()) {
                    cout << "  - " << item.getName() << " x " << item.getQuantity() 
                         << " (Rp" << fixed << setprecision(2) << item.getPrice() << ")\n";
                }
                cout << "\n";
            }
        }

        if (!found) {
            cout << "You have no order history.\n";
        }
        cout << "\n";
    } catch (...) {
        cerr << "\n[ERROR] Failed to load order history. Check the implementation of loadAllOrders().\n";
    }
}
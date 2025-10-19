#include <iostream>
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>

#include "./user.h"
#include "./buyer.h" 
#include "./seller.h" 
#include "../Bank/bank_customer.h"
#include "../Item/item.h"
#include "../Item/order.h"
#include "../Item/analytics.h"
#include "../Bank/bank.h"

using namespace std;

vector<shared_ptr<User>> users;
vector<Order> orders;
shared_ptr<User> currentUser = nullptr;

extern Bank systemBank;

shared_ptr<User> loginUser() {
    string inputName;
    string inputPassword;

    cout << "--- Login ---" << endl;
    cout << "Enter Your Name: ";
    cin >> inputName;
    cout << "Enter Your Password: ";
    cin >> inputPassword;

    for (const auto& user : users) {
        if (user->getName() == inputName) {
            if (user->getPassword() == inputPassword) {
                return user;
            } else {
                cout << "Incorrect password.\n";
                return nullptr;
            }
        }
    }
    return nullptr;
}

void handleRegister() {
    int choice;
    string inputName;
    string inputPassword;
    string inputStoreName;

    do {
        cout << "\n=== REGISTER MENU ===\n";
        cout << "1. Create Buyer Account\n";
        cout << "2. Create Seller Account\n";
        cout << "3. Back\n";
        cout << "Select an option: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n\n";
            continue;
        }

        if(choice == 3) {
            cout << "Returning to main menu...\n\n";
            break;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1 || choice == 2) {
            
            cout << "Enter Name: ";
            getline(cin, inputName);
            cout << "Enter Password: ";
            getline(cin, inputPassword);

            auto it = find_if(users.begin(), users.end(), 
                              [&](const shared_ptr<User>& u) {
                                  return u->getName() == inputName;
                              });

            if (choice == 1) {
                if (it != users.end()) {
                    cout << "Failed. Use another name to register\n\n";
                    continue;
                }
                users.push_back(make_shared<Buyer>(inputName, inputPassword)); 
                cout << "Buyer account for " << inputName << " created successfully.\n\n";  
            
            } else if (choice == 2) {
                if (it == users.end()) {
                    cout << "Failed. Buyer not found\n\n";
                    continue;
                }
                shared_ptr<User> existingUser = *it;

                if (existingUser->getPassword() != inputPassword) {
                    cout << "Failed. Wrong Password\n\n";
                    continue;
                }

                if (existingUser->getRole() == "Seller") {
                    cout << "Failed. Account already registered\n\n";
                    continue;
                }

                cout << "Enter Store Name: ";
                getline(cin, inputStoreName);
                
                shared_ptr<BankCustomer> existingAccount = existingUser->getAccount();

                shared_ptr<Seller> newSeller = make_shared<Seller>(
                    inputName, inputPassword, inputStoreName, existingAccount);

                *it = newSeller; 
                
                cout << "Success. Account already upgraded to be seller.\n\n";
            }
        } else {
            cout << "Invalid option.\n\n";
        }
    } while (true);
}

void handleLoginMenu() {
    int choice;
    
    if (currentUser->isAdmin()) {
        cout << "\nAccess granted: Bank Administrator.\n\n";
        systemBank.handleAdminMenu();
        currentUser = nullptr;
        return; 
    }

    const bool isSellerUser = currentUser->isSeller();
    int maxChoice = 5;

    do {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Check Account Status\n";
        cout << "2. Banking Functions\n";
        cout << "3. Browse Store\n";
        cout << "4. Order Functionality\n";
        cout << "5. Logout\n";

        if (isSellerUser) {
            cout << "\n--- Seller Menu ---\n";
            cout << "6. Check Inventory\n";
            cout << "7. Add Item\n";
            cout << "8. Remove Item\n";
            cout << "9. Store Capabilities\n";
            maxChoice = 9;
        }

        cout << "Select an option: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice < 1 || choice > maxChoice) {
            cout << "Option Invalid\n\n";
            continue;
        }

        if (choice == 5) {
            cout << "Logging out...\n\n";
            currentUser = nullptr;
            return; 
        }

        switch (choice) {
            case 1:
                cout << "Display Account Info...\n";
                currentUser->showAccountInfo();
                break;
            case 2:
                cout << "Banking Functions...\n";
                if (currentUser->getAccount()) {
                currentUser->getAccount()->handleBankingFunctions(); 
            } else {
                cout << "Please create a bank account first.\n\n";
            }
            break;
                break;
            case 3:
                cout << "Browsing store...\n";
                if (shared_ptr<Buyer> buyerUser = dynamic_pointer_cast<Buyer>(currentUser)) {
                    buyerUser->handleBrowseStore();}
                break;
            case 4:
                cout << "Order Functionality...\n";
                if (shared_ptr<Buyer> buyerUser = dynamic_pointer_cast<Buyer>(currentUser)) {
                    buyerUser->handleOrderFunctionality(); 
                }
                break;
            case 6: {
                shared_ptr<Seller> seller = dynamic_pointer_cast<Seller>(currentUser);

                if (isSellerUser) {
                    cout << "Check Inventory...\n";
                    seller->showInventory();
                } else {
                    cout << "Option only available for sellers.\n\n";
                }
                break;
            }
            case 7: {
                shared_ptr<Seller> seller = dynamic_pointer_cast<Seller>(currentUser);

                if (isSellerUser) {
                    int id = 0; 
                    int qty = 0;
                    double price = 0.0;
                    string name = ""; 

                    cout << "\n--- Add New Item ---\n";
                    
                    cout << "Enter Item ID: ";
                    if (!(cin >> id)) {
                        cout << "Invalid ID input. Cancel adding the item.\n\n";
                        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                    
                    cout << "Enter Item Name: ";
                    getline(cin, name);
                    
                    cout << "Enter Quantity: ";
                    if (!(cin >> qty)) {
                        cout << "Invalid quantity input. Cancel adding the item.\n\n";
                        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    
                    cout << "Enter Price: ";
                    if (!(cin >> price)) { 
                        cout << "Invalid price input. Cancel adding the item.\n\n";
                        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    
                    seller->addItem(id, name, qty, price);

                    } else {
                        cout << "Option only available for sellers.\n\n";
                    }
                break;
            }  
            case 8: {
                shared_ptr<Seller> seller = dynamic_pointer_cast<Seller>(currentUser);

                if (isSellerUser) {
                    int id = 0; 
                
                cout << "\n--- Remove Item ---\n";
                cout << "Enter Item ID to remove: ";
                if (!(cin >> id)) {
                    cout << "Invalid ID input. Cancellation.\n\n";
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                
                seller->removeItem(id);

                } else {
                    cout << "Option only available for sellers.\n\n";
                }
                break;
            }   
            case 9: {
                shared_ptr<Seller> seller = dynamic_pointer_cast<Seller>(currentUser);

                if (isSellerUser) {
                    seller->handleStoreCapabilitiesMenu();
                } else {
                    cout << "Option only available for sellers.\n\n";
                }
                break;
            }
            default:
                cout << "Invalid option.\n";  
                break;
        }
    } while (currentUser != nullptr);
}
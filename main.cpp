#include <iostream>
#include <memory>
#include <vector>
#include <limits>

#include "./library/User/buyer.h"
#include "./library/User/seller.h"
#include "./library/User/user.h"
#include "./library/Bank/bank.h"
#include "./library/Serialization/serialization.h"

using namespace std;

// Global variables
extern vector<shared_ptr<User>> users;
extern vector<Order> orders;
extern shared_ptr<User> currentUser;

Bank systemBank("Bank System");

// Enums for menu
enum PrimaryPrompt { LOGIN, REGISTER, EXIT };

shared_ptr<User> loginUser(); 
void handleRegister();
void handleLoginMenu();

int main() {
    loadAllData(users, orders);

    int choice;
    PrimaryPrompt prompt = LOGIN;

    do {
        cout << "-- Primary Menu --" << endl;
        cout << "1. Login" << endl;
        cout << "2. Register" << endl;
        cout << "3. Exit" << endl;
        cout << "Select an option: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }

        prompt = static_cast<PrimaryPrompt>(choice - 1);

        switch (prompt) {
            case LOGIN:
                currentUser = loginUser();
                if (currentUser) {
                    cout << "\nLogin Successful! Welcome, " << currentUser->getName() 
                         << " (" << currentUser->getRole() << ")." << endl;
                    handleLoginMenu();
                } else {
                    cout << "User not found. Login Failed.\n";
                }
                break;
        case REGISTER:
            handleRegister();
            break;
        case EXIT:
            cout << "Exiting program..." << endl;
            saveAllData(users, orders);
            return 0;
        default:
            cout << "Invalid option." << endl;
        }
    } while (prompt != EXIT);

    return 0;
}

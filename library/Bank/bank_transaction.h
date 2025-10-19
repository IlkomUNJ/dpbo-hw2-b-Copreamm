#include <chrono>
#include <string>
#include <vector>

class BankTransaction {
public:
    std::chrono::system_clock::time_point timestamp;
    int accountId;
    std::string type;
    double amount;
    std::string description;

    static std::vector<BankTransaction> loadFromFile(const std::string& filename);

    std::string toCSV() const;
    static BankTransaction fromCSV(const std::vector<std::string>& tokens);
};
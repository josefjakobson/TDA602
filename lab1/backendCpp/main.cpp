#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <stdexcept>

// Simulated global wallet and pocket
std::atomic<int> global_wallet_balance(30000);  // Starting balance
std::mutex wallet_mutex;
std::vector<std::string> global_pocket;
std::mutex pocket_mutex;

thread_local int thread_id = 0; // Thread-local variable to store thread ID

class Store {
private:
    static const std::vector<std::pair<std::string, int>>& getProducts() {
        static const std::vector<std::pair<std::string, int>> products = {
            {"candies", 1},
            {"car", 30000},
            {"pen", 40},
            {"book", 100}
        };
        return products;
    }

public:
    static int getProductPrice(const std::string& product) {
        const auto& products = getProducts();
        for (const auto& p : products) {
            if (p.first == product) return p.second;
        }
        throw std::invalid_argument("Product " + product + " is not in store");
    }

    static std::string asString() {
        const auto& products = getProducts();
        std::ostringstream oss;
        for (const auto& p : products) {
            oss << p.first << "\t" << p.second << "\n";
        }
        return oss.str();
    }
};


class Pocket {
public:
    void addProduct(const std::string& product) {
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock Pocket's addProduct()...\n";
        std::lock_guard<std::mutex> lock(pocket_mutex);
        std::cout << "[Thread " << thread_id << "]" << "addProduct() locked.\n";
        global_pocket.push_back(product);
        std::cout << "[Thread " << thread_id << "]" << "addProduct() unlocked.\n";
    }

    std::string getPocket() {
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock Pocket's getPocket()...\n";
        std::lock_guard<std::mutex> lock(pocket_mutex);
        std::cout << "[Thread " << thread_id << "]" << "getPocket() locked.\n";
        std::ostringstream oss; // Using ostringstream to build the string for the pocket
        for (const std::string& item : global_pocket) {
            oss << item << "\n"; // Appending each item to the string
        }
        std::cout << "[Thread " << thread_id << "]" << "getPocket() unocked.\n";
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Showing output
        return oss.str(); // Return the constructed string, e.g.,"car\nbook\n"
    }
};


class Wallet {
public:
    int getBalance() {
        return global_wallet_balance.load();
    }

    bool safeWithdraw(int amount) {
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock safeWithdraw()...\n";
        std::lock_guard<std::mutex> lock(wallet_mutex);
        std::cout << "[Thread " << thread_id << "]" << "safeWithdraw() locked.\n";
        int balance = global_wallet_balance.load();
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time

        if (amount > balance) {
            std::cout << "[Thread " << thread_id << "]" << "Purchase failed! Insufficient funds.\n";
            std::cout << "[Thread " << thread_id << "]" << "safeWithdraw unlocked().\n";
            return false;
        }

        int newBalance = balance - amount;
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        global_wallet_balance.store(newBalance);
        std::cout << "[Thread " << thread_id << "]" << "Purchase successful! New balance: " << newBalance << "\n";
        std::cout << "[Thread " << thread_id << "]" << "safeWithdraw unlocked().\n";
        return true;
    }
};

void shoppingCartSession() {
    Wallet wallet;
    Pocket pocket;
    std::cout << "[Thread " << thread_id << "]" << "Your current balance is: " << wallet.getBalance() << " credits.\n";
    std::cout << "[Thread " << thread_id << "]\n" << Store::asString();
    if (wallet.getBalance() <= 0) {
        std::cout << "[Thread " << thread_id << "]" << "You have no credits left.\n";
        return;
    }
    if (global_pocket.empty()) std::cout << "[Thread " << thread_id << "]" << "Your pocket is empty.\n";
    else std::cout << "[Thread " << thread_id << "]" << "Your current pocket is:\n" << pocket.getPocket();

    std::string product;
    while (true) {
        std::cout << "[Thread " << thread_id << "]" << "What do you want to buy? (type quit to stop) \n";
        std::getline(std::cin, product);

        if (product == "quit") break;

        std::cout << "[Thread " << thread_id << "]" << "Starting a transaction...\n";
        int price = Store::getProductPrice(product);
        bool success = wallet.safeWithdraw(price);
        if (!success) {
            std::cout << "[Thread " << thread_id << "]" << "Transaction incomplete.\n";
            break;
        }
        pocket.addProduct(product);
        std::cout << "[Thread " << thread_id << "]" << "Your current balance is: " << wallet.getBalance() << " credits.\n";
        std::cout << "[Thread " << thread_id << "]" << "Your current pocket is:\n" << pocket.getPocket() << "\n";
        std::cout << "[Thread " << thread_id << "]" << "Transaction completed.\n";
    }
}

int main() {
    // You could run multiple threads to test race conditions here
    std::cout << "Starting first thread...\n";
    std::thread t1([]() {
        thread_id = 1; // Assign thread ID
        shoppingCartSession();
        });
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Scheduling first output, otherwise undefined behaviour in printing
    std::cout << "\n\nStarting second thread...\n";
    std::thread t2([]() {
        thread_id = 2; // Assign thread ID
        shoppingCartSession();
        });
    t1.join();
    t2.join();

    return 0;
}


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
std::atomic<int> global_wallet_balance(30000); // Simulated wallet balance
std::mutex wallet_mutex;                       // Mutex for wallet access
std::vector<std::string> global_pocket;        // Simulated pocket
std::mutex pocket_mutex;                       // Mutex for pocket access
std::mutex transaction_mutex;                  // Mutex for entire transactions

std::atomic<bool> locks(false);
std::atomic<bool> transaction_lock(false);

thread_local int thread_id = 0; // Thread-local variable to store thread ID

class Store
{
private:
    static const std::vector<std::pair<std::string, int>>& getProducts()
    {
        static const std::vector<std::pair<std::string, int>> products = {
            {"candies", 1},
            {"car", 30000},
            {"pen", 40},
            {"book", 100} };
        return products;
    }

public:
    /*
     * This function returns the price of a product.
     * It uses a static method to access the product list.
     * If the product is not found, it throws an exception.
     */
    static int getProductPrice(const std::string& product)
    {
        const auto& products = getProducts(); // Get the static product list using a reference
        for (const auto& p : products)
        {
            if (p.first == product)
                return p.second; // Return the price if product is found
        }
        throw std::invalid_argument("Product " + product + " is not in store");
    }

    /*
     * This function returns a string representation of the products in the store.
     * It uses an ostringstream to build the string efficiently.
     */
    static std::string asString()
    {
        const auto& products = getProducts();
        std::ostringstream oss;
        for (const auto& p : products)
        {
            oss << p.first << "\t" << p.second << "\n";
        }
        return oss.str();
    }
};

class Pocket
{
public:
    /*
     * This function adds a product to the pocket.
     * It uses a mutex to ensure thread safety.
     */
    void addProduct(const std::string& product)
    {
        if (locks) {
            std::cout << "[Thread " << thread_id << "] " << "Trying to lock Pocket's addProduct()...\n";
            pocket_mutex.lock();
            std::cout << "[Thread " << thread_id << "] " << "addProduct() locked.\n";
        }
        global_pocket.push_back(product);
        if (locks) {
            std::cout << "[Thread " << thread_id << "] " << "addProduct() unlocked.\n";
            pocket_mutex.unlock();
        }
    }

    /*
     * This function retrieves the contents of the pocket.
     * It uses a mutex to ensure thread safety.
     * It returns a string representation of the pocket contents.
     */
    std::string getPocket()
    {
        if (locks) {
            std::cout << "[Thread " << thread_id << "] " << "Trying to lock Pocket's getPocket()...\n";
            pocket_mutex.lock();
            std::cout << "[Thread " << thread_id << "] " << "getPocket() locked.\n";
        }
        
        std::ostringstream oss; // Using ostringstream to build the string for the pocket
        for (const std::string& item : global_pocket)
        {
            oss << item << " "; // Appending each item to the string
        }
        if (locks) std::cout << "[Thread " << thread_id << "] " << "getPocket() unocked.\n";
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Showing output
        if (locks) pocket_mutex.unlock();
        return oss.str();                                     // Return the constructed string
    }
};

class Wallet
{
public:
    int getBalance() { return global_wallet_balance.load(); }
    bool safeWithdraw(int amount)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        if (locks){
            std::cout << "[Thread " << thread_id << "] " << "Trying to lock safeWithdraw()...\n";
            wallet_mutex.lock();
            std::cout << "[Thread " << thread_id << "] " << "safeWithdraw() locked.\n";
        }
        int balance = getBalance();
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time

        if (amount > balance)
        {
            std::cout << "[Thread " << thread_id << "] " << "Purchase failed! Insufficient funds.\n";
            if (locks) {
                std::cout << "[Thread " << thread_id << "] " << "safeWithdraw() unlocked.\n";
                wallet_mutex.unlock();
            }
            return false;
        }

        int newBalance = balance - amount;
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        global_wallet_balance.store(newBalance);
        std::cout << "[Thread " << thread_id << "] " << "Purchase successful! New balance: " << newBalance << "\n";
        if (locks) {
            std::cout << "[Thread " << thread_id << "] " << "safeWithdraw() unlocked.\n";
            wallet_mutex.unlock();
        }
        return true;
    }
};

/*
 * This function simulates a shopping cart session.
 * It creates a wallet and a pocket for the user.
 * It displays the current balance and available products.
 * It allows the user to buy products until they type "quit".
 * It uses a mutex to ensure thread safety when accessing the wallet and pocket.
 */
void shoppingCartSession()
{
    Wallet wallet;
    Pocket pocket;
    std::cout << "[Thread " << thread_id << "] " << "Your current balance is: " << wallet.getBalance() << " credits.\n";
    std::cout << "[Thread " << thread_id << "] Available Items: \n"
        << Store::asString();
    if (wallet.getBalance() <= 0)
    {
        std::cout << "[Thread " << thread_id << "] " << "You have no credits left.\n";
        return;
    }
    if (global_pocket.empty())
        std::cout << "[Thread " << thread_id << "] " << "Your pocket is empty.\n";
    else
        std::cout << "[Thread " << thread_id << "] " << "Your current pocket is: " << pocket.getPocket();

    std::string product;
    while (true)
    {
        std::cout << "[Thread " << thread_id << "] " << "What do you want to buy? (type quit to stop) \n";
        std::getline(std::cin, product);

        if (product == "quit")
            break;

        if (transaction_lock) {
            std::cout << "[Thread " << thread_id << "] Attempting to start a transcation. \n";
            transaction_mutex.lock();
        }
        std::cout << "[Thread " << thread_id << "] " << "Starting a transaction...\n";
        int price = Store::getProductPrice(product);
        bool success = wallet.safeWithdraw(price);
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        if (!success)
        {
            std::cout << "[Thread " << thread_id << "] " << "... transaction incomplete. X\n";
            if (transaction_lock) transaction_mutex.unlock();
            break;
        }
        pocket.addProduct(product);
        std::cout << "[Thread " << thread_id << "] " << "Your current balance is: " << wallet.getBalance() << " credits.\n";
        std::cout << "[Thread " << thread_id << "] " << "Your current pocket is: " << pocket.getPocket() << "\n";
        std::cout << "[Thread " << thread_id << "] " << "Your current wallet is: " << wallet.getBalance() << "\n";
        std::cout << "[Thread " << thread_id << "] " << "... transaction completed. :)\n";
        if (transaction_lock) transaction_mutex.unlock();
    }
}

int main()
{
    std::cout << "Enter required thread safety\n";
    std::cout << "0. None. \n";
    std::cout << "1. Wallet & pocket locks.\n";
    std::cout << "2. Transaction + Wallet & pocket locks.\n\n";
    std::string choice;
    while (true) {
        std::getline(std::cin, choice);
        if (choice == "0") break;
        else if (choice == "1") {
            locks = true;
            break;
        }
        else if (choice == "2") {
            locks = true;
            transaction_lock = true;
            break;
        }
        else std::cout << "Enter 0, 1 or 2.\n";
    }

    std::cout << "Starting first thread...\n";
    std::thread t1([]() {      // Lambda function to create a thread
        thread_id = 1;         // Assign thread ID
        shoppingCartSession(); // Call the shopping cart session function for the first thread
        });
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Scheduling first output, otherwise undefined behaviour in printing

    std::cout << "\n\nStarting second thread...\n";
    std::thread t2([]()
        {
            thread_id = 2;
            shoppingCartSession(); 
       });
    t1.join();
    t2.join();

    return 0;
}

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

thread_local int thread_id = 0; // Thread-local variable to store thread ID

class Store
{
private:
    static const std::vector<std::pair<std::string, int>> &getProducts()
    {
        static const std::vector<std::pair<std::string, int>> products = {
            {"candies", 1},
            {"car", 30000},
            {"pen", 40},
            {"book", 100}};
        return products;
    }

public:
    /*
     * This function returns the price of a product.
     * It uses a static method to access the product list.
     * If the product is not found, it throws an exception.
     */
    static int getProductPrice(const std::string &product)
    {
        const auto &products = getProducts(); // Get the static product list using a reference
        for (const auto &p : products)
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
        const auto &products = getProducts();
        std::ostringstream oss;
        for (const auto &p : products)
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
    void addProduct(const std::string &product)
    {
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock Pocket's addProduct()...\n";
        std::lock_guard<std::mutex> lock(pocket_mutex);
        std::cout << "[Thread " << thread_id << "]" << "addProduct() locked.\n";
        global_pocket.push_back(product);
        std::cout << "[Thread " << thread_id << "]" << "addProduct() unlocked.\n";
    }
    /*
     * This function retrieves the contents of the pocket.
     * It uses a mutex to ensure thread safety.
     * It returns a string representation of the pocket contents.
     */
    std::string getPocket()
    {
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock Pocket's getPocket()...\n";
        std::lock_guard<std::mutex> lock(pocket_mutex);
        std::cout << "[Thread " << thread_id << "]" << "getPocket() locked.\n";
        std::ostringstream oss; // Using ostringstream to build the string for the pocket
        for (const std::string &item : global_pocket)
        {
            oss << item << "\n"; // Appending each item to the string
        }
        std::cout << "[Thread " << thread_id << "]" << "getPocket() unocked.\n";
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Showing output
        return oss.str();                                     // Return the constructed string, e.g.,"car\nbook\n"
    }
};

class Wallet
{
public:
    /*
     * This function returns the current balance of the wallet.
     * It uses an atomic variable to ensure thread safety.
     */
    int getBalance()
    {
        return global_wallet_balance.load();
    }

    /*
     * This function attempts to withdraw an amount from the wallet.
     * It uses a mutex to ensure thread safety.
     * It simulates a delay to show the locking mechanism in action.
     * It returns true if the withdrawal is successful, false otherwise.
     */
    bool safeWithdraw(int amount)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time
        std::cout << "[Thread " << thread_id << "]" << "Trying to lock safeWithdraw()...\n";
        std::lock_guard<std::mutex> lock(wallet_mutex);
        std::cout << "[Thread " << thread_id << "]" << "safeWithdraw() locked.\n";
        int balance = global_wallet_balance.load();
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate some processing time

        if (amount > balance)
        {
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
    std::cout << "[Thread " << thread_id << "]" << "Your current balance is: " << wallet.getBalance() << " credits.\n";
    std::cout << "[Thread " << thread_id << "]\n"
              << Store::asString();
    if (wallet.getBalance() <= 0)
    {
        std::cout << "[Thread " << thread_id << "]" << "You have no credits left.\n";
        return;
    }
    if (global_pocket.empty())
        std::cout << "[Thread " << thread_id << "]" << "Your pocket is empty.\n";
    else
        std::cout << "[Thread " << thread_id << "]" << "Your current pocket is:\n"
                  << pocket.getPocket();

    std::string product;
    while (true)
    {
        std::cout << "[Thread " << thread_id << "]" << "What do you want to buy? (type quit to stop) \n";
        std::getline(std::cin, product);

        if (product == "quit")
            break;

        std::cout << "[Thread " << thread_id << "]" << "Starting a transaction...\n";
        int price = Store::getProductPrice(product);
        bool success = wallet.safeWithdraw(price);
        if (!success)
        {
            std::cout << "[Thread " << thread_id << "]" << "Transaction incomplete.\n";
            break;
        }
        pocket.addProduct(product);
        std::cout << "[Thread " << thread_id << "]" << "Your current balance is: " << wallet.getBalance() << " credits.\n";
        std::cout << "[Thread " << thread_id << "]" << "Your current pocket is:\n"
                  << pocket.getPocket() << "\n";
        std::cout << "[Thread " << thread_id << "]" << "Transaction completed.\n";
    }
}

/*
* Use preferred IDE to compile and run the program, such as Visual Studio or CLion.
* This is the main function that starts the program.
* To demonstrate the thread safety of the shopping cart session,
* it creates two threads that run the shopping cart session concurrently.
* It uses thread-local storage to keep track of the thread ID.

* If unsafe behaviour required for testing our code, comment out all the lock_guards.
*/
int main()
{

    std::cout << "Starting first thread...\n";
    std::thread t1([]() {      // Lambda function to create a thread
        thread_id = 1;         // Assign thread ID
        shoppingCartSession(); // Call the shopping cart session function for the first thread
    });
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Scheduling first output, otherwise undefined behaviour in printing

    std::cout << "\n\nStarting second thread...\n";
    std::thread t2([]()
                   {
        thread_id = 2; // Assign thread ID
        shoppingCartSession(); });
    t1.join();
    t2.join();

    return 0;
}

use std::sync::{Arc, Mutex};

/// A thread-safe wallet for storing wallet balance. Used instead of the text file.
/// Derives clone to allow multiple threads to share the same wallet instance.
/// The wallet is implemented using Arc<Mutex<>> to ensure that the balance are
/// guarded from race conditions and can be shared safely across threads.
#[derive(Clone)]
pub struct Wallet {
    global_balance: Arc<Mutex<i32>>,
}

impl Wallet {
    /// Creates a new, empty Wallet.
    pub fn new() -> Self {
        Wallet {
            global_balance: Arc::new(Mutex::new(30000)),
        }
    }
    /// Returns the current balance of the wallet.
    pub fn get_balance(&self) -> i32 {
        // Requests a lock on the balance int, which blocks until the lock is available.
        let balance = self.global_balance.lock().unwrap();

        // When lock has been acquired, derefrence balance MutexGuard to read content.
        return *balance
    }

    pub fn safe_withdraw(&self, value_to_withdraw: i32) -> bool {
        // pause thread for 1 second
        std::thread::sleep(std::time::Duration::from_secs(3));
        // Requests a lock on the balance int, which blocks until the lock is available.
        let mut balance = self.global_balance.lock().unwrap();
        std::thread::sleep(std::time::Duration::from_secs(3));

        if *balance >= value_to_withdraw {
            std::thread::sleep(std::time::Duration::from_secs(3));
            let new_balance = *balance - value_to_withdraw;
            *balance = new_balance;
            println!("Purchase Successful!");
            return true;
        } else {
            println!("Not enough money in wallet!");
            return false;
        }
    }

}
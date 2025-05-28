use std::sync::{Arc, Mutex};

/// A thread-safe pocket for storing product names. Used instead of the text file.
/// Derives clone to allow multiple threads to share the same pocket instance.
/// The pocket is implemented using Arc<Mutex<>> to ensure that pocket contents are
/// guarded from race conditions and can be shared safely across threads.
#[derive(Clone)]
pub struct Pocket {
    products: Arc<Mutex<Vec<String>>>,
}

impl Pocket {
    /// Creates a new, empty Pocket.
    pub fn new() -> Self {
        Pocket {
            products: Arc::new(Mutex::new(Vec::new())),
        }
    }

    // Adds a product to the pocket.
    pub fn add_product(&self, product: String) {
        println!("Adding product");
        // Requests a lock on the products vector, which blocks until the lock is available.
        let mut products = self.products.lock().unwrap();
        products.push(product);
        println!("Product added. Lock released.");
    } // Lock is automatically released when products goes out of scope.

    /// Returns the contents of the pocket as a single string.
    pub fn get_pocket(&self) -> String {
        // Waits for the lock to be available since Rust does not allow reading from Mutex variables w/o exclusive access
        let products = self.products.lock().unwrap();
        products.join(", ")
    }

}

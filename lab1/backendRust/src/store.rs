use std::collections::HashMap;

#[derive(Clone)]
pub struct Store {
    products: HashMap<String, i32>,
}

impl Store {
    pub fn new() -> Self {
        let mut products = HashMap::new();
        products.insert("candies".to_string(), 1);
        products.insert("car".to_string(), 30000);
        products.insert("pen".to_string(), 40);
        products.insert("book".to_string(), 100);

        Store { products }
    }

    /// Returns the product list as a String.
    pub fn as_string(&self) -> String {
        self.products
            .iter()
            .map(|(product, price)| format!("{}\t{}\n", product, price))
            .collect()
    }

    /// Returns the price of a product.
    pub fn get_product_price(&self, product: &str) -> i32 {
        self.products
            .get(product)
            .copied()
            .unwrap_or_else(|| panic!("Product {} is not in store", product))
    }
}

mod pocket;
mod wallet;
mod store;
use std::thread;
use std::io::{self};
use pocket::Pocket;
use wallet::Wallet;
use store::Store;


fn scan() -> io::Result<String> {
    let mut input = String::new();
    println!("What do you want to buy? (type quit to stop) ");
    io::stdin().read_line(&mut input)?;
    Ok(input.trim().to_string())
}


fn shoppingcart_session(thread_id: i32, wallet: Wallet, pocket: Pocket, store: Store) {
    println!("Thread {} started", thread_id);
    println!("Thread {}\n Wallet balance: {} \n Pocket: {:?}", thread_id, wallet.get_balance(), pocket.get_pocket());
    // read from stdin
    let mut input = scan().unwrap();

    while true {
        if input == "quit" {break;}
        let price: i32 = store.get_product_price(&input);
        let success: bool = wallet.safe_withdraw(price);
        if !success { break;}
        pocket.add_product(input.clone());
        println!("Thread {}: Added {} to pocket", thread_id, input.clone());
        println!("Thread {}: Wallet balance: {} \n Pocket: {:?}", thread_id, wallet.get_balance(), pocket.get_pocket());
        input = scan().unwrap();
    }
}

fn main() {
    let wallet = Wallet::new();
    let pocket = Pocket::new();
    let store = Store::new();
    
    let wallet1 = wallet.clone();
    let pocket1 = pocket.clone();
    let wallet2 = wallet.clone();
    let pocket2 = pocket.clone();
    let store1 = store.clone();
    let store2 = store.clone();

    println!("Product list:\n{}", store.as_string());
    let thread1 = thread::spawn(move || {
        shoppingcart_session(1, wallet1, pocket1, store1);
    });
    std::thread::sleep(std::time::Duration::from_secs(3));
    let thread2 = thread::spawn(move || {
        shoppingcart_session(2, wallet2, pocket2, store2);
    });
    thread1.join().unwrap();
    thread2.join().unwrap();


}

use std::fs::{File, OpenOptions};
use std::io::{self, BufRead, BufReader, Seek, SeekFrom, Write};
use std::path::Path;
use fs2::FileExt; // For file locking

pub struct Pocket {
    file: File,
}

impl Pocket {
    /// Creates a new Pocket object that interfaces with the pocket.txt file
    pub fn new() -> io::Result<Self> {
        let path = Path::new("backEnd/pocket.txt");
        std::fs::create_dir_all("backEnd")?;
        let file = OpenOptions::new().read(true).write(true).create(true).open(path)?;
        Ok(Pocket { file })
    }

    /// Adds a product to the pocket
    pub fn add_product(&mut self, product: &str) -> io::Result<()> {
        self.file.lock_exclusive()?; // Lock the file
        self.file.seek(SeekFrom::End(0))?;
        writeln!(self.file, "{}", product)?;
        self.file.unlock()?; // Unlock the file
        println!("Lock released.");
        Ok(())
    }

    /// Generates a string representation of the pocket
    pub fn get_pocket(&mut self) -> io::Result<String> {
        self.file.seek(SeekFrom::Start(0))?;
        let reader = BufReader::new(&self.file);
        let mut contents = String::new();
        for line in reader.lines() {
            contents.push_str(&line?);
            contents.push('\n');
        }
        Ok(contents)
    }

    /// Explicit close isn't needed in Rust; Drop will handle it,
    /// but here's a manual version if needed.
    pub fn close(self) {
        // File is closed when Pocket goes out of scope or explicitly dropped
    }
}

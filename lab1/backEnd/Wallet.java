package backEnd;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileLock;
import java.util.concurrent.TimeUnit;

public class Wallet {
    /**
     * The RandomAccessFile of the wallet file
     */
    private RandomAccessFile file;

    /**
     * Locking lock of thew wallet file
     */

    private FileLock lock;

    /**
     * Creates a Wallet object
     *
     * A Wallet object interfaces with the wallet RandomAccessFile
     */
    public Wallet () throws Exception {
        this.file = new RandomAccessFile(new File("backEnd/wallet.txt"), "rw");
    }

    /**
     * Gets the wallet balance.
     *
     * @return                   The content of the wallet file as an integer
     */
    public int getBalance() throws IOException {
        this.file.seek(0);
        return Integer.parseInt(this.file.readLine());
    }

    /**
     * Sets a new balance in the wallet
     *
     * @param  newBalance          new balance to write in the wallet
     */
    private void setBalance(int newBalance) throws Exception {
        this.file.setLength(0);
        String str = Integer.valueOf(newBalance).toString()+'\n';
        this.file.writeBytes(str);
    }

    /**
     * Closes the RandomAccessFile in this.file
     */
    public void close() throws Exception {
        this.file.close();
    }

    public boolean safeWithdraw(int valueToWithdraw) throws Exception {
        
        TimeUnit.SECONDS.sleep(3); // Fixed by the lock.
        try (FileLock lock = this.file.getChannel().lock()){
            int balance = this.getBalance();
            TimeUnit.SECONDS.sleep(3); // Fixed by the lock.

            if (valueToWithdraw > balance) {
                System.out.println("Purchase failed! Insufficient funds.");
                return false;
            }

            int newBalance = balance - valueToWithdraw;
            TimeUnit.SECONDS.sleep(3); // Fixed by the lock.
            this.setBalance(newBalance);
            System.out.println("Purchase successful! New balance: " + newBalance);
            return true;
        } finally {
            System.out.println("Lock released.");
        }
    }
}

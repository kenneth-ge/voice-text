import java.awt.*;
import java.awt.event.KeyEvent;

public class paste {
    public static void main(String[] args) throws AWTException, InterruptedException {
        Robot robot = new Robot();

        // Simulate pressing Ctrl+V
        robot.keyPress(KeyEvent.VK_CONTROL);
        robot.keyPress(KeyEvent.VK_V);

        // Introduce a short delay (e.g., 100 milliseconds) to ensure proper input handling
        Thread.sleep(100);

        // Release Ctrl+V
        robot.keyRelease(KeyEvent.VK_V);
        robot.keyRelease(KeyEvent.VK_CONTROL);
    }
}


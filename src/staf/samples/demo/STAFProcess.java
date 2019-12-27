/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;
import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.util.*;
import javax.swing.*;

public class STAFProcess extends Component
{
    // Data members
    private STAFHandle handle = null;
    private JFrame frame = null;
    private String machine = null;
    private String machine_name = null;
    private String the_title = null;
    private String the_message = null;
    private Font font = null;
    private FontMetrics fontmetrics = null;
    private STAFResult color = null;
    private STAFResult resource = null;
    private STAFResult monitor = null;
    private STAFResult semaphore = null;
    private STAFResult stop = null;

    public static void main(String[] args)
    {
        try
        {
            int corner = 0;
                // 0 is northwest corner
                // 1 is southwest corner
                // 2 is southeast corner
                // 3 is northeast corner

            if (args.length == 1)
            {
                corner = (new Integer(args[0]).intValue()) % 4;
            }

            STAFProcess process = new STAFProcess(
                "An Arbitrary Process", corner);
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public STAFProcess(String title, int corner)
    throws Exception
    {
        // Allocate a title and a message string
        the_title = new String();
        the_message = new String();

        frame = new JFrame(title);
        frame.getContentPane().add(this, "Center");
        frame.setSize(350, 200);
        frame.getContentPane().setForeground(Color.black);
        frame.getContentPane().setBackground(Color.white);

        Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();

        switch (corner)
        {
            case 0:
                frame.setLocation(0, 0);
                break;
            case 1:
                frame.setLocation(0, dim.height - frame.getHeight() - 30);
                break;
            case 2:
                frame.setLocation(dim.width - frame.getWidth(),
                    dim.height - frame.getHeight() - 30);
                break;
            case 3:
                frame.setLocation(dim.width - frame.getWidth(), 0);
                break;
        }

        frame.setVisible(true);

        frame.addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                terminate();
            }
        });

        // Get font metrics to position message on screen
        font = new Font("Helvetica", Font.PLAIN, 20);
        this.setFont(font);
        fontmetrics = getFontMetrics(font);

        // Set machine name
        machine = new String("local");

        String osName = System.getProperties().getProperty("os.name");

        if (osName.equals("Windows 2000"))
        {
            frame.setState(JFrame.ICONIFIED);
            frame.setState(JFrame.NORMAL);
        }
        else
        {
            frame.toFront();
        }

        // Start running!!!
        run();
    }

    public void run()
    {
        try
        {
            // Register with STAF
            handle = new STAFHandle("STAF_Demo");
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF RC: " + e.rc);
            terminate();
        }

        int h = handle.getHandle();
        int counter = 0;

        // Append the handle to the frame's title
        frame.setTitle("An Arbitrary Process: Handle " + h);

        // Set request strings for the different services
        
        String background_color_var = new String(
            "RESOLVE STRING {STAFDemo/BackgroundColor}");
        String continue_semaphore = new String(
            "WAIT EVENT STAFDemo/Handle/" + h + "/Continue");
        String items_respool = new String("REQUEST POOL STAFDemo RANDOM");
        String mesg_queue = new String("GET TYPE STAF/STAFDemo/Stop");

        // mesg_log is a partially constructed request, needs "level <level>
        // message <message>"
        String mesg_log = new String("LOG HANDLE LOGNAME STAFDemo ");

        // Now let's get serious
        writeMessage("Loop " + String.valueOf(counter),
                     "Now, wait on semaphore");

        // Used inside loop
        String previous_color = null;

        // Loop forever
        while (true)
        {
            // Refresh the frame
            repaint();

            String msgHeader = "Loop " + String.valueOf(counter) + ": ";

            // Use monitor as a checkpoint
            monitor = handle.submit2(
                machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                    msgHeader + "Checking Queue"));

            // Check queue for stop message
            stop = handle.submit2(machine, "queue", mesg_queue);

            if (stop != null && stop.rc == STAFResult.Ok)
            {
                // Break from the loop
                break;
            }

            // Use monitor as a checkpoint
            monitor = handle.submit2(
                machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                    msgHeader + "Checking Semaphore"));

            // Block if semaphore is reset, fall through if posted (or
            // if error!!!)
            semaphore = handle.submit2(machine, "SEM", continue_semaphore);

            // Use monitor as a checkpoint
            monitor = handle.submit2(
                machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                    msgHeader + "Resolving Color"));

            // Get the background color, save the old one
            previous_color = (color == null ? null : new String(color.result));
            color = handle.submit2(machine, "VAR", background_color_var);

            if (color != null && color.rc == STAFResult.Ok)
            {
                // If color changed, log an informational message
                if (previous_color != null &&
                    !previous_color.equals(color.result))
                {
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Changing Color"));
                }

                // Set background according to color
                frame.getContentPane().setBackground(
                    getColorFromString(color.result));
            }
            else
            {
                // Set background to default (white)
                frame.getContentPane().setBackground(Color.white);
            }

            // Every 7th iteration, switch resource in use
            if (counter % 7 == 0)
            {
                // Release it only if we have it
                if (resource != null)
                {
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Releasing Resource"));

                    handle.submit2(
                        "{STAFDemo/ResourcePoolMachine}",
                        "RESPOOL", "RELEASE POOL STAFDemo ENTRY " +
                        resource.result);
                }

                // Update message
                writeMessage("Loop " + String.valueOf(counter),
                             "Grabbing a Resource");

                // Use monitor as a checkpoint
                monitor = handle.submit2(
                    machine, "MONITOR", "LOG MESSAGE \"Loop " +
                    String.valueOf(counter)+": Grabbing Resource\"");

                // Block until resource is available
                resource = handle.submit2(
                    "{STAFDemo/ResourcePoolMachine}", "RESPOOL",
                    items_respool);
            }

            if (resource != null && resource.rc == STAFResult.Ok)
            {
                // Set message to be displayed
                writeMessage("Loop " + String.valueOf(counter),
                             "Using : " + resource.result);
            }
            else
            {
                // Indicate no resources found
                writeMessage("Loop " + String.valueOf(counter),
                             "No resources found");
            }

            // Randomly log an error, warning, debug or information message
            switch (counter % 7)
            {
                // Log an error message to the monitor and log file
                case 1:
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Logging Error"));

                    // Do the logging
                    handle.submit2(
                        machine, "LOG", mesg_log + "LEVEL ERROR MESSAGE " +
                        STAFUtil.wrapData(msgHeader + "Error Message"));

                    break;

                // Log a warning message to the monitor and log file
                case 2:
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Logging Warning"));

                    // Do the logging
                    handle.submit2(
                        machine, "LOG", mesg_log + "LEVEL warning MESSAGE " +
                        STAFUtil.wrapData(msgHeader + "Warning Message"));

                    break;

                // Log an info message to the monitor and log file
                case 3:
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Logging Info"));

                    // Do the logging
                    handle.submit2(
                        machine, "LOG", mesg_log + "LEVEL info MESSAGE " +
                        STAFUtil.wrapData(msgHeader + "Info Message"));

                    break;

                // Log a debug message to the monitor and log file
                case 4:
                    // Use monitor as a checkpoint
                    monitor = handle.submit2(
                        machine, "MONITOR", "LOG MESSAGE " + STAFUtil.wrapData(
                            msgHeader + "Logging Debug"));

                    // Do the logging
                    handle.submit2(
                        machine, "LOG", mesg_log + "LEVEL debug MESSAGE " +
                        STAFUtil.wrapData(msgHeader + "Debug message"));

                    break;

                default:
                    break;
            }

            // Sleep for 1 second before looping again
            handle.submit2(machine, "DELAY", "DELAY 1000");
            counter++;
        }

        writeMessage("Loop " + String.valueOf(counter), "Terminating ...");
        terminate();
    }

    public void writeMessage(String title, String mesg)
    {
        the_title = title;
        the_message = mesg;
        repaint();
    }

    public void paint(Graphics g)
    {
        // Paint message
        if (g != null && frame != null && fontmetrics != null)
        {
            int x_t = frame.getSize().width / 2 - fontmetrics.stringWidth(
                the_title) / 2;
            int x_m = frame.getSize().width / 2 - fontmetrics.stringWidth(
                the_message) / 2;
            int y = frame.getSize().height / 2;
            g.drawString(the_title, x_t, y - 1 * (fontmetrics.getHeight()));
            g.drawString(the_message, x_m, y);
              // + 1 * (fontmetrics.getHeight()));
        }
    }

    public void update(Graphics g)
    {
        if (g != null) paint(g);
    }

    public void sleepProcess(int secs)
    {
        try
        {
            Thread.sleep(secs * 1000);
        }
        catch(InterruptedException e)
        {
            /* Do nothing */
        }
    }

    public Color getColorFromString(String colorName)
    {
        colorName = colorName.toLowerCase();

        if (colorName.equals("black"))  return Color.black;
        if (colorName.equals("blue"))   return Color.blue;
        if (colorName.equals("cyan"))   return Color.cyan;
        if (colorName.equals("darkgray"))   return Color.darkGray;
        if (colorName.equals("gray"))   return Color.gray;
        if (colorName.equals("green"))  return Color.green;
        if (colorName.equals("lightgray"))  return Color.lightGray;
        if (colorName.equals("magenta"))    return Color.magenta;
        if (colorName.equals("orange")) return Color.orange;
        if (colorName.equals("pink"))   return Color.pink;
        if (colorName.equals("red"))    return Color.red;
        if (colorName.equals("white"))  return Color.white;
        if (colorName.equals("yellow")) return Color.yellow;
        return Color.white;
    }

    public void terminate()
    {
        try
        {
            if (handle != null)
            {
                // Use monitor as a checkpoint
                handle.submit2(
                    machine, "MONITOR", "LOG MESSAGE Terminating");

                // Releease resource
                if (resource != null)
                    handle.submit2(
                        "{STAFDemo/ResourcePoolMachine}",
                        "RESPOOL", "RELEASE POOL STAFDemo ENTRY " +
                        resource.result);

                // Delete semaphore
                handle.submit2(
                    machine, "SEM", "DELETE EVENT STAFDemo/handle/" + 
                    handle.getHandle() + "/Continue");

                // Do the logging
                handle.submit2(
                    machine, "LOG", "LOG HANDLE LOGNAME STAFDemo LEVEL info " +
                    "MESSAGE Terminating");

                // Unregister
                handle.unRegister();
            }
        }
        catch(STAFException e)
        {
            /* Do nothing */
        }
        finally
        {
            System.exit(0);
        }
    }
}

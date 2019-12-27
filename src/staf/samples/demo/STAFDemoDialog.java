/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;

class STAFDemoDialog extends JFrame implements ActionListener, Runnable
{
    private STAFHandle fHandle;
    private boolean fDialogCreated = false;
    private String fType;
    private String fMessage;
    private JTextPane fMessagePane;
    private String kNewMessage = "newMessage=";
    private String kAppendMessage = "appendMessage=";
    private String kLocation = "location=";
    private String kSize = "size=";
    private String kDemoEnd = "STAFDemoDialog/End";

    JButton ok = new JButton("OK");

    public static void main(String argv[])
    {
        new STAFDemoDialog(argv);
    }
    
    public STAFDemoDialog(String argv[])
    {
        setTitle("STAF Demo");
        
        try
        {
            // register with STAF
            fHandle = new STAFHandle("STAFDemoDialog");
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF");
            System.exit(1);
        }
        
        this.run();
    }
    
    public void actionPerformed(ActionEvent e) 
    {
        if (e.getSource() == ok)
        {
            STAFResult stafResult = fHandle.submit2(
                "local", "SEM", "POST EVENT STAFDemoDialog/Continue");

            if (stafResult.rc != STAFResult.Ok) 
            {
                System.out.println(
                    "Error posting STAFDemoDialog/Continue semaphore");
            }
        }
    }

    public void dialogInit()
    {
        fMessagePane = new JTextPane();                                                
        fMessagePane.setFont(new Font("Dialog", Font.BOLD, 12));
        fMessagePane.setEditable(false);
        fMessagePane.select(0,0);
        
        JScrollPane scrollPane = new JScrollPane(fMessagePane);
        
        JPanel mainPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout());
        mainPanel.add(BorderLayout.CENTER, scrollPane);
        
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));
        buttonPanel.add(ok);
        ok.addActionListener(this);

        mainPanel.add(BorderLayout.SOUTH, buttonPanel);
                    
        setSize(new Dimension(400, 250));
        
        getContentPane().add(mainPanel);
        
        Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
        int x = dim.width/2 - getWidth()/2;
        int y = dim.height/2 - getHeight()/2;

        setLocation(x > 0 ? x : 0, y > 0 ? y : 0);

        setVisible(true);
        
        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                dispose();
                System.exit(0);
            }
        });
             
        String osName = System.getProperties().getProperty("os.name");

        if (osName.equals("Windows 2000")) 
        {
            setState(JFrame.ICONIFIED);
            setState(JFrame.NORMAL);
        }
        else 
        {
            toFront();
        }
    }
    
    public void run()
    {    
        STAFResult queueGetResult;
        boolean continueRunning = true;

        while (continueRunning)
        {
            queueGetResult = fHandle.submit2("local", "QUEUE", "GET WAIT");
            
            if (queueGetResult.rc != STAFResult.Ok)
            {
                // Ignore errors from QUEUE GET WAIT requst
                continue;
            }
                
            // Unmarshall the result from a QUEUE GET request

            STAFMarshallingContext outputContext =
                STAFMarshallingContext.unmarshall(queueGetResult.result);

            Map queueMap = (Map)outputContext.getRootObject();

            fType = (String)queueMap.get("type");
            fMessage = (String)queueMap.get("message"); 
            
            if (fMessage.startsWith(kDemoEnd))
            {
                continueRunning = false;                
            }            
            else if (fMessage.startsWith(kNewMessage) || 
                     fMessage.startsWith(kAppendMessage))
            {
                if (!fDialogCreated)
                {
                    dialogInit();
                    fDialogCreated = true;
                }
                    
                if (fMessage.startsWith(kNewMessage))
                {
                    fMessage = fMessage.substring(kNewMessage.length());
                    
                    fMessagePane.setText(fMessage);
                }
                else if (fMessage.startsWith(kAppendMessage))
                {
                    fMessage = fMessage.substring(kAppendMessage.length());
                  
                    fMessagePane.setText(fMessagePane.getText() + 
                        "\n\n" + fMessage);
                }
                                    
                invalidate();
            }
            else if (fMessage.startsWith(kLocation))
            {
                fMessage = fMessage.substring(kLocation.length());
                   
                if (fMessage.equalsIgnoreCase("center"))
                {
                    Dimension dim = 
                        Toolkit.getDefaultToolkit().getScreenSize();
                            
                    int x = dim.width/2 - getWidth()/2;
                    int y = dim.height/2 - getHeight()/2;

                    setLocation(x > 0 ? x : 0, y > 0 ? y : 0);
                }
                else
                {
                    int index = fMessage.indexOf(",");
                   
                    String x = fMessage.substring(0, index);
                    String y = fMessage.substring(index + 1);
                    
                    setLocation((new Integer(x)).intValue(), 
                        (new Integer(y)).intValue());
                }
                    
                invalidate();
                validate();
            }
            else if (fMessage.startsWith(kSize))
            {
                fMessage = fMessage.substring(kSize.length());
                    
                if (fMessage.equalsIgnoreCase("default"))
                {
                    setSize(new Dimension(400, 250));
                }
                else
                {
                    int index = fMessage.indexOf(",");
                    
                    String width = fMessage.substring(0, index);
                    String height = fMessage.substring(index + 1);
                    
                    setSize(new Dimension((new Integer(width)).intValue(), 
                                          (new Integer(height)).intValue()));
                }
                    
                invalidate();
                validate();
            }
        }
        
        dispose();
        System.exit(0);      
    }
}

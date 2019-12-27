import com.ibm.staf.*;
import java.util.*;
import java.io.*;

public class ReturnTest implements Runnable
{    
    private STAFHandle handle = null;    
    private int counter;
    private int loopCounter;
    private int incrementSeconds;
    private int returnCode;
            
    public static void main(String[] args)
    {
        try
        {
            ReturnTest returnTest = new ReturnTest();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public ReturnTest()    
    {
        this.run();
    }

    public void run()
    {

        try
        {
          // Write to a file called returnfile1 in the current directory

          DataOutputStream out = new DataOutputStream(
                                     new FileOutputStream("returnfile1"));
          out.writeBytes("Here is some data written to returnfile1.\n");
          out.writeBytes("End of data written to returnfile1.\n");
          out.close(); 

          // Write to a file called returnfile2 in the current directory

          DataOutputStream out1 = new DataOutputStream(
                                     new FileOutputStream("returnfile2"));
          out1.writeBytes("Here is some data written to returnfile2.\n");
          out1.writeBytes("End of data written to returnfile2.\n");
          out1.close(); 

          // Write to a file called returnfile3 in the current directory

          DataOutputStream out2 = new DataOutputStream(
                                     new FileOutputStream("returnfile3"));
          out2.writeBytes("Here is some data written to returnfile3.\n");
          out2.writeBytes("End of data written to returnfile3.\n");
          out2.close(); 
          
          System.out.println("Here is some data written to the standard out file.");
          System.err.println("Here is some data written to the standard error file.");
          System.out.println("Here is some more data.");
          System.err.println("End of data written to standard error.");
          System.out.println("Is this the bad data? `Hi there'");
          System.out.println("Is this the bad data? ;Hi;\n;\\n::");
          System.out.println("tcc /usr/local/staf/temp.out");
          Thread.sleep(5000);
          System.err.println("Is this the bad data? `Hi there\"");
          System.out.println("		There's a tab before here");
          System.out.println("End of data written to standard out.");
          //Thread.sleep(1000);



        }
        catch(Exception e)
        {
            System.out.println("Error writing to a file");
            e.printStackTrace();
            System.exit(1);
        }        
        
        System.exit(0);
    }      
}
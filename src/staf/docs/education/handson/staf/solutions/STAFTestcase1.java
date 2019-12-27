import com.ibm.staf.*;
public class STAFTestcase1
{
    public static void main(String[] args)
    {
        int counter = 10;  
        STAFHandle handle = null;
        try
        {
            handle = new STAFHandle("STAFTestcase");
        }
        catch(STAFException e)
        {
            e.printStackTrace();  
            System.exit(1);
        }
        if (args.length > 0) counter = (new Integer(args[0])).intValue();
        for (int i=0; i < counter; i++)
        {
            System.out.println("Loop #" + i);
            STAFResult result = handle.submit2("local", "monitor", 
                "log message " + 
                STAFUtil.wrapData("Loop #" + i));
            STAFResult logResult = handle.submit2("local", "log", 
                "log machine logname STAFTestcase1.log level info message " + 
                STAFUtil.wrapData("Loop #" + i));    
            try
            {
                Thread.sleep(1000); // 1 second
            }
            catch(InterruptedException e)
            {
                e.printStackTrace();
            }                    
        }
        System.exit(0);
    }
}
import com.ibm.staf.*;
public class STAFTestcase4
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
            STAFResult queueResult = handle.submit2("local", "queue", 
                "get wait 1000");
            if ((queueResult.rc == 0) && !(queueResult.result.equals(null)))
            {
                STAFQueueMessage message = new STAFQueueMessage(queueResult.result);
                if (message.message.equals("STAFTestcase4/Terminate"))
                {
                    System.exit(0);
                }
            }
        }
        System.exit(0);
    }
}
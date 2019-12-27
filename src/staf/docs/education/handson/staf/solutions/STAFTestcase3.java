import com.ibm.staf.*;
public class STAFTestcase3
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
            STAFResult delayResult = handle.submit2("local", "delay", 
                "delay 1000");
            STAFResult varResult = handle.submit2("local", "var", 
                "resolve {HandsOn/Variables/MyVar}");
            if ((varResult.rc == 0) && (varResult.result.equals("STAFTestcase3/Terminate")))
            {
                System.exit(0);
            }
        }
        System.exit(0);
    }
}
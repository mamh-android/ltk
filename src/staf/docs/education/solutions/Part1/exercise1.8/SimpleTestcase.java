public class SimpleTestcase
{
    public static void main(String[] args)
    {
        int counter = 10;
        if (args.length > 0) counter = (new Integer(args[0])).intValue();
        for (int i=0; i < counter; i++)
        {
            System.out.println("Loop #" + String.valueOf(i));                       
            try
            {
                Thread.sleep(1000); // 1 second
            }
            catch(InterruptedException e)
            {
                e.printStackTrace();
            }                    
        }
    }
}
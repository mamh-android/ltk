import com.ibm.staf.*;
import java.util.*;
import java.text.DecimalFormat;

public class MarshallingTest
{
    public static void main(String [] args)
    {
        // First without any map-class

        List logList = new LinkedList();

        for (int i = 1; i < 31; ++i)
        {
            Map logRecord = new HashMap();

            logRecord.put("level", "info");
            logRecord.put("message", "Message #" + i);
            logRecord.put("machine", "crankin3.austin.ibm.com");

            DecimalFormat nf = new DecimalFormat("00");
            GregorianCalendar cal = new GregorianCalendar();
            StringBuffer timestamp = new StringBuffer();

            timestamp
                .append(nf.format(cal.get(Calendar.YEAR)))
                .append(nf.format(cal.get(Calendar.MONTH)))
                .append(nf.format(cal.get(Calendar.DAY_OF_MONTH)))
                .append("-")
                .append(nf.format(cal.get(Calendar.HOUR)))
                .append(":")
                .append(nf.format(cal.get(Calendar.MINUTE)))
                .append(":")
                .append(nf.format(cal.get(Calendar.SECOND)));

            logRecord.put("timestamp", timestamp.toString());

            logList.add(logRecord);
        }

        String output = STAFMarshallingContext.marshall(
                            logList, new STAFMarshallingContext());

        System.out.println("Output:");
        System.out.println(output);
        System.out.println();

        // printOutput(output);
        System.out.println();

        // Now with a map-class

        // Construct the map-class

        STAFMapClassDefinition logRecordClass =
            new STAFMapClassDefinition("STAF/Test/LogRecord");

        logRecordClass.addKey("timestamp", "Date-Time");
        logRecordClass.addKey("level", "Level");
        logRecordClass.addKey("machine", "Machine");
        logRecordClass.addKey("message", "Message");

        // Create the marshalling context

        STAFMarshallingContext mc = new STAFMarshallingContext();

        mc.setMapClassDefinition(logRecordClass);

        // Populate the list

        List logList2 = new LinkedList();

        for (int i = 1; i < 31; ++i)
        {
            Map logRecord = logRecordClass.createInstance();

            logRecord.put("level", "info");
            logRecord.put("message", "Message #" + i);
            logRecord.put("machine", "crankin3.austin.ibm.com");

            DecimalFormat nf = new DecimalFormat("00");
            GregorianCalendar cal = new GregorianCalendar();
            StringBuffer timestamp = new StringBuffer();

            timestamp
                .append(nf.format(cal.get(Calendar.YEAR)))
                .append(nf.format(cal.get(Calendar.MONTH)))
                .append(nf.format(cal.get(Calendar.DAY_OF_MONTH)))
                .append("-")
                .append(nf.format(cal.get(Calendar.HOUR)))
                .append(":")
                .append(nf.format(cal.get(Calendar.MINUTE)))
                .append(":")
                .append(nf.format(cal.get(Calendar.SECOND)));

            logRecord.put("timestamp", timestamp.toString());

            logList2.add(logRecord);
        }

        mc.setRootObject(logList2);

        String output2 = mc.marshall();

        System.out.println("Output MC:");
        System.out.println(output2);
        System.out.println();

        MarshallingTest.printOutput(output2);
    }

    public static void printOutput(String output)
    {
        STAFMarshallingContext outputContext =
            STAFMarshallingContext.unmarshall(output);
        List outputList = (List)outputContext.getRootObject();

        Iterator iter = outputList.iterator();
        int itemNumber = 1;

        while (iter.hasNext())
        {
            Map logRecord = (Map)iter.next();

            System.out.println("Log Record #" + itemNumber++);

            if (logRecord.containsKey("staf-map-class-name"))
            {
                STAFMapClassDefinition mapClass =
                    outputContext.getMapClassDefinition(
                        (String)logRecord.get("staf-map-class-name"));
                Iterator keyIter = mapClass.keyIterator();

                while (keyIter.hasNext())
                {
                    Map key = (Map)keyIter.next();

                    System.out.println(key.get("display-name") + ": " +
                                       logRecord.get(key.get("key")));
                }

                System.out.println();
            }
            else
            {
                System.out.println("Level  : " + logRecord.get("level"));
                System.out.println("Message: " + logRecord.get("message"));
                System.out.println("Machine: " + logRecord.get("machine"));
                System.out.println();
            }
        }
    }
}



/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXTimerAction extends STAXActionDefaultImpl
                             implements STAXTimedEventListener
{
    static final int INIT = 0;
    static final int CALLED_ACTION = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String CALLED_ACTION_STRING = "CALLED_ACTION";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXTimerAction()
    { /* Do Nothing */ }

    public STAXTimerAction(String durationString, STAXAction action)
    {
        fUnevalDurationString = durationString;
        fDurationString = durationString;
        fAction = action;
    }

    public void setDuration(String durationString)
    {
        fUnevalDurationString = durationString;
    }

    public void setTimerAction(STAXAction action)
    {
        fAction = action;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case CALLED_ACTION:
                return CALLED_ACTION_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getXMLInfo()
    {
        return  "<timer duration=\"" + fDurationString + "\">";  
    }

    public String getInfo()
    {
        return fDurationString;
    }

    public String getDetails()
    {
        return "Duration:" + fDurationString + 
               ";State:" + getStateAsString() +
               ";Action:" + fAction + 
               ";Thread:" + fThread +
               ";TimedEvent:" + fTimedEvent + 
               ";TimerExpiredCondition:" + fTimerExpiredCondition;
    }

    public void execute(STAXThread thread)
    {
        synchronized (this)
        {
            if (fState == INIT)
            {
                long multiplier = 1;
                String tempDurationString = null;

                try
                {
                    fDurationString = thread.pyStringEval(fUnevalDurationString);
                    
                    if (fDurationString.length() == 0)
                    {
                        throw new NumberFormatException(
                            "For an empty input string: \"\"");
                    }

                    switch (fDurationString.charAt(fDurationString.length() - 1))
                    {
                        case 's': { multiplier = 1000; break; }
                        case 'm': { multiplier = 60000; break; }
                        case 'h': { multiplier = 3600000; break; }
                        case 'd': { multiplier = 24 * 3600000; break; }
                        case 'w': { multiplier = 7 * 24 * 3600000; break; }
                        case 'y': { multiplier = 365 * 24 * 3600000; break; }
                        default: break;
                    }

                    if (multiplier == 1)
                        tempDurationString = fDurationString;
                    else
                        tempDurationString = fDurationString.substring(0,
                                             fDurationString.length() - 1);

                    fDuration = Long.parseLong(tempDurationString);
                    fDuration *= multiplier;
                }
                catch (NumberFormatException e)
                {
                    fState = COMPLETE;
                    thread.popAction();
                    thread.pySetVar("RC", new Integer(-1));

                    String msg = "The duration may be expressed in " +
                        "milliseconds, seconds, minutes, hours, days, weeks,"+
                        " or years and must be a valid Python string.  " +
                        "Its format is <Number>[s|m|h|d|w] where <Number> " +
                        "is an integer >= 0 and indicates milliseconds " +
                        "unless one of the following suffixes is specified:" +
                        "  s (for seconds), m (for minutes), h (for hours)," +
                        " d (for days), w (for weeks), or y (for years).  " +
                        "For example:\n" +
                        "  duration=\"'50'\" specifies 50 milliseconds\n" +
                        "  duration=\"'10s'\" specifies 10 seconds\n" +
                        "  duration=\"'5m'\" specifies 5 minutes\n" +
                        "  duration=\"'2h'\" specifies 2 hours\n" +
                        "  duration=\"'3d'\" specifies 3 days\n" +
                        "  duration=\"'1w'\" specifies 1 week\n" +
                        "  duration=\"'1y'\" specifies 1 year";

                    setElementInfo(new STAXElementInfo(
                        getElement(), "duration", msg));

                    thread.setSignalMsgVar(
                        "STAXInvalidTimerValueMsg",
                        STAXUtil.formatErrorMessage(this) + "\n\n" + e);

                    thread.raiseSignal("STAXInvalidTimerValue");

                    return;
                }
                catch (STAXPythonEvaluationException e)
                {
                    fState = COMPLETE;
                    thread.popAction();
                    thread.pySetVar("RC", new Integer(-1));

                    setElementInfo(new STAXElementInfo(
                        getElement(), "duration"));

                    thread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(this), e);

                    thread.raiseSignal("STAXPythonEvaluationError");

                    return;
                }

                thread.pushAction(fAction.cloneAction());
                fState = CALLED_ACTION;
                fThread = thread;

                fTimedEvent = new STAXTimedEvent(System.currentTimeMillis() +
                                                 fDuration, this);

                thread.getJob().getSTAX().getTimedEventQueue().addTimedEvent(
                    fTimedEvent);
            }
            else if (fState == CALLED_ACTION)
            {
                fState = COMPLETE;

                thread.getJob().getSTAX().getTimedEventQueue().removeTimedEvent(
                    fTimedEvent);

                thread.popAction();
                thread.pySetVar("RC", new Integer(0));
            }
            else
            {
                fState = COMPLETE;
                thread.popAction();
            }
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        synchronized (this)
        {
            if (cond instanceof STAXTimerExpiredCondition)
            {
                thread.pySetVar("RC", new Integer(1));
            }
            else if (fState == CALLED_ACTION)
            {
                thread.getJob().getSTAX().getTimedEventQueue().removeTimedEvent(
                    fTimedEvent);
            }

            fState = COMPLETE;
            thread.popAction();
            thread.removeCondition(fTimerExpiredCondition);
        }
    }

    public STAXAction cloneAction()
    {
        STAXTimerAction clone = new STAXTimerAction(
            fUnevalDurationString, fAction);

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        return clone;
    }

    public void timedEventOccurred(STAXTimedEvent timedEvent)
    {
        synchronized (this)
        {
            if (fState != COMPLETE)
            {
                fThread.addCondition(fTimerExpiredCondition);
                fThread.schedule();
            }
        }
    }

    private int fState = INIT;
    private String fUnevalDurationString = null;
    private String fDurationString = null;
    private long fDuration = 0;
    private STAXAction fAction = null;
    private STAXThread fThread = null;
    private STAXTimedEvent fTimedEvent = null;
    private STAXTimerExpiredCondition fTimerExpiredCondition =
        new STAXTimerExpiredCondition("Timer");
}

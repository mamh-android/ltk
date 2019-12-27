package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: SessionList                                                        */
/* Description: This class manages the list of web sessions for the service  */
/*                                                                           */
/*****************************************************************************/

import java.util.Vector;
import java.util.HashMap;
import com.ibm.staf.*;

public class SessionList extends Vector 
{
    
    // This variable keeps track of the index of the first available free ID
    // This helps to keep the idices dense and acess time low.
    protected int nextNewSessionIndex;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/*                                                                           */
/*****************************************************************************/    
    public SessionList()
    { 
        super(); 
        nextNewSessionIndex = 1;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: addSession                                                        */
/* Description: adds the WebSession to the list.  The index for the next add */
/*              is increased and the ID for the session is returned.         */
/* Parameters: session - the session to add to the list                      */
/* Returns: the ID which identifies the session in the list                  */
/*                                                                           */
/*****************************************************************************/        
    public int addSession(WebSession session)
    {
        int addIndex = nextNewSessionIndex;
        
        insertElementAt(session, addIndex - 1);
        nextNewSessionIndex = nextGap();
        
        return addIndex;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getSession                                                        */
/* Description: retrieves the WebSession with the corresponding id.  If the  */
/*             session does not exist an InvalidSessionIDException is thrown.*/
/* Parameters: id - the session ID to be retrieved                           */
/* Returns: WebSession corresponding to the specified session ID             */
/*                                                                           */
/*****************************************************************************/            
    public WebSession getSession (int sessionID) 
                       throws InvalidSessionIDException
    {
        int index = findID(sessionID);
        
        return (WebSession) elementAt(index);
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: deleteSession                                                     */
/* Description: deletes the WebSession with the corresponding id.  If the    */
/*              session does not exist no action is taken.                   */
/* Parameters: id - the session ID to be deleted                             */
/* Returns: STAFResult                                                       */
/*                                                                           */
/*****************************************************************************/        
    public STAFResult deleteSession(int sessionID) 
                                     throws InvalidSessionIDException
    {
        int index = findID(sessionID);
        removeElementAt(index);
            
        // update insert index
        if (sessionID < nextNewSessionIndex)
            nextNewSessionIndex = sessionID;
        
        return new STAFResult(STAFResult.Ok, "Session " + sessionID + 
                               " deleted.");
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: findID                                                            */
/* Description: finds the index in the vector which corresponds to the       */
/*              session id                                                   */
/* Parameters: id - the session ID to be searched for                        */
/* Returns: the index of the session with an ID of id                        */
/*                                                                           */
/*****************************************************************************/    
    protected int findID(int id) throws InvalidSessionIDException
    {
        int offsetIndex = id;
        
        while ((offsetIndex >= 0) && (elementCount > 0))
        {
            if (offsetIndex >= elementCount)
                offsetIndex = elementCount - 1;
            
            try
            {
                if (((WebSession) elementAt(offsetIndex)).getID() == id) 
                    return offsetIndex;
            }
            catch (ArrayIndexOutOfBoundsException e)
            {
                // This exception can occur if many sessions are being removed
                // simultaneously.  Can just ignore as the offsetIndex will be
                // reset at the top of the loop so that is no longer larger
                // than the highest index currently in the SessionList.
            }
               
            offsetIndex--;
        }
        
        throw new InvalidSessionIDException(
            id, "ID " + id + " is not in the Session List.");
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: nextGap                                                           */
/* Description: determines the next unused session ID value                  */
/* Parameters: None                                                          */
/* Returns: an int of the next unused session ID                             */
/*                                                                           */
/*****************************************************************************/
    protected int nextGap()
    {
        
        int index = nextNewSessionIndex + 1;
        
        while ((index <= elementCount) && 
                (((WebSession) elementAt(index - 1)).getID() == index))
                    index ++;
                  
        return index;
    }    
    
/*****************************************************************************/
/*                                                                           */
/* Method: listSessions                                                      */
/* Description: return a summary of all sessions in the list                 */
/* Parameters: None                                                          */
/* Returns: the summary of all WebSessions in the list                       */
/*                                                                           */
/*****************************************************************************/
    public HashMap[] listSessions()
    {
        HashMap[] contents = new HashMap[elementCount];
        
        for (int i = 0; i < elementCount; i++)
            contents[i] = ((WebSession) elementAt(i)).getSummary();
            
        return contents;
    }

}



/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXElseIf
{
    public STAXElseIf(String expression, STAXAction action)
    {
        fExpression = expression;
        fAction = action;     
    }
    public String getExpression() { return fExpression; }
    public void   setExpression(String expression) { fExpression = expression; }

    public STAXAction getAction() { return fAction; }
    public void       setAction(STAXAction action) { fAction = action; }

    private String fExpression;
    private STAXAction fAction;
}
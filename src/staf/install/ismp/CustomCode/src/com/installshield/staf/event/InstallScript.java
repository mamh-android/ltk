package com.installshield.staf.event;

import java.io.*;
import java.net.*;
import com.installshield.event.*;
import com.installshield.event.ui.*;
import com.installshield.event.wizard.*;
import com.installshield.event.product.*;
import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.wizard.awt.*;
import com.installshield.wizard.swing.*;
import com.installshield.wizard.console.*;
import com.installshield.product.*;
import com.installshield.util.*;
import com.installshield.ui.controls.*;
import com.installshield.database.designtime.*;

public class InstallScript
{

	public void onBeginSetup(com.installshield.event.wizard.WizardContext arg0)
	{
            try{
                String setupType = arg0.resolveString("$W(setupTypes.selectedSetupTypeId)");
                arg0.getServices().getISDatabase().setVariableValue("IS_SELECTED_INSTALLATION_TYPE", setupType);                
            }catch(Throwable e){e.printStackTrace();}
	}
}
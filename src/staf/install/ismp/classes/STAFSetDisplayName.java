/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.util.*;
import com.installshield.wizard.*;
import com.installshield.wizard.service.*;
import com.installshield.wizard.platform.win32.*;
import com.installshield.product.*;
import com.installshield.product.service.product.ProductService;

public class STAFSetDisplayName extends WizardAction
{
	private String valueName="DisplayName";
	private String displayNameValue="";

	private String []  productReferencesID = new String[0];

    public void execute(WizardBeanEvent event)
    {
        ProductService service = null;
        Win32RegistryService regserv = null;
		String key = "";
		String value = "";
        String id = "";
        String installLocation = "";

        try
        {
            displayNameValue = "STAF " +
                resolveString("$W(stafVersion.version)");

        	service = (ProductService) getService(ProductService.NAME);

			//Retrieve a SoftwareObjectKey through the ProductService API
			SoftwareObjectKey sok = (SoftwareObjectKey)
                service.getProductBeanProperty(service.DEFAULT_PRODUCT_SOURCE,
                                               null,
                                               "key");

            //The uid of the root componet of the project
            id = sok.getUID();

            installLocation = resolveString("$P(absoluteInstallLocation)");
            installLocation = FileUtils.normalizeFileName(installLocation,'/');

			key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" +
                  id + installLocation.hashCode();

			regserv = (Win32RegistryService)
                getService(Win32RegistryService.NAME);

		    regserv.setStringValue(Win32RegistryService.HKEY_LOCAL_MACHINE,
                                   key,
                                   valueName,
                                   false,
                                   displayNameValue);
		}
		catch( ServiceException se )
        {
            // In InstallShield 10.5 SP2 the format of the uninstall key was
            // changed from just the product key to the product key plus a
            // hash of the install location.  If this is an upgrade install
            // over a version of STAF prior to 3.1.2, then we will get this
            // exception since the old key format is being used, in which case
            // we will retry the setStringValue call with the old key format.

            try
            {
                key =
                  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" +
                  id;

			    regserv = (Win32RegistryService)
                    getService(Win32RegistryService.NAME);

		        regserv.setStringValue(Win32RegistryService.HKEY_LOCAL_MACHINE,
                                       key,
                                       valueName,
                                       false,
                                       displayNameValue);
            }
            catch( ServiceException se2 )
            {
                se2.printStackTrace();
            }
		}
    }

	public void build(WizardBuilderSupport support)
    {
		 //Ensure service classes are built into the setup.jar
		support.putRequiredService( Win32RegistryService.NAME );
		support.putRequiredService( ProductService.NAME );
	}


	public void setValueName(String valueName)
    {
		this.valueName = valueName;
	}

	public String getValueName()
    {
		return valueName;
	}

	public void setDisplayNameValue(String displayNameValue)
    {
		this.displayNameValue = displayNameValue;
	}

	public String getDisplayNameValue(){
		return displayNameValue;
	}
}

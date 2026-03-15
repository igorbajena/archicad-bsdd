#include "ACAPinc.h"
#include "Classification_Test.hpp"

static constexpr short AddOnInfoResID = 32000;
static constexpr short MenuResID = 32500;
static constexpr short MenuPromptResID = 32600;

GSErrCode APIMenuCommandProc_Main(const API_MenuParams* menuParams)
{
	if (menuParams->menuItemRef.menuResID == MenuResID) {
		switch (menuParams->menuItemRef.itemIndex) {
		case 1:
			DGAlert(
				DG_INFORMATION,
				"ArchicadBSDD",
				"Test command works.",
				"This is the first visible command of the plugin.",
				"OK"
			);
			break;

		default:
			break;
		}
	}

	return NoError;
}


API_AddonType CheckEnvironment(API_EnvirParams* envir)
{
	ACAPI_WriteReport("ArchicadBSDD: CheckEnvironment called", false);

	RSGetIndString(&envir->addOnInfo.name, AddOnInfoResID, 1, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, AddOnInfoResID, 2, ACAPI_GetOwnResModule());

	return APIAddon_Normal;
}


GSErrCode RegisterInterface(void)
{
	ACAPI_WriteReport("ArchicadBSDD: RegisterInterface called", false);

	GSErrCode err = ACAPI_MenuItem_RegisterMenu(MenuResID, MenuPromptResID, MenuCode_UserDef, MenuFlag_Default);

	if (err == NoError) {
		ACAPI_WriteReport("ArchicadBSDD: RegisterInterface success", false);
	}
	else {
		ACAPI_WriteReport("ArchicadBSDD: RegisterInterface failed", false);
	}

	return err;
}


GSErrCode Initialize(void)
{
	ACAPI_WriteReport("ArchicadBSDD: Initialize called", false);

	GSErrCode err = ACAPI_MenuItem_InstallMenuHandler(MenuResID, APIMenuCommandProc_Main);

	if (err == NoError) {
		ACAPI_WriteReport("ArchicadBSDD: InstallMenuHandler success", false);
	}
	else {
		ACAPI_WriteReport("ArchicadBSDD: InstallMenuHandler failed", false);
	}

	return err;
}


GSErrCode FreeData(void)
{
	return NoError;
}
#include "ACAPinc.h"
#include "Classification_Test.hpp"

static constexpr short AddOnInfoResID = 32000;
static constexpr short MenuResID = 32500;
static constexpr short MenuPromptResID = 32600;

enum MenuItems {
	Menu_SearchClass = 1,
	Menu_About = 2
};


GSErrCode APIMenuCommandProc_Main(const API_MenuParams* menuParams)
{
	if (menuParams->menuItemRef.menuResID != MenuResID)
		return NoError;

	switch (menuParams->menuItemRef.itemIndex) {
	case Menu_SearchClass:
		DGAlert(
			DG_INFORMATION,
			"ArchicadBSDD",
			"Search class...",
			"Next step: this command will open the bSDD search dialog.",
			"OK"
		);
		break;

	case Menu_About:
		DGAlert(
			DG_INFORMATION,
			"ArchicadBSDD",
			"ArchicadBSDD MVP",
			"Experimental Archicad add-on for bSDD class search and future property assignment.",
			"OK"
		);
		break;

	default:
		break;
	}

	return NoError;
}


API_AddonType CheckEnvironment(API_EnvirParams* envir)
{
	RSGetIndString(&envir->addOnInfo.name, AddOnInfoResID, 1, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, AddOnInfoResID, 2, ACAPI_GetOwnResModule());

	return APIAddon_Normal;
}


GSErrCode RegisterInterface(void)
{
	return ACAPI_MenuItem_RegisterMenu(MenuResID, MenuPromptResID, MenuCode_UserDef, MenuFlag_Default);
}


GSErrCode Initialize(void)
{
	return ACAPI_MenuItem_InstallMenuHandler(MenuResID, APIMenuCommandProc_Main);
}


GSErrCode FreeData(void)
{
	return NoError;
}
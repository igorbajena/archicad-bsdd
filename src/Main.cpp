#include "APIEnvir.h"
#include "ACAPinc.h"

#include "BSDDPalette.hpp"

static constexpr short AddOnInfoResID = 32000;
static constexpr short MenuResID = 32500;
static constexpr short MenuPromptResID = 32600;

enum MenuItems {
	Menu_OpenPalette = 1
};

static void OpenBSDDPalette()
{
	if (!BSDDPalette::HasInstance()) {
		BSDDPalette::CreateInstance();
	}

	BSDDPalette::GetInstance().Show();
}

GSErrCode MenuCommandHandler(const API_MenuParams* menuParams)
{
	if (menuParams->menuItemRef.menuResID != MenuResID) {
		return NoError;
	}

	switch (menuParams->menuItemRef.itemIndex) {
	case Menu_OpenPalette:
		OpenBSDDPalette();
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

	return APIAddon_Preload;
}

GSErrCode RegisterInterface(void)
{
	return ACAPI_MenuItem_RegisterMenu(MenuResID, MenuPromptResID, MenuCode_UserDef, MenuFlag_Default);
}

GSErrCode Initialize(void)
{
	GSErrCode err = ACAPI_MenuItem_InstallMenuHandler(MenuResID, MenuCommandHandler);
	if (err != NoError) {
		return err;
	}

	err = BSDDPalette::RegisterPaletteControlCallBack();
	if (err != NoError) {
		return err;
	}

	return NoError;
}

GSErrCode FreeData(void)
{
	return NoError;
}
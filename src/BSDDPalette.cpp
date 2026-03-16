#include "BSDDPalette.hpp"

static const GS::Guid paletteGuid("{A1D8F7F2-3D56-4D98-90B1-0C8F7D2E4B11}");

GS::Ref<BSDDPalette> BSDDPalette::instance;

static GSErrCode NotificationHandler(API_NotifyEventID notifID, Int32 /*param*/)
{
	switch (notifID) {
	case APINotify_Quit:
		BSDDPalette::DestroyInstance();
		break;

	default:
		break;
	}

	return NoError;
}

BSDDPalette::BSDDPalette() :
	DG::Palette(ACAPI_GetOwnResModule(), BSDDPaletteResId, ACAPI_GetOwnResModule(), paletteGuid),
	resultsText(GetReference(), ResultsTextId),
	mockSearchButton(GetReference(), MockSearchButtonId),
	clearButton(GetReference(), ClearButtonId),
	hideButton(GetReference(), HideButtonId)
{
	ACAPI_ProjectOperation_CatchProjectEvent(APINotify_Quit, NotificationHandler);

	Attach(*this);
	mockSearchButton.Attach(*this);
	clearButton.Attach(*this);
	hideButton.Attach(*this);

	BeginEventProcessing();
}

BSDDPalette::~BSDDPalette()
{
	hideButton.Detach(*this);
	clearButton.Detach(*this);
	mockSearchButton.Detach(*this);
	Detach(*this);

	EndEventProcessing();
}

bool BSDDPalette::HasInstance()
{
	return instance != nullptr;
}

void BSDDPalette::CreateInstance()
{
	DBASSERT(!HasInstance());
	instance = new BSDDPalette();
	ACAPI_KeepInMemory(true);
}

BSDDPalette& BSDDPalette::GetInstance()
{
	DBASSERT(HasInstance());
	return *instance;
}

void BSDDPalette::DestroyInstance()
{
	instance = nullptr;
}

void BSDDPalette::SetMenuItemCheckedState(bool isChecked)
{
	API_MenuItemRef itemRef{};
	GSFlags itemFlags{};

	itemRef.menuResID = BSDDPaletteMenuResId;
	itemRef.itemIndex = BSDDPaletteMenuItemIndex;

	ACAPI_MenuItem_GetMenuItemFlags(&itemRef, &itemFlags);

	if (isChecked) {
		itemFlags |= API_MenuItemChecked;
	}
	else {
		itemFlags &= ~API_MenuItemChecked;
	}

	ACAPI_MenuItem_SetMenuItemFlags(&itemRef, &itemFlags);
}

void BSDDPalette::SetMockResults()
{
	resultsText.SetText(GS::UniString("Wall\r\nDoor\r\nWindow"));
}

void BSDDPalette::ClearResults()
{
	resultsText.SetText(GS::UniString("No results yet."));
}

void BSDDPalette::Show()
{
	DG::Palette::Show();
	SetMenuItemCheckedState(true);
}

void BSDDPalette::Hide()
{
	DG::Palette::Hide();
	SetMenuItemCheckedState(false);
}

void BSDDPalette::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource() == &mockSearchButton) {
		SetMockResults();
		return;
	}

	if (ev.GetSource() == &clearButton) {
		ClearResults();
		return;
	}

	if (ev.GetSource() == &hideButton) {
		Hide();
		return;
	}
}

void BSDDPalette::PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accepted)
{
	Hide();
	*accepted = true;
}

GSErrCode BSDDPalette::PaletteControlCallBack(Int32, API_PaletteMessageID messageID, GS::IntPtr param)
{
	switch (messageID) {
	case APIPalMsg_OpenPalette:
		if (!HasInstance()) {
			CreateInstance();
		}
		GetInstance().Show();
		break;

	case APIPalMsg_ClosePalette:
		if (HasInstance()) {
			GetInstance().Hide();
		}
		break;

	case APIPalMsg_HidePalette_Begin:
		if (HasInstance() && GetInstance().IsVisible()) {
			GetInstance().Hide();
		}
		break;

	case APIPalMsg_HidePalette_End:
		if (HasInstance() && !GetInstance().IsVisible()) {
			GetInstance().Show();
		}
		break;

	case APIPalMsg_DisableItems_Begin:
		if (HasInstance() && GetInstance().IsVisible()) {
			GetInstance().DisableItems();
		}
		break;

	case APIPalMsg_DisableItems_End:
		if (HasInstance() && GetInstance().IsVisible()) {
			GetInstance().EnableItems();
		}
		break;

	case APIPalMsg_IsPaletteVisible:
		*(reinterpret_cast<bool*> (param)) = HasInstance() && GetInstance().IsVisible();
		break;

	default:
		break;
	}

	return NoError;
}

GSErrCode BSDDPalette::RegisterPaletteControlCallBack()
{
	return ACAPI_RegisterModelessWindow(
		GS::CalculateHashValue(paletteGuid),
		PaletteControlCallBack,
		API_PalEnabled_FloorPlan +
		API_PalEnabled_Section +
		API_PalEnabled_Elevation +
		API_PalEnabled_InteriorElevation +
		API_PalEnabled_3D +
		API_PalEnabled_Detail +
		API_PalEnabled_Worksheet +
		API_PalEnabled_Layout +
		API_PalEnabled_DocumentFrom3D,
		GSGuid2APIGuid(paletteGuid)
	);
}
#ifndef BSDDPALETTE_HPP
#define BSDDPALETTE_HPP

#include "APIEnvir.h"
#include "ACAPinc.h"
#include "DGModule.hpp"

#include "BSDDService.hpp"

#define BSDDPaletteResId 32520
#define BSDDPaletteMenuResId 32500
#define BSDDPaletteMenuItemIndex 1

class BSDDPalette final : public DG::Palette,
	public DG::PanelObserver,
	public DG::ButtonItemObserver
{
private:
	enum {
		SearchEditId = 5,
		ResultsTextId = 7,

		DetailLabelValueId = 11,
		DetailCodeValueId = 13,
		DetailDictionaryValueId = 15,

		SearchButtonId = 16,
		ClearButtonId = 17,
		HideButtonId = 18
	};

	static GS::Ref<BSDDPalette> instance;

	DG::TextEdit searchEdit;
	DG::LeftText resultsText;

	DG::LeftText detailLabelValue;
	DG::LeftText detailCodeValue;
	DG::LeftText detailDictionaryValue;

	DG::Button searchButton;
	DG::Button clearButton;
	DG::Button hideButton;

	void SetMenuItemCheckedState(bool isChecked);
	void RunMockSearch();
	void ClearResults();
	void ShowDetails(const BSDDService::SearchResult& item);
	void ClearDetails();

	static GSErrCode PaletteControlCallBack(Int32 paletteId, API_PaletteMessageID messageID, GS::IntPtr param);

	BSDDPalette();

public:
	~BSDDPalette() override;

	static bool HasInstance();
	static void CreateInstance();
	static BSDDPalette& GetInstance();
	static void DestroyInstance();

	void Show();
	void Hide();

	void ButtonClicked(const DG::ButtonClickEvent& ev) override;
	void PanelCloseRequested(const DG::PanelCloseRequestEvent& ev, bool* accepted) override;

	static GSErrCode RegisterPaletteControlCallBack();
};

#endif
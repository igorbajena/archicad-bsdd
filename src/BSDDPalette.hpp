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
	public DG::ButtonItemObserver,
	public DG::ListBoxObserver,
	public DG::TextEditBaseObserver
{
private:
	enum {
		TopSeparatorId = 3,
		SearchEditId = 5,
		DictionaryFilterEditId = 7,
		ResultsCaptionId = 8,
		ResultsListId = 9,
		BottomSeparatorId = 10,
		MetadataCaptionId = 11,
		MetadataListId = 12,

		SearchButtonId = 18,
		ClearButtonId = 20,
		HideButtonId = 21
	};

	enum class ResultMode {
		None,
		MockSearch,
		Dictionaries
	};

	static GS::Ref<BSDDPalette> instance;

	DG::Separator topSeparator;
	DG::Separator bottomSeparator;

	DG::TextEdit searchEdit;
	DG::TextEdit dictionaryFilterEdit;
	DG::LeftText resultsCaption;
	DG::SingleSelListBox resultsList;

	DG::LeftText metadataCaption;
	DG::SingleSelListBox metadataList;

	DG::Button searchButton;
	DG::Button clearButton;
	DG::Button hideButton;

	GS::Array<BSDDService::SearchResult> currentSearchResults;
	GS::Array<BSDDService::DictionaryInfo> currentDictionaries;
	GS::Array<UIndex> visibleDictionaryIndexes;

	ResultMode currentMode = ResultMode::None;
	GS::UniString lastDictionaryFilterText;
	bool isLoadingDictionaries = false;

	void SetMenuItemCheckedState(bool isChecked);

	void EnsureDictionariesLoaded();
	void ClearResultsList();
	void ClearMetadataList();
	void UpdateResultsCaption(UIndex visibleCount);
	void PopulateListFromSearchResults();
	void PopulateListFromDictionaries();
	void ApplyDictionaryFilter();
	void UpdateDetailsFromSelection();

	void RunMockSearch();
	void ClearResults();

	void ShowSearchResultMetadata(const BSDDService::SearchResult& item);
	void ShowDictionaryMetadata(const BSDDService::DictionaryInfo& item);
	void SetMetadataLines(const GS::Array<GS::UniString>& lines);
	void ClearMetadata();

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
	void ListBoxSelectionChanged(const DG::ListBoxSelectionEvent& ev) override;
	void TextEditChanged(const DG::TextEditChangeEvent& ev) override;
	void PanelResized(const DG::PanelResizeEvent& ev) override;
	void PanelCloseRequested(const DG::PanelCloseRequestEvent& ev, bool* accepted) override;

	static GSErrCode RegisterPaletteControlCallBack();
};

#endif
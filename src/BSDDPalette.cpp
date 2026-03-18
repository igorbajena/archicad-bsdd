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

static GS::UniString ToLowerCopy(const GS::UniString& input)
{
	GS::UniString result = input;
	result.ToLowerCase();
	return result;
}

static void AppendMetadataLine(GS::Array<GS::UniString>& lines, const char* label, const GS::UniString& value)
{
	if (value.IsEmpty()) {
		return;
	}

	GS::UniString line = label;
	line.Append(": ");
	line.Append(value);
	lines.Push(line);
}

BSDDPalette::BSDDPalette() :
	DG::Palette(ACAPI_GetOwnResModule(), BSDDPaletteResId, ACAPI_GetOwnResModule(), paletteGuid),
	topSeparator(GetReference(), TopSeparatorId),
	bottomSeparator(GetReference(), BottomSeparatorId),
	searchEdit(GetReference(), SearchEditId),
	dictionaryFilterEdit(GetReference(), DictionaryFilterEditId),
	resultsCaption(GetReference(), ResultsCaptionId),
	resultsList(GetReference(), ResultsListId),
	metadataCaption(GetReference(), MetadataCaptionId),
	metadataList(GetReference(), MetadataListId),
	searchButton(GetReference(), SearchButtonId),
	clearButton(GetReference(), ClearButtonId),
	hideButton(GetReference(), HideButtonId)
{
	ACAPI_ProjectOperation_CatchProjectEvent(APINotify_Quit, NotificationHandler);

	Attach(*this);
	resultsList.Attach(*this);
	dictionaryFilterEdit.Attach(*this);
	searchButton.Attach(*this);
	clearButton.Attach(*this);
	hideButton.Attach(*this);

	lastDictionaryFilterText = "";

	BeginEventProcessing();
}

BSDDPalette::~BSDDPalette()
{
	hideButton.Detach(*this);
	clearButton.Detach(*this);
	searchButton.Detach(*this);
	dictionaryFilterEdit.Detach(*this);
	resultsList.Detach(*this);
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

void BSDDPalette::EnsureDictionariesLoaded()
{
	if (isLoadingDictionaries || !currentDictionaries.IsEmpty()) {
		return;
	}

	isLoadingDictionaries = true;

	currentSearchResults.Clear();
	currentMode = ResultMode::None;
	lastDictionaryFilterText = ToLowerCopy(dictionaryFilterEdit.GetText());

	ClearResultsList();
	UpdateResultsCaption(0);
	ClearMetadata();

	GS::Array<GS::UniString> loadingLines;
	loadingLines.Push("Loading dictionaries...");
	SetMetadataLines(loadingLines);

	GS::UniString errorMessage;
	GS::Array<BSDDService::DictionaryInfo> loadedDictionaries;

	if (!BSDDService::TryLoadDictionaries(loadedDictionaries, errorMessage, 1000)) {
		currentDictionaries.Clear();
		visibleDictionaryIndexes.Clear();
		currentMode = ResultMode::None;
		ClearResultsList();
		UpdateResultsCaption(0);
		ClearMetadata();

		GS::Array<GS::UniString> errorLines;
		GS::UniString message = "API error";
		if (!errorMessage.IsEmpty()) {
			message.Append(": ");
			message.Append(errorMessage);
		}
		errorLines.Push(message);
		SetMetadataLines(errorLines);

		isLoadingDictionaries = false;
		return;
	}

	currentDictionaries = loadedDictionaries;
	currentMode = ResultMode::Dictionaries;
	PopulateListFromDictionaries();

	isLoadingDictionaries = false;
}

void BSDDPalette::ClearResultsList()
{
	resultsList.DeleteItem(DG::ListBox::AllItems);
}

void BSDDPalette::ClearMetadataList()
{
	metadataList.DeleteItem(DG::ListBox::AllItems);
}

void BSDDPalette::UpdateResultsCaption(UIndex visibleCount)
{
	GS::UniString caption = "Results (";
	caption.Append(GS::ValueToUniString(visibleCount));
	caption.Append(")");
	resultsCaption.SetText(caption);
}

void BSDDPalette::PopulateListFromSearchResults()
{
	ClearResultsList();

	for (UIndex i = 0; i < currentSearchResults.GetSize(); ++i) {
		resultsList.AppendItem();
		resultsList.SetTabItemText(DG::ListBox::BottomItem, 1, currentSearchResults[i].label);
	}

	UpdateResultsCaption(currentSearchResults.GetSize());

	if (!currentSearchResults.IsEmpty()) {
		resultsList.SelectItem(1);
		UpdateDetailsFromSelection();
	}
	else {
		ClearMetadata();
	}
}

void BSDDPalette::ApplyDictionaryFilter()
{
	ClearResultsList();
	visibleDictionaryIndexes.Clear();

	const GS::UniString filterText = ToLowerCopy(dictionaryFilterEdit.GetText());

	for (UIndex i = 0; i < currentDictionaries.GetSize(); ++i) {
		GS::UniString candidate = currentDictionaries[i].displayName;
		candidate.Append(" ");
		candidate.Append(currentDictionaries[i].uri);
		candidate = ToLowerCopy(candidate);

		if (filterText.IsEmpty() || candidate.Contains(filterText)) {
			visibleDictionaryIndexes.Push(i);
			resultsList.AppendItem();
			resultsList.SetTabItemText(DG::ListBox::BottomItem, 1, currentDictionaries[i].displayName);
		}
	}

	UpdateResultsCaption(visibleDictionaryIndexes.GetSize());

	if (!visibleDictionaryIndexes.IsEmpty()) {
		resultsList.SelectItem(1);
		UpdateDetailsFromSelection();
	}
	else {
		ClearMetadata();
	}
}

void BSDDPalette::PopulateListFromDictionaries()
{
	ApplyDictionaryFilter();
}

void BSDDPalette::SetMetadataLines(const GS::Array<GS::UniString>& lines)
{
	ClearMetadataList();

	for (UIndex i = 0; i < lines.GetSize(); ++i) {
		metadataList.AppendItem();
		metadataList.SetTabItemText(DG::ListBox::BottomItem, 1, lines[i]);
	}
}

void BSDDPalette::ShowSearchResultMetadata(const BSDDService::SearchResult& item)
{
	GS::Array<GS::UniString> lines;
	AppendMetadataLine(lines, "Label", item.label);
	AppendMetadataLine(lines, "Code / URI", item.codeOrUri);
	AppendMetadataLine(lines, "Dictionary", item.dictionary);

	if (lines.IsEmpty()) {
		lines.Push("No item selected.");
	}

	SetMetadataLines(lines);
}

void BSDDPalette::ShowDictionaryMetadata(const BSDDService::DictionaryInfo& item)
{
	GS::Array<GS::UniString> lines;

	AppendMetadataLine(lines, "Name", item.name);
	AppendMetadataLine(lines, "Version", item.version);
	AppendMetadataLine(lines, "Identifier (URI)", item.uri);
	AppendMetadataLine(lines, "Organization", item.organizationNameOwner);
	AppendMetadataLine(lines, "Organization code", item.organizationCodeOwner);
	AppendMetadataLine(lines, "Release date", item.releaseDate);
	AppendMetadataLine(lines, "Code", item.code);
	AppendMetadataLine(lines, "Primary language", item.defaultLanguageCode);
	AppendMetadataLine(lines, "Status", item.status);
	AppendMetadataLine(lines, "License", item.license);
	AppendMetadataLine(lines, "License URL", item.licenseUrl);
	AppendMetadataLine(lines, "Quality Assurance Procedure", item.qualityAssuranceProcedure);
	AppendMetadataLine(lines, "Quality Assurance Procedure URL", item.qualityAssuranceProcedureUrl);
	AppendMetadataLine(lines, "Available languages", item.availableLanguagesText);
	AppendMetadataLine(lines, "Last updated", item.lastUpdatedUtc);
	AppendMetadataLine(lines, "More info", item.moreInfoUrl);

	if (lines.IsEmpty()) {
		lines.Push("No item selected.");
	}

	SetMetadataLines(lines);
}

void BSDDPalette::ClearMetadata()
{
	GS::Array<GS::UniString> lines;
	lines.Push("No item selected.");
	SetMetadataLines(lines);
}

void BSDDPalette::UpdateDetailsFromSelection()
{
	short selectedItem = resultsList.GetSelectedItem();
	if (selectedItem <= 0) {
		ClearMetadata();
		return;
	}

	const UIndex selectedIndex = static_cast<UIndex> (selectedItem - 1);

	switch (currentMode) {
	case ResultMode::MockSearch:
		if (selectedIndex < currentSearchResults.GetSize()) {
			ShowSearchResultMetadata(currentSearchResults[selectedIndex]);
		}
		else {
			ClearMetadata();
		}
		break;

	case ResultMode::Dictionaries:
		if (selectedIndex < visibleDictionaryIndexes.GetSize()) {
			const UIndex dictionaryIndex = visibleDictionaryIndexes[selectedIndex];
			if (dictionaryIndex < currentDictionaries.GetSize()) {
				ShowDictionaryMetadata(currentDictionaries[dictionaryIndex]);
			}
			else {
				ClearMetadata();
			}
		}
		else {
			ClearMetadata();
		}
		break;

	case ResultMode::None:
	default:
		ClearMetadata();
		break;
	}
}

void BSDDPalette::RunMockSearch()
{
	visibleDictionaryIndexes.Clear();
	lastDictionaryFilterText = "";

	currentSearchResults = BSDDService::GetMockSearchResults(searchEdit.GetText());
	currentMode = ResultMode::MockSearch;

	if (currentSearchResults.IsEmpty()) {
		ClearResultsList();
		UpdateResultsCaption(0);
		ClearMetadata();
		return;
	}

	PopulateListFromSearchResults();
}

void BSDDPalette::ClearResults()
{
	searchEdit.SetText("");
	dictionaryFilterEdit.SetText("");

	currentSearchResults.Clear();
	visibleDictionaryIndexes.Clear();
	lastDictionaryFilterText = "";

	if (!currentDictionaries.IsEmpty()) {
		currentMode = ResultMode::Dictionaries;
		PopulateListFromDictionaries();
	}
	else {
		currentMode = ResultMode::None;
		ClearResultsList();
		UpdateResultsCaption(0);
		ClearMetadata();
	}
}

void BSDDPalette::Show()
{
	DG::Palette::Show();
	DG::Palette::BringToFront();
	SetMenuItemCheckedState(true);

	EnsureDictionariesLoaded();
}

void BSDDPalette::Hide()
{
	DG::Palette::Hide();
	SetMenuItemCheckedState(false);
}

void BSDDPalette::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource() == &searchButton) {
		RunMockSearch();
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

void BSDDPalette::ListBoxSelectionChanged(const DG::ListBoxSelectionEvent& ev)
{
	if (ev.GetSource() == &resultsList) {
		UpdateDetailsFromSelection();
	}
}

void BSDDPalette::TextEditChanged(const DG::TextEditChangeEvent& ev)
{
	if (ev.GetSource() == &dictionaryFilterEdit) {
		if (currentMode == ResultMode::Dictionaries && !isLoadingDictionaries) {
			lastDictionaryFilterText = ToLowerCopy(dictionaryFilterEdit.GetText());
			ApplyDictionaryFilter();
		}
	}
}

void BSDDPalette::PanelResized(const DG::PanelResizeEvent& ev)
{
	short dh = ev.GetHorizontalChange();
	short dv = ev.GetVerticalChange();

	if (dh == 0 && dv == 0) {
		return;
	}

	BeginMoveResizeItems();

	topSeparator.Resize(dh, 0);

	searchEdit.Resize(dh, 0);
	dictionaryFilterEdit.Resize(dh, 0);

	resultsCaption.Resize(dh, 0);
	resultsList.Resize(dh, dv);

	bottomSeparator.Move(0, dv);
	bottomSeparator.Resize(dh, 0);

	metadataCaption.Move(0, dv);
	metadataCaption.Resize(dh, 0);

	metadataList.Move(0, dv);
	metadataList.Resize(dh, 0);

	// PRZYCISKI: tylko pionowo, bez przesuwania w poziomie
	searchButton.Move(0, dv);
	clearButton.Move(0, dv);
	hideButton.Move(0, dv);

	EndMoveResizeItems();
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
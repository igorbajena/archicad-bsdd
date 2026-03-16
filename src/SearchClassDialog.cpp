#include "APIEnvir.h"
#include "ACAPinc.h"

#include "SearchClassDialog.hpp"

SearchClassDialog::SearchClassDialog() :
	DG::ModalDialog(ACAPI_GetOwnResModule(), 32520, ACAPI_GetOwnResModule()),
	closeButton(GetReference(), CloseButtonId),
	mockSearchButton(GetReference(), MockSearchButtonId)
{
	AttachToAllItems(*this);
}

SearchClassDialog::~SearchClassDialog()
{
	DetachFromAllItems(*this);
}

void SearchClassDialog::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource() == &closeButton) {
		PostCloseRequest(DG::ModalDialog::Accept);
		return;
	}

	if (ev.GetSource() == &mockSearchButton) {
		DGAlert(
			DG_INFORMATION,
			"ArchicadBSDD",
			"Mock results",
			"Wall\nDoor\nWindow",
			"OK"
		);
	}
}
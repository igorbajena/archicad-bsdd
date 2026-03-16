#ifndef SEARCHCLASSDIALOG_HPP
#define SEARCHCLASSDIALOG_HPP

#include "DGModule.hpp"

class SearchClassDialog : public DG::ModalDialog,
	public DG::ButtonItemObserver,
	public DG::CompoundItemObserver
{
public:
	enum Controls {
		CloseButtonId = 1,
		MockSearchButtonId = 2
	};

	SearchClassDialog();
	~SearchClassDialog() override;

private:
	DG::Button closeButton;
	DG::Button mockSearchButton;

	void ButtonClicked(const DG::ButtonClickEvent& ev) override;
};

#endif
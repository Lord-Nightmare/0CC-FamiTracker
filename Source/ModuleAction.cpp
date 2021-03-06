/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2014  Jonathan Liss
**
** 0CC-FamiTracker is (C) 2014-2018 HertzDevil
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include "ModuleAction.h"
#include "MainFrm.h"
#include "FamiTrackerView.h"
#include "FamiTrackerModule.h"
#include "InstrumentManager.h"
#include "Instrument.h"

#define GET_VIEW() static_cast<CFamiTrackerView *>(MainFrm.GetActiveView())
#define GET_MODULE() (*GET_VIEW()->GetModuleData())

void CModuleAction::SaveUndoState(const CMainFrame &MainFrm) {
}

void CModuleAction::SaveRedoState(const CMainFrame &MainFrm) {
}

void CModuleAction::RestoreUndoState(CMainFrame &MainFrm) const {
}

void CModuleAction::RestoreRedoState(CMainFrame &MainFrm) const {
}



bool ModuleAction::CComment::SaveState(const CMainFrame &MainFrm) {
	auto &modfile = GET_MODULE();
	oldComment_ = modfile.GetComment();
	oldShow_ = modfile.ShowsCommentOnOpen();
	return true; // no merge because the comment dialog is modal
}

void ModuleAction::CComment::Undo(CMainFrame &MainFrm) {
	GET_MODULE().SetComment(oldComment_, oldShow_);
}

void ModuleAction::CComment::Redo(CMainFrame &MainFrm) {
	GET_MODULE().SetComment(newComment_, newShow_);
}

void ModuleAction::CComment::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.SetMessageText(L"Comment settings changed");
}



ModuleAction::CTitle::CTitle(std::string_view str) :
	newStr_(str.substr(0, CFamiTrackerModule::METADATA_FIELD_LENGTH - 1))
{
}

bool ModuleAction::CTitle::SaveState(const CMainFrame &MainFrm) {
	oldStr_ = GET_MODULE().GetModuleName();
	return newStr_ != oldStr_;
}

void ModuleAction::CTitle::Undo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleName(oldStr_);
}

void ModuleAction::CTitle::Redo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleName(newStr_);
}

bool ModuleAction::CTitle::Merge(const CAction &other) {
	if (auto pAction = dynamic_cast<const CTitle *>(&other)) {
		newStr_ = pAction->newStr_;
		return true;
	}
	return false;
}

void ModuleAction::CTitle::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.SetSongInfo(GET_MODULE());
}



ModuleAction::CArtist::CArtist(std::string_view str) :
	newStr_(str.substr(0, CFamiTrackerModule::METADATA_FIELD_LENGTH - 1))
{
}

bool ModuleAction::CArtist::SaveState(const CMainFrame &MainFrm) {
	oldStr_ = GET_MODULE().GetModuleArtist();
	return newStr_ != oldStr_;
}

void ModuleAction::CArtist::Undo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleArtist(oldStr_);
}

void ModuleAction::CArtist::Redo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleArtist(newStr_);
}

bool ModuleAction::CArtist::Merge(const CAction &other) {
	if (auto pAction = dynamic_cast<const CArtist *>(&other)) {
		newStr_ = pAction->newStr_;
		return true;
	}
	return false;
}

void ModuleAction::CArtist::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.SetSongInfo(GET_MODULE());
}



ModuleAction::CCopyright::CCopyright(std::string_view str) :
	newStr_(str.substr(0, CFamiTrackerModule::METADATA_FIELD_LENGTH - 1))
{
}

bool ModuleAction::CCopyright::SaveState(const CMainFrame &MainFrm) {
	oldStr_ = GET_MODULE().GetModuleCopyright();
	return newStr_ != oldStr_;
}

void ModuleAction::CCopyright::Undo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleCopyright(oldStr_);
}

void ModuleAction::CCopyright::Redo(CMainFrame &MainFrm) {
	GET_MODULE().SetModuleCopyright(newStr_);
}

bool ModuleAction::CCopyright::Merge(const CAction &other) {
	if (auto pAction = dynamic_cast<const CCopyright *>(&other)) {
		newStr_ = pAction->newStr_;
		return true;
	}
	return false;
}

void ModuleAction::CCopyright::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.SetSongInfo(GET_MODULE());
}



ModuleAction::CAddInst::CAddInst(unsigned index, std::shared_ptr<CInstrument> pInst) :
	index_(index), inst_(pInst), prev_(INVALID_INSTRUMENT)
{
}

bool ModuleAction::CAddInst::SaveState(const CMainFrame &MainFrm) {
	prev_ = MainFrm.GetSelectedInstrumentIndex();
	return inst_ && index_ < MAX_INSTRUMENTS && !GET_MODULE().GetInstrumentManager()->IsInstrumentUsed(index_);
}

void ModuleAction::CAddInst::Undo(CMainFrame &MainFrm) {
	GET_MODULE().GetInstrumentManager()->RemoveInstrument(index_);
	if (prev_ != INVALID_INSTRUMENT)
		MainFrm.SelectInstrument(prev_);
}

void ModuleAction::CAddInst::Redo(CMainFrame &MainFrm) {
	GET_MODULE().GetInstrumentManager()->InsertInstrument(index_, inst_);
	MainFrm.SelectInstrument(index_);
}

void ModuleAction::CAddInst::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.GetActiveDocument()->UpdateAllViews(NULL, UPDATE_INSTRUMENT);
}



ModuleAction::CRemoveInst::CRemoveInst(unsigned index) :
	index_(index), nextIndex_(INVALID_INSTRUMENT) {
}

bool ModuleAction::CRemoveInst::SaveState(const CMainFrame &MainFrm) {
	const auto *pManager = GET_MODULE().GetInstrumentManager();
	if ((inst_ = pManager->GetInstrument(index_))) {
		for (unsigned i = index_ + 1; i < MAX_INSTRUMENTS; ++i)
			if (pManager->IsInstrumentUsed(i)) {
				nextIndex_ = i;
				return true;
			}
		for (int i = index_ - 1; i >= 0; --i)
			if (pManager->IsInstrumentUsed(i)) {
				nextIndex_ = i;
				return true;
			}
		return true;
	}
	return false;
}

void ModuleAction::CRemoveInst::Undo(CMainFrame &MainFrm) {
	GET_MODULE().GetInstrumentManager()->InsertInstrument(index_, inst_);
	MainFrm.SelectInstrument(index_);
}

void ModuleAction::CRemoveInst::Redo(CMainFrame &MainFrm) {
	GET_MODULE().GetInstrumentManager()->RemoveInstrument(index_);
	if (nextIndex_ != INVALID_INSTRUMENT)
		MainFrm.SelectInstrument(nextIndex_);
	else
		MainFrm.CloseInstrumentEditor();
}

void ModuleAction::CRemoveInst::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.GetActiveDocument()->UpdateAllViews(NULL, UPDATE_INSTRUMENT);
}



ModuleAction::CInstName::CInstName(unsigned index, std::string_view str) :
	index_(index), newStr_(str.substr(0, CInstrument::INST_NAME_MAX - 1))
{
}

bool ModuleAction::CInstName::SaveState(const CMainFrame &MainFrm) {
	if (auto pInst = GET_MODULE().GetInstrumentManager()->GetInstrument(index_)) {
		oldStr_ = pInst->GetName();
		return newStr_ != oldStr_;
	}
	return false;
}

void ModuleAction::CInstName::Undo(CMainFrame &MainFrm) {
	auto pInst = GET_MODULE().GetInstrumentManager()->GetInstrument(index_);
	pInst->SetName(oldStr_);
}

void ModuleAction::CInstName::Redo(CMainFrame &MainFrm) {
	auto pInst = GET_MODULE().GetInstrumentManager()->GetInstrument(index_);
	pInst->SetName(newStr_);
}

bool ModuleAction::CInstName::Merge(const CAction &other) {
	if (auto pAction = dynamic_cast<const CInstName *>(&other)) {
		if (index_ == pAction->index_) {
			newStr_ = pAction->newStr_;
			return true;
		}
	}
	return false;
}

void ModuleAction::CInstName::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.SelectInstrument(index_);
	MainFrm.UpdateInstrumentName();
}



ModuleAction::CSwapInst::CSwapInst(unsigned left, unsigned right) :
	left_(left), right_(right)
{
}

bool ModuleAction::CSwapInst::SaveState(const CMainFrame &MainFrm) {
	if (left_ == right_)
		return false;
	const auto *pManager = GET_MODULE().GetInstrumentManager();
	return pManager->IsInstrumentUsed(left_) && pManager->IsInstrumentUsed(right_);
}

void ModuleAction::CSwapInst::Undo(CMainFrame &MainFrm) {
	GET_MODULE().SwapInstruments(left_, right_);
	MainFrm.GetActiveDocument()->UpdateAllViews(NULL, UPDATE_PATTERN);
	MainFrm.SelectInstrument(left_);
}

void ModuleAction::CSwapInst::Redo(CMainFrame &MainFrm) {
	GET_MODULE().SwapInstruments(left_, right_);
	MainFrm.GetActiveDocument()->UpdateAllViews(NULL, UPDATE_PATTERN);
	MainFrm.SelectInstrument(right_);
}

void ModuleAction::CSwapInst::UpdateViews(CMainFrame &MainFrm) const {
	MainFrm.UpdateInstrumentList();
}

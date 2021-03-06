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


#pragma once

#include <memory>
#include "FamiTrackerDefines.h"

class CFamiTrackerModule;

class CModuleImporter {
public:
	CModuleImporter(CFamiTrackerModule &modfile, CFamiTrackerModule &imported);

	bool Validate() const;
	void DoImport(bool doInst, bool doGroove, bool doDetune);

private:
	void ImportInstruments();
	void ImportGrooves();
	void ImportDetune();
	void ImportSongs();

private:
	CFamiTrackerModule &modfile_;
	CFamiTrackerModule &imported_;

	int inst_index_[MAX_INSTRUMENTS] = { };
	int groove_index_[MAX_GROOVE] = { };
};

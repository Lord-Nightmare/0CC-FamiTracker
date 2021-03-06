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

// This file handles playing of VRC6 channels

#include "ChannelsVRC6.h"
#include "APU/Types.h"		// // //
#include "APU/APUInterface.h"		// // //
#include "Instrument.h"		// // //
#include "InstHandler.h"		// // //
#include "SeqInstHandler.h"		// // //
#include "SeqInstHandlerSawtooth.h"		// // //
#include "FamiTrackerEnv.h"		// // //
#include "Settings.h"		// // //

CChannelHandlerVRC6::CChannelHandlerVRC6(stChannelID ch, int MaxPeriod, int MaxVolume) :		// // //
	CChannelHandler(ch, MaxPeriod, MaxVolume)
{
}

bool CChannelHandlerVRC6::HandleEffect(stEffectCommand cmd)
{
	switch (cmd.fx) {
	case effect_t::DUTY_CYCLE:
		m_iDefaultDuty = m_iDutyPeriod = cmd.param;
		break;
	default: return CChannelHandler::HandleEffect(cmd);
	}

	return true;
}

void CChannelHandlerVRC6::HandleEmptyNote()
{
}

void CChannelHandlerVRC6::HandleCut()
{
	CutNote();
}

void CChannelHandlerVRC6::HandleRelease()
{
	if (!m_bRelease)
		ReleaseNote();
}

bool CChannelHandlerVRC6::CreateInstHandler(inst_type_t Type)
{
	switch (Type) {
	case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS:
		switch (m_iInstTypeCurrent) {
		case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS: break;
		default:
			m_pInstHandler = std::make_unique<CSeqInstHandler>(this, 0x0F, Type == INST_S5B ? 0x40 : 0);
			return true;
		}
	}
	return false;
}

void CChannelHandlerVRC6::ClearRegisters()		// // //
{
	unsigned Address = (GetChannelID().Subindex << 12) + 0x9000;		// // //
	m_pAPU->Write(Address, 0);
	m_pAPU->Write(Address + 1, 0);
	m_pAPU->Write(Address + 2, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// // // VRC6 Squares
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVRC6Square::RefreshChannel()
{
	unsigned Address = (GetChannelID().Subindex << 12) + 0x9000;		// // //

	unsigned int Period = CalculatePeriod();
	unsigned int Volume = CalculateVolume();
	unsigned char DutyCycle = m_iDutyPeriod << 4;

	unsigned char HiFreq = (Period & 0xFF);
	unsigned char LoFreq = (Period >> 8);

	if (!m_bGate) {		// // //
		m_pAPU->Write(Address, DutyCycle);
		return;
	}

	m_pAPU->Write(Address, DutyCycle | Volume);
	m_pAPU->Write(Address + 1, HiFreq);
	m_pAPU->Write(Address + 2, 0x80 | LoFreq);
}

int CVRC6Square::ConvertDuty(int Duty) const		// // //
{
	switch (m_iInstTypeCurrent) {
	case INST_2A03:	return DUTY_VRC6_FROM_2A03[Duty & 0x03];
	case INST_S5B:	return 0x07;
	default:		return Duty;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// VRC6 Sawtooth
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVRC6Sawtooth::RefreshChannel()
{
	if (!m_bGate) {		// // //
		m_pAPU->Write(0xB000, 0);
		return;
	}

	unsigned int Period = CalculatePeriod();
	unsigned int Volume = CalculateVolume();		// // //

	m_pAPU->Write(0xB000, Volume);
	m_pAPU->Write(0xB001, Period & 0xFF);
	m_pAPU->Write(0xB002, 0x80 | (Period >> 8));
}

bool CVRC6Sawtooth::CreateInstHandler(inst_type_t Type)		// // //
{
	switch (Type) {
	case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS:
		switch (m_iInstTypeCurrent) {
		case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS: break;
		default:
			m_pInstHandler = std::make_unique<CSeqInstHandlerSawtooth>(this, 0x0F, Type == INST_S5B ? 0x40 : 0);
			return true;
		}
	}
	return false;
}

int CVRC6Sawtooth::CalculateVolume() const		// // //
{
	bool use_64_steps = false;
	if (auto pHandler = dynamic_cast<CSeqInstHandlerSawtooth*>(m_pInstHandler.get()))
		use_64_steps = pHandler->IsDutyIgnored();

	if (use_64_steps) {
		if (!Env.GetSettings()->General.bFDSOldVolume)		// // // match NSF setting
			return LimitVolume(((m_iInstVolume + 1) * ((m_iVolume >> VOL_COLUMN_SHIFT) + 1) - 1) / 16 - GetTremolo());
		return CChannelHandler::CalculateVolume();
	}

	return (CChannelHandler::CalculateVolume() << 1) | ((m_iDutyPeriod & 0x01) << 5);
}

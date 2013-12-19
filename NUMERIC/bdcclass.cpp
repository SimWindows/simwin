/*
    SimWindows - 1D Semiconductor Device Simulator
    Copyright (C) 2013 David W. Winston

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "comincl.h"
#include "sim2dcar.h"
#include "simqw.h"
#include "simbdcar.h"

/******************************** TBoundElectron Class ****************************************

class TBoundElectron {
	friend T2DElectron;
protected:
	TQuantumWell *qw_ptr;
	prec equil_dos;
	prec non_equil_dos;
	prec wave_function;
	prec concentration;
	prec deriv_conc_eta_c;
	prec qw_energy_top;

public:
	TBoundElectron(TQuantumWell *qw);

// Comp functions.
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TBoundElectron::TBoundElectron(TQuantumWell *qw)
{
	qw_ptr=qw;
	equil_dos=0.0;
	non_equil_dos=0.0;
	wave_function=0.0;
	concentration=0.0;
	deriv_conc_eta_c=0.0;
	qw_energy_top=0.0;
}

void TBoundElectron::comp_conc(void)
{
	concentration=qw_ptr->T2DElectron::conc*sq(wave_function);
}

void TBoundElectron::comp_deriv_conc(void)
{
	deriv_conc_eta_c=qw_ptr->T2DElectron::deriv_conc_eta_c*sq(wave_function);
}

void TBoundElectron::comp_equil_dos(void)
{
	equil_dos=qw_ptr->T2DElectron::equil_dos/qw_ptr->T2DElectron::qw_length;
}

void TBoundElectron::comp_non_equil_dos(void)
{
	non_equil_dos=qw_ptr->T2DElectron::non_equil_dos/qw_ptr->T2DElectron::qw_length;
}

prec TBoundElectron::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case WAVE_FUNCTION: return_value=wave_function; break;
		case CONCENTRATION: return_value=concentration; break;
		case ENERGY_TOP: return_value=qw_energy_top; break;
		default: assert(FALSE); return(0.0);
	}
	if (scale==UNNORMALIZED) return_value*=get_normalize_value(BOUND_ELECTRON,flag_value);
	return(return_value);
}

void TBoundElectron::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(BOUND_ELECTRON,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case WAVE_FUNCTION: wave_function=value; return;
		case CONCENTRATION: concentration=value; return;
		case ENERGY_TOP: qw_energy_top=value; return;
		default: assert(FALSE); return;
	}
}

void TBoundElectron::read_state_file(FILE *file_ptr)
{
	fread(&wave_function,sizeof(wave_function),1,file_ptr);
	fread(&concentration,sizeof(concentration),1,file_ptr);
	fread(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
}

void TBoundElectron::write_state_file(FILE *file_ptr)
{
	fwrite(&wave_function,sizeof(wave_function),1,file_ptr);
	fwrite(&concentration,sizeof(concentration),1,file_ptr);
	fwrite(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
}

/******************************** TBoundHole Class *******************************************

class TBoundHole {
	friend T2DHole;
protected:
	TQuantumWell *qw_ptr;
	prec equil_dos;
	prec non_equil_dos;
	prec wave_function;
	prec concentration;
	prec deriv_conc_eta_v;
	prec qw_energy_top;

public:
	TBoundHole(TQuantumWell *qw);

// Comp functions
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TBoundHole::TBoundHole(TQuantumWell *qw)
{
	qw_ptr=qw;
	equil_dos=0.0;
	non_equil_dos=0.0;
	wave_function=0.0;
	concentration=0.0;
	deriv_conc_eta_v=0.0;
	qw_energy_top=0.0;
}

void TBoundHole::comp_conc(void)
{
	concentration=qw_ptr->T2DHole::conc*sq(wave_function);
}

void TBoundHole::comp_deriv_conc(void)
{
	deriv_conc_eta_v=qw_ptr->T2DHole::deriv_conc_eta_v*sq(wave_function);
}

void TBoundHole::comp_equil_dos(void)
{
	equil_dos=qw_ptr->T2DHole::equil_dos/qw_ptr->T2DHole::qw_length;
}

void TBoundHole::comp_non_equil_dos(void)
{
	non_equil_dos=qw_ptr->T2DHole::non_equil_dos/qw_ptr->T2DHole::qw_length;
}

prec TBoundHole::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case WAVE_FUNCTION: return_value=wave_function; break;
		case CONCENTRATION: return_value=concentration; break;
		case ENERGY_TOP: return_value=qw_energy_top; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(BOUND_HOLE,flag_value);
	return(return_value);
}

void TBoundHole::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(BOUND_HOLE,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case WAVE_FUNCTION: wave_function=value; return;
		case CONCENTRATION: concentration=value; return;
		case ENERGY_TOP: qw_energy_top=value; return;
		default: assert(FALSE); return;
	}
}

void TBoundHole::read_state_file(FILE *file_ptr)
{
	fread(&wave_function,sizeof(wave_function),1,file_ptr);
	fread(&concentration,sizeof(concentration),1,file_ptr);
	fread(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
}

void TBoundHole::write_state_file(FILE *file_ptr)
{
	fwrite(&wave_function,sizeof(wave_function),1,file_ptr);
	fwrite(&concentration,sizeof(concentration),1,file_ptr);
	fwrite(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
}


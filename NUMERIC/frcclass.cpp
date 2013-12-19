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
#include "simfrcar.h"

/******************************* class TFreeElectron ******************************************

class TFreeElectron {
protected:
	flag effects;
	prec equil_dos;
	prec non_equil_dos;
	prec mobility;
	prec concentration;
	prec deriv_conc_eta_c;

public:
	TFreeElectron(void);

// Comp functions
private:
	prec comp_dos(prec dos_mass, prec temp);

public:
	void comp_equil_dos(prec dos_mass, prec temp) { equil_dos=comp_dos(dos_mass, temp); }
	void comp_non_equil_dos(prec dos_mass, prec temp) { non_equil_dos=comp_dos(dos_mass,temp); }
	void comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
					   prec car_temp, prec donors, prec acceptors);
	void comp_conc(prec planck_potential, prec temp=0.0,
				   prec qw_energy_top=0.0, prec bound_dos=0.0);
	void comp_deriv_conc(prec planck_potential, prec qw_energy_top=0.0);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TFreeElectron::TFreeElectron(void)
{
	effects=0;
	equil_dos=0.0;
	non_equil_dos=0.0;
	mobility=0.0;
	concentration=0.0;
	deriv_conc_eta_c=0.0;
}

prec TFreeElectron::comp_dos(prec dos_mass, prec temp)
{
	return(2.0*pow((double) 2.0*SIM_pi*dos_mass*SIM_mo*SIM_k*temp*normalization.temp/sq(SIM_h),
				   (double) 1.5)/(1E6*normalization.conc));
}

void TFreeElectron::comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
								  prec car_temp, prec donors, prec acceptors)
{
	prec compute_lat_temp, compute_car_temp;
	prec compute_donors, compute_acceptors;

	if (effects & GRID_DOPING_MOBILITY) {
		compute_donors=donors*normalization.conc;
		compute_acceptors=acceptors*normalization.conc;
	}
	else {
		compute_donors=0.0;
		compute_acceptors=0.0;
	}

	if (effects & GRID_TEMP_MOBILITY) {
		compute_lat_temp=lat_temp*normalization.temp;
		compute_car_temp=car_temp*normalization.temp;
	}
	else {
		compute_lat_temp=300.0;
		compute_car_temp=300.0;
	}

	prec values[]= { material.alloy_conc, compute_lat_temp, compute_car_temp, compute_donors,
					 compute_acceptors, position*normalization.length/1e-4 };

	mobility=material_parameters.evaluate(MAT_ELECTRON_MOBILITY,material.material_type,material.alloy_type,
										  values)/normalization.mobility;
}

void TFreeElectron::comp_conc(prec planck_potential, prec temp,
							  prec qw_energy_top, prec bound_dos)
{
	if (qw_energy_top>0.0) {
		if (effects & GRID_QW_FREE_CARR)
			concentration=exp(planck_potential)*
						  (non_equil_dos*(1.0-incomp_gamma(qw_energy_top/temp)/(SIM_sqrtpi/2.0))-
						   bound_dos*exp(-qw_energy_top/temp));
		else
			concentration=0.0;
	}
	else {
		if (effects & GRID_FERMI_DIRAC) concentration=non_equil_dos*fermi_integral_1_half(planck_potential);
		else concentration=exp(planck_potential)*non_equil_dos;
	}
}

void TFreeElectron::comp_deriv_conc(prec planck_potential, prec qw_energy_top)
{
	if (qw_energy_top>0.0) deriv_conc_eta_c=concentration;
	else {
		if (effects & GRID_FERMI_DIRAC) deriv_conc_eta_c=non_equil_dos*fermi_integral_minus_1_half(planck_potential);
		else deriv_conc_eta_c=concentration;
	}
}

prec TFreeElectron::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case MOBILITY: return_value=mobility; break;
		case CONCENTRATION: return_value=concentration; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(FREE_ELECTRON,flag_value);
	return(return_value);
}

void TFreeElectron::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(FREE_ELECTRON,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case MOBILITY: mobility=value; return;
		case CONCENTRATION: concentration=value; return;
		default: assert(FALSE); return;
	}
}

void TFreeElectron::read_state_file(FILE *file_ptr)
{
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fread(&mobility,sizeof(mobility),1,file_ptr);
	fread(&concentration,sizeof(concentration),1,file_ptr);
}

void TFreeElectron::write_state_file(FILE *file_ptr)
{
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fwrite(&mobility,sizeof(mobility),1,file_ptr);
	fwrite(&concentration,sizeof(concentration),1,file_ptr);
}

/************************************** class TFreeHole ****************************************

class TFreeHole {
protected:
	flag effects;
	prec equil_dos;
	prec non_equil_dos;
	prec mobility;
	prec concentration;
	prec deriv_conc_eta_v;

public:
	TFreeHole(void);

// Comp functions
private:
	prec comp_dos(prec dos_mass, prec temp);
public:
	void comp_equil_dos(prec dos_mass, prec temp) { equil_dos=comp_dos(dos_mass, temp); }
	void comp_non_equil_dos(prec dos_mass, prec temp) { non_equil_dos=comp_dos(dos_mass,temp); }
	void comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
					   prec car_temp, prec donors, prec acceptors);
	void comp_conc(prec planck_potential, prec temp=0.0,
				   prec qw_energy_top=0.0, prec bound_dos=0.0);
	void comp_deriv_conc(prec planck_potential,prec qw_energy_top=0.0);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TFreeHole::TFreeHole(void)
{
	effects=0;
	equil_dos=0.0;
	non_equil_dos=0.0;
	mobility=0.0;
	concentration=0.0;
	deriv_conc_eta_v=0.0;
}

prec TFreeHole::comp_dos(prec dos_mass, prec temp)
{
	return(2.0*pow((double) 2.0*SIM_pi*dos_mass*SIM_mo*SIM_k*temp*normalization.temp/sq(SIM_h),
				   (double) 1.5)/(1E6*normalization.conc));
}

void TFreeHole::comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
							  prec car_temp, prec donors, prec acceptors)
{
	prec compute_car_temp, compute_lat_temp;
	prec compute_donors, compute_acceptors;

	if (effects & GRID_DOPING_MOBILITY) {
		compute_donors=donors*normalization.conc;
		compute_acceptors=acceptors*normalization.conc;
	}
	else {
		compute_donors=0.0;
		compute_acceptors=0.0;
	}

	if (effects & GRID_TEMP_MOBILITY) {
		compute_car_temp=car_temp*normalization.temp;
		compute_lat_temp=lat_temp*normalization.temp;
	}
	else {
		compute_car_temp=300.0;
		compute_lat_temp=300.0;
	}

	prec values[]= { material.alloy_conc, compute_lat_temp, compute_car_temp, compute_donors,
					 compute_acceptors, position*normalization.length/1e-4 };

	mobility=material_parameters.evaluate(MAT_HOLE_MOBILITY,material.material_type,
										  material.alloy_type,values)/normalization.mobility;
}

void TFreeHole::comp_conc(prec planck_potential, prec temp,
						  prec qw_energy_top, prec bound_dos)
{
	if (qw_energy_top<0.0) {
		if (effects & GRID_QW_FREE_CARR)
			concentration=exp(planck_potential)*
						  (non_equil_dos*(1.0-incomp_gamma(-qw_energy_top/temp)/(SIM_sqrtpi/2.0))-
						   bound_dos*exp(qw_energy_top/temp));
		else
			concentration=0.0;
	}
	else {
		if (effects & GRID_FERMI_DIRAC) concentration=non_equil_dos*fermi_integral_1_half(planck_potential);
		else concentration=exp(planck_potential)*non_equil_dos;
	}
}

void TFreeHole::comp_deriv_conc(prec planck_potential,prec qw_energy_top)
{
	if (qw_energy_top<0.0) deriv_conc_eta_v=concentration;
	else {
		if (effects & GRID_FERMI_DIRAC) deriv_conc_eta_v=non_equil_dos*fermi_integral_minus_1_half(planck_potential);
		else deriv_conc_eta_v=concentration;
	}
}

prec TFreeHole::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case MOBILITY: return_value=mobility; break;
		case CONCENTRATION: return_value=concentration; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(FREE_HOLE,flag_value);
	return(return_value);
}

void TFreeHole::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(FREE_HOLE,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case MOBILITY: mobility=value; return;
		case CONCENTRATION: concentration=value; return;
		default: assert(FALSE); return;
	}
}

void TFreeHole::read_state_file(FILE *file_ptr)
{
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fread(&mobility,sizeof(mobility),1,file_ptr);
	fread(&concentration,sizeof(concentration),1,file_ptr);
}

void TFreeHole::write_state_file(FILE *file_ptr)
{
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fwrite(&mobility,sizeof(mobility),1,file_ptr);
	fwrite(&concentration,sizeof(concentration),1,file_ptr);
}


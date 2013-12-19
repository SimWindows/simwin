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
#include "simbdcar.h"
#include "simcar.h"
#include "sim2dcar.h"
#include "simqw.h"

/********************************* class TElectron ********************************************

class TElectron: protected TFreeElectron, protected TBoundElectron {
private:
	RegionType region_type;
protected:
	prec temperature;
	prec stored_temperature;
	prec dos_mass;
	float cond_mass;
	prec doping_conc;
	long doping_degeneracy;
	prec doping_level;
	prec ionized_doping_conc;
	prec total_conc;
	prec total_deriv_conc_eta_c;
	prec total_deriv_ionized_doping_eta_c;
	prec planck_potential;					// (Ef-Ec)/kT
	prec equil_planck_potential;
	prec band_edge;
	prec shr_lifetime;
	prec energy_lifetime;
    prec auger_coefficient;
	float current;
	prec stimulated_factor;
	Hotcarriers hotcarriers;
	prec total_deriv_hotcarriers;
	long collision_factor;

public:
	TElectron(RegionType region, TQuantumWell *qw_ptr);

// Comp functions
	void comp_auger_coefficient(MaterialSpecification material, prec position);
	void comp_auger_hotcarriers(prec intrinsic_conc, prec hole_conc,
    						    prec hole_auger_coeff, prec rec_auger);
	void comp_b_b_hotcarriers(prec rec_b_b);
	void comp_kin_optical_generation_hotcarriers(prec rec_opt_gen, prec inc_pho_ene,
												 prec band_gap, prec r_dos_mass);
	void comp_ref_optical_generation_hotcarriers(prec rec_opt_gen);
	void comp_stim_hotcarriers(prec rec_stim, prec r_dos_mass, prec band_gap, prec inc_pho_ene);
	void comp_relax_hotcarriers(prec lat_temp);
	void comp_shr_hotcarriers(prec intrinsic_conc, prec lattice_temp, prec hole_conc,
							  prec hole_shr_lifetime, prec shr_recomb);
	void comp_total_hotcarriers(void);
	void comp_conc(void);
	void comp_cond_mass(MaterialSpecification material, prec position);
	void comp_collision_factor(MaterialSpecification material, prec position);
	void comp_current(int start_node_number, int node_number,
					  float position, prec recombination_total);
	void comp_deriv_conc(void);
	void comp_deriv_ionized_doping(void);
	void comp_deriv_hotcarriers(Recombination recombination, prec lat_temp,
								prec intrinsic_conc, prec hole_conc,
                                prec hole_shr_lifetime, prec hole_auger_coeff);
	void comp_dos_mass(MaterialSpecification material, prec position);
	void comp_equil_dos(prec lat_temp);
	void comp_non_equil_dos(void);
	void comp_planck_potential(void) { planck_potential=equil_planck_potential; }
	void comp_equil_planck_potential(prec acceptor_conc, prec hole_equil_dos,
									 long acceptor_degeneracy, prec acceptor_level,
									 prec band_gap, prec lattice_temp);
	void comp_equil_planck_potential(prec hole_equil_planck_pot, prec band_gap, prec lattice_temp);
	void comp_shr_lifetime(MaterialSpecification material, prec position);
	void comp_energy_lifetime(MaterialSpecification material, prec postion);
	void comp_stimulated_factor(float reduced_mass, prec mode_energy,
								prec band_gap);
	void comp_ionized_doping(void);
	void store_temperature(void) { stored_temperature=temperature; }

	//Init functions
	void init_conc(void);

// Get/Put functions.
	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TElectron::TElectron(RegionType region, TQuantumWell *qw_ptr)
	: TFreeElectron(), TBoundElectron(qw_ptr)
{
	region_type=region;
	temperature=0.0;
	stored_temperature=0.0;
	dos_mass=0.0;
	cond_mass=0.0;
	doping_conc=0.0;
	total_conc=0.0;
	total_deriv_conc_eta_c=0.0;
	total_deriv_ionized_doping_eta_c=0.0;
	total_deriv_hotcarriers=0.0;
	planck_potential=0.0;
	equil_planck_potential=0.0;
	band_edge=0.0;
	shr_lifetime=0.0;
	energy_lifetime=0.0;
    auger_coefficient=0.0;
	current=0.0;
	stimulated_factor=0.0;
	hotcarriers.b_b=0.0;
	hotcarriers.opt_ref=0.0;
	hotcarriers.opt_kin=0.0;
	hotcarriers.stim=0.0;
	hotcarriers.relax=0.0;
	hotcarriers.shr=0.0;
    hotcarriers.auger=0.0;
	hotcarriers.total=0.0;
	collision_factor=0;
	doping_degeneracy=0;
	doping_level=0.0;
	ionized_doping_conc=0.0;
}

void TElectron::comp_auger_coefficient(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	auger_coefficient=material_parameters.evaluate(MAT_ELECTRON_AUGER_COEFFICIENT,material.material_type,
												   material.alloy_type,values)*sq(normalization.conc)*normalization.time;
}

void TElectron::comp_auger_hotcarriers(prec intrinsic_conc, prec hole_conc,
									   prec hole_auger_coeff, prec rec_auger)
{
	prec n=TFreeElectron::concentration;
	prec p=hole_conc;
	prec ni=intrinsic_conc;

    if (effects & GRID_RECOMB_AUGER) {
		if (qw_ptr){
			hotcarriers.auger=(temperature+band_edge)*rec_auger;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				hotcarriers.auger=((3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
								  temperature+band_edge)*hole_auger_coeff*p*(n*p-sq(ni));
			else hotcarriers.auger=((3.0/2.0)*(temperature)+band_edge)*hole_auger_coeff*p*(n*p-sq(ni));
		}
    }
    else hotcarriers.auger=0.0;
}

void TElectron::comp_b_b_hotcarriers(prec rec_b_b)
{
	if (effects & GRID_RECOMB_B_B) {
		if (qw_ptr){
			hotcarriers.b_b=(temperature+band_edge)*rec_b_b;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				hotcarriers.b_b=((3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
								  temperature+band_edge)*rec_b_b;
			else hotcarriers.b_b=((3.0/2.0)*(temperature)+band_edge)*rec_b_b;
		}
    }
    else hotcarriers.b_b=0.0;
}

void TElectron::comp_kin_optical_generation_hotcarriers(prec rec_opt_gen, prec inc_pho_ene,
														prec band_gap, prec r_dos_mass)
{
	if (inc_pho_ene>band_gap) {
		hotcarriers.opt_kin+=rec_opt_gen*(inc_pho_ene - band_gap)*(r_dos_mass/dos_mass);
	}
	else hotcarriers.opt_kin+=0.0;
}

void TElectron::comp_ref_optical_generation_hotcarriers(prec rec_opt_gen)
{
	hotcarriers.opt_ref=band_edge*rec_opt_gen;
}

void TElectron::comp_stim_hotcarriers(prec rec_stim, prec r_dos_mass,
									  prec band_gap, prec inc_pho_ene)
{
	hotcarriers.stim=rec_stim*(r_dos_mass/dos_mass*(inc_pho_ene - band_gap)+band_edge);
}

void TElectron::comp_shr_hotcarriers(prec intrinsic_conc, prec lattice_temp, prec hole_conc,
									 prec hole_shr_lifetime, prec shr_recomb)
{
	prec n=TFreeElectron::concentration;
	prec p=hole_conc;
	prec ni=intrinsic_conc;

	if (effects & GRID_RECOMB_SHR) {
		if(qw_ptr){
			hotcarriers.shr=(temperature+band_edge)*shr_recomb;
		}
		else {
			hotcarriers.shr=1.5*(n*ni*(temperature-lattice_temp)*hole_shr_lifetime/shr_lifetime+
								 n*p*temperature-sq(ni)*lattice_temp)/
								(hole_shr_lifetime*(n+ni)+shr_lifetime*(p+ni));
			hotcarriers.shr+=band_edge*shr_recomb;
		}
	}
	else hotcarriers.shr=0.0;
}

void TElectron::comp_relax_hotcarriers(prec lat_temp)
{
	if (effects & GRID_RELAX) {
		if (fabs(temperature-lat_temp)<=1e-5) hotcarriers.relax=0.0;
		else {
			if (qw_ptr) {
				 hotcarriers.relax=TBoundElectron::concentration*(temperature - lat_temp)/energy_lifetime
				 + TFreeElectron::concentration*1.5*(temperature - lat_temp)/energy_lifetime ;
			}
			else {
				if (effects & GRID_FERMI_DIRAC)
					hotcarriers.relax=(3./2.)*total_conc*
									   fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential)*
									   (temperature - lat_temp)/energy_lifetime;
				else
					hotcarriers.relax=(3./2.)*total_conc*(temperature - lat_temp)/energy_lifetime;
			}
		}
	}
	else hotcarriers.relax=0.0;
}

void TElectron::comp_deriv_hotcarriers(Recombination recombination, prec lat_temp,
									   prec intrinsic_conc, prec hole_conc,
                                       prec hole_shr_lifetime, prec hole_auger_coeff)
{
	prec n=TFreeElectron::concentration;
	prec p=hole_conc;
	prec ni=intrinsic_conc;

	total_deriv_hotcarriers=0.0;

    if (effects & GRID_RECOMB_B_B) {
		if(qw_ptr){
			total_deriv_hotcarriers+=recombination.b_b;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				total_deriv_hotcarriers+=fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential)*
										(1.5*recombination.b_b);
			else
				total_deriv_hotcarriers+=1.5*recombination.b_b;
		}
    }

	if (effects & GRID_RECOMB_SHR) {
		if (qw_ptr) {
			total_deriv_hotcarriers+=recombination.shr;
		}
		else {
			total_deriv_hotcarriers+=1.5*(n*ni*hole_shr_lifetime/shr_lifetime+n*p)/
									 (hole_shr_lifetime*(n+ni)+shr_lifetime*(p+ni));
		}
	}

    if (effects & GRID_RECOMB_AUGER) {
		if (qw_ptr){
			total_deriv_hotcarriers+=recombination.auger;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				total_deriv_hotcarriers+=(3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
                						  hole_auger_coeff*p*(n*p-sq(ni));
			else total_deriv_hotcarriers+=(3.0/2.0)*hole_auger_coeff*p*(n*p-sq(ni));
		}
    }

	if (effects & GRID_RELAX) {
		if (fabs(temperature-lat_temp)>1e-5) {
			if (qw_ptr) {
				 total_deriv_hotcarriers+=TBoundElectron::concentration/energy_lifetime
				 + (3./2.)*TFreeElectron::concentration/energy_lifetime ;
			}
			else {
				if (effects & GRID_FERMI_DIRAC)
					total_deriv_hotcarriers+=(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
											 (3./2.)*(total_conc/energy_lifetime);
				else
					total_deriv_hotcarriers+=(3./2.)*(total_conc/energy_lifetime);
			}
		}
	}
}

void TElectron::comp_total_hotcarriers(void)
{
	hotcarriers.total=hotcarriers.b_b+hotcarriers.stim+hotcarriers.relax+
					  hotcarriers.shr+hotcarriers.auger;

	if(effects & GRID_OPTICAL_GEN) hotcarriers.total-=(hotcarriers.opt_kin+hotcarriers.opt_ref);
}

void TElectron::comp_conc(void)
{
	if (qw_ptr) {
		TBoundElectron::comp_conc();
		TFreeElectron::comp_conc(planck_potential,temperature,
								 TBoundElectron::qw_energy_top,
								 TBoundElectron::non_equil_dos);
		total_conc=TFreeElectron::concentration+TBoundElectron::concentration;
	}
	else {
		TFreeElectron::comp_conc(planck_potential);
		total_conc=TFreeElectron::concentration;
	}
}

void TElectron::comp_cond_mass(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	cond_mass=material_parameters.evaluate(MAT_ELECTRON_COND_MASS,material.material_type,
										   material.alloy_type,values);
}

void TElectron::comp_collision_factor(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	collision_factor=(long) (material_parameters.evaluate(MAT_ELECTRON_COLLISION_FACTOR,material.material_type,
														  material.alloy_type,values)*2.0);
}

void TElectron::comp_current(int start_node_number, int node_number,
							 float position, prec recombination_total)
{
	static prec prev_position, prev_current, prev_recombination;

	if (start_node_number!=node_number) {
		current=prev_current+((recombination_total+prev_recombination)/2.0)*
							 (position-prev_position);
	}
	prev_position=position;
	prev_current=current;
	prev_recombination=recombination_total;
}

void TElectron::comp_deriv_conc(void)
{
	if (qw_ptr) {
		TBoundElectron::comp_deriv_conc();
		TFreeElectron::comp_deriv_conc(planck_potential,TBoundElectron::qw_energy_top);
		total_deriv_conc_eta_c=TFreeElectron::deriv_conc_eta_c+
							   TBoundElectron::deriv_conc_eta_c;
	}
	else {
		TFreeElectron::comp_deriv_conc(planck_potential);
		total_deriv_conc_eta_c=TFreeElectron::deriv_conc_eta_c;
	}
}

void TElectron::comp_deriv_ionized_doping(void)
{
	if (effects & GRID_INCOMPLETE_IONIZATION)
		total_deriv_ionized_doping_eta_c=doping_conc*deriv_fermi(doping_level/temperature+planck_potential,
																 (prec)doping_degeneracy);
	else
		total_deriv_ionized_doping_eta_c=0.0;
}

void TElectron::comp_dos_mass(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	dos_mass=material_parameters.evaluate(MAT_ELECTRON_DOS_MASS,material.material_type,
										   material.alloy_type,values);
}

void TElectron::comp_equil_dos(prec lat_temp)
{
	TFreeElectron::comp_equil_dos(dos_mass,lat_temp);
	if (qw_ptr) TBoundElectron::comp_equil_dos();
}

void TElectron::comp_non_equil_dos(void)
{
	TFreeElectron::comp_non_equil_dos(dos_mass,temperature);
	if (qw_ptr) TBoundElectron::comp_non_equil_dos();
}

void TElectron::comp_equil_planck_potential(prec acceptor_conc, prec hole_equil_dos,
											long acceptor_degeneracy, prec acceptor_level,
											prec band_gap, prec lattice_temp)
{
	int i=0;
	prec iteration_error;
	prec new_planck_potential,update_planck_potential;
	prec conc_difference, deriv_conc_difference;
	prec doping_difference, deriv_doping_difference;
	prec conc_function, deriv_conc_function;
	prec doping_average;
	prec concentration;
	static prec old_donor_conc=0.0, old_acceptor_conc=0.0;
	static prec old_electron_equil_dos=0.0, old_hole_equil_dos=0.0;
	static long old_donor_degeneracy=0, old_acceptor_degeneracy=0;
	static prec old_donor_level=0.0, old_acceptor_level=0.0;
	static prec old_band_gap=0.0, old_lattice_temp=0.0;
	static logical old_fermi_method=FALSE, old_ionized_doping_method=FALSE;
	static prec old_result=0.0;
	logical same_values=FALSE;

	if (qw_ptr) {
		equil_planck_potential=qw_ptr->get_value(QW_ELECTRON,EQUIL_PLANCK_POT,NORMALIZED);
	}
	else {
		if ((doping_conc==old_donor_conc) && (acceptor_conc==old_acceptor_conc) &&
			(TFreeElectron::equil_dos==old_electron_equil_dos) && (hole_equil_dos==old_hole_equil_dos) &&
			(doping_degeneracy==old_donor_degeneracy) && (acceptor_degeneracy==old_acceptor_degeneracy) &&
			(doping_level==old_donor_level) && (acceptor_level==old_acceptor_level) &&
			(band_gap==old_band_gap) && (lattice_temp==old_lattice_temp)) same_values=TRUE;

		if ((effects & GRID_FERMI_DIRAC) || (effects & GRID_INCOMPLETE_IONIZATION)) {
			if ( same_values &&
				 (((effects & GRID_FERMI_DIRAC)!=0)==old_fermi_method) &&
				 (((effects & GRID_INCOMPLETE_IONIZATION)!=0)==old_ionized_doping_method) )
				 equil_planck_potential=old_result;
			else {
				new_planck_potential=equil_planck_potential;

				do {
					if (effects & GRID_FERMI_DIRAC) {
						conc_difference=TFreeElectron::equil_dos*fermi_integral_1_half(new_planck_potential)-
										hole_equil_dos*fermi_integral_1_half(-new_planck_potential-band_gap/lattice_temp);

						deriv_conc_difference=TFreeElectron::equil_dos*fermi_integral_minus_1_half(new_planck_potential)+
											  hole_equil_dos*fermi_integral_minus_1_half(-new_planck_potential-
																						  band_gap/lattice_temp);
					}
					else {
						conc_difference=TFreeElectron::equil_dos*exp(new_planck_potential)-
										hole_equil_dos*exp(-new_planck_potential-band_gap/lattice_temp);

						deriv_conc_difference=TFreeElectron::equil_dos*exp(new_planck_potential)+
											  hole_equil_dos*exp(-new_planck_potential-band_gap/lattice_temp);
					}

					if (effects & GRID_INCOMPLETE_IONIZATION) {
						doping_difference=
							acceptor_conc*fermi(acceptor_level/lattice_temp-new_planck_potential-band_gap/lattice_temp,
										  (prec)acceptor_degeneracy)-
							doping_conc*fermi(doping_level/lattice_temp+new_planck_potential,(prec)doping_degeneracy);

						deriv_doping_difference=
							-acceptor_conc*deriv_fermi(acceptor_level/lattice_temp-new_planck_potential-band_gap/lattice_temp,
										  (prec)acceptor_degeneracy)-
							doping_conc*deriv_fermi(doping_level/lattice_temp+new_planck_potential,(prec)doping_degeneracy);
					}
					else {
						doping_difference=acceptor_conc-doping_conc;

						deriv_doping_difference=0.0;
					}

					conc_function=conc_difference+doping_difference;
					deriv_conc_function=deriv_conc_difference+deriv_doping_difference;

					update_planck_potential=conc_function/deriv_conc_function;
					if (new_planck_potential!=0.0) iteration_error=fabs(update_planck_potential/new_planck_potential);
					else iteration_error=1.0;
					new_planck_potential-=update_planck_potential;
					i++;
				}
				while ((i<51) && (iteration_error>=1e-8));

				equil_planck_potential=new_planck_potential;

				old_donor_conc=doping_conc;
				old_acceptor_conc=acceptor_conc;
				old_electron_equil_dos=TFreeElectron::equil_dos;
				old_hole_equil_dos=hole_equil_dos;
				old_donor_degeneracy=doping_degeneracy;
				old_acceptor_degeneracy=acceptor_degeneracy;
				old_donor_level=doping_level;
				old_acceptor_level=acceptor_level;
				old_band_gap=band_gap;
				old_lattice_temp=lattice_temp;

				old_fermi_method=((effects & GRID_FERMI_DIRAC)!=0);
				old_ionized_doping_method=((effects & GRID_INCOMPLETE_IONIZATION)!=0);

				old_result=equil_planck_potential;
			}
		}
		else {
			if (!old_fermi_method && !old_ionized_doping_method && same_values)
				equil_planck_potential=old_result;
			else {
				doping_average=(doping_conc-acceptor_conc)/2.0;
				concentration=doping_average+sqrt(sq(doping_average)+
												  TFreeElectron::equil_dos*hole_equil_dos*exp(-band_gap/lattice_temp));
				equil_planck_potential=log(concentration/TFreeElectron::equil_dos);

				old_donor_conc=doping_conc;
				old_acceptor_conc=acceptor_conc;
				old_electron_equil_dos=TFreeElectron::equil_dos;
				old_hole_equil_dos=hole_equil_dos;
				old_donor_degeneracy=doping_degeneracy;
				old_acceptor_degeneracy=acceptor_degeneracy;
				old_donor_level=doping_level;
				old_acceptor_level=acceptor_level;
				old_band_gap=band_gap;
				old_lattice_temp=lattice_temp;

				old_fermi_method=FALSE;
				old_ionized_doping_method=FALSE;

				old_result=equil_planck_potential;
			}
		}
	}
}

void TElectron::comp_equil_planck_potential(prec hole_equil_planck_pot, prec band_gap,
											prec lattice_temp)
{
	equil_planck_potential=-hole_equil_planck_pot-band_gap/lattice_temp;
}

void TElectron::comp_shr_lifetime(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	shr_lifetime=material_parameters.evaluate(MAT_ELECTRON_SHR_LIFETIME,material.material_type,
											  material.alloy_type,values)/normalization.time;
}

void TElectron::comp_energy_lifetime(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	energy_lifetime=material_parameters.evaluate(MAT_ELECTRON_ENERGY_LIFETIME,material.material_type,
												 material.alloy_type,values)/normalization.time;
}

void TElectron::comp_stimulated_factor(float reduced_mass, prec mode_energy,
									   prec band_gap)
{
	if (mode_energy>band_gap) stimulated_factor=(reduced_mass/dos_mass)*(mode_energy-band_gap);
	else stimulated_factor=0.0;
}

void TElectron::comp_ionized_doping(void)
{
	if (effects & GRID_INCOMPLETE_IONIZATION)
		ionized_doping_conc=doping_conc*fermi(doping_level/temperature+planck_potential,
											 (prec)doping_degeneracy);
	else
		ionized_doping_conc=doping_conc;
}

void TElectron::init_conc(void)
{
	if (effects & GRID_FERMI_DIRAC)
		total_conc=TFreeElectron::equil_dos*fermi_integral_1_half(equil_planck_potential);
	else
		total_conc=TFreeElectron::equil_dos*exp(equil_planck_potential);
	TFreeElectron::concentration=total_conc;
	TBoundElectron::concentration=0.0;
}

prec TElectron::get_value(FlagType flag_type, flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case BOUND_ELECTRON: return(TBoundElectron::get_value(flag_value,scale));
		case FREE_ELECTRON: return(TFreeElectron::get_value(flag_value,scale));
		case ELECTRON:
			switch(flag_value) {
				case TEMPERATURE: return_value=temperature; break;
				case DOS_MASS: return_value=dos_mass; break;
				case DOPING_CONC: return_value=doping_conc; break;
				case DOPING_DEGENERACY: return_value=(prec)doping_degeneracy; break;
				case DOPING_LEVEL: return_value=doping_level; break;
				case IONIZED_DOPING: return_value=ionized_doping_conc; break;
				case CONCENTRATION: return_value=total_conc; break;
				case COND_MASS: return_value=cond_mass; break;
				case PLANCK_POT: return_value=planck_potential; break;
				case EQUIL_PLANCK_POT: return_value=equil_planck_potential; break;
				case BAND_EDGE: return_value=band_edge; break;
				case SHR_LIFETIME: return_value=shr_lifetime; break;
				case ENERGY_LIFETIME: return_value=energy_lifetime; break;
                case AUGER_COEFFICIENT: return_value=auger_coefficient; break;
				case CURRENT: return_value=current; break;
				case STIMULATED_FACTOR: return_value=stimulated_factor; break;
				case QUASI_FERMI: return_value=-planck_potential*temperature; break;
				case EQUIL_QUASI_FERMI: return_value=-equil_planck_potential*temperature; break;
				case B_B_HEAT:return_value=hotcarriers.b_b; break;
				case OPTICAL_GENERATION_REF:return_value=hotcarriers.opt_ref; break;
				case OPTICAL_GENERATION_KIN:return_value=hotcarriers.opt_kin;break;
				case STIM_HEAT:return_value=hotcarriers.stim; break;
				case RELAX_HEAT:return_value=hotcarriers.relax;break;
				case SHR_HEAT:return_value=hotcarriers.shr;break;
                case AUGER_HEAT:return_value=hotcarriers.auger; break;
				case TOTAL_HEAT:return_value=hotcarriers.total; break;
				case COLLISION_FACTOR: return_value=(prec)collision_factor/2.0; break;
				default: assert(FALSE); return(0.0);
			}
			if (scale==UNNORMALIZED) return_value*=get_normalize_value(ELECTRON,flag_value);
			return(return_value);
		default: assert(FALSE); return(0.0);
	}
}

void TElectron::put_value(FlagType flag_type, flag flag_value,
						  prec value, ScaleType scale)
{
	switch(flag_type) {
		case BOUND_ELECTRON: TBoundElectron::put_value(flag_value,value,scale); return;
		case FREE_ELECTRON: TFreeElectron::put_value(flag_value,value,scale); return;
		case ELECTRON:
			if (scale==UNNORMALIZED) value/=get_normalize_value(ELECTRON,flag_value);
			switch(flag_value) {
				case TEMPERATURE: temperature=value; return;
				case DOS_MASS: dos_mass=value; return;
				case DOPING_CONC: doping_conc=value; return;
				case DOPING_DEGENERACY: doping_degeneracy=(long)value; return;
				case DOPING_LEVEL: doping_level=value; return;
				case IONIZED_DOPING: ionized_doping_conc=value; return;
				case CONCENTRATION: total_conc=value; return;
				case COND_MASS: cond_mass=value; return;
				case PLANCK_POT: planck_potential=value; return;
				case EQUIL_PLANCK_POT: equil_planck_potential=value; return;
				case SHR_LIFETIME: shr_lifetime=value; return;
				case ENERGY_LIFETIME:energy_lifetime=value; return;
                case AUGER_COEFFICIENT: auger_coefficient=value; return;
				case CURRENT: current=value; return;
				case STIMULATED_FACTOR: stimulated_factor=value; return;
				case B_B_HEAT:hotcarriers.b_b=value; return;
				case OPTICAL_GENERATION_REF:hotcarriers.opt_ref=value; return;
				case OPTICAL_GENERATION_KIN:hotcarriers.opt_kin=value; return;
				case STIM_HEAT:hotcarriers.stim=value; return;
				case RELAX_HEAT:hotcarriers.relax=value; return;
				case SHR_HEAT:hotcarriers.shr=value; return;
                case AUGER_HEAT:hotcarriers.auger=value; return;
				case TOTAL_HEAT:hotcarriers.total=value; return;
				case COLLISION_FACTOR: collision_factor=(long)(value*2.0); return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

void TElectron::read_state_file(FILE *file_ptr)
{
	fread(&temperature,sizeof(temperature),1,file_ptr);
	fread(&dos_mass,sizeof(dos_mass),1,file_ptr);
	fread(&cond_mass,sizeof(cond_mass),1,file_ptr);

	fread(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fread(&doping_degeneracy,sizeof(doping_degeneracy),1,file_ptr);
	fread(&doping_level,sizeof(doping_level),1,file_ptr);
	fread(&ionized_doping_conc,sizeof(ionized_doping_conc),1,file_ptr);
	fread(&total_conc,sizeof(total_conc),1,file_ptr);
	fread(&planck_potential,sizeof(planck_potential),1,file_ptr);
	fread(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fread(&band_edge,sizeof(band_edge),1,file_ptr);
	fread(&shr_lifetime,sizeof(shr_lifetime),1,file_ptr);
	fread(&energy_lifetime,sizeof(energy_lifetime),1,file_ptr);
    fread(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
	fread(&current,sizeof(current),1,file_ptr);
	fread(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
	fread(&hotcarriers,sizeof(hotcarriers),1,file_ptr);
	fread(&collision_factor,sizeof(collision_factor),1,file_ptr);

	if (qw_ptr) TBoundElectron::read_state_file(file_ptr);
	TFreeElectron::read_state_file(file_ptr);
}

void TElectron::write_state_file(FILE *file_ptr)
{
	fwrite(&temperature,sizeof(temperature),1,file_ptr);
	fwrite(&dos_mass,sizeof(dos_mass),1,file_ptr);
	fwrite(&cond_mass,sizeof(cond_mass),1,file_ptr);

	fwrite(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fwrite(&doping_degeneracy,sizeof(doping_degeneracy),1,file_ptr);
	fwrite(&doping_level,sizeof(doping_level),1,file_ptr);
	fwrite(&ionized_doping_conc,sizeof(ionized_doping_conc),1,file_ptr);
	fwrite(&total_conc,sizeof(total_conc),1,file_ptr);
	fwrite(&planck_potential,sizeof(planck_potential),1,file_ptr);
	fwrite(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fwrite(&band_edge,sizeof(band_edge),1,file_ptr);
	fwrite(&shr_lifetime,sizeof(shr_lifetime),1,file_ptr);
	fwrite(&energy_lifetime,sizeof(energy_lifetime),1,file_ptr);
    fwrite(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
	fwrite(&current,sizeof(current),1,file_ptr);
	fwrite(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
	fwrite(&hotcarriers,sizeof(hotcarriers),1,file_ptr);
	fwrite(&collision_factor,sizeof(collision_factor),1,file_ptr);

	if (qw_ptr) TBoundElectron::write_state_file(file_ptr);
	TFreeElectron::write_state_file(file_ptr);
}

/************************************** THole Class *******************************************

class THole: protected TFreeHole, protected TBoundHole {
private:
	RegionType region_type;
protected:
	prec temperature;
	prec stored_temperature;
	prec dos_mass;
	float cond_mass;
	prec doping_conc;
	long doping_degeneracy;
	prec doping_level;
	prec ionized_doping_conc;
	prec total_conc;
	prec total_deriv_conc_eta_v;
	prec total_deriv_ionized_doping_eta_v;
	prec planck_potential;					//(Ev-Ef)/kT
	prec equil_planck_potential;
	prec band_edge;
	prec shr_lifetime;
	prec energy_lifetime;
    prec auger_coefficient;
	float current;
	prec stimulated_factor;
	Hotcarriers hotcarriers;
	prec total_deriv_hotcarriers;
	long collision_factor;

public:
	THole(RegionType region, TQuantumWell *qw_ptr);

// Comp functions
	void comp_auger_coefficient(MaterialSpecification material, prec position);
	void comp_auger_hotcarriers(prec intrinsic_conc, prec electron_conc,
    							prec electron_auger_coeff, prec rec_auger);
	void comp_b_b_hotcarriers(prec rec_b_b);
	void comp_kin_optical_generation_hotcarriers(prec rec_opt_gen, prec inc_pho_ene,
											 prec band_gap, prec r_dos_mass);
	void comp_ref_optical_generation_hotcarriers(prec rec_opt_gen);
	void comp_stim_hotcarriers(prec rec_stim, prec r_dos_mass, prec band_gap, prec inc_pho_ene);
	void comp_relax_hotcarriers(prec lat_temp);
	void comp_shr_hotcarriers(prec intrinsic_conc, prec lattice_temp, prec electron_conc,
							  prec electron_shr_lifetime, prec shr_recomb);
	void comp_total_hotcarriers(void);
	void comp_conc(void);
	void comp_cond_mass(MaterialSpecification material, prec position);
	void comp_collision_factor(MaterialSpecification material, prec position);
	void comp_current(int start_node_number, int node_number,
					  float position, prec recombination_total);
	void comp_deriv_conc(void);
	void comp_deriv_ionized_doping(void);
	void comp_deriv_hotcarriers(Recombination recombination, prec lat_temp,
    							prec intrinsic_conc, prec electron_conc,
                                prec electron_shr_lifetime, prec electron_auger_coeff);
	void comp_dos_mass(MaterialSpecification material, prec position);
	void comp_equil_dos(prec lat_temp);
	void comp_non_equil_dos(void);
	void comp_planck_potential(void) { planck_potential=equil_planck_potential; }
	void comp_equil_planck_potential(prec donor_conc, prec electron_equil_dos,
									 long donor_degeneracy, prec donor_level,
									 prec band_gap, prec lattice_temp);
	void comp_equil_planck_potential(prec hole_equil_planck_pot, prec band_gap, prec lattice_temp);
	void comp_shr_lifetime(MaterialSpecification material, prec position);
	void comp_energy_lifetime(MaterialSpecification material, prec position);
	void comp_stimulated_factor(float reduced_mass, prec mode_energy,
								prec band_gap);
	void comp_ionized_doping(void);
	void store_temperature(void) { stored_temperature=temperature; }

// Init functions
	void init_conc(void);

// Get/Put functions
	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

THole::THole(RegionType region, TQuantumWell *qw_ptr)
	: TFreeHole(), TBoundHole(qw_ptr)
{
	region_type=region;
	temperature=0.0;
	stored_temperature=0.0;
	dos_mass=0.0;
	cond_mass=0.0;
	doping_conc=0.0;
	total_conc=0.0;
	total_deriv_conc_eta_v=0.0;
	total_deriv_ionized_doping_eta_v=0.0;
	total_deriv_hotcarriers=0.0;
	planck_potential=0.0;
	equil_planck_potential=0.0;
	band_edge=0.0;
	shr_lifetime=0.0;
	energy_lifetime=0.0;
    auger_coefficient=0.0;
	current=0.0;
	stimulated_factor=0.0;
	hotcarriers.b_b=0.0;
	hotcarriers.opt_kin=0.0;
	hotcarriers.opt_ref=0.0;
	hotcarriers.stim=0.0;
	hotcarriers.relax=0.0;
	hotcarriers.shr=0.0;
    hotcarriers.auger=0.0;
	hotcarriers.total=0.0;
	collision_factor=0.0;
	doping_degeneracy=0;
	doping_level=0.0;
	ionized_doping_conc=0.0;
}

void THole::comp_auger_coefficient(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	auger_coefficient=material_parameters.evaluate(MAT_HOLE_AUGER_COEFFICIENT,material.material_type,
												   material.alloy_type,values)*sq(normalization.conc)*normalization.time;
}

void THole::comp_auger_hotcarriers(prec intrinsic_conc, prec electron_conc,
								   prec electron_auger_coeff, prec rec_auger)
{
	prec n=electron_conc;
	prec p=TFreeHole::concentration;
	prec ni=intrinsic_conc;

    if (effects & GRID_RECOMB_AUGER) {
		if (qw_ptr){
			hotcarriers.auger=(temperature+band_edge)*rec_auger;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				hotcarriers.auger=((3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
								  temperature+band_edge)*electron_auger_coeff*n*(n*p-sq(ni));
			else hotcarriers.auger=((3.0/2.0)*(temperature)+band_edge)*electron_auger_coeff*n*(n*p-sq(ni));
		}
    }
    else hotcarriers.auger=0.0;
}

void THole::comp_b_b_hotcarriers(prec rec_b_b)
{
	if (effects & GRID_RECOMB_B_B) {
		if (qw_ptr){
			hotcarriers.b_b=(temperature+band_edge)*rec_b_b;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				hotcarriers.b_b=((3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
								  temperature+band_edge)*rec_b_b;
			else hotcarriers.b_b=((3.0/2.0)*(temperature)+band_edge)*rec_b_b;
		}
    }
    else hotcarriers.b_b=0.0;
}

void THole::comp_kin_optical_generation_hotcarriers(prec rec_opt_gen, prec inc_pho_ene,
													prec band_gap, prec r_dos_mass)
{
	if (inc_pho_ene>=band_gap)
		hotcarriers.opt_kin+=rec_opt_gen*(inc_pho_ene - band_gap )*(r_dos_mass/dos_mass);
	else hotcarriers.opt_kin+=0.0;
}

void THole::comp_ref_optical_generation_hotcarriers(prec rec_opt_gen)
{
	hotcarriers.opt_ref=band_edge*rec_opt_gen;
}

void THole::comp_stim_hotcarriers(prec rec_stim,prec r_dos_mass, prec band_gap, prec inc_pho_ene)
{
	hotcarriers.stim=rec_stim*(r_dos_mass/dos_mass*(inc_pho_ene - band_gap)+band_edge);
}

void THole::comp_shr_hotcarriers(prec intrinsic_conc, prec lattice_temp, prec electron_conc,
							     prec electron_shr_lifetime, prec shr_recomb)
{
	prec n=electron_conc;
	prec p=TFreeHole::concentration;
	prec ni=intrinsic_conc;

	if (effects & GRID_RECOMB_SHR) {
		if(qw_ptr){
			hotcarriers.shr=(temperature+band_edge)*shr_recomb;
		}
		else {
			hotcarriers.shr=1.5*(p*ni*(temperature-lattice_temp)*electron_shr_lifetime/shr_lifetime+
								 n*p*temperature-sq(ni)*lattice_temp)/
								(electron_shr_lifetime*(p+ni)+shr_lifetime*(n+ni));
			hotcarriers.shr+=band_edge*shr_recomb;
		}
    }
    else hotcarriers.shr=0.0;
}

void THole::comp_relax_hotcarriers(prec lat_temp)
{
	if (effects & GRID_RELAX) {
		if (fabs(temperature-lat_temp)<=1e-5) hotcarriers.relax=0.0;
		else {
			if (qw_ptr) {
				 hotcarriers.relax=TBoundHole::concentration*(temperature - lat_temp)/energy_lifetime
				 + TFreeHole::concentration*1.5*(temperature - lat_temp)/energy_lifetime ;
			}
			else {
				if (effects & GRID_FERMI_DIRAC)
					hotcarriers.relax=(3./2.)*total_conc*
									   fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential)*
									   (temperature - lat_temp)/energy_lifetime;
				else
					hotcarriers.relax=(3./2.)*total_conc*(temperature - lat_temp)/energy_lifetime;
			}
		}
	}
	else hotcarriers.relax=0.0;
}

void THole::comp_deriv_hotcarriers(Recombination recombination, prec lat_temp,
								   prec intrinsic_conc, prec electron_conc,
                                   prec electron_shr_lifetime, prec electron_auger_coeff)
{
	prec n=electron_conc;
	prec p=TFreeHole::concentration;
	prec ni=intrinsic_conc;

	total_deriv_hotcarriers=0.0;

    if (effects & GRID_RECOMB_B_B) {
		if(qw_ptr){
			total_deriv_hotcarriers+=recombination.b_b;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				total_deriv_hotcarriers+=fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential)*
										(1.5*recombination.b_b);
			else
				total_deriv_hotcarriers+=1.5*recombination.b_b;
		}
    }

	if (effects & GRID_RECOMB_SHR) {
		if (qw_ptr) {
			total_deriv_hotcarriers+=recombination.shr;
		}
		else {
			total_deriv_hotcarriers+=1.5*(p*ni*electron_shr_lifetime/shr_lifetime+n*p)/
									 (electron_shr_lifetime*(p+ni)+shr_lifetime*(n+ni));
		}
	}

    if (effects & GRID_RECOMB_AUGER) {
		if (qw_ptr){
			total_deriv_hotcarriers+=recombination.auger;
		}
		else {
			if (effects & GRID_FERMI_DIRAC)
				total_deriv_hotcarriers+=(3.0/2.0)*(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
                						  electron_auger_coeff*n*(n*p-sq(ni));
			else total_deriv_hotcarriers+=(3.0/2.0)*electron_auger_coeff*n*(n*p-sq(ni));
		}
    }

	if (effects & GRID_RELAX) {
		if (fabs(temperature-lat_temp)>1e-5) {
			if (qw_ptr) {
				 total_deriv_hotcarriers+=TBoundHole::concentration/energy_lifetime
				 + (3./2.)*TFreeHole::concentration/energy_lifetime ;
			}
			else {
				if (effects & GRID_FERMI_DIRAC)
					total_deriv_hotcarriers+=(fermi_integral_3_half(planck_potential)/fermi_integral_1_half(planck_potential))*
											 (3./2.)*(total_conc/energy_lifetime);
				else
					total_deriv_hotcarriers+=(3./2.)*(total_conc/energy_lifetime);
			}
		}
	}
}

void THole::comp_total_hotcarriers(void)
{
	hotcarriers.total=hotcarriers.b_b+hotcarriers.stim+hotcarriers.relax+
    				  hotcarriers.shr+hotcarriers.auger;

	if(effects & GRID_OPTICAL_GEN) hotcarriers.total-=(hotcarriers.opt_kin+hotcarriers.opt_ref);
}

void THole::comp_conc(void)
{
	if (qw_ptr) {
		TBoundHole::comp_conc();
		TFreeHole::comp_conc(planck_potential,temperature,
							 TBoundHole::qw_energy_top,
							 TBoundHole::non_equil_dos);
		total_conc=TFreeHole::concentration+TBoundHole::concentration;
	}
	else {
		TFreeHole::comp_conc(planck_potential);
		total_conc=TFreeHole::concentration;
	}
}

void THole::comp_cond_mass(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	cond_mass=material_parameters.evaluate(MAT_HOLE_COND_MASS,material.material_type,
										   material.alloy_type,values);
}

void THole::comp_collision_factor(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	collision_factor=(long) (material_parameters.evaluate(MAT_HOLE_COLLISION_FACTOR,material.material_type,
														  material.alloy_type,values)*2.0);
}

void THole::comp_deriv_conc(void)
{
	if (qw_ptr) {
		TBoundHole::comp_deriv_conc();
		TFreeHole::comp_deriv_conc(planck_potential,TBoundHole::qw_energy_top);
		total_deriv_conc_eta_v=TFreeHole::deriv_conc_eta_v+
							   TBoundHole::deriv_conc_eta_v;
	}
	else {
		TFreeHole::comp_deriv_conc(planck_potential);
		total_deriv_conc_eta_v=TFreeHole::deriv_conc_eta_v;
	}
}

void THole::comp_deriv_ionized_doping(void)
{
	if (effects & GRID_INCOMPLETE_IONIZATION)
		total_deriv_ionized_doping_eta_v=doping_conc*deriv_fermi(doping_level/temperature+planck_potential,
																 (prec)doping_degeneracy);
	else
		total_deriv_ionized_doping_eta_v=0.0;
}

void THole::comp_current(int start_node_number, int node_number,
						 float position, prec recombination_total)
{
	static prec prev_position, prev_current, prev_recombination;

	if (start_node_number!=node_number) {
		current=prev_current-((recombination_total+prev_recombination)/2.0)*
							 (position-prev_position);
	}

	prev_position=position;
	prev_current=current;
	prev_recombination=recombination_total;
}

void THole::comp_dos_mass(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	dos_mass=material_parameters.evaluate(MAT_HOLE_DOS_MASS,material.material_type,
										   material.alloy_type,values);
}

void THole::comp_equil_dos(prec lat_temp)
{
	TFreeHole::comp_equil_dos(dos_mass,lat_temp);
	if (qw_ptr) TBoundHole::comp_equil_dos();
}

void THole::comp_non_equil_dos(void)
{
	TFreeHole::comp_non_equil_dos(dos_mass,temperature);
	if (qw_ptr) TBoundHole::comp_non_equil_dos();
}

void THole::comp_equil_planck_potential(prec donor_conc, prec electron_equil_dos,
										long donor_degeneracy, prec donor_level,
										prec band_gap, prec lattice_temp)
{
	int i=0;
	prec iteration_error;
	prec new_planck_potential,update_planck_potential;
	prec conc_difference, deriv_conc_difference;
	prec doping_difference, deriv_doping_difference;
	prec conc_function, deriv_conc_function;
	prec doping_average;
	prec concentration;
	static prec old_donor_conc=0.0, old_acceptor_conc=0.0;
	static prec old_electron_equil_dos=0.0, old_hole_equil_dos=0.0;
	static long old_donor_degeneracy=0, old_acceptor_degeneracy=0;
	static prec old_donor_level=0.0, old_acceptor_level=0.0;
	static prec old_band_gap=0.0, old_lattice_temp=0.0;
	static logical old_fermi_method=FALSE, old_ionized_doping_method=FALSE;
	static prec old_result=0.0;
	logical same_values=FALSE;

	if ((donor_conc==old_donor_conc) && (doping_conc==old_acceptor_conc) &&
		(electron_equil_dos==old_electron_equil_dos) && (TFreeHole::equil_dos==old_hole_equil_dos) &&
		(donor_degeneracy==old_donor_degeneracy) && (doping_degeneracy==old_acceptor_degeneracy) &&
		(donor_level==old_donor_level) && (doping_level==old_acceptor_level) &&
		(band_gap==old_band_gap) && (lattice_temp==old_lattice_temp)) same_values=TRUE;

	if ((effects & GRID_FERMI_DIRAC) || (effects & GRID_INCOMPLETE_IONIZATION)) {
		if ( same_values &&
			 (((effects & GRID_FERMI_DIRAC)!=0)==old_fermi_method) &&
			 (((effects & GRID_INCOMPLETE_IONIZATION)!=0)==old_ionized_doping_method) )
			 equil_planck_potential=old_result;
		else {
			new_planck_potential=equil_planck_potential;

			do {
				if (effects & GRID_FERMI_DIRAC) {
					conc_difference=electron_equil_dos*fermi_integral_1_half(-new_planck_potential-band_gap/lattice_temp)-
									TFreeHole::equil_dos*fermi_integral_1_half(new_planck_potential);

					deriv_conc_difference=-electron_equil_dos*fermi_integral_minus_1_half(-new_planck_potential-
																						   band_gap/lattice_temp)-
										   TFreeHole::equil_dos*fermi_integral_minus_1_half(new_planck_potential);
				}
				else {
					conc_difference=electron_equil_dos*exp(-new_planck_potential-band_gap/lattice_temp)-
									TFreeHole::equil_dos*exp(new_planck_potential);

					deriv_conc_difference=-electron_equil_dos*exp(-new_planck_potential-band_gap/lattice_temp)-
										   TFreeHole::equil_dos*exp(new_planck_potential);
				}

				if (effects & GRID_INCOMPLETE_IONIZATION) {
					doping_difference=
						doping_conc*fermi(doping_level/lattice_temp+new_planck_potential,
										 (prec)doping_degeneracy)-
						donor_conc*fermi(donor_level/lattice_temp-new_planck_potential-band_gap/lattice_temp,
										 (prec)donor_degeneracy);

					deriv_doping_difference=
						doping_conc*deriv_fermi(doping_level/lattice_temp+new_planck_potential,
												(prec)doping_degeneracy)+
						donor_conc*deriv_fermi(donor_level/lattice_temp-new_planck_potential-band_gap/lattice_temp,
											   (prec)donor_degeneracy);
				}
				else {
					doping_difference=doping_conc-donor_conc;

					deriv_doping_difference=0.0;
				}


				conc_function=conc_difference+doping_difference;
				deriv_conc_function=deriv_conc_difference+deriv_doping_difference;

				update_planck_potential=conc_function/deriv_conc_function;
				if (new_planck_potential!=0.0) iteration_error=fabs(update_planck_potential/new_planck_potential);
				else iteration_error=1.0;
				new_planck_potential-=update_planck_potential;
				i++;
			}
			while ((i<51) && (iteration_error>=1e-8));

			equil_planck_potential=new_planck_potential;

			old_donor_conc=donor_conc;
			old_acceptor_conc=doping_conc;
			old_electron_equil_dos=electron_equil_dos;
			old_hole_equil_dos=TFreeHole::equil_dos;
			old_donor_degeneracy=donor_degeneracy;
			old_acceptor_degeneracy=doping_degeneracy;
			old_donor_level=donor_level;
			old_acceptor_level=doping_level;
			old_band_gap=band_gap;
			old_lattice_temp=lattice_temp;

			old_fermi_method=((effects & GRID_FERMI_DIRAC)!=0);
			old_ionized_doping_method=((effects & GRID_INCOMPLETE_IONIZATION)!=0);

			old_result=equil_planck_potential;
		}
	}
	else {
		if (!old_fermi_method && !old_ionized_doping_method && same_values)
			equil_planck_potential=old_result;
		else {
			doping_average=(doping_conc-donor_conc)/2.0;
			concentration=doping_average+sqrt(sq(doping_average)+
											  electron_equil_dos*TFreeHole::equil_dos*exp(-band_gap/lattice_temp));
			equil_planck_potential=log(concentration/TFreeHole::equil_dos);

			old_donor_conc=donor_conc;
			old_acceptor_conc=doping_conc;
			old_electron_equil_dos=electron_equil_dos;
			old_hole_equil_dos=TFreeHole::equil_dos;
			old_donor_degeneracy=donor_degeneracy;
			old_acceptor_degeneracy=doping_degeneracy;
			old_donor_level=donor_level;
			old_acceptor_level=doping_level;
			old_band_gap=band_gap;
			old_lattice_temp=lattice_temp;

			old_fermi_method=FALSE;
			old_ionized_doping_method=FALSE;

			old_result=equil_planck_potential;
		}
	}
}

void THole::comp_equil_planck_potential(prec electron_equil_planck_pot, prec band_gap,
										prec lattice_temp)
{
	equil_planck_potential=-electron_equil_planck_pot-band_gap/lattice_temp;
}

void THole::comp_shr_lifetime(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	shr_lifetime=material_parameters.evaluate(MAT_HOLE_SHR_LIFETIME,material.material_type,
											  material.alloy_type,values)/normalization.time;
}

void THole::comp_energy_lifetime(MaterialSpecification material, prec position)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	energy_lifetime=material_parameters.evaluate(MAT_HOLE_ENERGY_LIFETIME,material.material_type,
												 material.alloy_type,values)/normalization.time;
}

void THole::comp_stimulated_factor(float reduced_mass, prec mode_energy,
								   prec band_gap)
{
	if (mode_energy>band_gap) stimulated_factor=(reduced_mass/dos_mass)*(mode_energy-band_gap);
	else stimulated_factor=0.0;
}

void THole::comp_ionized_doping(void)
{
	if (effects & GRID_INCOMPLETE_IONIZATION)
		ionized_doping_conc=doping_conc*fermi(doping_level/temperature+planck_potential,
											  (prec)doping_degeneracy);
	else
		ionized_doping_conc=doping_conc;
}

void THole::init_conc(void)
{
	if (effects & GRID_FERMI_DIRAC)
		total_conc=TFreeHole::equil_dos*fermi_integral_1_half(equil_planck_potential);
	else
		total_conc=TFreeHole::equil_dos*exp(equil_planck_potential);
	TFreeHole::concentration=total_conc;
	TBoundHole::concentration=0.0;
}

prec THole::get_value(FlagType flag_type, flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case BOUND_HOLE: return(TBoundHole::get_value(flag_value,scale));
		case FREE_HOLE: return(TFreeHole::get_value(flag_value,scale));
		case HOLE:
			switch(flag_value) {
				case TEMPERATURE: return_value=temperature; break;
				case DOS_MASS: return_value=dos_mass; break;
				case DOPING_CONC: return_value=doping_conc; break;
				case DOPING_DEGENERACY: return_value=(prec)doping_degeneracy; break;
				case DOPING_LEVEL: return_value=doping_level; break;
				case IONIZED_DOPING: return_value=ionized_doping_conc; break;
				case CONCENTRATION: return_value=total_conc; break;
				case COND_MASS: return_value=cond_mass; break;
				case PLANCK_POT: return_value=planck_potential; break;
				case EQUIL_PLANCK_POT: return_value=equil_planck_potential; break;
				case BAND_EDGE: return_value=band_edge; break;
				case SHR_LIFETIME: return_value=shr_lifetime; break;
				case ENERGY_LIFETIME: return_value=energy_lifetime; break;
                case AUGER_COEFFICIENT: return_value=auger_coefficient; break;
				case CURRENT: return_value=current; break;
				case STIMULATED_FACTOR: return_value=stimulated_factor; break;
				case QUASI_FERMI: return_value=-planck_potential*temperature; break;
				case EQUIL_QUASI_FERMI: return_value=-equil_planck_potential*temperature; break;
				case B_B_HEAT:return_value=hotcarriers.b_b; break;
				case OPTICAL_GENERATION_REF:return_value=hotcarriers.opt_ref; break;
				case OPTICAL_GENERATION_KIN:return_value=hotcarriers.opt_kin;break;
				case STIM_HEAT:return_value=hotcarriers.stim; break;
				case RELAX_HEAT:return_value=hotcarriers.relax; break;
				case SHR_HEAT:return_value=hotcarriers.shr; break;
                case AUGER_HEAT:return_value=hotcarriers.auger; break;
				case TOTAL_HEAT:return_value=hotcarriers.total; break;
				case COLLISION_FACTOR:return_value=(prec)collision_factor/2.0; break;
				default: assert(FALSE); return(0.0);
			}
			if (scale==UNNORMALIZED) return_value*=get_normalize_value(HOLE,flag_value);
			return(return_value);
		default: assert(FALSE); return(0.0);
	}
}

void THole::put_value(FlagType flag_type, flag flag_value,
						  prec value, ScaleType scale)
{
	switch(flag_type) {
		case BOUND_HOLE: TBoundHole::put_value(flag_value,value,scale); return;
		case FREE_HOLE: TFreeHole::put_value(flag_value,value,scale); return;
		case HOLE:
			if (scale==UNNORMALIZED) value/=get_normalize_value(HOLE,flag_value);
			switch(flag_value) {
				case TEMPERATURE: temperature=value; return;
				case DOS_MASS: dos_mass=value; return;
				case DOPING_CONC: doping_conc=value; return;
				case DOPING_DEGENERACY: doping_degeneracy=(long)value; return;
				case DOPING_LEVEL: doping_level=value; return;
				case IONIZED_DOPING: ionized_doping_conc=value; return;
				case CONCENTRATION: total_conc=value; return;
				case COND_MASS: cond_mass=value; return;
				case PLANCK_POT: planck_potential=value; return;
				case EQUIL_PLANCK_POT: equil_planck_potential=value; return;
				case SHR_LIFETIME: shr_lifetime=value; return;
				case ENERGY_LIFETIME: energy_lifetime=value; return;
                case AUGER_COEFFICIENT: auger_coefficient=value; return;
				case CURRENT: current=value; return;
				case STIMULATED_FACTOR: stimulated_factor=value; return;
				case B_B_HEAT:hotcarriers.b_b=value; return;
				case OPTICAL_GENERATION_REF:hotcarriers.opt_ref=value; return;
				case OPTICAL_GENERATION_KIN:hotcarriers.opt_kin=value; return;
				case STIM_HEAT:hotcarriers.stim=value; return;
				case RELAX_HEAT:hotcarriers.relax=value;return;
				case SHR_HEAT:hotcarriers.shr=value;return;
                case AUGER_HEAT:hotcarriers.auger=value; return;
				case TOTAL_HEAT:hotcarriers.total=value; return;
				case COLLISION_FACTOR:collision_factor=(long)(value*2.0); return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

void THole::read_state_file(FILE *file_ptr)
{
	fread(&temperature,sizeof(temperature),1,file_ptr);
	fread(&dos_mass,sizeof(dos_mass),1,file_ptr);
	fread(&cond_mass,sizeof(cond_mass),1,file_ptr);

	fread(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fread(&doping_degeneracy,sizeof(doping_degeneracy),1,file_ptr);
	fread(&doping_level,sizeof(doping_level),1,file_ptr);
	fread(&ionized_doping_conc,sizeof(ionized_doping_conc),1,file_ptr);
	fread(&total_conc,sizeof(total_conc),1,file_ptr);
	fread(&planck_potential,sizeof(planck_potential),1,file_ptr);
	fread(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fread(&band_edge,sizeof(band_edge),1,file_ptr);
	fread(&shr_lifetime,sizeof(shr_lifetime),1,file_ptr);
	fread(&energy_lifetime,sizeof(energy_lifetime),1,file_ptr);
    fread(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
	fread(&current,sizeof(current),1,file_ptr);
	fread(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
	fread(&hotcarriers,sizeof(hotcarriers),1,file_ptr);
	fread(&collision_factor,sizeof(collision_factor),1,file_ptr);

	if (qw_ptr) TBoundHole::read_state_file(file_ptr);
	TFreeHole::read_state_file(file_ptr);
}

void THole::write_state_file(FILE *file_ptr)
{
	fwrite(&temperature,sizeof(temperature),1,file_ptr);
	fwrite(&dos_mass,sizeof(dos_mass),1,file_ptr);
	fwrite(&cond_mass,sizeof(cond_mass),1,file_ptr);

	fwrite(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fwrite(&doping_degeneracy,sizeof(doping_degeneracy),1,file_ptr);
	fwrite(&doping_level,sizeof(doping_level),1,file_ptr);
	fwrite(&ionized_doping_conc,sizeof(ionized_doping_conc),1,file_ptr);
	fwrite(&total_conc,sizeof(total_conc),1,file_ptr);
	fwrite(&planck_potential,sizeof(planck_potential),1,file_ptr);
	fwrite(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fwrite(&band_edge,sizeof(band_edge),1,file_ptr);
	fwrite(&shr_lifetime,sizeof(shr_lifetime),1,file_ptr);
	fwrite(&energy_lifetime,sizeof(energy_lifetime),1,file_ptr);
    fwrite(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
	fwrite(&current,sizeof(current),1,file_ptr);
	fwrite(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
	fwrite(&hotcarriers,sizeof(hotcarriers),1,file_ptr);
	fwrite(&collision_factor,sizeof(collision_factor),1,file_ptr);

	if (qw_ptr) TBoundHole::write_state_file(file_ptr);
	TFreeHole::write_state_file(file_ptr);
}


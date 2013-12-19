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
#include "simgrid.h"
#include "simnode.h"
#include "simcont.h"

/************************************ class TNode **********************************************

class TNode: protected TElectron, protected THole, protected TGrid {
	friend TElectricalServices;
	friend TElectricalElement;
	friend TThermalElement;
	friend TBulkElectricalElement;
	friend TBulkThermalElement;
	friend TQWElectricalElement;
	friend TQWThermalElement;
	friend TOhmicBoundaryElement;
	friend TBoundaryThermalElement;
private:
	prec reduced_dos_mass;
	prec total_charge;
	prec intrinsic_conc;
	Recombination recombination;
	RecombDerivParam deriv_local_gain;
	RecombDerivParam deriv_recomb;
	RadiativeHeat radiative_heat;
	prec total_heat;
public:
	TNode(int node_number, RegionType region_type, TQuantumWell *qw_ptr=NULL);
	void comp_charge(void)
		{ total_charge=(THole::total_conc-TElectron::total_conc+
						TElectron::ionized_doping_conc-THole::ionized_doping_conc); }
	void comp_current(FlagType flag_type, int start_node_number);
	void comp_deriv_conc(void)
		{ TElectron::comp_deriv_conc(); THole::comp_deriv_conc();
		  TElectron::comp_deriv_ionized_doping(); THole::comp_deriv_ionized_doping(); }
	void comp_deriv_gain(void);
	void comp_deriv_recomb(void);
	void comp_deriv_electron_hotcarriers(void)
		{ TElectron::comp_deriv_hotcarriers(recombination,lattice_temp,intrinsic_conc,
											TFreeHole::concentration, THole::shr_lifetime); }
	void comp_deriv_hole_hotcarriers(void)
		{ THole::comp_deriv_hotcarriers(recombination,lattice_temp); }
	void comp_deriv_thermal_conduct(void)
		{ TGrid::comp_deriv_thermal_conduct(); TGrid::comp_deriv_lateral_conduct(); }
	void comp_field(int start_node_number)
		{ TGrid::comp_field(start_node_number,total_charge); }
	void comp_incident_optical_field(int start_node_number)
		{ TGrid::comp_incident_optical_field(start_node_number); }
	void comp_incident_total_poynting(prec intensity_multiplier, int max_overflow_count)
		{ TGrid::comp_incident_total_poynting(intensity_multiplier, max_overflow_count); }
	void comp_mode_optical_field(int start_node_number)
		{ TGrid::comp_mode_optical_field(start_node_number); }
	void comp_intrinsic_conc(void);
	void comp_gain(void);
	void comp_optical_generation(void);
	void comp_b_b_recombination(void);
	void comp_b_b_heat(void);
	void comp_shr_recombination(void);
    void comp_auger_recombination(void);
	void comp_stim_recombination(void);
	void comp_stim_heat(void);
	void comp_total_recombination(void);
	void comp_radiative_heat(void);
	void comp_heat(void);
	void comp_reduced_dos_mass(void);
	void comp_value(FlagType flag_type, flag flag_value);
	void store_temperature(void) { TElectron::store_temperature();
		THole::store_temperature(); TGrid::store_temperature(); }
	void init_value(FlagType flag_type, flag flag_value,
					TNode *ref_node, TContact *ref_contact);
	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TNode::TNode(int node_number, RegionType region_type, TQuantumWell *qw_ptr)
	: TElectron(region_type, qw_ptr),
	  THole(region_type, qw_ptr),
	  TGrid(node_number,region_type, qw_ptr)
{
	total_charge=0.0;
	intrinsic_conc=0.0;
	reduced_dos_mass=0.0;

	recombination.b_b=0.0;
	recombination.shr=0.0;
    recombination.auger=0.0;
	recombination.stim=0.0;
	recombination.total=0.0;
	recombination.opt_gen=0.0;

	deriv_recomb.eta_c=0.0;
	deriv_recomb.eta_v=0.0;

	deriv_local_gain.eta_c=0.0;
	deriv_local_gain.eta_v=0.0;

	radiative_heat.total=0.0;
	radiative_heat.stim=0.0;
	radiative_heat.b_b=0.0;
	radiative_heat.opt_gen=0.0;

	total_heat=0.0;
}

void TNode::comp_current(FlagType flag_type, int start_node_number)
{
	if (flag_type==ELECTRON) {
		TElectron::comp_current(start_node_number, node_number,
								position, recombination.total);
	}

	if (flag_type==HOLE) {
		THole::comp_current(start_node_number, node_number,
							position,recombination.total);
	}
}

void  TNode::comp_deriv_gain(void)
{
	if (TGrid::qw_ptr) {
		deriv_local_gain.eta_c=TGrid::qw_ptr->deriv_qw_gain.eta_c/TGrid::qw_ptr->qw_length;
		deriv_local_gain.eta_v=TGrid::qw_ptr->deriv_qw_gain.eta_v/TGrid::qw_ptr->qw_length;
	}
	else {
		deriv_local_gain.eta_c=-mode_refractive_index.absorption*
								deriv_fermi(TElectron::stimulated_factor/TElectron::temperature-TElectron::planck_potential);
		deriv_local_gain.eta_v=-mode_refractive_index.absorption*
								deriv_fermi(-THole::stimulated_factor/THole::temperature+THole::planck_potential);
	}
}

void TNode::comp_deriv_recomb(void)
{
	prec electron_shr_lifetime, hole_shr_lifetime;
    prec electron_auger_coef, hole_auger_coef;
	prec n,p,ni;
	prec deriv_n_eta_c, deriv_p_eta_v;

	deriv_recomb.eta_c=0;
	deriv_recomb.eta_v=0;

	if (TGrid::qw_ptr) {
		deriv_recomb.eta_c=TGrid::qw_ptr->deriv_recomb_2D.eta_c/
						   TGrid::qw_ptr->qw_length;
		deriv_recomb.eta_v=TGrid::qw_ptr->deriv_recomb_2D.eta_v/
						   TGrid::qw_ptr->qw_length;
	}
	else {
		deriv_n_eta_c=TFreeElectron::deriv_conc_eta_c;
		deriv_p_eta_v=TFreeHole::deriv_conc_eta_v;

		n=TFreeElectron::concentration;
		p=TFreeHole::concentration;

        ni=intrinsic_conc;

		if (TGrid::effects & GRID_RECOMB_B_B){
			deriv_recomb.eta_c=b_b_recomb_const*p*deriv_n_eta_c;
			deriv_recomb.eta_v=b_b_recomb_const*n*deriv_p_eta_v;
		}

		if (TGrid::effects & GRID_RECOMB_SHR) {
			electron_shr_lifetime=TElectron::shr_lifetime;
			hole_shr_lifetime=THole::shr_lifetime;

			deriv_recomb.eta_c+=(deriv_n_eta_c*(electron_shr_lifetime*sq(p)+
											   (electron_shr_lifetime+hole_shr_lifetime)*p*ni+
												sq(ni)*hole_shr_lifetime))/
											   (sq(electron_shr_lifetime*(p+ni)+hole_shr_lifetime*(n+ni)));

			deriv_recomb.eta_v+=(deriv_p_eta_v*(hole_shr_lifetime*sq(n)+
											   (electron_shr_lifetime+hole_shr_lifetime)*n*ni+
												sq(ni)*electron_shr_lifetime))/
											   (sq(electron_shr_lifetime*(p+ni)+hole_shr_lifetime*(n+ni)));
		}

        if (TGrid::effects & GRID_RECOMB_AUGER) {
        	electron_auger_coef=TElectron::auger_coefficient;
            hole_auger_coef=THole::auger_coefficient;

            deriv_recomb.eta_c+=deriv_n_eta_c*(p*(electron_auger_coef*n+hole_auger_coef*p)+
            								   electron_auger_coef*(n*p-sq(ni)));
            deriv_recomb.eta_v+=deriv_p_eta_v*(n*(electron_auger_coef*n+hole_auger_coef*p)+
            								   hole_auger_coef*(n*p-sq(ni)));
        }
	}

	if (TGrid::effects & GRID_RECOMB_STIM) {
		comp_deriv_gain();

		deriv_recomb.eta_c+=deriv_local_gain.eta_c*group_velocity*sq(mode_field.total_magnitude)*total_mode_photons;
		deriv_recomb.eta_v+=deriv_local_gain.eta_v*group_velocity*sq(mode_field.total_magnitude)*total_mode_photons;
	}
}

void TNode::comp_intrinsic_conc(void)
{
	intrinsic_conc=sqrt(TFreeElectron::equil_dos*TFreeHole::equil_dos)*
				   exp(-band_gap/(2.0*lattice_temp));

	if (TGrid::effects & GRID_FERMI_DIRAC) {
		intrinsic_conc*=sqrt((fermi_integral_1_half(TElectron::equil_planck_potential)/exp(TElectron::equil_planck_potential))*
							 (fermi_integral_1_half(THole::equil_planck_potential)/exp(THole::equil_planck_potential)));
	}
}

void TNode::comp_gain(void)
{
	if (TGrid::qw_ptr) {
		mode_refractive_index.local_gain=TGrid::qw_ptr->mode_gain/TGrid::qw_ptr->qw_length;
	}
	else {
		mode_refractive_index.local_gain=mode_refractive_index.absorption*
										(fermi(TElectron::stimulated_factor/TElectron::temperature-
											   TElectron::planck_potential)-
										 fermi(-THole::stimulated_factor/THole::temperature+
												THole::planck_potential));
	}
}

void TNode::comp_optical_generation(void)
{
	complex i(0.0,1.0);
	prec wave_vector;
	prec new_recombination;

	wave_vector=2.0*SIM_pi/get_value(GRID_OPTICAL,INCIDENT_PHOTON_WAVELENGTH,NORMALIZED);

	new_recombination=(incident_refractive_index.absorption*(incident_field.forward_poynting+
															 incident_field.reverse_poynting)-
					   wave_vector*real(1.0/incident_impedance)*real(i*2.0*real(incident_field.reverse_field*
																		   conj(incident_field.forward_field))/
																	 conj(incident_impedance)))/incident_photon_energy;
	recombination.opt_gen+=new_recombination;

	radiative_heat.opt_gen+=new_recombination*incident_photon_energy;

	TElectron::comp_kin_optical_generation_hotcarriers(new_recombination, incident_photon_energy,
													   band_gap, reduced_dos_mass);
	THole::comp_kin_optical_generation_hotcarriers(new_recombination, incident_photon_energy,
												   band_gap, reduced_dos_mass);
}

void TNode::comp_b_b_recombination(void)
{
	if (TGrid::effects & GRID_RECOMB_B_B) {
		if (TGrid::qw_ptr) {
			recombination.b_b=TGrid::qw_ptr->recombination_2D.b_b/
							  TGrid::qw_ptr->qw_length;
		}
		else recombination.b_b=b_b_recomb_const*(TFreeElectron::concentration*TFreeHole::concentration-
												 sq(intrinsic_conc));
	}
	else recombination.b_b=0.0;
}

void TNode::comp_b_b_heat(void)
{
	if (TGrid::effects & GRID_RECOMB_B_B) {
		if (TGrid::qw_ptr) {
			radiative_heat.b_b=TGrid::qw_ptr->spontaneous_heat_2D/TGrid::qw_ptr->qw_length;
		}
		else {
			radiative_heat.b_b=recombination.b_b*band_gap;
		}
	}
	else radiative_heat.b_b=0.0;
}

void TNode::comp_shr_recombination(void)
{
	prec p=TFreeHole::concentration;
	prec n=TFreeElectron::concentration;

	if (TGrid::effects & GRID_RECOMB_SHR) {
		if (TGrid::qw_ptr) {
			recombination.shr=TGrid::qw_ptr->recombination_2D.shr/
							  TGrid::qw_ptr->qw_length;
		}
		else {
			recombination.shr=(p*n-sq(intrinsic_conc))/
							  (TElectron::shr_lifetime*(p+intrinsic_conc)+
								   THole::shr_lifetime*(n+intrinsic_conc));
		}
	}
	else recombination.shr=0.0;
}

void TNode::comp_auger_recombination(void)
{
	prec p=TFreeHole::concentration;
	prec n=TFreeElectron::concentration;

	if (TGrid::effects & GRID_RECOMB_AUGER) {
		if (TGrid::qw_ptr) {
			recombination.auger=TGrid::qw_ptr->recombination_2D.auger/
            					TGrid::qw_ptr->qw_length;
		}
		else {
			recombination.auger=(TElectron::auger_coefficient*n+THole::auger_coefficient*p)*
            					(n*p-sq(intrinsic_conc));
		}
	}
	else recombination.auger=0.0;
}

void TNode::comp_stim_recombination(void)
{
	if (TGrid::effects & GRID_RECOMB_STIM) {
		comp_gain();
		recombination.stim=mode_refractive_index.local_gain*group_velocity*sq(mode_field.total_magnitude)*total_mode_photons;
	}
	else recombination.stim=0.0;
}

void TNode::comp_stim_heat(void)
{
	if (TGrid::effects & GRID_RECOMB_STIM) {
		radiative_heat.stim=recombination.stim*mode_photon_energy;
	}
	else radiative_heat.stim=0.0;
}

void TNode::comp_total_recombination(void)
{
	recombination.total=recombination.b_b+recombination.shr+
    					recombination.stim+recombination.auger;

	if (TGrid::effects & GRID_OPTICAL_GEN)
		recombination.total-=recombination.opt_gen;
}

void TNode::comp_radiative_heat(void)
{
	radiative_heat.total=radiative_heat.b_b+radiative_heat.stim;

	if (TGrid::effects & GRID_OPTICAL_GEN)
		radiative_heat.total-=radiative_heat.opt_gen;
}

void TNode::comp_heat(void)
{
	total_heat=TElectron::hotcarriers.total+THole::hotcarriers.total
			 -(TElectron::hotcarriers.b_b+TElectron::hotcarriers.stim)
			 -(THole::hotcarriers.b_b+THole::hotcarriers.stim);

	if (TGrid::effects & GRID_OPTICAL_GEN)
		total_heat+=TElectron::hotcarriers.opt_kin+TElectron::hotcarriers.opt_ref+
					THole::hotcarriers.opt_kin+THole::hotcarriers.opt_ref;
}

void TNode::comp_reduced_dos_mass(void)
{
	reduced_dos_mass=(TElectron::dos_mass*THole::dos_mass)/
					 (TElectron::dos_mass+THole::dos_mass);
}

void TNode::comp_value(FlagType flag_type, flag flag_value)
{
	switch(flag_type) {
		case ELECTRON:
			switch(flag_value) {
				case TEMPERATURE: TElectron::temperature=lattice_temp; return;
				case EQUIL_DOS: TElectron::comp_equil_dos(lattice_temp); return;
				case NON_EQUIL_DOS: TElectron::comp_non_equil_dos(); return;
				case MOBILITY: TElectron::comp_mobility(material,position, lattice_temp, TElectron::temperature,
														TElectron::doping_conc, THole::doping_conc); return;
				case CONCENTRATION:	TElectron::comp_conc();	return;
				case DOS_MASS: TElectron::comp_dos_mass(material,position); return;
				case COND_MASS: TElectron::comp_cond_mass(material,position); return;
				case IONIZED_DOPING: TElectron::comp_ionized_doping(); return;
				case EQUIL_PLANCK_POT:
					if(TElectron::doping_conc>=THole::doping_conc) {
						TElectron::comp_equil_planck_potential(THole::doping_conc,TFreeHole::equil_dos,
															   THole::doping_degeneracy, THole::doping_level,
															   band_gap,lattice_temp);
						THole::comp_equil_planck_potential(TElectron::equil_planck_potential,
														   band_gap,lattice_temp);
					}
					else {
						THole::comp_equil_planck_potential(TElectron::doping_conc,TFreeElectron::equil_dos,
														   TElectron::doping_degeneracy, TElectron::doping_level,
														   band_gap,lattice_temp);
						TElectron::comp_equil_planck_potential(THole::equil_planck_potential,
															   band_gap,lattice_temp);
					}
					return;
				case PLANCK_POT: TElectron::comp_planck_potential(); return;
				case BAND_EDGE:
					TElectron::band_edge=-potential-electron_affinity;
					return;
				case SHR_LIFETIME: TElectron::comp_shr_lifetime(material,position); return;
				case ENERGY_LIFETIME: TElectron::comp_energy_lifetime(material,position); return;
                case AUGER_COEFFICIENT: TElectron::comp_auger_coefficient(material,position); return;
				case STIMULATED_FACTOR:
					TElectron::comp_stimulated_factor(reduced_dos_mass,mode_photon_energy,band_gap);
					return;
				case B_B_HEAT: TElectron::comp_b_b_hotcarriers(recombination.b_b); return;
				case OPTICAL_GENERATION_REF:
					TElectron::comp_ref_optical_generation_hotcarriers(recombination.opt_gen);
					return;
				case STIM_HEAT:
					TElectron::comp_stim_hotcarriers(recombination.stim,reduced_dos_mass,
													 band_gap,incident_photon_energy);
					return;
				case RELAX_HEAT: TElectron::comp_relax_hotcarriers(lattice_temp); return;
				case SHR_HEAT: TElectron::comp_shr_hotcarriers(intrinsic_conc, lattice_temp, TFreeHole::concentration,
															   THole::shr_lifetime,recombination.shr); return;
                case AUGER_HEAT: TElectron::comp_auger_hotcarriers(intrinsic_conc, TFreeHole::concentration,
                												   THole::auger_coefficient, recombination.auger);
                	return;
				case TOTAL_HEAT: TElectron::comp_total_hotcarriers(); return;
				case COLLISION_FACTOR: TElectron::comp_collision_factor(material,position); return;
				default: assert(FALSE); return;
			}
		case HOLE:
			switch(flag_value) {
				case TEMPERATURE: THole::temperature=lattice_temp; return;
				case EQUIL_DOS: THole::comp_equil_dos(lattice_temp); return;
				case NON_EQUIL_DOS: THole::comp_non_equil_dos(); return;
				case MOBILITY: THole::comp_mobility(material,position,lattice_temp,THole::temperature,
													TElectron::doping_conc, THole::doping_conc); return;
				case CONCENTRATION:	THole::comp_conc(); return;
				case DOS_MASS: THole::comp_dos_mass(material,position); return;
				case COND_MASS: THole::comp_cond_mass(material,position); return;
				case IONIZED_DOPING: THole::comp_ionized_doping(); return;
				case EQUIL_PLANCK_POT: comp_value(ELECTRON,EQUIL_PLANCK_POT); return;
				case PLANCK_POT: THole::comp_planck_potential(); return;
				case BAND_EDGE:
					THole::band_edge=-potential-electron_affinity-band_gap;
					return;
				case SHR_LIFETIME: THole::comp_shr_lifetime(material,position); return;
				case ENERGY_LIFETIME: THole::comp_energy_lifetime(material,position); return;
                case AUGER_COEFFICIENT: THole::comp_auger_coefficient(material,position); return;
				case STIMULATED_FACTOR:
					THole::comp_stimulated_factor(reduced_dos_mass,mode_photon_energy,band_gap);
					return;
				case B_B_HEAT: THole::comp_b_b_hotcarriers(recombination.b_b); return;
				case OPTICAL_GENERATION_REF:
					THole::comp_ref_optical_generation_hotcarriers(recombination.opt_gen); return;
				case STIM_HEAT:
					THole::comp_stim_hotcarriers(recombination.stim,reduced_dos_mass,band_gap,incident_photon_energy);
					return;
				case RELAX_HEAT:THole::comp_relax_hotcarriers(lattice_temp); return;
				case SHR_HEAT:THole::comp_shr_hotcarriers(recombination.shr, lattice_temp, TFreeElectron::concentration,
                										  TElectron::shr_lifetime,recombination.shr); return;
                case AUGER_HEAT: THole::comp_auger_hotcarriers(intrinsic_conc, TFreeElectron::concentration,
                											   TElectron::auger_coefficient, recombination.auger);
                	return;
				case TOTAL_HEAT:THole::comp_total_hotcarriers(); return;
				case COLLISION_FACTOR: THole::comp_collision_factor(material,position); return;
				default: assert(FALSE); return;
			}
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case TEMPERATURE:
					lattice_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);
					return;
				case ELECTRON_AFFINITY: comp_electron_affinity(); return;
				case PERMITIVITY: comp_permitivity(); return;
				case THERMAL_CONDUCT: comp_thermal_conductivity(); return;
				case LATERAL_THERMAL_CONDUCT: comp_lateral_thermal_conduct(); return;
				case BAND_GAP: comp_band_gap(); return;
				case B_B_RECOMB_CONSTANT: comp_b_b_recomb_const(); return;
				default: assert(FALSE); return;
			}
		case GRID_OPTICAL:
			switch(flag_value) {
				case INCIDENT_IMPEDANCE_REAL:
				case INCIDENT_IMPEDANCE_IMAG: comp_incident_impedance(); return;
				case MODE_IMPEDANCE_REAL:
				case MODE_IMPEDANCE_IMAG: comp_mode_impedance(); return;
				case MODE_GAIN: comp_gain(); return;
				case INCIDENT_ABSORPTION:
				case INCIDENT_REFRACTIVE_INDEX: comp_incident_refractive_index(); return;
				case MODE_ABSORPTION:
				case MODE_REFRACTIVE_INDEX: comp_mode_refractive_index(); return;
				default: assert(FALSE); return;
			}
		case NODE:
			switch(flag_value) {
				case TOTAL_CHARGE: comp_charge(); return;
				case INTRINSIC_CONC: comp_intrinsic_conc(); return;
				case SHR_RECOMB: comp_shr_recombination(); return;
                case AUGER_RECOMB: comp_auger_recombination(); return;
				case B_B_RECOMB: comp_b_b_recombination(); return;
				case B_B_HEAT: comp_b_b_heat(); return;
				case STIM_RECOMB: comp_stim_recombination(); return;
				case STIM_HEAT: comp_stim_heat(); return;
				case TOTAL_RECOMB: comp_total_recombination(); return;
				case TOTAL_RADIATIVE_HEAT: comp_radiative_heat(); return;
				case TOTAL_HEAT: comp_heat(); return;
				case OPTICAL_GENERATION: comp_optical_generation(); return;
				case REDUCED_DOS_MASS: comp_reduced_dos_mass(); return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

void TNode::init_value(FlagType flag_type, flag flag_value,
					   TNode *ref_node, TContact *ref_contact)
{
	flag contact_effects;

    if (ref_contact!=NULL)
    	contact_effects=ref_contact->get_value(EFFECTS,NORMALIZED);
    else
    	contact_effects=VALUE_NONE;

	switch(flag_type) {
		case ELECTRON:
			switch(flag_value) {
				case CONCENTRATION:	TElectron::init_conc(); return;
				case PLANCK_POT:
                	if (contact_effects & CONTACT_SCHOTTKY) {
						TElectron::planck_potential=
							-(ref_contact->get_value(BARRIER_HEIGHT,NORMALIZED)+
							  ref_node->electron_affinity+ref_node->potential-
							  electron_affinity-potential)/TElectron::temperature;
                    }
                    else {
						TElectron::planck_potential=
							-(ref_node->get_value(ELECTRON,QUASI_FERMI,NORMALIZED)+
							  ref_node->electron_affinity+ref_node->potential-
							  electron_affinity-potential)/TElectron::temperature;
                    }
					return;
				default: assert(FALSE); return;
			}
		case HOLE:
			switch(flag_value) {
				case CONCENTRATION: THole::init_conc(); return;
				case PLANCK_POT:
                	if (contact_effects & CONTACT_SCHOTTKY) {
    					THole::planck_potential=
							-(band_gap-(ref_contact->get_value(BARRIER_HEIGHT,NORMALIZED)+
										ref_node->electron_affinity+ref_node->potential-
										electron_affinity-potential))/THole::temperature;
                    }
                    else {
						THole::planck_potential=
							-(band_gap-(ref_node->band_gap-
										ref_node->get_value(HOLE,QUASI_FERMI,NORMALIZED)+
										ref_node->electron_affinity+ref_node->potential-
										electron_affinity-potential))/THole::temperature;
                    }
					return;
				default: assert(FALSE); return;
			}
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case POTENTIAL:
                	potential=ref_node->get_value(ELECTRON,QUASI_FERMI,NORMALIZED)+
                    		  ref_node->electron_affinity-
                              electron_affinity-get_value(ELECTRON,QUASI_FERMI,NORMALIZED);
					return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

prec TNode::get_value(FlagType flag_type, flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case ELECTRON: return(TElectron::get_value(flag_type,flag_value,scale));
		case FREE_HOLE:
		case BOUND_HOLE:
		case HOLE: return(THole::get_value(flag_type,flag_value,scale));
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case EFFECTS: return_value=(prec)TGrid::effects; break;
				case TOTAL_CURRENT: return_value=TElectron::current+THole::current; break;
				default: return(TGrid::get_value(flag_type,flag_value,scale));
			}
			break;
		case GRID_OPTICAL:
			return(TGrid::get_value(flag_type,flag_value,scale));
		case NODE:
			switch(flag_value) {
				case TOTAL_CHARGE: return_value=total_charge; break;
				case INTRINSIC_CONC: return_value=intrinsic_conc; break;
				case TOTAL_RECOMB: return_value=recombination.total; break;
				case SHR_RECOMB: return_value=recombination.shr; break;
				case B_B_RECOMB: return_value=recombination.b_b; break;
                case AUGER_RECOMB: return_value=recombination.auger; break;
				case STIM_RECOMB: return_value=recombination.stim; break;
				case OPTICAL_GENERATION: return_value=recombination.opt_gen; break;
				case TOTAL_RADIATIVE_HEAT: return_value=radiative_heat.total; break;
				case TOTAL_HEAT: return_value=total_heat; break;
				case B_B_HEAT: return_value=radiative_heat.b_b; break;
				case STIM_HEAT: return_value=radiative_heat.stim; break;
				case OPTICAL_GENERATION_HEAT: return_value=radiative_heat.opt_gen; break;
				case REDUCED_DOS_MASS: return_value=reduced_dos_mass; break;
				case TOTAL_CURRENT: return_value=TElectron::current+THole::current; break;
				default: assert(FALSE); return(0.0);
			}
	}
	if (scale==UNNORMALIZED) return_value*=get_normalize_value(flag_type,flag_value);
	return(return_value);
}

void TNode::put_value(FlagType flag_type, flag flag_value, prec value,
					  ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(flag_type,flag_value);

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case ELECTRON: TElectron::put_value(flag_type,flag_value,value,NORMALIZED); return;
		case FREE_HOLE:
		case BOUND_HOLE:
		case HOLE: THole::put_value(flag_type,flag_value,value,NORMALIZED); return;
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case EFFECTS:
					environment.set_effects_change_flags(GRID_ELECTRICAL,TGrid::effects^(flag)value);
					TGrid::effects=(flag)value;
					TElectron::effects=(flag)value;
					THole::effects=(flag)value;
					return;
				default: TGrid::put_value(flag_type,flag_value,value,NORMALIZED); return;
			}
		case GRID_OPTICAL:
			TGrid::put_value(flag_type,flag_value,value,NORMALIZED); return;
		case NODE:
			switch(flag_value) {
				case TOTAL_CHARGE: total_charge=value; return;
				case INTRINSIC_CONC: intrinsic_conc=value; return;
				case TOTAL_RECOMB: recombination.total=value; return;
				case SHR_RECOMB: recombination.shr=value; return;
				case B_B_RECOMB: recombination.b_b=value; return;
                case AUGER_RECOMB: recombination.auger=value; return;
				case STIM_RECOMB: recombination.stim=value; return;
				case OPTICAL_GENERATION: recombination.opt_gen=value; return;
				case TOTAL_RADIATIVE_HEAT: radiative_heat.total=value; return;
				case TOTAL_HEAT: total_heat=value; return;
				case B_B_HEAT: radiative_heat.b_b=value; return;
				case STIM_HEAT: radiative_heat.stim=value; return;
				case OPTICAL_GENERATION_HEAT: radiative_heat.opt_gen=value; return;
				case REDUCED_DOS_MASS: reduced_dos_mass=value; return;
				default: assert(FALSE); return;
			}
	}
}

void TNode::read_state_file(FILE *file_ptr)
{
	fread(&reduced_dos_mass,sizeof(reduced_dos_mass),1,file_ptr);
	fread(&intrinsic_conc,sizeof(intrinsic_conc),1,file_ptr);
	fread(&total_charge,sizeof(total_charge),1,file_ptr);
	fread(&recombination,sizeof(recombination),1,file_ptr);
	fread(&radiative_heat,sizeof(radiative_heat),1,file_ptr);
	fread(&total_heat,sizeof(total_heat),1,file_ptr);

	TGrid::read_state_file(file_ptr);
	TElectron::read_state_file(file_ptr);
	THole::read_state_file(file_ptr);
}

void TNode::write_state_file(FILE *file_ptr)
{
	fwrite(&reduced_dos_mass,sizeof(reduced_dos_mass),1,file_ptr);
	fwrite(&intrinsic_conc,sizeof(intrinsic_conc),1,file_ptr);
	fwrite(&total_charge,sizeof(total_charge),1,file_ptr);
	fwrite(&recombination,sizeof(recombination),1,file_ptr);
	fwrite(&radiative_heat,sizeof(radiative_heat),1,file_ptr);
	fwrite(&total_heat,sizeof(total_heat),1,file_ptr);

	TGrid::write_state_file(file_ptr);
	TElectron::write_state_file(file_ptr);
	THole::write_state_file(file_ptr);
}


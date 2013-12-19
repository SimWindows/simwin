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


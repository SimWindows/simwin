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
											TFreeHole::concentration,
                                            THole::shr_lifetime, THole::auger_coefficient); }
	void comp_deriv_hole_hotcarriers(void)
		{ THole::comp_deriv_hotcarriers(recombination,lattice_temp,intrinsic_conc,
        							    TFreeElectron::concentration,
                                        TElectron::shr_lifetime, TElectron::auger_coefficient); }
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



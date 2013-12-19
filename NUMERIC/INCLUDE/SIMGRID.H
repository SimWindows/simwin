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

class TGrid {
protected:
	int node_number;
	prec position;
	RegionType region_type;
	NodeType node_type;
	flag effects;
	TQuantumWell *qw_ptr;
	MaterialSpecification material;
	prec radius;
	prec lattice_temp;
	prec stored_lattice_temp;
	prec electron_affinity;
	prec permitivity;
	prec thermal_conduct;
	prec deriv_therm_cond_temp;
	prec lateral_thermal_conduct;
	prec deriv_lateral_cond_temp;
	prec b_b_recomb_const;
	prec band_gap;
	prec potential;
	prec field;
	prec incident_photon_energy;
	ComplexRefractiveIndex incident_refractive_index;
	complex incident_impedance;
	OpticalField incident_field;
	prec mode_photon_energy;
	ComplexRefractiveIndex mode_refractive_index;
	complex mode_impedance;
	OpticalField mode_field;
	prec total_mode_photons;
	prec group_velocity;

public:
	TGrid(int node_num, RegionType region, TQuantumWell *qw);

	void comp_b_b_recomb_const(void);
	void comp_band_gap(void);
	void comp_deriv_thermal_conduct(void);
	void comp_deriv_lateral_conduct(void);
	void comp_electron_affinity(void);
	void comp_field(int start_node_number, prec total_charge);
	void comp_incident_optical_field(int start_node_number);
	void comp_incident_refractive_index(void);
	void comp_incident_impedance(void);
	void comp_incident_total_poynting(prec intensity_multiplier, int max_overflow_count);
	void comp_mode_optical_field(int start_node_number);
	void comp_mode_refractive_index(void);
	void comp_mode_impedance(void);
	void comp_permitivity(void);
	void comp_thermal_conductivity(void);
	void comp_lateral_thermal_conduct(void);
	void store_temperature(void) { stored_lattice_temp=lattice_temp; }
	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



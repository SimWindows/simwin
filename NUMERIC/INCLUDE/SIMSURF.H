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

class TSurface {
private:
	TNode *surface_node;
	int surface_node_number;
	TNode *second_node;
	int second_node_number;
	flag effects;
	prec temp;
	prec electron_temp;
	prec hole_temp;
	prec thermal_conduct;
	prec incident_refractive_index;
	OpticalField incident_field;
	prec mode_refractive_index;
	OpticalField mode_field;

public:
	TSurface(TNode* surface_node_ptr, TNode* next_node_ptr);
	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);
	void comp_incident_surface_field(void);
	void comp_incident_internal_field(void);
	void comp_emitted_total_poynting(prec multiplier);
	prec comp_input_total_poynting(int spectral_component);
	void comp_mode_surface_field(void);
	void comp_mode_internal_field(void);
	void comp_value(flag flag_value);
	void init_forward_incident_field(void);
	void init_reverse_incident_field(void);
	void init_forward_mode_field(void);
	void init_reverse_mode_field(void);
	void init_value(flag flag_value);
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



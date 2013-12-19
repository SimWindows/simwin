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

class TMode {
private:
	TNode** grid_ptr;
	flag effects;
	prec total_photons;
	prec previous_total_photons;
	prec mode_gain;
	prec previous_mode_gain;
	prec total_spont;
	prec previous_total_spont;
	prec photon_lifetime;
	prec spont_factor;
	prec group_velocity;
	prec mirror_loss;
	prec waveguide_loss;
	prec energy;
public:
	TMode(TNode** grid);
	void init(void);

	void comp_group_velocity(int start_node, int end_node);
	void comp_mirror_loss(prec reflectivity_1, prec reflectivity_2,
						  prec length);
	void comp_mode_gain(int start_node, int end_node, prec cavity_area);
	void comp_mode_normalization(int start_node, int end_node, prec cavity_area);
	void comp_mode_optical_field(int start_node, int end_node);
	void comp_photon_lifetime(void);
	void comp_total_spontaneous(int start_node, int end_node, prec cavity_area);
	error field_iterate(prec& iteration_error, prec initial_error, int iteration_number);
	error photon_iterate(prec& iteration_error);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



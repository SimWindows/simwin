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

class TContact {
private:
	TNode *contact_node;
	int contact_node_number;
	TNode *second_node;
	int second_node_number;
	flag effects;
	float bias;
	prec built_in_pot;
	prec electron_recomb_vel;
	prec equil_electron_conc;
	prec hole_recomb_vel;
	prec equil_hole_conc;
    prec barrier_height;

public:
	TContact(TNode* contact_node_ptr, TNode* next_node_ptr);
	void init(void);

	void comp_built_pot(void);
	void comp_electron_current(void);
	void comp_equil_electron_conc(void);
	void comp_hole_current(void);
	void comp_equil_hole_conc(void);
	void comp_contact_field(void);
	void comp_value(flag flag_type);
	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



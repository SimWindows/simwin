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

class TDevice {
private:
	logical modified;
	TDeviceFileInput device_input;
	flag device_effects;
	StatusType current_status;
	SolveType current_solution;
	short curr_inner_elect_iter;
	short curr_inner_therm_iter;
	short curr_inner_mode_iter;
	short curr_outer_optic_iter;
	short curr_outer_therm_iter;
	FundamentalParam curr_elect_error;
	prec curr_optic_error;
	prec curr_mode_error;
	prec curr_therm_error;
	short grid_points;
	TNode **grid_ptr;
	short quantum_wells;
	TQuantumWell **qw_ptr;
	short number_contacts;
	TContact **contact_ptr;
	short number_surfaces;
	TSurface **surface_ptr;
	TCavity *cavity_ptr;
	TSolution *solution_ptr;

// Constructor/Destructor
public:
	TDevice(TDeviceFileInput new_device_input);
	TDevice(FILE *file_ptr);
	~TDevice(void);

// get_value/put_value functions.
public:
	int get_number_objects(FlagType flag_type);
	prec get_value(FlagType flag_type, flag flag_value, int number=0,
					ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
					int start_object=-1, int end_object=-1,
					ScaleType scale=UNNORMALIZED);

// General Get/Put functions
public:
	short get_node(prec position, short start_node=-1, short end_node=-1,
				   ScaleType scale=UNNORMALIZED);

// Init functions
public:
	void init_device(void);
	void init_value(FlagType flag_type, flag flag_value,
					int ref_node_number=0,
					int start_object=-1, int end_object=-1);
private:
	void init_device_param(void);

// Comp functions
public:
	void comp_value(FlagType flag_type, flag flag_value,
					int start_object=-1, int end_object=-1);
private:
	void comp_grid_value(FlagType flag_type, flag flag_value, int start_object, int end_object);
	void comp_current(void);
	void comp_field(void);
	void comp_optical_generation(int start_object, int end_object);

// Read/Write functions
public:
	void read_data_file(const char *filename);
	void write_data_file(ofstream& output_file, TValueFlag write_flags,
						 FlagType ref_flag_type=(FlagType)NULL,
						 flag ref_flag_value=(flag)NULL);
private:
	void read_state_file(FILE *file_ptr);
public:
	void write_state_file(FILE *file_ptr);

// Misc functions
public:
	logical ismodified(void) { return(modified); }
	void enable_modified(logical enable) { modified=enable; }
	void solve(void);
	void update_solution_param(void);
private:
	void establish_grid(void);
	void process_input_param(void);
};




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

class TEnvironment {
private:
	logical undo_ready;
	string undo_filepath;
    logical stop_solution;
    logical solving;
	TDevice *device_ptr;
	TValueFlag recompute_flags;
	TValueFlag update_flags;
	TEffectFlag effects_change_flags;
	float max_electrical_error;
	float max_optic_error;
	float max_thermal_error;
	float coarse_mode_error;
	float fine_mode_error;
	short max_inner_elect_iter;
	short max_inner_therm_iter;
	short max_inner_mode_iter;
	short max_outer_optic_iter;
	short max_outer_therm_iter;
	prec clamp_value;
	prec temp_clamp_value;
	prec temp_relax_value;
	flag env_effects;
	prec temperature;
	prec radius;
	OpticalParam optical_param;
	Spectrum optical_spectrum;
	prec spectrum_multiplier;

// Constructor/Destructor
public:
	TEnvironment(void);
	~TEnvironment(void) { delete_device(); delete_spectrum(); delete_undo_file(); }

// Get/Put functions
public:
	prec get_value(FlagType flag_type, flag flag_value,
				   int object=0, ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   int start_object=-1, int end_object=-1,
				   ScaleType scale=UNNORMALIZED);
	int get_number_objects(FlagType flag_type);
	short get_node(prec position, short start_node=-1, short end_node=-1,
				   ScaleType scale=UNNORMALIZED);
	int get_spectral_comp(prec wavelength, int start_comp=-1, int end_comp=-1,
						  ScaleType scale=UNNORMALIZED);

// Init functions
public:
	void init_environment(void);
	void init_device(void);
	void init_value(FlagType flag_type, flag flag_value,
					int ref_node_number=0, int start_object=-1, int end_object=-1);

// Comp. Functions
	void comp_value(FlagType flag_type, flag flag_value, int start_object=-1, int end_object=-1);
	void solve(void);


// Device functions
public:
	void delete_device(void);
	logical device(void) { return(device_ptr!=(TDevice *)0); }
	logical ismodified(void);
	logical canundo(void);
	void undo(void);
    void set_stop_solution(logical stop) { stop_solution=stop; }
    logical do_stop_solution(void) { return(stop_solution); }
    logical is_solving(void) { return(solving); }

// File functions
public:
	void load_file(const char *filename);
	void read_data_file(const char *filename);
	void write_data_file(const char *filename, TValueFlag write_flags,
						  FlagType ref_flag_type=(FlagType)NULL,
						  flag ref_flag_value=(flag)NULL);
	void write_state_file(const char *filename);
private:
	void read_state_file(FILE *file_ptr);
	void delete_undo_file(void)
		{ if (access(undo_filepath.c_str(),0)==0) remove(undo_filepath.c_str()); }

// Spectrum functions
public:
	void add_spectral_comp(void);
	void load_spectrum(const char *filename);
	void delete_spectrum(void);

// Flag functions
private:
	void effects_change_to_compute_flags(void);
	void update_to_compute_flags(void);
public:
	logical process_recompute_flags(void);
	void set_update_flags(FlagType flag_type, flag flag_value)
		{ update_flags.set(flag_type,flag_value); }
	void clear_update_flags(FlagType flag_type, flag flag_value)
		{ update_flags.clear(flag_type,flag_value); }
	void set_effects_change_flags(FlagType flag_type, flag flag_value)
		{ effects_change_flags.set(flag_type,flag_value); }
	void clear_effects_change_flags(FlagType flag_type, flag flag_value)
		{ effects_change_flags.clear(flag_type,flag_value); }

// Debug Functions
#ifndef NDEBUG
	logical recompute_flags_any_set(void) { return(recompute_flags.any_set()); }
	logical update_flags_any_set(void) { return(update_flags.any_set()); }
	logical effects_change_flags_any_set(void) { return(effects_change_flags.any_set()); }
#endif
};


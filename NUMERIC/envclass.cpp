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
#include "simdev.h"
#include "simparse.h"

/********************************** class TEnvironment *****************************************

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

*/

TEnvironment::TEnvironment(void)
{
	device_ptr=(TDevice *)0;
	temperature=300.0;
	radius=100.0;
	spectrum_multiplier=1.0;
	optical_param.number_wavelengths=0;
	optical_param.start_pos=0.0;
	optical_param.end_pos=0.0;
	optical_spectrum.first_wavelength=(OpticalParam *)0;
	optical_spectrum.last_wavelength=(OpticalParam *)0;
	env_effects=ENV_SPEC_ENTIRE_DEVICE | ENV_SPEC_LEFT_INCIDENT | ENV_CLAMP_POTENTIAL;
	undo_ready=FALSE;
	undo_filepath="";
    stop_solution=FALSE;
    solving=FALSE;

	max_electrical_error=1e-8;
	max_optic_error=1e-8;
	max_thermal_error=1e-8;
	coarse_mode_error=1e-3;
	fine_mode_error=1e-4;
	max_inner_elect_iter=15;
	max_inner_therm_iter=15;
	max_outer_optic_iter=15;
	max_outer_therm_iter=15;
	max_inner_mode_iter=15;
	clamp_value=6.0;
	temp_clamp_value=6.0;
	temp_relax_value=1.0;
}

prec TEnvironment::get_value(FlagType flag_type, flag flag_value,
							 int object, ScaleType scale)
{
	prec return_value;
	OpticalComponent *current_wavelength;
	int i;

	switch(flag_type) {
		case ENVIRONMENT:
			switch(flag_value) {
				case TEMPERATURE: return_value=temperature; break;
				case RADIUS: return_value=radius; break;
				case EFFECTS: return_value=(prec)env_effects; break;
				case MAX_ELECTRICAL_ERROR: return_value=(prec) max_electrical_error; break;
				case MAX_THERMAL_ERROR: return_value=(prec) max_thermal_error; break;
				case MAX_OPTIC_ERROR: return_value=(prec) max_optic_error; break;
				case COARSE_MODE_ERROR: return_value=(prec) coarse_mode_error; break;
				case FINE_MODE_ERROR: return_value=(prec) fine_mode_error; break;
				case MAX_INNER_ELECT_ITER: return_value=(prec) max_inner_elect_iter; break;
				case MAX_INNER_THERM_ITER: return_value=(prec) max_inner_therm_iter; break;
				case MAX_INNER_MODE_ITER: return_value=(prec) max_inner_mode_iter; break;
				case MAX_OUTER_OPTIC_ITER: return_value=(prec) max_outer_optic_iter; break;
				case MAX_OUTER_THERM_ITER: return_value=(prec) max_outer_therm_iter; break;
				case POT_CLAMP_VALUE: return_value=clamp_value; break;
				case TEMP_CLAMP_VALUE: return_value=temp_clamp_value; break;
				case TEMP_RELAX_VALUE: return_value=temp_relax_value; break;
				case SPEC_START_POSITION: return_value=optical_param.start_pos; break;
				case SPEC_END_POSITION: return_value=optical_param.end_pos; break;
				case SPECTRUM_MULTIPLIER: return_value=spectrum_multiplier; break;
				default: assert(FALSE); return(0.0);
			}
			break;
		case SPECTRUM:
			if (optical_param.number_wavelengths==0) return(0.0);

			if (object>=optical_param.number_wavelengths)
				current_wavelength=optical_spectrum.last_wavelength;
			else {
				current_wavelength=optical_spectrum.first_wavelength;
				for (i=0;i<object;i++) {
					current_wavelength=current_wavelength->next_wavelength;
				}
			}

			switch(flag_value) {
				case INCIDENT_INPUT_INTENSITY: return_value=current_wavelength->input_intensity; break;
				case INCIDENT_EMITTED_INTENSITY: return_value=current_wavelength->output_intensity; break;
				case INCIDENT_PHOTON_WAVELENGTH: return_value=1.242/current_wavelength->energy; break;
				case INCIDENT_PHOTON_ENERGY: return_value=current_wavelength->energy; break;
				case INCIDENT_REFLECT_INTENSITY: return_value=current_wavelength->reflected_intensity; break;
				default: assert(FALSE); return(0.0);
			}
			break;
		default:
			assert(device());
			return(device_ptr->get_value(flag_type,flag_value,object,scale));
	}

	if (scale==UNNORMALIZED) return(return_value);
	else return(return_value/get_normalize_value(flag_type,flag_value));
}

void TEnvironment::put_value(FlagType flag_type, flag flag_value, prec value,
							 int start_object, int end_object, ScaleType scale)
{
	OpticalComponent *current_wavelength;
	int i,comp_number;
	prec prev_value;

	switch(flag_type) {
		case ENVIRONMENT:
			if (scale==NORMALIZED) value*=get_normalize_value(ENVIRONMENT,flag_value);
			switch(flag_value) {
				case TEMPERATURE:
					prev_value=temperature;
					temperature=value;
					if (prev_value!=temperature) set_update_flags(ENVIRONMENT,TEMPERATURE);
					return;
				case RADIUS:
					prev_value=radius;
					radius=value;
					if (prev_value!=radius) set_update_flags(ENVIRONMENT,RADIUS);
					return;
				case EFFECTS:
					set_effects_change_flags(ENVIRONMENT,env_effects^(flag)value);
					env_effects=(flag)value;
					return;
				case MAX_ELECTRICAL_ERROR: max_electrical_error=(float)value; return;
				case MAX_THERMAL_ERROR: max_thermal_error=(float)value; return;
				case MAX_OPTIC_ERROR: max_optic_error=(float)value; return;
				case COARSE_MODE_ERROR: coarse_mode_error=(float)value; return;
				case FINE_MODE_ERROR: fine_mode_error=(float)value; return;
				case MAX_INNER_ELECT_ITER: max_inner_elect_iter=(short)value; return;
				case MAX_INNER_THERM_ITER: max_inner_therm_iter=(short)value; return;
				case MAX_INNER_MODE_ITER: max_inner_mode_iter=(short)value; return;
				case MAX_OUTER_OPTIC_ITER: max_outer_optic_iter=(short)value; return;
				case MAX_OUTER_THERM_ITER: max_outer_therm_iter=(short)value; return;
				case POT_CLAMP_VALUE: clamp_value=value; return;
				case TEMP_CLAMP_VALUE: temp_clamp_value=value; return;
				case TEMP_RELAX_VALUE: temp_relax_value=value; return;
				case SPEC_START_POSITION:
					prev_value=optical_param.start_pos;
					optical_param.start_pos=value;
					if (prev_value != optical_param.start_pos)
						set_update_flags(ENVIRONMENT,SPEC_START_POSITION);
					return;
				case SPEC_END_POSITION:
					prev_value=optical_param.end_pos;
					optical_param.end_pos=value;
					if (prev_value != optical_param.end_pos)
						set_update_flags(ENVIRONMENT,SPEC_END_POSITION);
					return;
				case SPECTRUM_MULTIPLIER:
					prev_value=spectrum_multiplier;
					spectrum_multiplier=value;
					if (prev_value != spectrum_multiplier)
						set_update_flags(ENVIRONMENT,SPECTRUM_MULTIPLIER);
					return;
				default: assert(FALSE); return;
			}
		case SPECTRUM:
			if (scale==NORMALIZED) value*=get_normalize_value(SPECTRUM,flag_value);

			if (end_object==-1) {
				if (start_object==-1) {
					start_object=0;
					end_object=optical_param.number_wavelengths-1;
				}
				else end_object=start_object;
			}
			else if (start_object>end_object) swap(start_object,end_object);

			for (comp_number=start_object;comp_number<=end_object;comp_number++) {
				current_wavelength=optical_spectrum.first_wavelength;
				for (i=0;i<comp_number;i++) {
					current_wavelength=current_wavelength->next_wavelength;
				}

				switch(flag_value) {
					case INCIDENT_INPUT_INTENSITY:
						prev_value=current_wavelength->input_intensity;
						current_wavelength->input_intensity=value;
						if (prev_value != current_wavelength->input_intensity)
							set_update_flags(SPECTRUM,INCIDENT_INPUT_INTENSITY);
						return;
					case INCIDENT_EMITTED_INTENSITY:
						current_wavelength->output_intensity=value;
						return;
					case INCIDENT_REFLECT_INTENSITY:
						current_wavelength->reflected_intensity=value;
						return;
					case INCIDENT_PHOTON_WAVELENGTH:
						prev_value=current_wavelength->energy;
						current_wavelength->energy=(1.242/value);
						if (prev_value != current_wavelength->energy)
							set_update_flags(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH);
						return;
					case INCIDENT_PHOTON_ENERGY:
						prev_value=current_wavelength->energy;
						current_wavelength->energy=value;
						if (prev_value != current_wavelength->energy)
							set_update_flags(SPECTRUM,INCIDENT_PHOTON_ENERGY);
						return;
					default: assert(FALSE); return;
				}
			}
			break;
		default:
			assert(device());
			device_ptr->put_value(flag_type,flag_value,value,start_object,end_object,scale);
	}
}

int TEnvironment::get_number_objects(FlagType flag_type)
{
	switch(flag_type) {
		case ENVIRONMENT: return(1);
		case SPECTRUM: return(optical_param.number_wavelengths);
		case DEVICE:
			if (device()) return(1);
			else return(0);
		default:
			if (device()) return(device_ptr->get_number_objects(flag_type));
			else return(0);
	}
}

short TEnvironment::get_node(prec position, short start_node, short end_node,ScaleType scale)
{
	assert(device());
	return(device_ptr->get_node(position,start_node,end_node,scale));
}

int TEnvironment::get_spectral_comp(prec wavelength, int start_comp, int end_comp,
									ScaleType scale)
{
	int test_comp, return_comp;
	prec test_wavelength;
	prec start_wavelength, end_wavelength;

	if (optical_param.number_wavelengths<=1) return(0);

	if (scale==NORMALIZED) wavelength*=get_normalize_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH);

	if (start_comp==-1) start_comp=0;
	if (end_comp==-1) end_comp=optical_param.number_wavelengths-1;

	if ((end_comp-start_comp)==1) {
		start_wavelength=get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,start_comp);
		end_wavelength=get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,end_comp);
		if ((wavelength-start_wavelength)<(end_wavelength-wavelength)) return_comp=start_comp;
		else return_comp=end_comp;
	}
	else {
		test_comp=(int)(start_comp+(end_comp-start_comp)/2);
		test_wavelength=get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,test_comp);

		if (test_wavelength>wavelength) return_comp=get_spectral_comp(wavelength,start_comp,test_comp);
		else return_comp=get_spectral_comp(wavelength,test_comp,end_comp);
	}
	return(return_comp);
}

void TEnvironment::init_environment(void)
{
	put_value(ENVIRONMENT,TEMPERATURE,300.0);
	put_value(ENVIRONMENT,RADIUS,100.0);
	put_value(ENVIRONMENT,SPECTRUM_MULTIPLIER,1.0);
	if (device_ptr) {
		put_value(ENVIRONMENT,SPEC_START_POSITION,device_ptr->get_value(CONTACT,POSITION,0));
		put_value(ENVIRONMENT,SPEC_END_POSITION,device_ptr->get_value(CONTACT,POSITION,1));
	}
	else {
		put_value(ENVIRONMENT,SPEC_START_POSITION,0.0);
		put_value(ENVIRONMENT,SPEC_END_POSITION,0.0);
	}
	put_value(ENVIRONMENT,EFFECTS,(prec) (ENV_SPEC_ENTIRE_DEVICE | ENV_SPEC_LEFT_INCIDENT));

	delete_spectrum();
	process_recompute_flags();
}

void TEnvironment::init_device(void)
{
	if (device()) device_ptr->init_device();
}

void TEnvironment::init_value(FlagType flag_type, flag flag_value,
							  int ref_node_number, int start_object, int end_object)
{
	switch(flag_type) {
		case ENVIRONMENT:
		case SPECTRUM: assert(FALSE); return;
		default:
			assert(device());
			device_ptr->init_value(flag_type,flag_value,ref_node_number,start_object,end_object);
	}
}

void TEnvironment::comp_value(FlagType flag_type, flag flag_value,
							  int start_object, int end_object)
{
	switch(flag_type) {
		case ENVIRONMENT:
		case SPECTRUM: assert(FALSE); return;
		default:
			assert(device());
			device_ptr->comp_value(flag_type,flag_value,start_object,end_object);
	}
}

void TEnvironment::solve(void)
{
	extern char undo_filename[];
	extern char executable_path[];

	if (device()) {
    	solving=TRUE;
		if (env_effects & ENV_UNDO_SIMULATION) {
			undo_filepath=string(executable_path)+string(undo_filename);
			write_state_file(undo_filepath.c_str());
			undo_ready=TRUE;
		}
		device_ptr->solve();
        stop_solution=FALSE;
        solving=FALSE;
	}
}

void TEnvironment::delete_device(void)
{
	int i;

	if (device()) {
		undo_ready=FALSE;
		delete device_ptr;
		device_ptr=(TDevice *)0;

		if (env_effects & ENV_SPEC_ENTIRE_DEVICE) {
			optical_param.start_pos=0.0;
			optical_param.end_pos=0.0;
		}

		for (i=0;i<optical_param.number_wavelengths;i++) {
			put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,get_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,i),i);
			put_value(SPECTRUM,INCIDENT_REFLECT_INTENSITY,0.0,i);
		}
	}
}

logical TEnvironment::ismodified(void)
{
	if (device()) return(device_ptr->ismodified());
	else return(FALSE);
}

logical TEnvironment::canundo(void)
{
	logical result;

	result=(env_effects & ENV_UNDO_SIMULATION) && undo_ready && !undo_filepath.is_null() && device() && (!solving);
	if (result) result&=access(undo_filepath.c_str(),0)==0;
	return(result);
}

void TEnvironment::undo(void)
{
	assert(canundo());
	load_file(undo_filepath.c_str());
	device_ptr->enable_modified(TRUE);
}

void TEnvironment::load_file(const char *filename)
{
	FileType file_type;
	FILE *file_ptr;
	extern char state_string[], state_version_string[];
	extern int state_string_size, state_version_string_size;
	char new_state_string[40], new_state_version_string[40];
	TParseDevice *device_parser;
	TDeviceFileInput device_input;
	prec contact_pos_0, contact_pos_1;

	delete_device();

	file_ptr=fopen(filename,"rb");
	if (!file_ptr) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	fread(new_state_string,state_string_size,1,file_ptr);
	if (!strncmp(new_state_string,state_string,state_string_size)) {
		file_type=STATE_FILE;
		fread(new_state_version_string,state_version_string_size,1,file_ptr);
		if (!strncmp(new_state_version_string,state_version_string,state_version_string_size))
			read_state_file(file_ptr);
		else {
			error_handler.set_error(ERROR_FILE_OLD_STATE,0,"",filename);
			fclose(file_ptr);
			return;
		}
		fclose(file_ptr);
	}
	else {
		fclose(file_ptr);
		file_type=INPUT_FILE;
		device_parser=new TParseDevice(filename);
		if (!error_handler.fail()) device_input=device_parser->parse_device();
		delete device_parser;
		if (!error_handler.fail()) device_input.check_device();
		if (!error_handler.fail())
			device_ptr = new TDevice(device_input);
	}

	if (error_handler.fail()) {
		delete_device();
		return;
	}
	else {
		if (file_type==INPUT_FILE) {
			if (device_ptr && (env_effects & ENV_SPEC_ENTIRE_DEVICE)) {
				contact_pos_0=device_ptr->get_value(CONTACT,POSITION,0);
				contact_pos_1=device_ptr->get_value(CONTACT,POSITION,1);
				if (env_effects & ENV_SPEC_LEFT_INCIDENT) {
					put_value(ENVIRONMENT,SPEC_START_POSITION,contact_pos_0);
					put_value(ENVIRONMENT,SPEC_END_POSITION,contact_pos_1);
				}
				else {
					put_value(ENVIRONMENT,SPEC_START_POSITION,contact_pos_1);
					put_value(ENVIRONMENT,SPEC_END_POSITION,contact_pos_0);
				}
			}
			init_device();
		}
	}
}

void TEnvironment::read_data_file(const char *filename)
{
	if (device()) device_ptr->read_data_file(filename);
}

void TEnvironment::write_data_file(const char *filename, TValueFlag write_flags,
									FlagType ref_flag_type,
									flag ref_flag_value)
{
	int i,j;
	flag test_flag, valid_flag;
	int precision=4;
	int max_bit;
	logical env_dep=FALSE, spec_dep=FALSE;
	ofstream output_file(filename);

	if (!output_file) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	if (write_flags.any_set(ENVIRONMENT)) env_dep=TRUE;
	if (write_flags.any_set(SPECTRUM)) spec_dep=TRUE;

	output_file.setf(ios::scientific,ios::floatfield);
	output_file.precision(precision);

	if (env_dep) {
		test_flag=1;
		max_bit=bit_position(write_flags.get_max(ENVIRONMENT));
		valid_flag=write_flags.get_valid(ENVIRONMENT);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(ENVIRONMENT,test_flag))
				output_file << get_short_string(ENVIRONMENT,test_flag) << ',';
			test_flag<<=1;
		}

		output_file << '\n';

		test_flag=1;
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(ENVIRONMENT,test_flag))
				output_file << get_value(ENVIRONMENT,test_flag) << ',';
			test_flag<<=1;
		}
		output_file << '\n';
	}

	if (spec_dep) {
		write_flags.clear(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH);
		output_file << get_short_string(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH);

		test_flag=1;
		max_bit=bit_position(write_flags.get_max(SPECTRUM));
		valid_flag=write_flags.get_valid(SPECTRUM);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(SPECTRUM,test_flag))
				output_file << ',' << get_short_string(SPECTRUM,test_flag);
			test_flag<<=1;
		}

		output_file << '\n';

		for (i=0;i<get_number_objects(SPECTRUM);i++) {
			output_file << get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,i);

			test_flag=1;
			for (j=0;j<=max_bit;j++) {
				if ((valid_flag & test_flag) && write_flags.is_set(SPECTRUM,test_flag)) output_file << ',' <<
																						get_value(SPECTRUM,test_flag,i);
				test_flag<<=1;
			}
			output_file << '\n';
		}
	}

	if (device()) device_ptr->write_data_file(output_file,write_flags,ref_flag_type,ref_flag_value);
	output_file.close();
}

void TEnvironment::write_state_file(const char *filename)
{
	int i;
	extern char state_string[], state_version_string[];
	extern int state_string_size, state_version_string_size;
	FILE *file_ptr;
	prec energy,intensity_in,intensity_out;

	file_ptr=fopen(filename,"wb");
	if (!file_ptr) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	fwrite(state_string,state_string_size,1,file_ptr);
	fwrite(state_version_string,state_version_string_size,1,file_ptr);

	fwrite(&normalization,sizeof(normalization),1,file_ptr);

	fwrite(&max_electrical_error,sizeof(max_electrical_error),1,file_ptr);
	fwrite(&max_thermal_error,sizeof(max_thermal_error),1,file_ptr);
	fwrite(&max_optic_error,sizeof(max_optic_error),1,file_ptr);
	fwrite(&coarse_mode_error,sizeof(coarse_mode_error),1,file_ptr);
	fwrite(&fine_mode_error,sizeof(fine_mode_error),1,file_ptr);
	fwrite(&max_inner_elect_iter,sizeof(max_inner_elect_iter),1,file_ptr);
	fwrite(&max_inner_therm_iter,sizeof(max_inner_therm_iter),1,file_ptr);
	fwrite(&max_outer_optic_iter,sizeof(max_outer_optic_iter),1,file_ptr);
	fwrite(&max_outer_therm_iter,sizeof(max_outer_therm_iter),1,file_ptr);
	fwrite(&max_inner_mode_iter,sizeof(max_inner_mode_iter),1,file_ptr);
	fwrite(&clamp_value,sizeof(clamp_value),1,file_ptr);
	fwrite(&temp_clamp_value,sizeof(temp_clamp_value),1,file_ptr);
	fwrite(&temp_relax_value,sizeof(temp_relax_value),1,file_ptr);
	fwrite(&env_effects,sizeof(env_effects),1,file_ptr);
	fwrite(&temperature,sizeof(temperature),1,file_ptr);
	fwrite(&radius,sizeof(radius),1,file_ptr);
	fwrite(&optical_param,sizeof(optical_param),1,file_ptr);
	fwrite(&spectrum_multiplier,sizeof(spectrum_multiplier),1,file_ptr);

	for (i=0;i<optical_param.number_wavelengths;i++) {
		energy=get_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,i);
		intensity_in=get_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,i);
		intensity_out=get_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,i);

		fwrite(&energy,sizeof(energy),1,file_ptr);
		fwrite(&intensity_in,sizeof(intensity_in),1,file_ptr);
		fwrite(&intensity_out,sizeof(intensity_out),1,file_ptr);
	}

	if (device()) device_ptr->write_state_file(file_ptr);

	fclose(file_ptr);
}

void TEnvironment::read_state_file(FILE *file_ptr)
{
	int i, char_test;
	prec energy, input_intensity, output_intensity;
	int number_wavelengths;

	fread(&normalization,sizeof(normalization),1,file_ptr);

	fread(&max_electrical_error,sizeof(max_electrical_error),1,file_ptr);
	fread(&max_thermal_error,sizeof(max_thermal_error),1,file_ptr);
	fread(&max_optic_error,sizeof(max_optic_error),1,file_ptr);
	fread(&coarse_mode_error,sizeof(coarse_mode_error),1,file_ptr);
	fread(&fine_mode_error,sizeof(fine_mode_error),1,file_ptr);
	fread(&max_inner_elect_iter,sizeof(max_inner_elect_iter),1,file_ptr);
	fread(&max_inner_therm_iter,sizeof(max_inner_therm_iter),1,file_ptr);
	fread(&max_outer_optic_iter,sizeof(max_outer_optic_iter),1,file_ptr);
	fread(&max_outer_therm_iter,sizeof(max_outer_therm_iter),1,file_ptr);
	fread(&max_inner_mode_iter,sizeof(max_inner_mode_iter),1,file_ptr);
	fread(&clamp_value,sizeof(clamp_value),1,file_ptr);
	fread(&temp_clamp_value,sizeof(temp_clamp_value),1,file_ptr);
	fread(&temp_relax_value,sizeof(temp_relax_value),1,file_ptr);
	fread(&env_effects,sizeof(env_effects),1,file_ptr);
	fread(&temperature,sizeof(temperature),1,file_ptr);
	fread(&radius,sizeof(radius),1,file_ptr);
	delete_spectrum();
	fread(&optical_param,sizeof(optical_param),1,file_ptr);
	fread(&spectrum_multiplier,sizeof(spectrum_multiplier),1,file_ptr);
	number_wavelengths=optical_param.number_wavelengths;
	optical_param.number_wavelengths=0;

	for (i=0;i<number_wavelengths;i++) {
		fread(&energy,sizeof(energy),1,file_ptr);
		fread(&input_intensity,sizeof(input_intensity),1,file_ptr);
		fread(&output_intensity,sizeof(output_intensity),1,file_ptr);

		add_spectral_comp();

		put_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,energy,i);
		put_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,input_intensity,i);
		put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,output_intensity,i);
	}

	char_test=getc(file_ptr);

	if (char_test!=EOF) {
		ungetc(char_test,file_ptr);
		device_ptr = new TDevice(file_ptr);
	}

	fclose(file_ptr);
	effects_change_flags.clear_all();
	update_flags.clear_all();
	recompute_flags.clear_all();
}

void TEnvironment::add_spectral_comp(void)
{
	if (optical_param.number_wavelengths) {
		optical_spectrum.last_wavelength->next_wavelength=new OpticalComponent;
		if (!optical_spectrum.last_wavelength->next_wavelength) {
			error_handler.set_error(ERROR_MEM_SPECTRAL_COMP,0,"","");
			return;
		}
		optical_spectrum.last_wavelength=optical_spectrum.last_wavelength->next_wavelength;
	}
	else {
		optical_spectrum.first_wavelength=new OpticalComponent;
		if (!optical_spectrum.first_wavelength) {
			error_handler.set_error(ERROR_MEM_SPECTRAL_COMP,0,"","");
			return;
		}
		optical_spectrum.last_wavelength=optical_spectrum.first_wavelength;
	}

	optical_param.number_wavelengths++;

	optical_spectrum.last_wavelength->energy=0.0;
	optical_spectrum.last_wavelength->input_intensity=0.0;
	optical_spectrum.last_wavelength->output_intensity=0.0;
	optical_spectrum.last_wavelength->reflected_intensity=0.0;
	optical_spectrum.last_wavelength->next_wavelength=(OpticalComponent *)0;
}

void TEnvironment::load_spectrum(const char *filename)
{
	FILE *file_ptr;
	float wave_length,intensity;
	int number_wavelengths=0;

	file_ptr=fopen(filename,"r");

	if (!file_ptr) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	delete_spectrum();

	while(!feof(file_ptr)) {
		fscanf(file_ptr,"%f,%f",&wave_length,&intensity);

		if (!feof(file_ptr)) {
			add_spectral_comp();
			if (error_handler.fail()) return;
			put_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,wave_length,number_wavelengths);
			put_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,intensity,number_wavelengths);
			put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,intensity,number_wavelengths);
			put_value(SPECTRUM,INCIDENT_REFLECT_INTENSITY,0,number_wavelengths);
			number_wavelengths++;
		}
	}

	fclose(file_ptr);
}

void TEnvironment::delete_spectrum(void)
{
	int i;
	OpticalComponent *curr_wavelength, *next_wavelength;

	if (optical_param.number_wavelengths) {
		next_wavelength=optical_spectrum.first_wavelength->next_wavelength;
		delete optical_spectrum.first_wavelength;

		for (i=1;i<optical_param.number_wavelengths;i++) {
			curr_wavelength=next_wavelength;
			next_wavelength=curr_wavelength->next_wavelength;
			delete curr_wavelength;
		}

		optical_param.number_wavelengths=0;
		optical_spectrum.first_wavelength=(OpticalParam *)0;

		set_update_flags(SPECTRUM,INCIDENT_INPUT_INTENSITY);
	}
}

/***********************************************************************************************
Function: void TEnvironment::effects_change_to_compute_flags(void)

Purpose: When effects are changed, the appropriate bit is changed in effects_change_flags. This
means that certain items must be updated and recomputed. This function determines which items
must be recomputed and sets the bits in update_flags and recompute_flags accordingly.

Parameters: None

Return Value: None
*/

void TEnvironment::effects_change_to_compute_flags(void)
{
	flag grid_effects, device_effects;
	SolveType current_solution;
	prec contact_pos_0, contact_pos_1;

	if (!effects_change_flags.any_set()) return;

// Environment effect flags

	if (device()) {

		grid_effects=(flag)get_value(GRID_ELECTRICAL,EFFECTS,0);
		device_effects=(flag)get_value(DEVICE,EFFECTS);
		current_solution=(SolveType)get_value(DEVICE,CURRENT_SOLUTION);

// ENV_OPTICAL_GEN
		if (effects_change_flags.is_set(ENVIRONMENT,ENV_OPTICAL_GEN)) {
			if (env_effects & ENV_OPTICAL_GEN) grid_effects|=GRID_OPTICAL_GEN;
			else grid_effects&=(~GRID_OPTICAL_GEN);
			put_value(GRID_ELECTRICAL,EFFECTS,(prec)grid_effects);
#ifndef NDEBUG
			effects_change_flags.clear(ENVIRONMENT,ENV_OPTICAL_GEN);
#endif
		}

//ENV_INCIDENT_REFLECTION
		if (effects_change_flags.is_set(ENVIRONMENT,ENV_INCIDENT_REFLECTION)) {
			if (env_effects & ENV_INCIDENT_REFLECTION) grid_effects|=GRID_INCIDENT_REFLECTION;
			else grid_effects&=(~GRID_INCIDENT_REFLECTION);
			put_value(GRID_ELECTRICAL,EFFECTS,(prec)grid_effects);
#ifndef NDEBUG
			effects_change_flags.clear(ENVIRONMENT,ENV_INCIDENT_REFLECTION);
#endif
		}

//ENV_UNDO_SIMULATION
		if (effects_change_flags.is_set(ENVIRONMENT,ENV_UNDO_SIMULATION)) {
			if (!(env_effects & ENV_UNDO_SIMULATION)) {
				undo_ready=FALSE;
				delete_undo_file();
			}
#ifndef NDEBUG
			effects_change_flags.clear(ENVIRONMENT,ENV_UNDO_SIMULATION);
#endif
		}

//ENV_SPEC_ENTIRE_DEVICE and ENV_SPEC_LEFT_INCIDENT
		if (effects_change_flags.is_set(ENVIRONMENT,ENV_SPEC_ENTIRE_DEVICE) ||
			effects_change_flags.is_set(ENVIRONMENT,ENV_SPEC_LEFT_INCIDENT)) {
			if (env_effects & ENV_SPEC_ENTIRE_DEVICE) {
				contact_pos_0=environment.get_value(CONTACT,POSITION,0);
				contact_pos_1=environment.get_value(CONTACT,POSITION,1);
				if (env_effects & ENV_SPEC_LEFT_INCIDENT) {
					put_value(ENVIRONMENT,SPEC_START_POSITION,contact_pos_0);
					put_value(ENVIRONMENT,SPEC_END_POSITION,contact_pos_1);
				}
				else {
					put_value(ENVIRONMENT,SPEC_START_POSITION,contact_pos_1);
					put_value(ENVIRONMENT,SPEC_END_POSITION,contact_pos_0);
				}
			}
#ifndef NDEBUG
			effects_change_flags.clear(ENVIRONMENT,ENV_SPEC_ENTIRE_DEVICE | ENV_SPEC_LEFT_INCIDENT);
#endif
		}

//Device effect flags

//DEVICE_NON_ISOTHERMAL
		if (effects_change_flags.is_set(DEVICE,DEVICE_NON_ISOTHERMAL)) {
			if (!(device_effects & DEVICE_NON_ISOTHERMAL)) {
				recompute_flags.set(SURFACE,TEMPERATURE);
				update_flags.set(SURFACE,TEMPERATURE);
				recompute_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				update_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				recompute_flags.set(SURFACE,HOLE_TEMPERATURE);
				update_flags.set(SURFACE,HOLE_TEMPERATURE);
				recompute_flags.set(GRID_ELECTRICAL,TEMPERATURE);
				update_flags.set(GRID_ELECTRICAL,TEMPERATURE);
				recompute_flags.set(ELECTRON,TEMPERATURE);
				update_flags.set(ELECTRON,TEMPERATURE);
				recompute_flags.set(HOLE,TEMPERATURE);
				update_flags.set(HOLE,TEMPERATURE);
			}
#ifndef NDEBUG
			effects_change_flags.clear(DEVICE,DEVICE_NON_ISOTHERMAL);
#endif
		}

//DEVICE_SINGLE_TEMP
		if (effects_change_flags.is_set(DEVICE,DEVICE_SINGLE_TEMP)) {
			if (device_effects & DEVICE_SINGLE_TEMP) {
				recompute_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				update_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				recompute_flags.set(SURFACE,HOLE_TEMPERATURE);
				update_flags.set(SURFACE,HOLE_TEMPERATURE);
				recompute_flags.set(ELECTRON,TEMPERATURE);
				update_flags.set(ELECTRON,TEMPERATURE);
				recompute_flags.set(HOLE,TEMPERATURE);
				update_flags.set(HOLE,TEMPERATURE);
			}
			else {
				if (device_effects & DEVICE_VARY_ELECTRON_TEMP) {
					recompute_flags.set(SURFACE,TEMPERATURE);
					update_flags.set(SURFACE,TEMPERATURE);
					recompute_flags.set(SURFACE,HOLE_TEMPERATURE);
					update_flags.set(SURFACE,HOLE_TEMPERATURE);
					recompute_flags.set(GRID_ELECTRICAL,TEMPERATURE);
					update_flags.set(GRID_ELECTRICAL,TEMPERATURE);
					recompute_flags.set(HOLE,TEMPERATURE);
					update_flags.set(HOLE,TEMPERATURE);
				}
			}
#ifndef NDEBUG
			effects_change_flags.clear(DEVICE,DEVICE_SINGLE_TEMP);
#endif
		}

// Contact Effects

// CONTACT_SCHOTTKY
		if (effects_change_flags.is_set(CONTACT,CONTACT_SCHOTTKY)) {
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			effects_change_flags.clear(CONTACT,CONTACT_SCHOTTKY);
#endif
        }

// Mode Effects
		if (effects_change_flags.is_set(MODE,MODE_COMPUTE)) {
			if (device_effects & DEVICE_LASER) {
				if ((flag)get_value(MODE,EFFECTS) & MODE_COMPUTE) {
					recompute_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
					update_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
				}
			}
#ifndef NDEBUG
			effects_change_flags.clear(MODE,MODE_COMPUTE);
#endif
		}

//Grid effect flags

// GRID_QW_FREE_CARR
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_QW_FREE_CARR)) {
			recompute_flags.set(ELECTRON,CONCENTRATION);
			update_flags.set(ELECTRON,CONCENTRATION);
			recompute_flags.set(HOLE,CONCENTRATION);
			update_flags.set(HOLE,CONCENTRATION);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_QW_FREE_CARR);
#endif
		}

// GRID_FERMI_DIRAC
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_FERMI_DIRAC)) {
			recompute_flags.set(ELECTRON, EQUIL_PLANCK_POT);
			update_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			recompute_flags.set(HOLE,EQUIL_PLANCK_POT);
			update_flags.set(HOLE,EQUIL_PLANCK_POT);
			recompute_flags.set(ELECTRON,CONCENTRATION);
			update_flags.set(ELECTRON,CONCENTRATION);
			recompute_flags.set(HOLE,CONCENTRATION);
			update_flags.set(HOLE,CONCENTRATION);
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_FERMI_DIRAC);
#endif
		}

// GRID_INCOMPLETE_IONIZATION
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_INCOMPLETE_IONIZATION)) {
			recompute_flags.set(ELECTRON, EQUIL_PLANCK_POT);
			update_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			recompute_flags.set(HOLE,EQUIL_PLANCK_POT);
			update_flags.set(HOLE,EQUIL_PLANCK_POT);
			recompute_flags.set(ELECTRON,IONIZED_DOPING);
			update_flags.set(ELECTRON,IONIZED_DOPING);
			recompute_flags.set(HOLE,IONIZED_DOPING);
			update_flags.set(HOLE,IONIZED_DOPING);
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_INCOMPLETE_IONIZATION);
#endif
		}

// GRID_DOPING_MOBILITY, GRID_TEMP_MOBILITY
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_DOPING_MOBILITY) ||
			effects_change_flags.is_set(GRID_ELECTRICAL,GRID_TEMP_MOBILITY)) {
			recompute_flags.set(ELECTRON, MOBILITY);
			update_flags.set(ELECTRON,MOBILITY);
			recompute_flags.set(HOLE, MOBILITY);
			update_flags.set(HOLE,MOBILITY);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_DOPING_MOBILITY | GRID_TEMP_MOBILITY);
#endif
		}

//GRID_RECOMB_SHR
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_RECOMB_SHR)) {
			if (current_solution==STEADY_STATE) {
				recompute_flags.set(NODE,SHR_RECOMB);
				update_flags.set(NODE,SHR_RECOMB);
				recompute_flags.set(ELECTRON,SHR_HEAT);
				update_flags.set(ELECTRON,SHR_HEAT);
				recompute_flags.set(HOLE,SHR_HEAT);
				update_flags.set(HOLE,SHR_HEAT);
			}
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_RECOMB_SHR);
#endif
		}

//GRID_RECOMB_B_B
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_RECOMB_B_B)) {
			if (current_solution==STEADY_STATE) {
				recompute_flags.set(NODE,B_B_RECOMB);
				update_flags.set(NODE,B_B_RECOMB);
				recompute_flags.set(NODE,B_B_HEAT);
				update_flags.set(NODE,B_B_HEAT);
				recompute_flags.set(ELECTRON,B_B_HEAT);
				update_flags.set(ELECTRON,B_B_HEAT);
				recompute_flags.set(HOLE,B_B_HEAT);
				update_flags.set(HOLE,B_B_HEAT);
			}
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_RECOMB_B_B);
#endif
		}

//GRID_RECOMB_AUGER
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_RECOMB_AUGER)) {
			if (current_solution==STEADY_STATE) {
				recompute_flags.set(NODE,AUGER_RECOMB);
				update_flags.set(NODE,AUGER_RECOMB);
			}
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_RECOMB_AUGER);
#endif
		}

//GRID_RECOMB_STIM
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_RECOMB_STIM)) {
			if (current_solution==STEADY_STATE) {
				recompute_flags.set(NODE,STIM_RECOMB);
				update_flags.set(NODE,STIM_RECOMB);
				recompute_flags.set(NODE,STIM_HEAT);
				update_flags.set(NODE,STIM_HEAT);
				recompute_flags.set(ELECTRON,STIM_HEAT);
				update_flags.set(ELECTRON,STIM_HEAT);
				recompute_flags.set(HOLE,STIM_HEAT);
				update_flags.set(HOLE,STIM_HEAT);
			}
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_RECOMB_STIM);
#endif
		}

//GRID_BAND_NARROWING
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_BAND_NARROWING)) {
			recompute_flags.set(GRID_ELECTRICAL,BAND_GAP);
			update_flags.set(GRID_ELECTRICAL,BAND_GAP);
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_BAND_NARROWING);
#endif
		}

//GRID_TEMP_ELECTRON_AFFINITY
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_TEMP_ELECTRON_AFFINITY)) {
			recompute_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			update_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_TEMP_ELECTRON_AFFINITY);
#endif
		}

//GRID_TEMP_THERMAL_COND
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_TEMP_THERMAL_COND)) {
			recompute_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
			update_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
			recompute_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
			update_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_TEMP_THERMAL_COND);
#endif
		}

//GRID_TEMP_INC_PERM
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_TEMP_INC_PERM)) {
			recompute_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_TEMP_INC_PERM);
#endif
		}

//GRID_TEMP_MODE_PERM
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_TEMP_MODE_PERM)) {
			if (device_effects & DEVICE_LASER) {
				recompute_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
				update_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			}
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_TEMP_MODE_PERM);
#endif
		}

// GRID_RELAX
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_RELAX)) {
			recompute_flags.set(ELECTRON,RELAX_HEAT);
			update_flags.set(ELECTRON,RELAX_HEAT);
			recompute_flags.set(HOLE,RELAX_HEAT);
			update_flags.set(HOLE,RELAX_HEAT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_RELAX);
#endif
		}

//GRID_OPTICAL_GEN
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_OPTICAL_GEN)) {
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
			recompute_flags.set(ELECTRON,TOTAL_HEAT);
			update_flags.set(ELECTRON,TOTAL_HEAT);
			recompute_flags.set(HOLE,TOTAL_HEAT);
			update_flags.set(HOLE,TOTAL_HEAT);
			recompute_flags.set(NODE,TOTAL_RADIATIVE_HEAT);
			update_flags.set(NODE,TOTAL_RADIATIVE_HEAT);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_OPTICAL_GEN);
#endif
		}

//GRID_INCIDENT_REFLECTION
		if (effects_change_flags.is_set(GRID_ELECTRICAL,GRID_INCIDENT_REFLECTION)) {
			recompute_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION);
#ifndef NDEBUG
			effects_change_flags.clear(GRID_ELECTRICAL,GRID_INCIDENT_REFLECTION);
#endif
		}

// Unused effects flags

#ifndef NDEBUG

//ENV_CLAMP_POTENTIAL
		effects_change_flags.clear(ENVIRONMENT,ENV_CLAMP_POTENTIAL);

//ENV_CLAMP_TEMPERATURE
		effects_change_flags.clear(ENVIRONMENT,ENV_CLAMP_TEMPERATURE);

//DEVICE_LASER
//		No code required - flag can not be changed when a device is loaded

//DEVICE_VARY_ELECTRON_TEMP
		effects_change_flags.clear(DEVICE,DEVICE_VARY_ELECTRON_TEMP);

//SURFACE_HEAT_SINK
		effects_change_flags.clear(SURFACE,SURFACE_HEAT_SINK);

// CONTACT_OHMIC
		effects_change_flags.clear(CONTACT,CONTACT_IDEALOHMIC);

// CONTACT_FINITERECOMB
		effects_change_flags.clear(CONTACT,CONTACT_FINITERECOMB);

// MODE_SEARCH_WAVELENGTH
		effects_change_flags.clear(MODE,MODE_SEARCH_WAVELENGTH);

//GRID_LATERAL_HEAT
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_LATERAL_HEAT);

//GRID_JOULE_HEAT
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_JOULE_HEAT);

//GRID_THERMOELECTRIC_HEAT
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_THERMOELECTRIC_HEAT);

//GRID_THERMIONIC
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_THERMIONIC);

//GRID_TUNNELING
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_TUNNELING);

//GRID_ABRUPT_MATIERIALS
		effects_change_flags.clear(GRID_ELECTRICAL,GRID_ABRUPT_MATERIALS);

#endif

	}
#ifndef NDEBUG
	else effects_change_flags.clear_all();
#else
	effects_change_flags.clear_all();
#endif
}

/***********************************************************************************************
Function: void TEnvironment::update_to_compute_flags(void)

Purpose: When some value is changed, the appropriate bit is changed in update_flags. This means
that certain items must be updated and recomputed. This function determines which items must
be recomputed and sets the bits in update_flags and recompute flags accordingly. Note that the
order in which flags are set is very important becuase this function reflects the order in
which items are calculated.

Parameters: None

Return Value: None
*/

void TEnvironment::update_to_compute_flags(void)
{
	flag grid_effects, device_effects, mode_effects;
	SolveType device_solution;

	if (!update_flags.any_set()) return;

	if (device()) {

		device_effects=(flag)get_value(DEVICE,EFFECTS);
		grid_effects=(flag)get_value(GRID_ELECTRICAL,EFFECTS,0);
		device_solution=(SolveType)get_value(DEVICE,CURRENT_SOLUTION);
		if (device_effects & DEVICE_LASER) mode_effects=(flag)get_value(MODE,EFFECTS);
		else mode_effects=(flag)0;


// Spectrum Value Flags

// INCIDENT_INPUT_INTENSITY, INCIDENT_PHOTON_WAVELENGTH, and INCIDENT_PHOTON_ENERGY
		if (update_flags.is_set(SPECTRUM,INCIDENT_INPUT_INTENSITY) ||
			update_flags.is_set(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH) ||
			update_flags.is_set(SPECTRUM,INCIDENT_PHOTON_ENERGY)) {
			recompute_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION_HEAT);
			update_flags.set(ELECTRON,OPTICAL_GENERATION_KIN);
			update_flags.set(HOLE,OPTICAL_GENERATION_KIN);
#ifndef NDEBUG
			update_flags.clear(SPECTRUM,INCIDENT_INPUT_INTENSITY | INCIDENT_PHOTON_WAVELENGTH | INCIDENT_PHOTON_ENERGY);
#endif
		}

// INCIDENT_EMITTED_INTENSITY
// 		No code required

// Environment Value Flags

// TEMPERATURE
		if (update_flags.is_set(ENVIRONMENT,TEMPERATURE)) {
			if (!(device_effects & DEVICE_NON_ISOTHERMAL)) {
				recompute_flags.set(SURFACE,TEMPERATURE);
				update_flags.set(SURFACE,TEMPERATURE);
				recompute_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				update_flags.set(SURFACE,ELECTRON_TEMPERATURE);
				recompute_flags.set(HOLE,ELECTRON_TEMPERATURE);
				update_flags.set(HOLE,ELECTRON_TEMPERATURE);
				recompute_flags.set(GRID_ELECTRICAL,TEMPERATURE);
				update_flags.set(GRID_ELECTRICAL, TEMPERATURE);
				recompute_flags.set(ELECTRON,TEMPERATURE);
				update_flags.set(ELECTRON, TEMPERATURE);
				recompute_flags.set(HOLE,TEMPERATURE);
				update_flags.set(HOLE, TEMPERATURE);
			}
#ifndef NDEBUG
			update_flags.clear(ENVIRONMENT,TEMPERATURE);
#endif
		}

// SPECTRUM_MULTIPLIER, SPEC_START_POSITION and SPEC_END_POSITION
		if (update_flags.is_set(ENVIRONMENT,SPEC_START_POSITION) ||
			update_flags.is_set(ENVIRONMENT,SPEC_END_POSITION) ||
			update_flags.is_set(ENVIRONMENT,SPECTRUM_MULTIPLIER)) {
			recompute_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION_HEAT);
			update_flags.set(ELECTRON,OPTICAL_GENERATION_KIN);
			update_flags.set(HOLE,OPTICAL_GENERATION_KIN);
#ifndef NDEBUG
			update_flags.clear(ENVIRONMENT,SPEC_START_POSITION | SPEC_END_POSITION | SPECTRUM_MULTIPLIER);
#endif
		}

// RADIUS
		if (update_flags.is_set(ENVIRONMENT,RADIUS)) {
			recompute_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
			update_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
#ifndef NDEBUG
			update_flags.clear(ENVIRONMENT,RADIUS);
#endif
		}

// Contact Values

// BARRIER_HEIGHT
		if (update_flags.is_set(CONTACT,BARRIER_HEIGHT)) {
		 	recompute_flags.set(CONTACT,BUILT_IN_POT);
	   		update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			update_flags.clear(CONTACT,BARRIER_HEIGHT);
#endif
		}

// Surface Values

// INCIDENT_PERM_REAL
		if (update_flags.is_set(SURFACE,INCIDENT_REFRACTIVE_INDEX)) {
			if (grid_effects & GRID_INCIDENT_REFLECTION) {
				recompute_flags.set(NODE,OPTICAL_GENERATION);
				update_flags.set(NODE,OPTICAL_GENERATION);
			}
#ifndef NDEBUG
			update_flags.clear(SURFACE,INCIDENT_REFRACTIVE_INDEX);
#endif
		}

// Cavity Values

// LENGTH
		if (update_flags.is_set(CAVITY,LENGTH)) {
			recompute_flags.set(MODE,MIRROR_LOSS);
			update_flags.set(MODE,MIRROR_LOSS);
#ifndef NDEBUG
			update_flags.clear(CAVITY,LENGTH);
#endif
		}

// AREA
		if (update_flags.is_set(CAVITY,AREA)) {
			recompute_flags.set(MODE,MODE_NORMALIZATION);
			update_flags.set(MODE,MODE_NORMALIZATION);
#ifndef NDEBUG
			update_flags.clear(CAVITY,AREA);
#endif
		}

// Mirror Values
// REFLECTIVITY
		if (update_flags.is_set(MIRROR,REFLECTIVITY)) {
			recompute_flags.set(MODE,MIRROR_LOSS);
			update_flags.set(MODE,MIRROR_LOSS);
#ifndef NDEBUG
			update_flags.clear(MIRROR, REFLECTIVITY);
#endif
		}

// Other Values

// MODE_PHOTON_ENERGY

		if (update_flags.is_set(MODE,MODE_PHOTON_ENERGY)) {
			recompute_flags.set(GRID_OPTICAL,MODE_PHOTON_ENERGY);
			update_flags.set(GRID_OPTICAL,MODE_PHOTON_ENERGY);
#ifndef NDEBUG
			update_flags.clear(MODE,MODE_PHOTON_ENERGY);
#endif
		}

// WAVEGUIDE_LOSS
		if (update_flags.is_set(MODE,WAVEGUIDE_LOSS)) {
			recompute_flags.set(MODE,PHOTON_LIFETIME);
			update_flags.set(MODE,PHOTON_LIFETIME);
#ifndef NDEBUG
			update_flags.clear(MODE,WAVEGUIDE_LOSS);
#endif
		}

// PHOTON_LIFETIME
		if (update_flags.is_set(MODE,MIRROR_LOSS)) {
			recompute_flags.set(MODE,PHOTON_LIFETIME);
			update_flags.set(MODE,PHOTON_LIFETIME);
#ifndef NDEBUG
			update_flags.clear(MODE,MIRROR_LOSS);
#endif
		}

// TEMPERATURE

		if (update_flags.is_set(ELECTRON,TEMPERATURE)) {
			recompute_flags.set(ELECTRON,NON_EQUIL_DOS);
			update_flags.set(ELECTRON,NON_EQUIL_DOS);
			if (grid_effects & GRID_TEMP_MOBILITY) {
				recompute_flags.set(ELECTRON,MOBILITY);
				update_flags.set(ELECTRON,MOBILITY);
			}
			if (grid_effects & GRID_INCOMPLETE_IONIZATION) {
				recompute_flags.set(ELECTRON,IONIZED_DOPING);
				update_flags.set(ELECTRON,IONIZED_DOPING);
			}
			if (grid_effects & GRID_RELAX) {
				recompute_flags.set(ELECTRON,RELAX_HEAT);
				update_flags.set(ELECTRON,RELAX_HEAT);
			}
#ifndef NDEBUG
			update_flags.clear(ELECTRON,TEMPERATURE);
#endif
		}

		if (update_flags.is_set(HOLE, TEMPERATURE)) {
			recompute_flags.set(HOLE,NON_EQUIL_DOS);
			update_flags.set(HOLE,NON_EQUIL_DOS);
			if (grid_effects & GRID_TEMP_MOBILITY) {
				recompute_flags.set(HOLE,MOBILITY);
				update_flags.set(HOLE,MOBILITY);
			}
			if (grid_effects & GRID_INCOMPLETE_IONIZATION) {
				recompute_flags.set(HOLE,IONIZED_DOPING);
				update_flags.set(HOLE,IONIZED_DOPING);
			}
			if (grid_effects & GRID_RELAX) {
				recompute_flags.set(HOLE,RELAX_HEAT);
				update_flags.set(HOLE,RELAX_HEAT);
			}
#ifndef NDEBUG
			update_flags.clear(HOLE,TEMPERATURE);
#endif
		}

		if (update_flags.is_set(GRID_ELECTRICAL,TEMPERATURE)) {
			recompute_flags.set(ELECTRON,EQUIL_DOS);
			update_flags.set(ELECTRON,EQUIL_DOS);
			recompute_flags.set(HOLE,EQUIL_DOS);
			update_flags.set(HOLE,EQUIL_DOS);
			recompute_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			update_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			recompute_flags.set(HOLE,EQUIL_PLANCK_POT);
			update_flags.set(HOLE,EQUIL_PLANCK_POT);
			if (grid_effects & GRID_BAND_NARROWING) {
				recompute_flags.set(GRID_ELECTRICAL,BAND_GAP);
				update_flags.set(GRID_ELECTRICAL,BAND_GAP);
			}
			if (grid_effects & GRID_TEMP_ELECTRON_AFFINITY) {
				recompute_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
				update_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			}
			if (grid_effects & GRID_TEMP_THERMAL_COND) {
				recompute_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
				update_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
				recompute_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
				update_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
			}
			if (grid_effects & GRID_TEMP_INC_PERM) {
				recompute_flags.set(NODE,OPTICAL_GENERATION);
				update_flags.set(NODE,OPTICAL_GENERATION);
			}
			if (grid_effects & GRID_TEMP_MODE_PERM) {
				if (device_effects & DEVICE_LASER) {
					recompute_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
					update_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
				}
			}
			if (grid_effects & GRID_RELAX) {
				recompute_flags.set(ELECTRON,RELAX_HEAT);
				update_flags.set(ELECTRON,RELAX_HEAT);
				recompute_flags.set(HOLE,RELAX_HEAT);
				update_flags.set(HOLE,RELAX_HEAT);
			}
			recompute_flags.set(CONTACT,BUILT_IN_POT);
			update_flags.set(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			update_flags.clear(GRID_ELECTRICAL,TEMPERATURE);
#endif
		}

// MATERIAL, ALLOY_CONC, and ALLOY_TYPE
		if (update_flags.is_set(GRID_ELECTRICAL,MATERIAL) ||
			update_flags.is_set(GRID_ELECTRICAL,ALLOY_CONC) ||
			update_flags.is_set(GRID_ELECTRICAL,ALLOY_TYPE)) {
			recompute_flags.set(ELECTRON,COLLISION_FACTOR);
			update_flags.set(ELECTRON,COLLISION_FACTOR);
			recompute_flags.set(HOLE,COLLISION_FACTOR);
			update_flags.set(HOLE,COLLISION_FACTOR);
			recompute_flags.set(GRID_ELECTRICAL,BAND_GAP);
			update_flags.set(GRID_ELECTRICAL,BAND_GAP);
			recompute_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			update_flags.set(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			recompute_flags.set(GRID_ELECTRICAL,PERMITIVITY);
			update_flags.set(GRID_ELECTRICAL,PERMITIVITY);
			recompute_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
			update_flags.set(GRID_ELECTRICAL,THERMAL_CONDUCT);
			recompute_flags.set(ELECTRON,MOBILITY);
			update_flags.set(ELECTRON,MOBILITY);
			recompute_flags.set(HOLE,MOBILITY);
			update_flags.set(HOLE,MOBILITY);
			recompute_flags.set(ELECTRON,DOS_MASS);
			update_flags.set(ELECTRON,DOS_MASS);
			recompute_flags.set(HOLE,DOS_MASS);
			update_flags.set(HOLE,DOS_MASS);
			recompute_flags.set(ELECTRON,COND_MASS);
			update_flags.set(ELECTRON,COND_MASS);
			recompute_flags.set(HOLE,COND_MASS);
			update_flags.set(HOLE,COND_MASS);
			recompute_flags.set(ELECTRON,SHR_LIFETIME);
			update_flags.set(ELECTRON,SHR_LIFETIME);
			recompute_flags.set(HOLE,SHR_LIFETIME);
			update_flags.set(HOLE,SHR_LIFETIME);
			recompute_flags.set(ELECTRON,AUGER_COEFFICIENT);
			update_flags.set(ELECTRON,AUGER_COEFFICIENT);
			recompute_flags.set(HOLE,AUGER_COEFFICIENT);
			update_flags.set(HOLE,AUGER_COEFFICIENT);
			recompute_flags.set(ELECTRON,ENERGY_LIFETIME);
			update_flags.set(ELECTRON,ENERGY_LIFETIME);
			recompute_flags.set(HOLE,ENERGY_LIFETIME);
			update_flags.set(HOLE,ENERGY_LIFETIME);
			recompute_flags.set(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT);
			update_flags.set(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT);
			recompute_flags.set(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX);
			update_flags.set(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX);
			if (device_effects & DEVICE_LASER) {
				recompute_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
				update_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			}
#ifndef NDEBUG
			update_flags.clear(GRID_ELECTRICAL,MATERIAL | ALLOY_CONC | ALLOY_TYPE);
#endif
		}

		if (update_flags.is_set(GRID_ELECTRICAL,RADIUS)) {
			recompute_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
			update_flags.set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
#ifndef NDEBUG
			update_flags.clear(GRID_ELECTRICAL,RADIUS);
#endif
		}

// DOPING_CONC
		if (update_flags.is_set(ELECTRON,DOPING_CONC)) {
			recompute_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			update_flags.set(ELECTRON,EQUIL_PLANCK_POT);
			if (grid_effects & GRID_DOPING_MOBILITY) {
				recompute_flags.set(ELECTRON,MOBILITY);
				update_flags.set(ELECTRON,MOBILITY);
				recompute_flags.set(HOLE,MOBILITY);
				update_flags.set(HOLE,MOBILITY);
			}
#ifndef NDEBUG
			update_flags.clear(ELECTRON,DOPING_CONC);
#endif
		}

		if (update_flags.is_set(HOLE,DOPING_CONC)) {
			recompute_flags.set(HOLE,EQUIL_PLANCK_POT);
			update_flags.set(HOLE,EQUIL_PLANCK_POT);
			if (grid_effects & GRID_DOPING_MOBILITY) {
				recompute_flags.set(ELECTRON,MOBILITY);
				update_flags.set(ELECTRON,MOBILITY);
				recompute_flags.set(HOLE,MOBILITY);
				update_flags.set(HOLE,MOBILITY);
			}
#ifndef NDEBUG
			update_flags.clear(HOLE,DOPING_CONC);
#endif
		}



// DOS_MASS
		if (update_flags.is_set(ELECTRON,DOS_MASS)) {
			recompute_flags.set(ELECTRON,NON_EQUIL_DOS);
			update_flags.set(ELECTRON,NON_EQUIL_DOS);
			recompute_flags.set(ELECTRON,EQUIL_DOS);
			update_flags.set(ELECTRON,EQUIL_DOS);
			recompute_flags.set(NODE,REDUCED_DOS_MASS);
			update_flags.set(NODE,REDUCED_DOS_MASS);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,DOS_MASS);
#endif
		}

		if (update_flags.is_set(HOLE,DOS_MASS)) {
			recompute_flags.set(HOLE,NON_EQUIL_DOS);
			update_flags.set(HOLE,NON_EQUIL_DOS);
			recompute_flags.set(HOLE,EQUIL_DOS);
			update_flags.set(HOLE,EQUIL_DOS);
			recompute_flags.set(NODE,REDUCED_DOS_MASS);
			update_flags.set(NODE,REDUCED_DOS_MASS);
#ifndef NDEBUG
			update_flags.clear(HOLE,DOS_MASS);
#endif
		}

// NON_EQUIL_DOS
		if (update_flags.is_set(ELECTRON,NON_EQUIL_DOS)) {
			recompute_flags.set(ELECTRON,CONCENTRATION);
			update_flags.set(ELECTRON,CONCENTRATION);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,NON_EQUIL_DOS);
#endif
		}

		if (update_flags.is_set(HOLE,NON_EQUIL_DOS)) {
			recompute_flags.set(HOLE,CONCENTRATION);
			update_flags.set(HOLE,CONCENTRATION);
#ifndef NDEBUG
			update_flags.clear(HOLE,NON_EQUIL_DOS);
#endif
		}

// EQUIL_DOS
		if (update_flags.is_set(ELECTRON,EQUIL_DOS) ||
			update_flags.is_set(HOLE,EQUIL_DOS)) {
			recompute_flags.set(NODE,INTRINSIC_CONC);
			update_flags.set(NODE,INTRINSIC_CONC);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,EQUIL_DOS);
			update_flags.clear(HOLE,EQUIL_DOS);
#endif
		}

// EQUIL_PLANCK_POT
		if (update_flags.is_set(ELECTRON,EQUIL_PLANCK_POT)) {
			if (device_solution==CHARGE_NEUTRAL) {
				recompute_flags.set(ELECTRON,PLANCK_POT);
				update_flags.set(ELECTRON,PLANCK_POT);
			}
			recompute_flags.set(NODE,INTRINSIC_CONC);
			update_flags.set(NODE,INTRINSIC_CONC);
			recompute_flags.set(CONTACT,EQUIL_ELECTRON_CONC);
			update_flags.set(CONTACT,EQUIL_ELECTRON_CONC);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,EQUIL_PLANCK_POT);
#endif
		}

		if (update_flags.is_set(HOLE,EQUIL_PLANCK_POT)) {
			if (device_solution==CHARGE_NEUTRAL) {
				recompute_flags.set(HOLE,PLANCK_POT);
				update_flags.set(HOLE,PLANCK_POT);
			}
			recompute_flags.set(NODE,INTRINSIC_CONC);
			update_flags.set(NODE,INTRINSIC_CONC);
			recompute_flags.set(CONTACT,EQUIL_HOLE_CONC);
			update_flags.set(CONTACT,EQUIL_HOLE_CONC);
#ifndef NDEBUG
			update_flags.clear(HOLE,EQUIL_PLANCK_POT);
#endif
		}

// PLANCK_POT and QUASI_FERMI (not charge neutral)
		if (device_solution!=CHARGE_NEUTRAL) {
			if (update_flags.is_set(ELECTRON,PLANCK_POT) ||
				update_flags.is_set(ELECTRON,QUASI_FERMI)) {
				if (grid_effects & GRID_INCOMPLETE_IONIZATION) {
					recompute_flags.set(ELECTRON,IONIZED_DOPING);
					update_flags.set(ELECTRON,IONIZED_DOPING);
				}
				recompute_flags.set(ELECTRON,CONCENTRATION);
				update_flags.set(ELECTRON,CONCENTRATION);
#ifndef NDEBUG
				update_flags.clear(ELECTRON,PLANCK_POT | QUASI_FERMI);
#endif
			}

			if (update_flags.is_set(HOLE,PLANCK_POT) ||
				update_flags.is_set(HOLE,QUASI_FERMI)) {
				if (grid_effects & GRID_INCOMPLETE_IONIZATION) {
					recompute_flags.set(HOLE,IONIZED_DOPING);
					update_flags.set(HOLE,IONIZED_DOPING);
				}
				recompute_flags.set(HOLE,CONCENTRATION);
				update_flags.set(HOLE,CONCENTRATION);
#ifndef NDEBUG
				update_flags.clear(HOLE,PLANCK_POT | QUASI_FERMI);
#endif
			}
		}

// CONCENTRATION
		if (update_flags.is_set(ELECTRON,CONCENTRATION)) {
			if (grid_effects & GRID_RECOMB_B_B) {
				recompute_flags.set(NODE,B_B_RECOMB);
				update_flags.set(NODE,B_B_RECOMB);
			}
			if (grid_effects & GRID_RECOMB_SHR) {
				recompute_flags.set(NODE,SHR_RECOMB);
				update_flags.set(NODE,SHR_RECOMB);
			}
			if (grid_effects & GRID_RECOMB_AUGER) {
				recompute_flags.set(NODE,AUGER_RECOMB);
				update_flags.set(NODE,AUGER_RECOMB);
			}
			if (grid_effects & GRID_RECOMB_STIM) {
				recompute_flags.set(NODE,STIM_RECOMB);
				update_flags.set(NODE,STIM_RECOMB);
			}
			if (grid_effects & GRID_RELAX) {
				recompute_flags.set(ELECTRON,RELAX_HEAT);
				update_flags.set(ELECTRON,RELAX_HEAT);
			}
			recompute_flags.set(NODE,TOTAL_CHARGE);
			update_flags.set(NODE,TOTAL_CHARGE);
			if (device_solution==CHARGE_NEUTRAL) {
				recompute_flags.set(ELECTRON,PLANCK_POT);
				update_flags.set(ELECTRON,PLANCK_POT);
			}
#ifndef NDEBUG
			update_flags.clear(ELECTRON,CONCENTRATION);
#endif
		}

		if (update_flags.is_set(HOLE,CONCENTRATION)) {
			if (grid_effects & GRID_RECOMB_B_B) {
				recompute_flags.set(NODE,B_B_RECOMB);
				update_flags.set(NODE,B_B_RECOMB);
			}
			if (grid_effects & GRID_RECOMB_SHR) {
				recompute_flags.set(NODE,SHR_RECOMB);
				update_flags.set(NODE,SHR_RECOMB);
			}
        	if (grid_effects & GRID_RECOMB_AUGER) {
				recompute_flags.set(NODE,AUGER_RECOMB);
				update_flags.set(NODE,AUGER_RECOMB);
			}
			if (grid_effects & GRID_RECOMB_STIM) {
				recompute_flags.set(NODE,STIM_RECOMB);
				update_flags.set(NODE,STIM_RECOMB);
			}
			if (grid_effects & GRID_RELAX) {
				recompute_flags.set(HOLE,RELAX_HEAT);
				update_flags.set(HOLE,RELAX_HEAT);
			}
			recompute_flags.set(NODE,TOTAL_CHARGE);
			update_flags.set(NODE,TOTAL_CHARGE);
			if (device_solution==CHARGE_NEUTRAL) {
				recompute_flags.set(HOLE,PLANCK_POT);
				update_flags.set(HOLE,PLANCK_POT);
			}
#ifndef NDEBUG
			update_flags.clear(HOLE,CONCENTRATION);
#endif
		}

// PLANCK_POT and QUASI_FERMI (charge neutral)
		if (device_solution==CHARGE_NEUTRAL) {
			if (update_flags.is_set(ELECTRON,PLANCK_POT) ||
				update_flags.is_set(ELECTRON,QUASI_FERMI)) {
				recompute_flags.set(ELECTRON,IONIZED_DOPING);
				update_flags.set(ELECTRON,IONIZED_DOPING);
				recompute_flags.set(GRID_ELECTRICAL,POTENTIAL);
				update_flags.set(GRID_ELECTRICAL,POTENTIAL);
#ifndef NDEBUG
				update_flags.clear(ELECTRON,PLANCK_POT | QUASI_FERMI);
#endif
			}

			if (update_flags.is_set(HOLE,PLANCK_POT) ||
				update_flags.is_set(HOLE,QUASI_FERMI)) {
				recompute_flags.set(HOLE,IONIZED_DOPING);
				update_flags.set(HOLE,IONIZED_DOPING);
				recompute_flags.set(GRID_ELECTRICAL,POTENTIAL);
				update_flags.set(GRID_ELECTRICAL,POTENTIAL);
#ifndef NDEBUG
				update_flags.clear(HOLE,PLANCK_POT | QUASI_FERMI);
#endif
			}
		}

// IONIZED_DOPING
		if (update_flags.is_set(ELECTRON,IONIZED_DOPING) ||
			update_flags.is_set(HOLE,IONIZED_DOPING)) {
			recompute_flags.set(NODE,TOTAL_CHARGE);
			update_flags.set(NODE,TOTAL_CHARGE);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,IONIZED_DOPING);
			update_flags.clear(HOLE,IONIZED_DOPING);
#endif
		}

// POTENTIAL
		if (update_flags.is_set(GRID_ELECTRICAL,POTENTIAL) ||
			update_flags.is_set(GRID_ELECTRICAL,ELECTRON_AFFINITY)) {
			update_flags.set(ELECTRON,BAND_EDGE);
			recompute_flags.set(ELECTRON,BAND_EDGE);
			update_flags.set(HOLE,BAND_EDGE);
			recompute_flags.set(HOLE,BAND_EDGE);
#ifndef NDEBUG
			update_flags.clear(GRID_ELECTRICAL,POTENTIAL | ELECTRON_AFFINITY);
#endif
		}

// BAND_GAP
		if (update_flags.is_set(GRID_ELECTRICAL,BAND_GAP)) {
			recompute_flags.set(NODE,INTRINSIC_CONC);
			update_flags.set(NODE,INTRINSIC_CONC);
			if (device_effects & DEVICE_LASER) {
				recompute_flags.set(ELECTRON,STIMULATED_FACTOR);
				update_flags.set(ELECTRON,STIMULATED_FACTOR);
				recompute_flags.set(HOLE,STIMULATED_FACTOR);
				update_flags.set(HOLE,STIMULATED_FACTOR);
				recompute_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
				update_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			}
			recompute_flags.set(NODE,OPTICAL_GENERATION);
			update_flags.set(NODE,OPTICAL_GENERATION);
#ifndef NDEBUG
			update_flags.clear(GRID_ELECTRICAL,BAND_GAP);
#endif
		}

// MODE_PHOTON_ENERGY
		if (update_flags.is_set(GRID_OPTICAL,MODE_PHOTON_ENERGY)) {
			recompute_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			update_flags.set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			recompute_flags.set(ELECTRON,STIMULATED_FACTOR);
			update_flags.set(ELECTRON,STIMULATED_FACTOR);
			recompute_flags.set(HOLE,STIMULATED_FACTOR);
			update_flags.set(HOLE,STIMULATED_FACTOR);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_PHOTON_ENERGY);
#endif
		}

// MODE_REFRACTIVE_INDEX
		if (update_flags.is_set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX)) {
			if (mode_effects & MODE_COMPUTE) {
				recompute_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
				update_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
			}
			recompute_flags.set(MODE,MODE_GROUP_VELOCITY);
			update_flags.set(MODE,MODE_GROUP_VELOCITY);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
#endif
		}

// MODE_GROUP_VELOCITY

		if (update_flags.is_set(MODE,MODE_GROUP_VELOCITY)) {
			recompute_flags.set(GRID_OPTICAL,MODE_GROUP_VELOCITY);
			update_flags.set(GRID_OPTICAL,MODE_GROUP_VELOCITY);
#ifndef NDEBUG
			update_flags.clear(MODE,MODE_GROUP_VELOCITY);
#endif
		}

		if (update_flags.is_set(GRID_OPTICAL,MODE_GROUP_VELOCITY)) {
			recompute_flags.set(NODE,STIM_RECOMB);
			update_flags.set(NODE,STIM_RECOMB);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_GROUP_VELOCITY);
#endif
		}

// REDUCED_DOS_MASS
		if (update_flags.is_set(NODE,REDUCED_DOS_MASS)) {
			if (device_effects & DEVICE_LASER) {
				recompute_flags.set(ELECTRON,STIMULATED_FACTOR);
				update_flags.set(ELECTRON,STIMULATED_FACTOR);
				recompute_flags.set(HOLE,STIMULATED_FACTOR);
				update_flags.set(HOLE,STIMULATED_FACTOR);
			}
#ifndef NDEBUG
			update_flags.clear(NODE,REDUCED_DOS_MASS);
#endif
		}

// MODE_ABSORPTION, STIMULATED_FACTOR
		if (update_flags.is_set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX) ||
			update_flags.is_set(ELECTRON,STIMULATED_FACTOR) ||
			update_flags.is_set(HOLE,STIMULATED_FACTOR)) {
			recompute_flags.set(GRID_OPTICAL,MODE_GAIN);
			update_flags.set(GRID_OPTICAL,MODE_GAIN);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			update_flags.clear(ELECTRON,STIMULATED_FACTOR);
			update_flags.clear(HOLE,STIMULATED_FACTOR);
#endif
		}

// MODE_GAIN
		if (update_flags.is_set(GRID_OPTICAL,MODE_GAIN)) {
			if (mode_effects & MODE_COMPUTE) {
				recompute_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
				update_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
			}
			recompute_flags.set(MODE,MODE_GAIN);
			update_flags.set(MODE,MODE_GAIN);
			recompute_flags.set(NODE,STIM_RECOMB);
			update_flags.set(NODE,STIM_RECOMB);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_GAIN);
#endif
		}

// MODE_TOTAL_FIELD_MAG
		if (update_flags.is_set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG)) {
			recompute_flags.set(MODE,MODE_GAIN);
			update_flags.set(MODE,MODE_GAIN);
			recompute_flags.set(MODE,MODE_NORMALIZATION);
			update_flags.set(MODE,MODE_NORMALIZATION);
			recompute_flags.set(NODE,STIM_RECOMB);
			update_flags.set(NODE,STIM_RECOMB);
#ifndef NDEBUG
			update_flags.clear(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
#endif
		}

// INTRINSIC_CONC
		if (update_flags.is_set(NODE,INTRINSIC_CONC)) {
			recompute_flags.set(NODE,B_B_RECOMB);
			update_flags.set(NODE,B_B_RECOMB);
			recompute_flags.set(NODE,SHR_RECOMB);
			update_flags.set(NODE,SHR_RECOMB);
			recompute_flags.set(NODE,AUGER_RECOMB);
			update_flags.set(NODE,AUGER_RECOMB);
#ifndef NDEBUG
			update_flags.clear(NODE,INTRINSIC_CONC);
#endif
		}

// B_B_RECOMB
		if (update_flags.is_set(NODE,B_B_RECOMB)) {
			recompute_flags.set(ELECTRON,B_B_HEAT);
			update_flags.set(ELECTRON,B_B_HEAT);
			recompute_flags.set(HOLE,B_B_HEAT);
			update_flags.set(HOLE,B_B_HEAT);
			recompute_flags.set(NODE,B_B_HEAT);
			update_flags.set(NODE,B_B_HEAT);
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
#ifndef NDEBUG
			update_flags.clear(NODE,B_B_RECOMB);
#endif
		}

// STIM_RECOMB
		if (update_flags.is_set(NODE,STIM_RECOMB)) {
			recompute_flags.set(ELECTRON,STIM_HEAT);
			update_flags.set(ELECTRON,STIM_HEAT);
			recompute_flags.set(HOLE,STIM_HEAT);
			update_flags.set(HOLE,STIM_HEAT);
			recompute_flags.set(NODE,STIM_HEAT);
			update_flags.set(NODE,STIM_HEAT);
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
#ifndef NDEBUG
			update_flags.clear(NODE,STIM_RECOMB);
#endif
		}

// SHR_RECOMB
		if (update_flags.is_set(NODE,SHR_RECOMB)) {
			recompute_flags.set(ELECTRON,SHR_HEAT);
			update_flags.set(ELECTRON,SHR_HEAT);
			recompute_flags.set(HOLE,SHR_HEAT);
			update_flags.set(HOLE,SHR_HEAT);
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
#ifndef NDEBUG
			update_flags.clear(NODE,SHR_RECOMB);
#endif
		}

// AUGER_RECOMB
		if (update_flags.is_set(NODE,AUGER_RECOMB)) {
			recompute_flags.set(ELECTRON,AUGER_HEAT);
			update_flags.set(ELECTRON,AUGER_HEAT);
			recompute_flags.set(HOLE,AUGER_HEAT);
			update_flags.set(HOLE,AUGER_HEAT);
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
#ifndef NDEBUG
			update_flags.clear(NODE,AUGER_RECOMB);
#endif
		}

// OPTICAL_GENERATION
		if (update_flags.is_set(NODE,OPTICAL_GENERATION)) {
			recompute_flags.set(NODE,TOTAL_RECOMB);
			update_flags.set(NODE,TOTAL_RECOMB);
			recompute_flags.set(ELECTRON,OPTICAL_GENERATION_REF);
			update_flags.set(ELECTRON,OPTICAL_GENERATION_REF);
			recompute_flags.set(HOLE,OPTICAL_GENERATION_REF);
			update_flags.set(HOLE,OPTICAL_GENERATION_REF);
#ifndef NDEBUG
			update_flags.clear(NODE,OPTICAL_GENERATION);
#endif
		}

// TOTAL_RECOMB
		if (update_flags.is_set(NODE,TOTAL_RECOMB)) {
			recompute_flags.set(ELECTRON,CURRENT);
			update_flags.set(ELECTRON,CURRENT);
			recompute_flags.set(HOLE,CURRENT);
			update_flags.set(HOLE,CURRENT);
#ifndef NDEBUG
			update_flags.clear(NODE,TOTAL_RECOMB);
#endif
		}

// B_B_HEAT, STIM_HEAT, SHR_HEAT, AUGER_HEAT, OPTICAL_GENERATION_REF, OPTICAL_GENERATION_KIN
		if (update_flags.is_set(ELECTRON,B_B_HEAT) ||
			update_flags.is_set(ELECTRON,STIM_HEAT) ||
			update_flags.is_set(ELECTRON,SHR_HEAT) ||
			update_flags.is_set(ELECTRON,AUGER_HEAT) ||
			update_flags.is_set(ELECTRON,OPTICAL_GENERATION_KIN) ||
			update_flags.is_set(ELECTRON,OPTICAL_GENERATION_REF) ||
			update_flags.is_set(ELECTRON,RELAX_HEAT)) {
			recompute_flags.set(ELECTRON,TOTAL_HEAT);
			update_flags.set(ELECTRON,TOTAL_HEAT);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,B_B_HEAT | STIM_HEAT | SHR_HEAT | AUGER_HEAT |
							   OPTICAL_GENERATION_KIN | OPTICAL_GENERATION_REF | RELAX_HEAT);
#endif
		}

		if (update_flags.is_set(HOLE,B_B_HEAT) ||
			update_flags.is_set(HOLE,STIM_HEAT) ||
			update_flags.is_set(HOLE,SHR_HEAT) ||
			update_flags.is_set(HOLE,AUGER_HEAT) ||
			update_flags.is_set(HOLE,OPTICAL_GENERATION_KIN) ||
			update_flags.is_set(HOLE,OPTICAL_GENERATION_REF) ||
			update_flags.is_set(HOLE,RELAX_HEAT)) {
			recompute_flags.set(HOLE,TOTAL_HEAT);
			update_flags.set(HOLE,TOTAL_HEAT);
#ifndef NDEBUG
			update_flags.clear(HOLE,B_B_HEAT | STIM_HEAT | SHR_HEAT | AUGER_HEAT |
							   OPTICAL_GENERATION_KIN | OPTICAL_GENERATION_REF | RELAX_HEAT);
#endif
		}

		if (update_flags.is_set(NODE,OPTICAL_GENERATION_HEAT) ||
			update_flags.is_set(NODE,B_B_HEAT) ||
			update_flags.is_set(NODE,STIM_HEAT)) {
			recompute_flags.set(NODE,TOTAL_RADIATIVE_HEAT);
			update_flags.set(NODE,TOTAL_RADIATIVE_HEAT);
#ifndef NDEBUG
			update_flags.clear(NODE,B_B_HEAT | STIM_HEAT | OPTICAL_GENERATION_HEAT);
#endif
		}

// TOTAL_HEAT
		if (update_flags.is_set(ELECTRON,TOTAL_HEAT) ||
			update_flags.is_set(HOLE,TOTAL_HEAT)) {
			recompute_flags.set(NODE,TOTAL_HEAT);
			update_flags.set(NODE,TOTAL_HEAT);
#ifndef NDEBUG
			update_flags.clear(ELECTRON,TOTAL_HEAT);
			update_flags.clear(HOLE,TOTAL_HEAT);
#endif
		}

// TOTAL_CHARGE
		if (update_flags.is_set(NODE,TOTAL_CHARGE)) {
			update_flags.set(GRID_ELECTRICAL,FIELD);
			recompute_flags.set(GRID_ELECTRICAL,FIELD);
#ifndef NDEBUG
			update_flags.clear(NODE,TOTAL_CHARGE);
#endif
		}


// Unused update_flags

#ifndef NDEBUG

// TEMPERATURE
		update_flags.clear(SURFACE,TEMPERATURE);
		update_flags.clear(SURFACE,ELECTRON_TEMPERATURE);
		update_flags.clear(SURFACE,HOLE_TEMPERATURE);

// Mirror POSITION
		update_flags.clear(MIRROR,POSITION);

// MODE_NORMALIZATION
		update_flags.clear(MODE,MODE_NORMALIZATION);

// MODE_GAIN
		update_flags.clear(MODE,MODE_GAIN);

// BUILT_IN_POT
		update_flags.clear(CONTACT,BUILT_IN_POT);

// COLLISION_FACTOR
		update_flags.clear(ELECTRON,COLLISION_FACTOR);
		update_flags.clear(HOLE,COLLISION_FACTOR);

// COND_MASS
		update_flags.clear(ELECTRON,COND_MASS);
		update_flags.clear(HOLE,COND_MASS);

// BAND_EDGE
		update_flags.clear(ELECTRON,BAND_EDGE);
		update_flags.clear(HOLE,BAND_EDGE);

// MOBILITY
		update_flags.clear(ELECTRON,MOBILITY);
		update_flags.clear(HOLE,MOBILITY);

// PERMITIVITY
		update_flags.clear(GRID_ELECTRICAL,PERMITIVITY);

// B_B_RECOMB_CONSTANT
		update_flags.clear(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT);

// INCIDENT_REFRACTIVE_INDEX
		update_flags.clear(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX);

// SHR_LIFETIME
		update_flags.clear(ELECTRON,SHR_LIFETIME);
		update_flags.clear(HOLE,SHR_LIFETIME);

// ENERGY_LIFETIME
		update_flags.clear(ELECTRON,ENERGY_LIFETIME);
		update_flags.clear(HOLE,ENERGY_LIFETIME);

// AUGER_COEFFICIENT
		update_flags.clear(ELECTRON,AUGER_COEFFICIENT);
		update_flags.clear(HOLE,AUGER_COEFFICIENT);

// CURRENT
		update_flags.clear(ELECTRON,CURRENT);
		update_flags.clear(HOLE,CURRENT);

// THERMAL_CONDUCT
		update_flags.clear(GRID_ELECTRICAL,THERMAL_CONDUCT);

// LATERAL_THERMAL_CONDUCT
		update_flags.clear(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);

// PHOTON_LIFETIME
		update_flags.clear(MODE,PHOTON_LIFETIME);

// SPONT_FACTOR
		update_flags.clear(MODE,SPONT_FACTOR);

// TOTAL_HEAT
		update_flags.clear(NODE,TOTAL_HEAT);

// TOTAL_RADIATIVE_HEAT
		update_flags.clear(NODE,TOTAL_RADIATIVE_HEAT);

// FIELD
		update_flags.clear(GRID_ELECTRICAL,FIELD);

// EQUIL_ELECTRON_CONC
		update_flags.clear(CONTACT,EQUIL_ELECTRON_CONC);

// EQUIL_HOLE_CONC
		update_flags.clear(CONTACT,EQUIL_HOLE_CONC);

#endif

	}

#ifndef NDEBUG
	else update_flags.clear_all();
#else
	update_flags.clear_all();
#endif
}

logical TEnvironment::process_recompute_flags(void)
{
	logical plot_update=FALSE;

	effects_change_to_compute_flags();
	assert(!effects_change_flags.any_set());

	if (update_flags.any_set()) {
		if (update_flags.any_set(FREE_ELECTRON) || update_flags.any_set(BOUND_ELECTRON) ||
			update_flags.any_set(FREE_HOLE) || update_flags.any_set(BOUND_HOLE) ||
			update_flags.any_set(ELECTRON) || update_flags.any_set(HOLE) ||
			update_flags.any_set(GRID_ELECTRICAL) || update_flags.any_set(GRID_OPTICAL) ||
			update_flags.any_set(NODE) || update_flags.any_set(ENVIRONMENT) ||
			update_flags.any_set(SPECTRUM))
			plot_update=TRUE;
	}

	update_to_compute_flags();
	assert(!update_flags.any_set());

	if (!recompute_flags.any_set()) return(plot_update);

	if (device()) {

		if (recompute_flags.any_set(FREE_ELECTRON) || recompute_flags.any_set(BOUND_ELECTRON) ||
			recompute_flags.any_set(FREE_HOLE) || recompute_flags.any_set(BOUND_HOLE) ||
			recompute_flags.any_set(ELECTRON) || recompute_flags.any_set(HOLE) ||
			recompute_flags.any_set(GRID_ELECTRICAL) || recompute_flags.any_set(GRID_OPTICAL) ||
			recompute_flags.any_set(NODE) || recompute_flags.any_set(SPECTRUM))
			plot_update=TRUE;

// TEMPERATURE
		if (recompute_flags.is_set(GRID_ELECTRICAL,TEMPERATURE)) {
			comp_value(GRID_ELECTRICAL,TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,TEMPERATURE);
#endif
		}

		if (recompute_flags.is_set(ELECTRON,TEMPERATURE)) {
			comp_value(ELECTRON,TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,TEMPERATURE);
#endif
		}

		if (recompute_flags.is_set(HOLE,TEMPERATURE)) {
			comp_value(HOLE,TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,TEMPERATURE);
#endif
		}

		if (recompute_flags.is_set(SURFACE,TEMPERATURE)) {
			comp_value(SURFACE,TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(SURFACE,TEMPERATURE);
#endif
		}

		if (recompute_flags.is_set(SURFACE,ELECTRON_TEMPERATURE)) {
			comp_value(SURFACE,ELECTRON_TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(SURFACE,ELECTRON_TEMPERATURE);
#endif
		}

		if (recompute_flags.is_set(SURFACE,HOLE_TEMPERATURE)) {
			comp_value(SURFACE,HOLE_TEMPERATURE);
#ifndef NDEBUG
			recompute_flags.clear(SURFACE,HOLE_TEMPERATURE);
#endif
		}

// COLLISION_FACTOR
		if (recompute_flags.is_set(ELECTRON,COLLISION_FACTOR)) {
			comp_value(ELECTRON,COLLISION_FACTOR);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,COLLISION_FACTOR);
#endif
		}

		if (recompute_flags.is_set(HOLE,COLLISION_FACTOR)) {
			comp_value(HOLE,COLLISION_FACTOR);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,COLLISION_FACTOR);
#endif
		}

// DOS_MASS
		if (recompute_flags.is_set(ELECTRON,DOS_MASS)) {
			comp_value(ELECTRON,DOS_MASS);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,DOS_MASS);
#endif
		}

		if (recompute_flags.is_set(HOLE,DOS_MASS)) {
			comp_value(HOLE,DOS_MASS);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,DOS_MASS);
#endif
		}

// COND_MASS
		if (recompute_flags.is_set(ELECTRON,COND_MASS)) {
			comp_value(ELECTRON,COND_MASS);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,COND_MASS);
#endif
		}

		if (recompute_flags.is_set(HOLE,COND_MASS)) {
			comp_value(HOLE,COND_MASS);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,COND_MASS);
#endif
		}

// REDUCED_DOS_MASS
		if (recompute_flags.is_set(NODE,REDUCED_DOS_MASS)) {
			comp_value(NODE,REDUCED_DOS_MASS);
#ifndef NDEBUG
			recompute_flags.clear(NODE,REDUCED_DOS_MASS);
#endif
		}

// EQUIL_DOS
		if (recompute_flags.is_set(ELECTRON,EQUIL_DOS)) {
			comp_value(ELECTRON,EQUIL_DOS);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,EQUIL_DOS);
#endif
		}

		if (recompute_flags.is_set(HOLE,EQUIL_DOS)) {
			comp_value(HOLE,EQUIL_DOS);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,EQUIL_DOS);
#endif
		}

// NON_EQUIL_DOS
		if (recompute_flags.is_set(ELECTRON,NON_EQUIL_DOS)) {
			comp_value(ELECTRON,NON_EQUIL_DOS);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,NON_EQUIL_DOS);
#endif
		}

		if (recompute_flags.is_set(HOLE,NON_EQUIL_DOS)) {
			comp_value(HOLE,NON_EQUIL_DOS);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,NON_EQUIL_DOS);
#endif
		}

// MOBILITY
		if (recompute_flags.is_set(ELECTRON,MOBILITY)) {
			comp_value(ELECTRON,MOBILITY);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,MOBILITY);
#endif
		}

		if (recompute_flags.is_set(HOLE,MOBILITY)) {
			comp_value(HOLE,MOBILITY);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,MOBILITY);
#endif
		}

// SHR_LIFETIME
		if (recompute_flags.is_set(ELECTRON,SHR_LIFETIME)) {
			comp_value(ELECTRON,SHR_LIFETIME);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,SHR_LIFETIME);
#endif
		}

		if (recompute_flags.is_set(HOLE,SHR_LIFETIME)) {
			comp_value(HOLE,SHR_LIFETIME);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,SHR_LIFETIME);
#endif
		}

// ENERGY_LIFETIME
		if (recompute_flags.is_set(ELECTRON,ENERGY_LIFETIME)) {
			comp_value(ELECTRON,ENERGY_LIFETIME);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,ENERGY_LIFETIME);
#endif
		}

		if (recompute_flags.is_set(HOLE,ENERGY_LIFETIME)) {
			comp_value(HOLE,ENERGY_LIFETIME);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,ENERGY_LIFETIME);
#endif
		}

// AUGER_COEFFICIENT
		if (recompute_flags.is_set(ELECTRON,AUGER_COEFFICIENT)) {
			comp_value(ELECTRON,AUGER_COEFFICIENT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,AUGER_COEFFICIENT);
#endif
		}

		if (recompute_flags.is_set(HOLE,AUGER_COEFFICIENT)) {
			comp_value(HOLE,AUGER_COEFFICIENT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,AUGER_COEFFICIENT);
#endif
		}

// ELECTRON_AFFINITY
		if (recompute_flags.is_set(GRID_ELECTRICAL,ELECTRON_AFFINITY)) {
			comp_value(GRID_ELECTRICAL,ELECTRON_AFFINITY);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,ELECTRON_AFFINITY);
#endif
		}

// PERMITIVITY
		if (recompute_flags.is_set(GRID_ELECTRICAL,PERMITIVITY)) {
			comp_value(GRID_ELECTRICAL,PERMITIVITY);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,PERMITIVITY);
#endif
		}

// THERMAL_CONDUCT
		if (recompute_flags.is_set(GRID_ELECTRICAL,THERMAL_CONDUCT)) {
			comp_value(GRID_ELECTRICAL,THERMAL_CONDUCT);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,THERMAL_CONDUCT);
#endif
		}

// LATERAL_THERMAL_CONDUCT
		if (recompute_flags.is_set(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT)) {
			comp_value(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,LATERAL_THERMAL_CONDUCT);
#endif
		}

// BAND_GAP
		if (recompute_flags.is_set(GRID_ELECTRICAL,BAND_GAP)) {
			comp_value(GRID_ELECTRICAL,BAND_GAP);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,BAND_GAP);
#endif
		}

// B_B_RECOMB_CONSTANT
		if (recompute_flags.is_set(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT)) {
			comp_value(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT);
#endif
		}

// EQUIL_PLANCK_POT
		if (recompute_flags.is_set(ELECTRON,EQUIL_PLANCK_POT) |
			recompute_flags.is_set(HOLE,EQUIL_PLANCK_POT)) {
			comp_value(ELECTRON,EQUIL_PLANCK_POT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,EQUIL_PLANCK_POT);
			recompute_flags.clear(HOLE,EQUIL_PLANCK_POT);
#endif
		}

// INTRINSIC_CONC
		if (recompute_flags.is_set(NODE,INTRINSIC_CONC)) {
			comp_value(NODE,INTRINSIC_CONC);
#ifndef NDEBUG
			recompute_flags.clear(NODE,INTRINSIC_CONC);
#endif
		}

// CONCENTRATION
		if (recompute_flags.is_set(ELECTRON,CONCENTRATION)) {
			comp_value(ELECTRON,CONCENTRATION);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,CONCENTRATION);
#endif
		}

		if (recompute_flags.is_set(HOLE,CONCENTRATION)) {
			comp_value(HOLE,CONCENTRATION);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,CONCENTRATION);
#endif
		}

// PLANCK_POT
		if (recompute_flags.is_set(ELECTRON,PLANCK_POT)) {
			comp_value(ELECTRON,PLANCK_POT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,PLANCK_POT);
#endif
		}

		if (recompute_flags.is_set(HOLE,PLANCK_POT)) {
			comp_value(HOLE,PLANCK_POT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,PLANCK_POT);
#endif
		}

// IONIZED_DOPING
		if (recompute_flags.is_set(ELECTRON,IONIZED_DOPING)) {
			comp_value(ELECTRON,IONIZED_DOPING);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,IONIZED_DOPING);
#endif
		}

		if (recompute_flags.is_set(HOLE,IONIZED_DOPING)) {
			comp_value(HOLE,IONIZED_DOPING);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,IONIZED_DOPING);
#endif
		}

// POTENTIAL
		if (recompute_flags.is_set(GRID_ELECTRICAL,POTENTIAL)) {
			comp_value(GRID_ELECTRICAL,POTENTIAL);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,POTENTIAL);
#endif
		}

// BAND_EDGE
		if (recompute_flags.is_set(ELECTRON,BAND_EDGE)) {
			comp_value(ELECTRON,BAND_EDGE);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,BAND_EDGE);
#endif
		}

		if (recompute_flags.is_set(HOLE,BAND_EDGE)) {
			comp_value(HOLE,BAND_EDGE);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,BAND_EDGE);
#endif
		}

// BUILT_IN_POT
		if (recompute_flags.is_set(CONTACT,BUILT_IN_POT)) {
			comp_value(CONTACT,BUILT_IN_POT);
#ifndef NDEBUG
			recompute_flags.clear(CONTACT,BUILT_IN_POT);
#endif
		}

// INCIDENT_REFRACTIVE_INDEX
		if (recompute_flags.is_set(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX) ||
			recompute_flags.is_set(GRID_OPTICAL,INCIDENT_ABSORPTION)) {
			comp_value(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,INCIDENT_REFRACTIVE_INDEX);
			recompute_flags.clear(GRID_OPTICAL,INCIDENT_ABSORPTION);
#endif
		}

// OPTICAL_GENERATION
		if (recompute_flags.is_set(NODE,OPTICAL_GENERATION)) {
			comp_value(NODE,OPTICAL_GENERATION);
#ifndef NDEBUG
			recompute_flags.clear(NODE,OPTICAL_GENERATION);
#endif
		}

// OPTICAL_GENERATION_REF
		if (recompute_flags.is_set(ELECTRON,OPTICAL_GENERATION_REF)) {
			comp_value(ELECTRON,OPTICAL_GENERATION_REF);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,OPTICAL_GENERATION_REF);
#endif
		}

		if (recompute_flags.is_set(HOLE,OPTICAL_GENERATION_REF)) {
			comp_value(HOLE,OPTICAL_GENERATION_REF);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,OPTICAL_GENERATION_REF);
#endif
		}

// SHR_RECOMB
		if (recompute_flags.is_set(NODE,SHR_RECOMB)) {
			comp_value(NODE,SHR_RECOMB);
#ifndef NDEBUG
			recompute_flags.clear(NODE,SHR_RECOMB);
#endif
		}

// SHR_HEAT
		if (recompute_flags.is_set(ELECTRON,SHR_HEAT)) {
			comp_value(ELECTRON,SHR_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,SHR_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,SHR_HEAT)) {
			comp_value(HOLE,SHR_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,SHR_HEAT);
#endif
		}

// AUGER_RECOMB
		if (recompute_flags.is_set(NODE,AUGER_RECOMB)) {
			comp_value(NODE,AUGER_RECOMB);
#ifndef NDEBUG
			recompute_flags.clear(NODE,AUGER_RECOMB);
#endif
		}

// AUGER_HEAT
		if (recompute_flags.is_set(ELECTRON,AUGER_HEAT)) {
			comp_value(ELECTRON,AUGER_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,AUGER_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,AUGER_HEAT)) {
			comp_value(HOLE,AUGER_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,AUGER_HEAT);
#endif
		}

// B_B_RECOMB
		if (recompute_flags.is_set(NODE,B_B_RECOMB)) {
			comp_value(NODE,B_B_RECOMB);
#ifndef NDEBUG
			recompute_flags.clear(NODE,B_B_RECOMB);
#endif
		}

// B_B_HEAT
		if (recompute_flags.is_set(ELECTRON,B_B_HEAT)) {
			comp_value(ELECTRON,B_B_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,B_B_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,B_B_HEAT)) {
			comp_value(HOLE,B_B_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,B_B_HEAT);
#endif
		}

		if (recompute_flags.is_set(NODE,B_B_HEAT)) {
			comp_value(NODE,B_B_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(NODE,B_B_HEAT);
#endif
		}

// MODE_PHOTON_ENERGY
		if (recompute_flags.is_set(GRID_OPTICAL,MODE_PHOTON_ENERGY)) {
			comp_value(GRID_OPTICAL,MODE_PHOTON_ENERGY);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,MODE_PHOTON_ENERGY);
#endif
		}

// MODE_REFRACTIVE_INDEX
		if (recompute_flags.is_set(GRID_OPTICAL,MODE_REFRACTIVE_INDEX) ||
			recompute_flags.is_set(GRID_OPTICAL,MODE_ABSORPTION)) {
			comp_value(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,MODE_REFRACTIVE_INDEX);
			recompute_flags.clear(GRID_OPTICAL,MODE_ABSORPTION);
#endif
		}

// MODE_GROUP_VELOCITY
		if (recompute_flags.is_set(MODE,MODE_GROUP_VELOCITY)) {
			comp_value(MODE,MODE_GROUP_VELOCITY);
#ifndef NDEBUG
			recompute_flags.clear(MODE,MODE_GROUP_VELOCITY);
#endif
		}

		if (recompute_flags.is_set(GRID_OPTICAL,MODE_GROUP_VELOCITY)) {
			comp_value(GRID_OPTICAL,MODE_GROUP_VELOCITY);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,MODE_GROUP_VELOCITY);
#endif
		}

// STIMULATED_FACTOR
		if (recompute_flags.is_set(ELECTRON,STIMULATED_FACTOR)) {
			comp_value(ELECTRON,STIMULATED_FACTOR);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,STIMULATED_FACTOR);
#endif
		}

		if (recompute_flags.is_set(HOLE,STIMULATED_FACTOR)) {
			comp_value(HOLE,STIMULATED_FACTOR);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,STIMULATED_FACTOR);
#endif
		}

// MODE_GAIN
		if (recompute_flags.is_set(GRID_OPTICAL,MODE_GAIN)) {
			comp_value(GRID_OPTICAL,MODE_GAIN);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,MODE_GAIN);
#endif
		}

// MODE_TOTAL_FIELD_MAG
		if (recompute_flags.is_set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG)) {
			comp_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
#ifndef NDEBUG
			recompute_flags.clear(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
#endif
		}

// MODE_NORMALIZATION
		if (recompute_flags.is_set(MODE,MODE_NORMALIZATION)) {
			comp_value(MODE,MODE_NORMALIZATION);
#ifndef NDEBUG
			recompute_flags.clear(MODE,MODE_NORMALIZATION);
#endif
		}

// STIM_RECOMB
		if (recompute_flags.is_set(NODE,STIM_RECOMB)) {
			comp_value(NODE,STIM_RECOMB);
#ifndef NDEBUG
			recompute_flags.clear(NODE,STIM_RECOMB);
#endif
		}

// STIM_HEAT
		if (recompute_flags.is_set(ELECTRON,STIM_HEAT)) {
			comp_value(ELECTRON,STIM_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,STIM_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,STIM_HEAT)) {
			comp_value(HOLE,STIM_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,STIM_HEAT);
#endif
		}

		if (recompute_flags.is_set(NODE,STIM_HEAT)) {
			comp_value(NODE,STIM_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(NODE,STIM_HEAT);
#endif
		}

// RELAX_HEAT
		if (recompute_flags.is_set(ELECTRON,RELAX_HEAT)) {
			comp_value(ELECTRON,RELAX_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,RELAX_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,RELAX_HEAT)) {
			comp_value(HOLE,RELAX_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,RELAX_HEAT);
#endif
		}

// TOTAL_RECOMB
		if (recompute_flags.is_set(NODE,TOTAL_RECOMB)) {
			comp_value(NODE,TOTAL_RECOMB);
#ifndef NDEBUG
			recompute_flags.clear(NODE,TOTAL_RECOMB);
#endif
		}

// TOTAL_HEAT
		if (recompute_flags.is_set(ELECTRON,TOTAL_HEAT)) {
			comp_value(ELECTRON,TOTAL_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,TOTAL_HEAT);
#endif
		}

		if (recompute_flags.is_set(HOLE,TOTAL_HEAT)) {
			comp_value(HOLE,TOTAL_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(HOLE,TOTAL_HEAT);
#endif
		}

		if (recompute_flags.is_set(NODE,TOTAL_HEAT)) {
			comp_value(NODE,TOTAL_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(NODE,TOTAL_HEAT);
#endif
		}

// TOTAL_RADIATIVE_HEAT
		if (recompute_flags.is_set(NODE,TOTAL_RADIATIVE_HEAT)) {
			comp_value(NODE,TOTAL_RADIATIVE_HEAT);
#ifndef NDEBUG
			recompute_flags.clear(NODE,TOTAL_RADIATIVE_HEAT);
#endif
		}

// TOTAL_CHARGE
		if (recompute_flags.is_set(NODE,TOTAL_CHARGE)) {
			comp_value(NODE,TOTAL_CHARGE);
#ifndef NDEBUG
			recompute_flags.clear(NODE,TOTAL_CHARGE);
#endif
		}

// FIELD
		if (recompute_flags.is_set(GRID_ELECTRICAL,FIELD)) {
			comp_value(GRID_ELECTRICAL,FIELD);
#ifndef NDEBUG
			recompute_flags.clear(GRID_ELECTRICAL,FIELD);
#endif
		}

// CURRENT
		if (recompute_flags.is_set(ELECTRON,CURRENT) ||
			recompute_flags.is_set(HOLE,CURRENT)) {
			comp_value(ELECTRON,CURRENT);
#ifndef NDEBUG
			recompute_flags.clear(ELECTRON,CURRENT);
			recompute_flags.clear(HOLE,CURRENT);
#endif
		}

// MODE_GAIN
		if (recompute_flags.is_set(MODE,MODE_GAIN)) {
			comp_value(MODE,MODE_GAIN);
#ifndef NDEBUG
			recompute_flags.clear(MODE,MODE_GAIN);
#endif
		}

// MIRROR_LOSS
		if (recompute_flags.is_set(MODE,MIRROR_LOSS)) {
			comp_value(MODE,MIRROR_LOSS);
#ifndef NDEBUG
			recompute_flags.clear(MODE,MIRROR_LOSS);
#endif
		}

// PHOTON_LIFETIME
		if (recompute_flags.is_set(MODE,PHOTON_LIFETIME)) {
			comp_value(MODE,PHOTON_LIFETIME);
#ifndef NDEBUG
			recompute_flags.clear(MODE,PHOTON_LIFETIME);
#endif
		}

// EQUIL_ELECTRON_CONC
		if (recompute_flags.is_set(CONTACT,EQUIL_ELECTRON_CONC)) {
			comp_value(CONTACT,EQUIL_ELECTRON_CONC);
#ifndef NDEBUG
			recompute_flags.clear(CONTACT,EQUIL_ELECTRON_CONC);
#endif
		}

// EQUIL_HOLE_CONC
		if (recompute_flags.is_set(CONTACT,EQUIL_HOLE_CONC)) {
			comp_value(CONTACT,EQUIL_HOLE_CONC);
#ifndef NDEBUG
			recompute_flags.clear(CONTACT,EQUIL_HOLE_CONC);
#endif
		}

		device_ptr->update_solution_param();
	}
#ifndef NDEBUG
	else recompute_flags.clear_all();
#else
	recompute_flags.clear_all();
#endif
	return(plot_update);
}



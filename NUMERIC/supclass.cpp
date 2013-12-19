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

/****************************** class TErrorHandler ********************************************

class TErrorHandler {
private:
	error error_number;
	float misc_parameter;
	string misc_string;
	string file;
#ifndef NDEBUG
	string source_file;
	int line_number;
#endif

public:
	TErrorHandler(void) { clear(); }
	logical fail(void) { return(error_number!=ERROR_NONE); }
#ifndef NDEBUG
	void clear(void)
		{ error_number=ERROR_NONE; misc_parameter=0; misc_string=""; file=""; source_file=""; line_number=0; }
	void set(error number, float new_parameter, string new_string, string new_file,
			 string new_source_file, int new_line_number);
#else
	void clear(void)
		{ error_number=ERROR_NONE; misc_parameter=0; misc_string=""; file=""; }
	void set(error number, float new_parameter, string new_string, string new_file);
#endif
	error get_error_number(void) { return(error_number); }
	float get_misc_parameter(void) { return(misc_parameter); }
	string get_misc_string(void) { return(misc_string); }
	string get_file(void) { return(file); }
	string get_error_string(void);

#ifndef NDEBUG
	string get_source_file(void) { return(source_file); }
	int get_line_number(void) { return(line_number); }
#endif
};

#ifndef NDEBUG
#define set_error(number,parameter,string,file) set(number,parameter,string,file,__FILE__,__LINE__)
#else
#define set_error(number,parameter,string,file) set(number,parameter,string,file)
#endif


*/

#ifndef NDEBUG
void TErrorHandler::set(error number, float new_parameter, string new_string, string new_file,
						string new_source_file, int new_line_number)
{
	if (number==ERROR_SIMULATION) assert ((error_number==0) || (error_number==ERROR_SIMULATION));
	else assert(error_number==0);
	error_number=number;
	misc_parameter=new_parameter;
	misc_string=new_string;
	file=new_file;
	source_file=new_source_file;
	line_number=new_line_number;
}
#else
void TErrorHandler::set(error number, float new_parameter, string new_string, string new_file)
{
	error_number=number;
	file=new_file;
	misc_parameter=new_parameter;
	misc_string=new_string;
}
#endif

string TErrorHandler::get_error_string(void)
{
	string error_string, number_string;

	number_string=int_to_string((int)misc_parameter);

	switch(error_number) {
		case ERROR_MEM_DEVICE_GRID:
			return("Memory error creating device grid");
		case ERROR_MEM_QW:
			return("Memory error creating quantum wells");
		case ERROR_MEM_CONTACT:
			return("Memory error creating contacts");
		case ERROR_MEM_SURFACE:
			return("Memory error creating surfaces");
		case ERROR_MEM_CAVITY:
			return("Memory error creating cavity");
		case ERROR_MEM_SOLUTION_GRID:
			return("Memory error creating solution grid");
		case ERROR_MEM_SOLUTION_ELEMENT:
			return("Memory error creating solution elements");
		case ERROR_MEM_PLOT:
			return("Memory error creating plot");
		case ERROR_MEM_SPECTRAL_COMP:
			return("Memory error creating spectral component");
		case ERROR_MEM_PARSE_DEVICE:
			return("Memory error parsing device file");
		case ERROR_MEM_PARSE_TERMS:
			return("Memory error parsing polynomial terms");
		case ERROR_MEM_DEVICE_INPUT:
			return("Memory error allocating device input");
		case ERROR_MEM_STRING_LIST:
			return("Memory error storing strings");
		case ERROR_MEM_SOLUTION_ARRAYS:
			return("Memory error allocating solution arrays");
		case ERROR_MEM_USER_FUNCTION:
			return("Memory error allocating user function");
		case ERROR_MEM_ALLOY:
			return("Memory error allocating alloy");
		case ERROR_MEM_MATERIAL:
			return("Memory error allocating material");
		case ERROR_MEM_MATERIAL_MODEL:
			return("Memory error allocating material model");

		case ERROR_FILE_NOT_OPEN:
			error_string="The file "+shorten_path(file)+" could not be opened";
			return(error_string);
		case ERROR_FILE_OLD_STATE:
			error_string="The state file "+shorten_path(file)+" was made with a previous version of SimWindows";
			return(error_string);

		case ERROR_PARSE_TERMS:
			error_string="Error parsing terms in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_FLOAT:
			error_string="Invalid floating point for "+misc_string+" in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_NAME_STRING:
			error_string="Error parsing "+misc_string+" in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_NAME:
			error_string="Error parsing a value for "+misc_string+" in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_REGION_TYPE:
			error_string="Error parsing a region type in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_CAVITY_TYPE:
			error_string="Error parsing a cavity type in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_MIRROR_TYPE:
			error_string="Error parsing a mirror type in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_DOUBLE_REPEAT:
			error_string="A REPEAT START found before a REPEAT NUMBER= in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_NO_REPEAT_START:
			error_string="A REPEAT NUMBER= found before a REPEAT START in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_UNKNOWN_SYMBOL:
			error_string="An unknown symbol was found in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_FUNCTION:
			error_string="The function for "+misc_string+" found in "+shorten_path(file)+" at line "
						 +number_string+" is invalid";
			return(error_string);
		case ERROR_PARSE_MODEL_INVALID:
			error_string="The model "+misc_string+" found in "+shorten_path(file)+" at line "+number_string+" is invalid";
			return(error_string);
		case ERROR_PARSE_MODEL_INCORRECT_TERMS:
			error_string="There are an incorrect number of terms found in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_MODEL_UNKNOWN:
			error_string="The model "+misc_string+" found in "+shorten_path(file)+" at line "+number_string+" is unknown";
			return(error_string);
		case ERROR_PARSE_LONG:
			error_string="Invalid integer for "+misc_string+" in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_RADIUS:
			error_string="A second radius was found in "+shorten_path(file)+" at line "+number_string;
			return(error_string);
		case ERROR_PARSE_KEYWORD:
			error_string="An unknown keyword was found in "+shorten_path(file)+" at line "+number_string;
			return(error_string);

		case ERROR_DEVICE_NO_GRID:
			return("No device grid found");
		case ERROR_DEVICE_NO_STRUCTURE:
			return("No device structure found");
		case ERROR_DEVICE_GRID_LENGTH:
			return("The total length for the grid and the structure are not equal");
		case ERROR_DEVICE_REGION_LENGTH:
			return("The total length for the region and the structure are not equal");
		case ERROR_DEVICE_DOPING_LENGTH:
			return("The total length for the doping and the structure are not equal");
		case ERROR_DEVICE_NUMBER_CAVITY:
			return("An incorrect number of cavities were specified");
		case ERROR_DEVICE_NUMBER_MIRROR:
			return("An incorrect number of mirrors were specified");
		case ERROR_DEVICE_MIRROR_REF:
			error_string="The reflectivity for mirror "+number_string+" was incorrect";
			return(error_string);
		case ERROR_DEVICE_MATERIAL_PARAM_LENGTH:
			return("The total length for material parameters and the structure are not equal");

		case ERROR_MAT_INVALID_NAME:
			error_string="The material "+misc_string+" in "+shorten_path(file)+" at line "
						 +number_string+" is not present in the material parameters file";
			return(error_string);
		case ERROR_MAT_INVALID_TYPE:
			error_string="The material number "+number_string+" in "+shorten_path(file)+" at line "
						 +number_string+" is not present in the material parameters file";
			return(error_string);

		case ERROR_ALLOY_INVALID_NAME:
			error_string="The alloy "+misc_string+" in "+shorten_path(file)+" at line "
						 +number_string+" is not present in the material parameters file";
			return(error_string);

		case ERROR_ALLOY_INVALID_TYPE:
			error_string="The alloy number "+number_string+"in "+shorten_path(file)+" at line "
						 +number_string+" is not present in the material parameters file";
			return(error_string);

		case ERROR_SIMULATION:
		return("An numerical error occurred during the simulation. You should either \"Reset device\" or \"Undo simulation\"");

		case ERROR_FUNCTION_TRANSLATE:
			return("The user function could not be translated");

		case WARNING_PLOT_SAME_SCALE:
			return("Scale has the same\nmaximum and minimum");

		default:
			assert(FALSE);
			return("");
	}
}

/********************************** class TPreferences ****************************************
class TPreferences {
private:
	logical tool_bar;
	logical status_bar;
	string material_parameters_file;
	int write_grid_multiplier;
public:
	TPreferences(void)
    	: tool_bar(TRUE), status_bar(TRUE),
          material_parameters_file("material.prm"),
          write_grid_multiplier(1) {}
	void enable_toolbar(logical enable) { tool_bar=enable; }
	logical is_toolbar(void) { return(tool_bar); }
	void enable_statusbar(logical enable) { status_bar=enable; }
	logical is_statusbar(void) { return(status_bar); }
	void put_material_parameters_file(const string& file)
		{ material_parameters_file=file; }
	string get_material_parameters_file(void) { return(material_parameters_file); }
	void put_write_grid_multiplier(int multiplier) { write_grid_multiplier=multiplier; }
	int get_write_grid_multiplier(void) { return(write_grid_multiplier); }
};
*/

/********************************** class TFlag *********************************************

class TFlag {
protected:
	FlagGroup flag_group;
	flag flag_array[NUMBER_FLAG_TYPES];
protected:
	TFlag(FlagGroup new_group) { flag_group=new_group; clear_all(); }
	TFlag(FlagGroup new_group, FlagType flag_type, flag flag_value)
		{ flag_group=new_group; clear_all(); set(flag_type,flag_value); }
	TFlag(const TFlag& new_flag);
public:
	~TFlag(void) {}
	int count(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(bit_count(flag_array[flag_type-1])); }
	int count_all(void);
	void clear(FlagType flag_type, flag flag_value) { flag_array[flag_type-1]&=(~flag_value); }
	void clear_all(void);
	flag get_flag(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value) { flag_array[flag_type-1]|=flag_value; }
	logical is_set(FlagType flag_type, flag flag_value) { return((flag_array[flag_type-1]&flag_value)!=0); }
	logical any_set(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(flag_array[flag_type-1]!=0); }
	logical any_set(void);
	TFlag& operator=(const TFlag& new_flag);
	TFlag& operator~(void);
	TFlag& operator|=(const TFlag& new_flag);
	TFlag& operator&=(const TFlag& new_flag);
#ifndef NDEBUG
	static logical valid_flag_type(FlagType flag_type);
#endif
};

*/

TFlag::TFlag(const TFlag& new_flag)
{
	int i;

	flag_group=new_flag.flag_group;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]=new_flag.flag_array[i];
}

int TFlag::count_all(void)
{
	int i,flag_count=0;
	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++) flag_count+=count((FlagType)i);
	return(flag_count);
}

void TFlag::clear_all(void)
{
	int i;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]=VALUE_CLEAR_ALL;
}

logical TFlag::any_set(void)
{
	int i;

	for (i=0;i<NUMBER_FLAG_TYPES;i++)
		if (flag_array[i]!=0) return(TRUE);
	return(FALSE);
}

TFlag& TFlag::operator=(const TFlag& new_flag)
{
	int i;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]=new_flag.flag_array[i];
	return (*this);
}

TFlag& TFlag::operator~(void)
{
	int i;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]=~flag_array[i];
	return (*this);
}

TFlag& TFlag::operator|=(const TFlag& new_flag)
{
	int i;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]|=new_flag.flag_array[i];
	return (*this);
}

TFlag& TFlag::operator&=(const TFlag& new_flag)
{
	int i;
	for (i=0;i<NUMBER_FLAG_TYPES;i++) flag_array[i]&=new_flag.flag_array[i];
	return (*this);
}

#ifndef NDEBUG

logical TFlag::valid_flag_type(FlagType flag_type)
{
	logical result=TRUE;
	result&=((flag_type>=START_FLAG_TYPE) && (flag_type<=END_FLAG_TYPE)) ? TRUE : FALSE;
	return(result);
}

#endif

/************************************** class TValueFlag **************************************

class TValueFlag : public TFlag {
protected:
	static flag max_flag_array[];
	static flag valid_flag_array[];

#ifndef NDEBUG
	static flag valid_write_flag_array[];
	static flag valid_plot_flag_array[];
#endif

public:
	TValueFlag(void) : TFlag(VALUE_FLAGS) {}
	TValueFlag(FlagType flag_type, flag flag_value) : TFlag(VALUE_FLAGS,flag_type,flag_value) {}
	TValueFlag(const TValueFlag& new_flag) : TFlag(new_flag) {}
	~TValueFlag(void) {}
	void clear(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::clear(flag_type,flag_value); }
	static flag get_max(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(max_flag_array[flag_type-1]); }
	static flag get_valid(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::set(flag_type,flag_value); }
	logical is_set(FlagType flag_type, flag flag_value)
		{ assert(valid_single_flag(flag_type,flag_value)); return(TFlag::is_set(flag_type,flag_value)); }
	void set_all(void);
	void set_plot_combo(FlagCombo flag_combo);
	void set_write_combo(FlagCombo flag_combo);

#ifndef NDEBUG
	static logical valid_single_flag(FlagType flag_type, flag flag_value);
	static logical valid_multiple_flag(FlagType flag_type, flag flag_value);
	static flag get_write(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_write_flag_array[flag_type-1]); }
	static logical valid_write_flag(FlagType flag_type, flag flag_value);
	static flag get_plot(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_plot_flag_array[flag_type-1]); }
	static logical valid_plot_flag(FlagType flag_type, flag flag_value);
#endif

};
*/

flag TValueFlag::max_flag_array[]={ FREE_ELECTRON_MAX, BOUND_ELECTRON_MAX, FREE_HOLE_MAX,
									BOUND_HOLE_MAX, ELECTRON_MAX, HOLE_MAX, GRID_ELECTRICAL_MAX,
									GRID_OPTICAL_MAX, NODE_MAX, QW_ELECTRON_MAX, QW_HOLE_MAX,
									QUANTUM_WELL_MAX, MODE_MAX, MIRROR_MAX, CAVITY_MAX,
									CONTACT_MAX, SURFACE_MAX, DEVICE_MAX, ENVIRONMENT_MAX,
									SPECTRUM_MAX };

flag TValueFlag::valid_flag_array[]={ FREE_ELECTRON_ALL, BOUND_ELECTRON_ALL, FREE_HOLE_ALL,
									  BOUND_HOLE_ALL, ELECTRON_ALL, HOLE_ALL, GRID_ELECTRICAL_ALL,
									  GRID_OPTICAL_ALL, NODE_ALL, QW_ELECTRON_ALL, QW_HOLE_ALL,
									  QUANTUM_WELL_ALL, MODE_ALL, MIRROR_ALL, CAVITY_ALL,
									  CONTACT_ALL, SURFACE_ALL, DEVICE_ALL, ENVIRONMENT_ALL,
									  SPECTRUM_ALL };

#ifndef NDEBUG
flag TValueFlag::valid_write_flag_array[]={ FREE_ELECTRON_WRITE, BOUND_ELECTRON_WRITE, FREE_HOLE_WRITE,
											BOUND_HOLE_WRITE, ELECTRON_WRITE, HOLE_WRITE, GRID_ELECTRICAL_WRITE,
											GRID_OPTICAL_WRITE, NODE_WRITE, QW_ELECTRON_WRITE, QW_HOLE_WRITE,
											QUANTUM_WELL_WRITE, MODE_WRITE, MIRROR_WRITE, CAVITY_WRITE,
											CONTACT_WRITE, SURFACE_WRITE, DEVICE_WRITE, ENVIRONMENT_WRITE,
											SPECTRUM_WRITE };

flag TValueFlag::valid_plot_flag_array[]={ FREE_ELECTRON_PLOT, BOUND_ELECTRON_PLOT, FREE_HOLE_PLOT,
										   BOUND_HOLE_PLOT, ELECTRON_PLOT, HOLE_PLOT, GRID_ELECTRICAL_PLOT,
										   GRID_OPTICAL_PLOT, NODE_PLOT, QW_ELECTRON_PLOT, QW_HOLE_PLOT,
										   QUANTUM_WELL_PLOT, MODE_PLOT, MIRROR_PLOT, CAVITY_PLOT,
										   CONTACT_PLOT, SURFACE_PLOT, DEVICE_PLOT, ENVIRONMENT_PLOT,
										   SPECTRUM_PLOT };
#endif


void TValueFlag::set_all(void)
{
	int i;

	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++)
		set((FlagType)i,get_valid((FlagType)i));
}

void TValueFlag::set_plot_combo(FlagCombo flag_combo)
{
	clear_all();

	switch(flag_combo) {
		case COMBO_BAND:
			set(ELECTRON, BAND_EDGE | QUASI_FERMI);
			set(HOLE, BAND_EDGE | QUASI_FERMI);
			return;
		case COMBO_RECOMB:
			set(NODE, TOTAL_RECOMB | SHR_RECOMB | B_B_RECOMB | AUGER_RECOMB | STIM_RECOMB | OPTICAL_GENERATION);
			return;
		case COMBO_ELECTROSTATICS:
			set(GRID_ELECTRICAL, FIELD | POTENTIAL);
			set(NODE, TOTAL_CHARGE);
			return;
		case COMBO_CURRENT:
			set(NODE, TOTAL_CURRENT);
			set(ELECTRON,CURRENT);
			set(HOLE,CURRENT);
			return;
		case COMBO_FREE_CONC:
			set(FREE_ELECTRON,CONCENTRATION);
			set(FREE_HOLE,CONCENTRATION);
			return;
		case COMBO_BOUND_CONC:
			set(BOUND_ELECTRON,CONCENTRATION);
			set(BOUND_HOLE,CONCENTRATION);
			return;
		case COMBO_TOTAL_CONC:
			set(ELECTRON,CONCENTRATION);
			set(HOLE,CONCENTRATION);
			return;
		case COMBO_ALL_CONC:
			set(FREE_ELECTRON,CONCENTRATION);
			set(FREE_HOLE,CONCENTRATION);
			set(BOUND_ELECTRON,CONCENTRATION);
			set(BOUND_HOLE,CONCENTRATION);
			set(ELECTRON,CONCENTRATION);
			set(HOLE,CONCENTRATION);
			return;
		case COMBO_DOPING:
			set(ELECTRON, DOPING_CONC);
			set(HOLE,DOPING_CONC);
			return;
		case COMBO_MATERIAL:
			set(GRID_OPTICAL, INCIDENT_REFRACTIVE_INDEX);
			set(GRID_ELECTRICAL,ELECTRON_AFFINITY | PERMITIVITY |
								THERMAL_CONDUCT | B_B_RECOMB_CONSTANT | BAND_GAP);
			set(ELECTRON, DOS_MASS | COND_MASS | SHR_LIFETIME | ENERGY_LIFETIME | COLLISION_FACTOR);
			set(FREE_ELECTRON, MOBILITY);
			set(HOLE, DOS_MASS | COND_MASS | SHR_LIFETIME | ENERGY_LIFETIME | COLLISION_FACTOR);
			set(FREE_HOLE, MOBILITY);
			return;
		case COMBO_LASER:
			set(GRID_OPTICAL, MODE_GAIN | MODE_TOTAL_POYNTING | MODE_PHOTON_DENSITY);
			set(ELECTRON,STIMULATED_FACTOR);
			set(HOLE,STIMULATED_FACTOR);
			return;
		case COMBO_STRUCTURE:
			set(ELECTRON, DOPING_CONC | DOPING_DEGENERACY | DOPING_LEVEL);
			set(HOLE,DOPING_CONC | DOPING_DEGENERACY | DOPING_LEVEL);
			set(GRID_ELECTRICAL, REGION_TYPE | MATERIAL | ALLOY_TYPE | ALLOY_CONC);
			return;
		case COMBO_INCIDENT_SPECTRUM:
			set(SPECTRUM, INCIDENT_INPUT_INTENSITY | INCIDENT_EMITTED_INTENSITY | INCIDENT_REFLECT_INTENSITY);
			return;
		case COMBO_RETURN_ALL:
			set(FREE_ELECTRON,FREE_ELECTRON_WRITE);
			set(BOUND_ELECTRON,BOUND_ELECTRON_WRITE);
			set(FREE_HOLE,FREE_HOLE_WRITE);
			set(BOUND_HOLE,BOUND_HOLE_WRITE);
			set(ELECTRON,ELECTRON_WRITE);
			set(HOLE,HOLE_WRITE);
			set(GRID_ELECTRICAL,GRID_ELECTRICAL_WRITE);
			set(GRID_OPTICAL,GRID_OPTICAL_WRITE);
			set(NODE,NODE_WRITE);
			set(CONTACT,CONTACT_WRITE);
			set(QW_ELECTRON,QW_ELECTRON_WRITE);
			set(QW_HOLE,QW_HOLE_WRITE);
			set(QUANTUM_WELL,QUANTUM_WELL_WRITE);
			set(MODE,MODE_WRITE);
			set(CAVITY,CAVITY_WRITE);
			set(MIRROR,MIRROR_WRITE);
			set(DEVICE,DEVICE_WRITE);
			set(ENVIRONMENT,ENVIRONMENT_WRITE);
			set(SPECTRUM,SPECTRUM_WRITE);
			return;
		default: assert(FALSE);
	}
}

void TValueFlag::set_write_combo(FlagCombo flag_combo)
{
	set_plot_combo(flag_combo);

	switch(flag_combo) {
		case COMBO_BAND:
		case COMBO_RECOMB:
		case COMBO_ELECTROSTATICS:
		case COMBO_CURRENT:
		case COMBO_FREE_CONC:
		case COMBO_BOUND_CONC:
		case COMBO_TOTAL_CONC:
		case COMBO_ALL_CONC:
		case COMBO_DOPING:
		case COMBO_MATERIAL:
		case COMBO_STRUCTURE:
		case COMBO_LASER: set(GRID_ELECTRICAL,POSITION); return;
		case COMBO_INCIDENT_SPECTRUM: set(SPECTRUM, INCIDENT_PHOTON_WAVELENGTH); return;
		default: assert(FALSE);
	}
}

#ifndef NDEBUG

logical TValueFlag::valid_single_flag(FlagType flag_type, flag flag_value)
{
	logical result;

	result=valid_multiple_flag(flag_type,flag_value);
	result&=(bit_count(flag_value)==1) ? TRUE : FALSE;

	return(result);
}

logical TValueFlag::valid_multiple_flag(FlagType flag_type, flag flag_value)
{
	logical result=TRUE;

	result&=valid_flag_type(flag_type);
	result&=((flag_value & (~get_valid(flag_type)))==0) ? TRUE : FALSE;

	return(result);
}

logical TValueFlag::valid_write_flag(FlagType flag_type, flag flag_value)
{
	logical result=TRUE;

	result&=valid_multiple_flag(flag_type,flag_value);
	result&=((flag_value & (~get_write(flag_type)))==0) ? TRUE : FALSE;

	return(result);
}

logical TValueFlag::valid_plot_flag(FlagType flag_type, flag flag_value)
{
	logical result=TRUE;

	result&=valid_multiple_flag(flag_type,flag_value);
	result&=((flag_value & (~get_plot(flag_type)))==0) ? TRUE : FALSE;

	return(result);
}

#endif


//**************************** class TFlagWithObject ******************************************
/*
class TValueFlagWithObject : public TValueFlag {
protected:
	int number_objects[NUMBER_FLAG_TYPES];
	ObjectEntry *first_entry[NUMBER_FLAG_TYPES];
	ObjectEntry *last_entry[NUMBER_FLAG_TYPES];
public:
	TValueFlagWithObject(void);
	TValueFlagWithObject(FlagType flag_type, flag flag_value);
	TValueFlagWithObject(const TFlagValueWithObject& new_flag);
	~TValueFlagWithObject(void) { clear_all_objects(); }
	void add_object(FlagType flag_type, int object_number);
	void clear_objects(FlagType flag_type);
	void clear_all_objects(void);
	int count_with_object(FlagType flag_type) { return(count(flag_type)*number_objects[flag_type-1]); }
	int count_all_with_object(void);
	int get_number_objects(FlagType flag_type) { return(number_objects[flag_type-1]); }
	int get_object(FlagType flag_type, int object_number);
	void put_object(FlagType flag_type, int object_number);
	TValueFlagWithObject& operator=(const TValueFlagWithObject& new_flag);
};
*/

TValueFlagWithObject::TValueFlagWithObject(void)
	: TValueFlag()
{
	int i;

	for (i=0;i<NUMBER_FLAG_TYPES;i++) number_objects[i]=0;
	clear_all_objects();
}

TValueFlagWithObject::TValueFlagWithObject(FlagType flag_type, flag flag_value)
	: TValueFlag(flag_type,flag_value)
{
	int i;

	for (i=0;i<NUMBER_FLAG_TYPES;i++) number_objects[i]=0;
	clear_all_objects();
}

TValueFlagWithObject::TValueFlagWithObject(const TValueFlagWithObject& new_flag)
	: TValueFlag(new_flag)
{
	int i,j;
	int object;
	ObjectEntry *curr_entry_ptr;

	for (i=0;i<NUMBER_FLAG_TYPES;i++) {
		number_objects[i]=0;
		first_entry[i]=(ObjectEntry *)0;
		last_entry[i]=(ObjectEntry *)0;
		curr_entry_ptr=new_flag.first_entry[i];
		for (j=0;j<new_flag.number_objects[i];j++) {
			object=curr_entry_ptr->object;
			curr_entry_ptr=curr_entry_ptr->next_entry;
			add_object((FlagType)(i+1),object);
		}
	}
}

void TValueFlagWithObject::add_object(FlagType flag_type, int object_number)
{
	assert(valid_flag_type(flag_type));

	if (!number_objects[flag_type-1]) {
		first_entry[flag_type-1]=new ObjectEntry;
		first_entry[flag_type-1]->object=object_number;
		first_entry[flag_type-1]->next_entry=(ObjectEntry *)0;
		last_entry[flag_type-1]=first_entry[flag_type-1];
	}
	else {
		last_entry[flag_type-1]->next_entry=new ObjectEntry;
		last_entry[flag_type-1]=last_entry[flag_type-1]->next_entry;
		last_entry[flag_type-1]->object=object_number;
		last_entry[flag_type-1]->next_entry=(ObjectEntry *)0;
	}
	number_objects[flag_type-1]++;
}

void TValueFlagWithObject::clear_objects(FlagType flag_type)
{
	int i;
	ObjectEntry *curr_entry_ptr, *next_entry_ptr;

	assert(valid_flag_type(flag_type));

	curr_entry_ptr=first_entry[flag_type-1];
	for (i=0;i<number_objects[flag_type-1];i++) {
		next_entry_ptr=curr_entry_ptr->next_entry;
		delete curr_entry_ptr;
		curr_entry_ptr=next_entry_ptr;
	}

	number_objects[flag_type-1]=0;
	first_entry[flag_type-1]=(ObjectEntry *)0;
	last_entry[flag_type-1]=(ObjectEntry *)0;
}

void TValueFlagWithObject::clear_all_objects(void)
{
	int i;
	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++) clear_objects((FlagType)i);
}

int TValueFlagWithObject::count_all_with_object(void)
{
	int i, count=0;

	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++) count+=count_with_object((FlagType)i);
	return(count);
}

int TValueFlagWithObject::get_object(FlagType flag_type, int object_number)
{
	int i;
	ObjectEntry *curr_entry_ptr;

	assert(valid_flag_type(flag_type));

	if (!number_objects[flag_type-1]) return(NULL);

	if (object_number>=number_objects[flag_type-1]) object_number=number_objects[flag_type-1]-1;

	curr_entry_ptr=first_entry[flag_type-1];
	for (i=0;i<object_number;i++) curr_entry_ptr=curr_entry_ptr->next_entry;

	return(curr_entry_ptr->object);
}

TValueFlagWithObject& TValueFlagWithObject::operator=(const TValueFlagWithObject& new_flag)
{
	int i,j;
	int object;
	ObjectEntry *curr_entry_ptr;

	for (i=0;i<NUMBER_FLAG_TYPES;i++) {
		clear_objects((FlagType)(i+1));
		flag_array[i]=new_flag.flag_array[i];
		curr_entry_ptr=new_flag.first_entry[i];
		for (j=0;j<new_flag.number_objects[i];j++) {
			object=curr_entry_ptr->object;
			curr_entry_ptr=curr_entry_ptr->next_entry;
			add_object((FlagType)(i+1),object);
		}
	}
	return (*this);
}

/************************************** class TEffectFlag **************************************

class TEffectFlag : public TFlag {
protected:
	static flag max_flag_array[];
	static flag valid_flag_array[];
public:
	TEffectFlag(void) : TFlag(EFFECT_FLAGS) {}
	TEffectFlag(FlagType flag_type, flag flag_value) : TFlag(EFFECT_FLAGS,flag_type,flag_value) {}
	TEffectFlag(const TEffectFlag& new_flag) : TFlag(new_flag) {}
	~TEffectFlag(void) {}
	void clear(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::clear(flag_type,flag_value); }
	static flag get_max(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(max_flag_array[flag_type-1]); }
	static flag get_valid(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::set(flag_type,flag_value); }
	logical is_set(FlagType flag_type, flag flag_value)
		{ assert(valid_single_flag(flag_type,flag_value)); return(TFlag::is_set(flag_type,flag_value)); }
	void set_all(void);

#ifndef NDEBUG
	static logical valid_single_flag(FlagType flag_type, flag flag_value);
	static logical valid_multiple_flag(FlagType flag_type, flag flag_value);
#endif

};
*/

flag TEffectFlag::max_flag_array[]={ VALUE_NONE, VALUE_NONE, VALUE_NONE, VALUE_NONE,
									 VALUE_NONE, VALUE_NONE, GRID_EFFECTS_MAX, VALUE_NONE,
									 VALUE_NONE, VALUE_NONE, VALUE_NONE, QW_EFFECTS_MAX,
									 MODE_EFFECTS_MAX, VALUE_NONE, VALUE_NONE, CONTACT_EFFECTS_MAX,
									 SURFACE_EFFECTS_MAX, DEVICE_EFFECTS_MAX, ENV_EFFECTS_MAX,
									 VALUE_NONE };

flag TEffectFlag::valid_flag_array[]={ VALUE_NONE, VALUE_NONE, VALUE_NONE, VALUE_NONE,
									   VALUE_NONE, VALUE_NONE, GRID_EFFECTS_ALL, VALUE_NONE,
									   VALUE_NONE, VALUE_NONE, VALUE_NONE, QW_EFFECTS_ALL,
									   MODE_EFFECTS_ALL, VALUE_NONE, VALUE_NONE, CONTACT_EFFECTS_ALL,
									   SURFACE_EFFECTS_ALL, DEVICE_EFFECTS_ALL, ENV_EFFECTS_ALL,
									   VALUE_NONE };

void TEffectFlag::set_all(void)
{
	int i;

	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++)
		set((FlagType)i,get_valid((FlagType)i));
}

#ifndef NDEBUG

logical TEffectFlag::valid_single_flag(FlagType flag_type, flag flag_value)
{
	logical result;

	result=valid_multiple_flag(flag_type,flag_value);
	result&=(bit_count(flag_value)==1) ? TRUE : FALSE;

	return(result);
}

logical TEffectFlag::valid_multiple_flag(FlagType flag_type, flag flag_value)
{
	logical result=TRUE;

	result&=valid_flag_type(flag_type);
	result&=((flag_value & (~get_valid(flag_type)))==0) ? TRUE : FALSE;

	return(result);
}

#endif



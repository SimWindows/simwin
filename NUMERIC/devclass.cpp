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
#include "simfrcar.h"
#include "simbdcar.h"
#include "simcar.h"
#include "simgrid.h"
#include "simnode.h"
#include "sim2dcar.h"
#include "simqw.h"
#include "simsol.h"
#include "simsurf.h"
#include "simcont.h"
#include "simmode.h"
#include "simmir.h"
#include "simcav.h"
#include "simdev.h"

/************************************* class TDevice *******************************************

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

*/


TDevice::TDevice(TDeviceFileInput new_device_input)
	: device_input(new_device_input)
{
	init_device_param();
	number_contacts=2;
	number_surfaces=2;

	material_parameters.put_device_file(&device_input);
	material_parameters.set_normalization(device_input.get_material_type(0));

	establish_grid();
	if (error_handler.fail()) return;
	process_input_param();

	solution_ptr=new TSolution(this,grid_ptr,qw_ptr);
}

TDevice::TDevice(FILE *file_ptr)
{
	int i;

	init_device_param();
	number_contacts=2;
	number_surfaces=2;

	read_state_file(file_ptr);
	material_parameters.put_device_file(&device_input);

	establish_grid();
	if (error_handler.fail()) return;

	for (i=0;i<grid_points;i++)
		(*(grid_ptr+i))->read_state_file(file_ptr);

	for (i=0;i<quantum_wells;i++) {
		(*(qw_ptr+i))->read_state_file(file_ptr);
	}

	if (quantum_wells) {
		comp_value(QUANTUM_WELL,POSITION);
		comp_value(QUANTUM_WELL,MIDPOINT);
	}

	for (i=0;i<number_contacts;i++)
		(*(contact_ptr+i))->read_state_file(file_ptr);

	for (i=0;i<number_surfaces;i++)
		(*(surface_ptr+i))->read_state_file(file_ptr);

	if (cavity_ptr) cavity_ptr->read_state_file(file_ptr);

	solution_ptr=new TSolution(this,grid_ptr,qw_ptr);
	if (error_handler.fail()) return;
	solution_ptr->comp_independent_param();
}

TDevice::~TDevice(void)
{
	int i;

	for (i=0;i<grid_points;i++) delete grid_ptr[i];
	if (grid_ptr) delete[] grid_ptr;

	for (i=0;i<quantum_wells;i++) delete qw_ptr[i];
	if (qw_ptr) delete[] qw_ptr;

	for (i=0;i<number_contacts;i++) delete contact_ptr[i];
	if (contact_ptr) delete[] contact_ptr;

	for (i=0;i<number_surfaces;i++) delete surface_ptr[i];
	if (surface_ptr) delete[] surface_ptr;

	if (cavity_ptr) delete cavity_ptr;

	if (solution_ptr) delete solution_ptr;

    material_parameters.put_device_file(NULL);
}

int TDevice::get_number_objects(FlagType flag_type)
{
	assert(TFlag::valid_flag_type(flag_type));

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case FREE_HOLE:
		case BOUND_HOLE:
		case ELECTRON:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE: return(grid_points);
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL: return(quantum_wells);
		case MODE:
			if (cavity_ptr) return(1);
			else return(0);
		case CAVITY:
			if (cavity_ptr) return(1);
			return(0);
		case MIRROR:
			if (cavity_ptr) return(2);
			else return(2);
		case CONTACT: return(number_contacts);
		case SURFACE: return(number_surfaces);
		default: assert(FALSE); return(0);
	}
}

prec TDevice::get_value(FlagType flag_type, flag flag_value, int number,
						ScaleType scale)
{
	assert(TValueFlag::valid_single_flag(flag_type,flag_value));

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case ELECTRON:
		case FREE_HOLE:
		case BOUND_HOLE:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE:
			assert((number>=0) && (number<grid_points));
			return((*(grid_ptr+number))->get_value(flag_type,flag_value,scale));
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			assert((number>=0) && (number<quantum_wells));
			return((*(qw_ptr+number))->get_value(flag_type,flag_value,scale));
		case CONTACT:
			assert((number>=0) && (number<number_contacts));
			return((*(contact_ptr+number))->get_value(flag_value,scale));
		case SURFACE:
			assert((number>=0) && (number<number_surfaces));
			return((*(surface_ptr+number))->get_value(flag_value,scale));
		case MIRROR:
		case MODE:
		case CAVITY:
			assert(cavity_ptr);
			return(cavity_ptr->get_value(flag_type,flag_value,number,scale));
		case DEVICE:
			switch(flag_value) {
				case ERROR_PSI: return(curr_elect_error.psi);
				case ERROR_ETA_C: return(curr_elect_error.eta_c);
				case ERROR_ETA_V: return(curr_elect_error.eta_v);
				case ERROR_TEMP: return(curr_therm_error);
				case ERROR_MODE: return(curr_mode_error);
				case ERROR_PHOTON: return(curr_optic_error);
				case INNER_ELECT_ITER: return((prec) curr_inner_elect_iter);
				case INNER_THERM_ITER: return((prec) curr_inner_therm_iter);
				case INNER_MODE_ITER: return((prec) curr_inner_mode_iter);
				case OUTER_OPTIC_ITER: return((prec) curr_outer_optic_iter);
				case OUTER_THERM_ITER: return((prec) curr_outer_therm_iter);
				case CURRENT_SOLUTION: return((prec) current_solution);
				case CURRENT_STATUS: return((prec) current_status);
				case EFFECTS: return((prec) device_effects);
				default: assert(FALSE); return(0.0);
			}
		default: assert(FALSE); return(0.0);
	}
}

void TDevice::put_value(FlagType flag_type, flag flag_value, prec value,
						int start_object, int end_object,
						ScaleType scale)
{
	TNode** temp_grid_ptr;
	TQuantumWell **temp_qw_ptr;

	assert(TValueFlag::valid_single_flag(flag_type,flag_value));

	if (flag_type!=DEVICE) {
		if (end_object==-1) {
			if (start_object==-1) {
				start_object=0;
				end_object=get_number_objects(flag_type)-1;
			}
			else end_object=start_object;
		}
		else if (start_object>end_object) swap(start_object,end_object);
	}

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case ELECTRON:
		case FREE_HOLE:
		case BOUND_HOLE:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE:
			assert((start_object>=0) && (end_object<grid_points));
			for (temp_grid_ptr=grid_ptr+start_object;
				 temp_grid_ptr<=grid_ptr+end_object;
				 temp_grid_ptr++)
				(*temp_grid_ptr)->put_value(flag_type,flag_value,value,scale);
			return;
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			assert((start_object>=0) && (start_object<quantum_wells));
			for (temp_qw_ptr=qw_ptr+start_object;
				 temp_qw_ptr<=qw_ptr+end_object;
				 temp_qw_ptr++)
				(*temp_qw_ptr)->put_value(flag_type,flag_value,value,scale);
			return;
		case CONTACT:
			assert((start_object>=0) && (start_object<number_contacts));
			(*(contact_ptr+start_object))->put_value(flag_value,value,scale);
			return;
		case SURFACE:
			assert((start_object>=0) && (start_object<number_surfaces));
			(*(surface_ptr+start_object))->put_value(flag_value,value,scale);
			return;
		case MIRROR:
		case MODE:
		case CAVITY:
			assert(cavity_ptr);
			cavity_ptr->put_value(flag_type,flag_value,start_object,value,scale);
			return;
		case DEVICE:
			switch(flag_value) {
				case EFFECTS:
					environment.set_effects_change_flags(DEVICE,device_effects^(flag)value);
					device_effects=(flag)value;
					return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

short TDevice::get_node(prec position, short start_node, short end_node, ScaleType scale)
{
	short test_node, return_node;
	prec test_position;
	prec start_position, end_position;

	if (scale==UNNORMALIZED) position/=get_normalize_value(GRID_ELECTRICAL,POSITION);

	if (start_node==-1) start_node=0;
	if (end_node==-1) end_node=(short)(grid_points-1);

	if ((end_node-start_node)==1) {
		start_position=get_value(GRID_ELECTRICAL,POSITION,start_node,NORMALIZED);
		end_position=get_value(GRID_ELECTRICAL,POSITION,end_node,NORMALIZED);
		if ((position-start_position)<(end_position-position)) return_node=start_node;
		else return_node=end_node;
	}
	else {
		test_node=(short)(start_node+(end_node-start_node)/2);
		test_position=get_value(GRID_ELECTRICAL,POSITION,test_node,NORMALIZED);

		if (test_position>position) return_node=get_node(position,start_node,test_node,NORMALIZED);
		else return_node=get_node(position,test_node,end_node,NORMALIZED);
	}
	return(return_node);
}

void TDevice::init_device(void)
{
	int i;
	SolveType previous_solution;

	modified=FALSE;
	current_status=SIMULATE;

	previous_solution=current_solution;
	current_solution=CHARGE_NEUTRAL;

	if (((previous_solution==STEADY_STATE) && (device_effects & DEVICE_NON_ISOTHERMAL)) ||
		 (previous_solution==NO_SOLUTION)) {
		comp_value(GRID_ELECTRICAL,TEMPERATURE);
		comp_value(ELECTRON,TEMPERATURE);
		comp_value(HOLE,TEMPERATURE);
		comp_value(SURFACE,TEMPERATURE);
		comp_value(SURFACE,ELECTRON_TEMPERATURE);
		comp_value(SURFACE,HOLE_TEMPERATURE);
		environment.set_update_flags(GRID_ELECTRICAL,TEMPERATURE);
		environment.set_update_flags(ELECTRON,TEMPERATURE);
		environment.set_update_flags(HOLE,TEMPERATURE);
		environment.set_update_flags(SURFACE,TEMPERATURE);
		environment.set_update_flags(SURFACE,ELECTRON_TEMPERATURE);
		environment.set_update_flags(SURFACE,HOLE_TEMPERATURE);
		environment.process_recompute_flags();
	}
	else {
		comp_value(ELECTRON,CONCENTRATION);
		comp_value(HOLE,CONCENTRATION);
		environment.set_update_flags(ELECTRON,CONCENTRATION);
		environment.set_update_flags(HOLE,CONCENTRATION);
		environment.process_recompute_flags();
	}

	for (i=0;i<number_contacts;i++) (*(contact_ptr+i))->init();
	if (cavity_ptr) cavity_ptr->init();
}

void TDevice::init_value(FlagType flag_type, flag flag_value,
						 int ref_node_number,
						 int start_object, int end_object)
{
	TNode** temp_grid_ptr;
	TNode* ref_ptr;

	assert(TValueFlag::valid_single_flag(flag_type,flag_value));

	if (end_object==-1) {
		if (start_object==-1) {
			start_object=0;
			end_object=get_number_objects(flag_type)-1;
		}
		else end_object=start_object;
	}

	switch(flag_type) {
		case FREE_ELECTRON:
		case BOUND_ELECTRON:
		case ELECTRON:
		case FREE_HOLE:
		case BOUND_HOLE:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE:
			assert((start_object>=0) && (end_object<grid_points));
			assert((ref_node_number>=0) && (ref_node_number<grid_points));
			ref_ptr=*(grid_ptr+ref_node_number);
			if (start_object<=end_object) {
				for (temp_grid_ptr=grid_ptr+start_object;
					 temp_grid_ptr<=grid_ptr+end_object;
					 temp_grid_ptr++)
					(*temp_grid_ptr)->init_value(flag_type,flag_value,ref_ptr,*contact_ptr);
				}
			else {
				for (temp_grid_ptr=grid_ptr+start_object;
					 temp_grid_ptr>=grid_ptr+end_object;
					 temp_grid_ptr--)
					(*temp_grid_ptr)->init_value(flag_type,flag_value,ref_ptr,*contact_ptr);
			}
			return;
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			assert(FALSE); return;
		case CONTACT:
			assert(FALSE); return;
		case SURFACE:
			assert(ref_node_number==0);
			assert((start_object>=0) && (start_object<number_surfaces));
			(*(surface_ptr+start_object))->init_value(flag_value);
			return;
		case MIRROR:
		case MODE:
		case CAVITY:
			assert(FALSE); return;
		case DEVICE:
			assert(FALSE); return;
		default: assert(FALSE); return;
	}
}

void TDevice::init_device_param(void)
{
	modified=FALSE;
	device_effects=0;
	curr_inner_elect_iter=curr_inner_therm_iter=curr_inner_mode_iter=curr_outer_optic_iter=curr_outer_therm_iter=0;
	curr_elect_error.psi=curr_elect_error.eta_c=curr_elect_error.eta_v=0.0;
	curr_mode_error=curr_optic_error=curr_therm_error=0.0;
	current_solution=NO_SOLUTION;
	current_status=SIMULATE;
	grid_points=0;
	grid_ptr=(TNode**)0;
	quantum_wells=0;
	qw_ptr=(TQuantumWell **)0;
	number_contacts=0;
	contact_ptr=(TContact **)0;
	surface_ptr=(TSurface **)0;
	cavity_ptr=(TCavity *)0;
	solution_ptr=(TSolution *)0;
}

void TDevice::comp_value(FlagType flag_type, flag flag_value,
						 int start_object, int end_object)
{
	TQuantumWell **temp_qw_ptr;
	TContact **temp_cont_ptr;
	TSurface **temp_surf_ptr;

	assert(TValueFlag::valid_single_flag(flag_type,flag_value));

	if (end_object==-1) {
		if (start_object==-1) {
			start_object=0;
			end_object=get_number_objects(flag_type)-1;
		}
		else end_object=start_object;
	}

	switch(flag_type) {
		case ELECTRON:
			switch(flag_value) {
				case EQUIL_DOS:
					if (quantum_wells) comp_value(QW_ELECTRON,EQUIL_DOS);
					comp_grid_value(ELECTRON,EQUIL_DOS,start_object,end_object);
					break;
				case NON_EQUIL_DOS:
					if (quantum_wells) comp_value(QW_ELECTRON,NON_EQUIL_DOS);
					comp_grid_value(ELECTRON,NON_EQUIL_DOS,start_object,end_object);
					break;
				case EQUIL_PLANCK_POT:
					if (quantum_wells) comp_value(QW_ELECTRON,EQUIL_PLANCK_POT);
					comp_grid_value(ELECTRON,EQUIL_PLANCK_POT,start_object,end_object);
					break;
				case CONCENTRATION:
					if (current_solution==CHARGE_NEUTRAL) init_value(ELECTRON,CONCENTRATION);
					else {
						if (quantum_wells) {
							comp_value(QW_ELECTRON,ENERGY_TOP);
							comp_value(QW_ELECTRON,CONCENTRATION);
						}
						comp_grid_value(ELECTRON,CONCENTRATION,start_object,end_object);
					}
					break;
				case CURRENT:
					assert((start_object==0) && (end_object==grid_points-1));
					if (current_solution==STEADY_STATE) comp_current();
					else {
						put_value(ELECTRON,CURRENT,0.0,start_object,end_object,NORMALIZED);
						put_value(HOLE,CURRENT,0.0,start_object,end_object,NORMALIZED);
					}
					break;
				case STIMULATED_FACTOR:
					assert(cavity_ptr);
					if (quantum_wells) comp_value(QW_ELECTRON,STIMULATED_FACTOR);
					comp_grid_value(ELECTRON,STIMULATED_FACTOR,start_object,end_object);
					break;
                case AUGER_COEFFICIENT:
                	if (quantum_wells) comp_value(QW_ELECTRON,AUGER_COEFFICIENT);
					comp_grid_value(ELECTRON,AUGER_COEFFICIENT,start_object,end_object);
                    break;
				case B_B_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(ELECTRON,B_B_HEAT,start_object,end_object);
					}
					else put_value(ELECTRON,B_B_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case STIM_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(ELECTRON,STIM_HEAT,start_object,end_object);
					}
					else put_value(ELECTRON,STIM_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case SHR_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(ELECTRON,SHR_HEAT,start_object,end_object);
					}
					else put_value(ELECTRON,SHR_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case RELAX_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(ELECTRON,RELAX_HEAT,start_object,end_object);
					}
					else put_value(ELECTRON,RELAX_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				default:
					comp_grid_value(ELECTRON,flag_value,start_object,end_object);
					break;
			}
			break;
		case HOLE:
			switch(flag_value) {
				case EQUIL_DOS:
					if (quantum_wells) comp_value(QW_HOLE,EQUIL_DOS);
					comp_grid_value(HOLE,EQUIL_DOS,start_object,end_object);
					break;
				case NON_EQUIL_DOS:
					if (quantum_wells) comp_value(QW_HOLE,NON_EQUIL_DOS);
					comp_grid_value(HOLE,NON_EQUIL_DOS,start_object,end_object);
					break;
				case EQUIL_PLANCK_POT:
					if (quantum_wells) comp_value(QW_HOLE,EQUIL_PLANCK_POT);
					comp_grid_value(HOLE,EQUIL_PLANCK_POT,start_object,end_object);
					break;
				case CONCENTRATION:
					if (current_solution==CHARGE_NEUTRAL) init_value(HOLE,CONCENTRATION);
					else {
						if (quantum_wells) {
							comp_value(QW_HOLE,ENERGY_TOP);
							comp_value(QW_HOLE,CONCENTRATION);
						}
						comp_grid_value(HOLE,CONCENTRATION,start_object,end_object);
					}
					break;
				case CURRENT:
					assert((start_object==0) && (end_object==grid_points-1));
					if (current_solution==STEADY_STATE) comp_current();
					else {
						put_value(ELECTRON,CURRENT,0.0,start_object,end_object,NORMALIZED);
						put_value(HOLE,CURRENT,0.0,start_object,end_object,NORMALIZED);
					}
					break;
				case STIMULATED_FACTOR:
					assert(cavity_ptr);
					if (quantum_wells) comp_value(QW_HOLE,STIMULATED_FACTOR);
					comp_grid_value(HOLE,STIMULATED_FACTOR,start_object,end_object);
					break;
                case AUGER_COEFFICIENT:
                	if (quantum_wells) comp_value(QW_HOLE,AUGER_COEFFICIENT);
					comp_grid_value(HOLE,AUGER_COEFFICIENT,start_object,end_object);
                    break;
				case B_B_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(HOLE,B_B_HEAT,start_object,end_object);
					}
					else put_value(HOLE,B_B_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case STIM_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(HOLE,STIM_HEAT,start_object,end_object);
					}
					else put_value(HOLE,STIM_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case SHR_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(HOLE,SHR_HEAT,start_object,end_object);
					}
					else put_value(HOLE,SHR_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case RELAX_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(HOLE,RELAX_HEAT,start_object,end_object);
					}
					else put_value(HOLE,RELAX_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				default:
					comp_grid_value(flag_type,flag_value,start_object,end_object);
					break;
			}
			break;

		case GRID_ELECTRICAL:
			switch(flag_value) {
				case FIELD:
					assert((start_object==0) && (end_object==grid_points-1));
					if ((current_solution==EQUILIBRIUM) || (current_solution==STEADY_STATE)) {
						comp_field();
					}
					else put_value(GRID_ELECTRICAL,FIELD,0.0,start_object,end_object,NORMALIZED);
					break;
				case BAND_GAP:
					comp_grid_value(GRID_ELECTRICAL,BAND_GAP,start_object,end_object);
					if (quantum_wells) {
						comp_value(QW_ELECTRON,WAVE_FUNCTION);
						comp_value(QW_HOLE,WAVE_FUNCTION);
						comp_value(QUANTUM_WELL,OVERLAP);
						comp_value(QW_ELECTRON,ENERGY_LEVEL);
						comp_value(QW_HOLE,ENERGY_LEVEL);
						comp_value(QUANTUM_WELL,BAND_GAP);
					}
					break;
				case B_B_RECOMB_CONSTANT:
					if (quantum_wells) comp_value(QUANTUM_WELL,B_B_RECOMB_CONSTANT);
					comp_grid_value(GRID_ELECTRICAL,B_B_RECOMB_CONSTANT,start_object,end_object);
					break;
				case POTENTIAL:
					assert(current_solution==CHARGE_NEUTRAL);
					init_value(GRID_ELECTRICAL,POTENTIAL);
					break;
				default: comp_grid_value(GRID_ELECTRICAL,flag_value,start_object,end_object);
			}
			break;

		case GRID_OPTICAL:
			switch(flag_value) {
				case INCIDENT_ABSORPTION:
				case INCIDENT_REFRACTIVE_INDEX:
					if (quantum_wells) comp_value(QUANTUM_WELL,INCIDENT_ABSORPTION);
					comp_grid_value(GRID_OPTICAL,INCIDENT_ABSORPTION,start_object,end_object);
					break;
				case MODE_ABSORPTION:
                case MODE_REFRACTIVE_INDEX:
					assert(cavity_ptr);
					if (quantum_wells) comp_value(QUANTUM_WELL,MODE_ABSORPTION);
					comp_grid_value(GRID_OPTICAL,MODE_ABSORPTION,start_object,end_object);
					break;
				case MODE_GAIN:
					assert(cavity_ptr);
					if (quantum_wells) comp_value(QUANTUM_WELL,MODE_GAIN);
					comp_grid_value(GRID_OPTICAL,MODE_GAIN,start_object,end_object);
					break;
				case MODE_PHOTON_ENERGY:
					assert(cavity_ptr);
					put_value(GRID_OPTICAL,MODE_PHOTON_ENERGY,get_value(MODE,MODE_PHOTON_ENERGY));
					break;
				case MODE_GROUP_VELOCITY:
					assert(cavity_ptr);
					put_value(GRID_OPTICAL,MODE_GROUP_VELOCITY,get_value(MODE,MODE_GROUP_VELOCITY));
					break;
				case MODE_TOTAL_FIELD_MAG:
					comp_value(MODE,MODE_TOTAL_FIELD_MAG);
					break;
				default:
					comp_grid_value(GRID_OPTICAL,flag_value,start_object,end_object);
					break;
			}
			break;

		case NODE:
			switch(flag_value) {
				case INTRINSIC_CONC:
					if (quantum_wells) comp_value(QUANTUM_WELL,INTRINSIC_CONC);
					comp_grid_value(NODE,INTRINSIC_CONC,start_object,end_object);
					break;
				case SHR_RECOMB:
					if (current_solution==STEADY_STATE) {
						if (quantum_wells) comp_value(QUANTUM_WELL,SHR_RECOMB);
						comp_grid_value(NODE,SHR_RECOMB,start_object,end_object);
					}
					else put_value(NODE,SHR_RECOMB,0.0,start_object,end_object,NORMALIZED);
					break;
				case AUGER_RECOMB:
					if (current_solution==STEADY_STATE) {
						if (quantum_wells) comp_value(QUANTUM_WELL,AUGER_RECOMB);
						comp_grid_value(NODE,AUGER_RECOMB,start_object,end_object);
					}
					else put_value(NODE,AUGER_RECOMB,0.0,start_object,end_object,NORMALIZED);
					break;
				case B_B_RECOMB:
					if (current_solution==STEADY_STATE) {
						if (quantum_wells) comp_value(QUANTUM_WELL,B_B_RECOMB);
						comp_grid_value(NODE,B_B_RECOMB,start_object,end_object);
					}
					else put_value(NODE,B_B_RECOMB,0.0,start_object,end_object,NORMALIZED);
					break;
				case B_B_HEAT:
					if (current_solution==STEADY_STATE) {
						if (quantum_wells) comp_value(QUANTUM_WELL,B_B_HEAT);
						comp_grid_value(NODE,B_B_HEAT,start_object,end_object);
					}
					else put_value(NODE,B_B_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case STIM_RECOMB:
					if (current_solution==STEADY_STATE) {
						if (quantum_wells) comp_value(QUANTUM_WELL,MODE_GAIN);
						comp_grid_value(NODE,STIM_RECOMB,start_object,end_object);
					}
					else put_value(NODE,STIM_RECOMB,0.0,start_object,end_object,NORMALIZED);
					break;
				case STIM_HEAT:
					if (current_solution==STEADY_STATE) {
						comp_grid_value(NODE,STIM_HEAT,start_object,end_object);
					}
					else put_value(NODE,STIM_HEAT,0.0,start_object,end_object,NORMALIZED);
					break;
				case OPTICAL_GENERATION:
					comp_optical_generation(start_object,end_object);
					break;
				default: comp_grid_value(NODE,flag_value,start_object,end_object);
			}
			break;

		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			assert((start_object>=0) && (start_object<quantum_wells));
			assert((end_object>=0) && (end_object<quantum_wells));

			if (start_object<=end_object) {
				for (temp_qw_ptr=qw_ptr+start_object;
					 temp_qw_ptr<=qw_ptr+end_object;
					 temp_qw_ptr++)
					 (*temp_qw_ptr)->comp_value(flag_type,flag_value);
			}
			else {
				for (temp_qw_ptr=qw_ptr+start_object;
					 temp_qw_ptr>=qw_ptr+end_object;
					 temp_qw_ptr--)
					 (*temp_qw_ptr)->comp_value(flag_type,flag_value);
			}

			break;

		case CONTACT:
			assert((start_object>=0) && (start_object<number_contacts));
			assert((end_object>=0) && (end_object<number_contacts));

			if (start_object<=end_object) {
				for (temp_cont_ptr=contact_ptr+start_object;
					 temp_cont_ptr<=contact_ptr+end_object;
					 temp_cont_ptr++)
					 (*temp_cont_ptr)->comp_value(flag_value);
			}
			else {
				for (temp_cont_ptr=contact_ptr+start_object;
					 temp_cont_ptr>=contact_ptr+end_object;
					 temp_cont_ptr--)
					 (*temp_cont_ptr)->comp_value(flag_value);
			}
			break;

		case SURFACE:
			assert((start_object>=0) && (start_object<number_surfaces));
			assert((end_object>=0) && (end_object<number_surfaces));

			if (start_object<=end_object) {
				for (temp_surf_ptr=surface_ptr+start_object;
					 temp_surf_ptr<=surface_ptr+end_object;
					 temp_surf_ptr++)
					 (*temp_surf_ptr)->comp_value(flag_value);
			}
			else {
				for (temp_surf_ptr=surface_ptr+start_object;
					 temp_surf_ptr>=surface_ptr+end_object;
					 temp_surf_ptr--)
					 (*temp_surf_ptr)->comp_value(flag_value);
			}
			break;

		case MODE:
		case MIRROR:
		case CAVITY:
			assert(cavity_ptr);
			cavity_ptr->comp_value(flag_type,flag_value,start_object,end_object);
			break;
		default: assert(FALSE); break;
	}
}

void TDevice::comp_grid_value(FlagType flag_type, flag flag_value, int start_object, int end_object)
{
	TNode **temp_grid_ptr;

	assert((start_object>=0) && (start_object<grid_points));
	assert((end_object>=0) && (end_object<grid_points));

	if (start_object<=end_object) {
		for (temp_grid_ptr=grid_ptr+start_object;
			 temp_grid_ptr<=grid_ptr+end_object;
			 temp_grid_ptr++)
			 (*temp_grid_ptr)->comp_value(flag_type,flag_value);
	}
	else {
		for (temp_grid_ptr=grid_ptr+start_object;
			 temp_grid_ptr>=grid_ptr+end_object;
			 temp_grid_ptr--)
			 (*temp_grid_ptr)->comp_value(flag_type,flag_value);
	}
}

void TDevice::comp_current(void)
{
	TNode **temp_grid_ptr;

	if (get_value(ELECTRON,CONCENTRATION,get_value(CONTACT,NODE_NUMBER,0)) <=
		get_value(ELECTRON,CONCENTRATION,get_value(CONTACT,NODE_NUMBER,1))) {

		comp_value(CONTACT,ELECTRON_CURRENT,0);

		for (temp_grid_ptr=grid_ptr;
			 temp_grid_ptr<grid_ptr+grid_points;
			 temp_grid_ptr++) {
			(*temp_grid_ptr)->comp_current(ELECTRON,(*contact_ptr)->get_value(NODE_NUMBER));
		}

		put_value(CONTACT,ELECTRON_CURRENT,get_value(ELECTRON,CURRENT,grid_points-1),1);
		comp_value(CONTACT,HOLE_CURRENT,1);

		for (temp_grid_ptr=grid_ptr+grid_points-1;
			 temp_grid_ptr>=grid_ptr;
			 temp_grid_ptr--) {
			(*temp_grid_ptr)->comp_current(HOLE,(*(contact_ptr+1))->get_value(NODE_NUMBER));
		}

		put_value(CONTACT,HOLE_CURRENT,get_value(HOLE,CURRENT,0),0);
	}
	else {
		comp_value(CONTACT,ELECTRON_CURRENT,1);

		for (temp_grid_ptr=grid_ptr+grid_points-1;
			 temp_grid_ptr>=grid_ptr;
			 temp_grid_ptr--) {
			(*temp_grid_ptr)->comp_current(ELECTRON,(*(contact_ptr+1))->get_value(NODE_NUMBER));
		}

		put_value(CONTACT,ELECTRON_CURRENT,get_value(ELECTRON,CURRENT,0),0);
		comp_value(CONTACT,HOLE_CURRENT,0);

		for (temp_grid_ptr=grid_ptr;
			 temp_grid_ptr<grid_ptr+grid_points;
			 temp_grid_ptr++) {
			(*temp_grid_ptr)->comp_current(HOLE,(*contact_ptr)->get_value(NODE_NUMBER));
		}

		put_value(CONTACT,HOLE_CURRENT,get_value(HOLE,CURRENT,grid_points-1),1);
	}
}

void TDevice::comp_field(void)
{
	TNode **temp_grid_ptr;

	comp_value(CONTACT,FIELD,0);

	for (temp_grid_ptr=grid_ptr;
		 temp_grid_ptr<grid_ptr+grid_points;
		 temp_grid_ptr++) {
		(*temp_grid_ptr)->comp_field((*contact_ptr)->get_value(NODE_NUMBER));
	}

	put_value(CONTACT,FIELD,get_value(GRID_ELECTRICAL,FIELD,grid_points-1),1);
}

void TDevice::comp_optical_generation(int start_object, int end_object)
{
	int j,number_wavelengths;
	prec intensity_multiplier;
	TNode** temp_grid_ptr;
	prec spectrum_multiplier;
    int max_overflow_count;

	spectrum_multiplier=environment.get_value(ENVIRONMENT,SPECTRUM_MULTIPLIER);

	assert((start_object>=0) && (start_object<grid_points));
	assert((end_object>=0) && (end_object<grid_points));

	put_value(GRID_OPTICAL,INCIDENT_OVERFLOW,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_REAL,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_IMAG,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_FORWARD_POYNTING,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_REAL,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_IMAG,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_REVERSE_POYNTING,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_TOTAL_POYNTING,0.0,start_object,end_object);
	put_value(GRID_OPTICAL,INCIDENT_TOTAL_FIELD_MAG,0.0,start_object,end_object);
	put_value(NODE,OPTICAL_GENERATION,0.0,start_object,end_object);
	put_value(NODE,OPTICAL_GENERATION_HEAT,0.0,start_object,end_object);
	put_value(ELECTRON,OPTICAL_GENERATION_KIN,0.0,start_object,end_object);
	put_value(HOLE,OPTICAL_GENERATION_KIN,0.0,start_object,end_object);

	start_object=get_node(environment.get_value(ENVIRONMENT,SPEC_START_POSITION));
	end_object=get_node(environment.get_value(ENVIRONMENT,SPEC_END_POSITION));

	number_wavelengths=environment.get_number_objects(SPECTRUM);
	for (j=0;j<number_wavelengths;j++) {
		put_value(GRID_OPTICAL,INCIDENT_PHOTON_ENERGY,environment.get_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,j),
				  start_object, end_object);
        put_value(GRID_OPTICAL,INCIDENT_OVERFLOW,0.0,start_object,end_object);
		comp_value(GRID_OPTICAL,INCIDENT_ABSORPTION,start_object,end_object);
		comp_value(GRID_OPTICAL,INCIDENT_IMPEDANCE_REAL,start_object,end_object);

		if (start_object<end_object) {
			init_value(SURFACE,INCIDENT_FORWARD_FIELD_REAL,0,1);
			comp_value(SURFACE,INCIDENT_INTERNAL_FIELD,1);
			for (temp_grid_ptr=grid_ptr+end_object;
				 temp_grid_ptr>=grid_ptr+start_object;
				 temp_grid_ptr--) {
				(*temp_grid_ptr)->comp_incident_optical_field(end_object);
			}
			comp_value(SURFACE,INCIDENT_SURFACE_FIELD,0);

			intensity_multiplier=(*surface_ptr)->comp_input_total_poynting(j);
            max_overflow_count=(int)get_value(GRID_OPTICAL,INCIDENT_OVERFLOW,start_object);
			for (temp_grid_ptr=grid_ptr+start_object;
				 temp_grid_ptr<=grid_ptr+end_object;
				 temp_grid_ptr++) {
				(*temp_grid_ptr)->comp_incident_total_poynting(intensity_multiplier*spectrum_multiplier,max_overflow_count);
				(*temp_grid_ptr)->comp_value(NODE,OPTICAL_GENERATION);
			}
			(*(surface_ptr+1))->comp_emitted_total_poynting(intensity_multiplier);
			environment.put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,get_value(SURFACE,INCIDENT_TOTAL_POYNTING,1),j);
			environment.put_value(SPECTRUM,INCIDENT_REFLECT_INTENSITY,get_value(SURFACE,INCIDENT_REVERSE_POYNTING,0),j);
		}
		else {
			init_value(SURFACE,INCIDENT_REVERSE_FIELD_REAL,0,0);
			comp_value(SURFACE,INCIDENT_INTERNAL_FIELD,0);
			for (temp_grid_ptr=grid_ptr+end_object;
				 temp_grid_ptr<=grid_ptr+start_object;
				 temp_grid_ptr++) {
				(*temp_grid_ptr)->comp_incident_optical_field(end_object);
			}
			comp_value(SURFACE,INCIDENT_SURFACE_FIELD,1);

			intensity_multiplier=(*(surface_ptr+1))->comp_input_total_poynting(j);
            max_overflow_count=(int)get_value(GRID_OPTICAL,INCIDENT_OVERFLOW,start_object);
			for (temp_grid_ptr=grid_ptr+start_object;
				 temp_grid_ptr>=grid_ptr+end_object;
				 temp_grid_ptr--) {
				(*temp_grid_ptr)->comp_incident_total_poynting(intensity_multiplier*spectrum_multiplier,max_overflow_count);
				(*temp_grid_ptr)->comp_value(NODE,OPTICAL_GENERATION);
			}
			(*surface_ptr)->comp_emitted_total_poynting(intensity_multiplier);
			environment.put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,-get_value(SURFACE,INCIDENT_TOTAL_POYNTING,0),j);
			environment.put_value(SPECTRUM,INCIDENT_REFLECT_INTENSITY,get_value(SURFACE,INCIDENT_FORWARD_POYNTING,1),j);
		}
	}
}

void TDevice::read_data_file(const char *filename)
{
	int i,node,columns=0;
	FILE *file_ptr;
	char header_string[200];
	FlagType flag_type_array[30];
	flag flag_array[30];
	char *token;
	float position, value[30];

	file_ptr=fopen(filename,"r");
	if (!file_ptr) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}
	fgets(header_string,200,file_ptr);
	header_string[strlen(header_string)-1]=0;

	token=strtok(header_string,",");

	while(token) {
		short_string_to_flag(token,flag_type_array[columns],flag_array[columns]);
		if (columns>0) environment.set_update_flags(flag_type_array[columns],flag_array[columns]);
		columns++;

		token=strtok(NULL,",");
	}

	while (!feof(file_ptr)) {
		fscanf(file_ptr,"%f",&position);
		node=get_node(position);

		for (i=1;i<columns;i++) fscanf(file_ptr,",%f",&value[i]);

		if (!feof(file_ptr)) {
			for (i=1;i<columns;i++) put_value(flag_type_array[i],flag_array[i],value[i],node);
		}
	}

	fclose(file_ptr);
}

void TDevice::write_data_file(ofstream& output_file, TValueFlag write_flags,
							  FlagType ref_flag_type, flag ref_flag_value)
{
	int i,j;
	int flag_type;
	int max_bit;
	int write_grid_multiplier;
	flag test_flag, valid_flag;
	prec reference_value;
	logical position_dep=FALSE, contact_dep=FALSE, qw_dep=FALSE;
	logical mode_dep=FALSE, cavity_dep=FALSE, mirror_dep=FALSE;
	logical surface_dep=FALSE, device_dep=FALSE;

	switch(ref_flag_type) {
		case ELECTRON:
			if (ref_flag_value==BAND_EDGE)
				reference_value=get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY);
			else
				reference_value=get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)+
								get_value(ELECTRON,QUASI_FERMI);
			break;
		case HOLE:
			if (ref_flag_value==BAND_EDGE)
				reference_value=get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)+
								get_value(GRID_ELECTRICAL,BAND_GAP);
			else
				reference_value=get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)+
								get_value(GRID_ELECTRICAL,BAND_GAP)-
								get_value(HOLE,QUASI_FERMI);
			break;
		default:
			reference_value=0.0;
			break;
	}

	reference_value+=get_value(GRID_ELECTRICAL,POTENTIAL);

	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (write_flags.any_set((FlagType)flag_type)) {
			switch(flag_type) {
				case FREE_ELECTRON:
				case BOUND_ELECTRON:
				case FREE_HOLE:
				case BOUND_HOLE:
				case ELECTRON:
				case HOLE:
				case GRID_ELECTRICAL:
				case GRID_OPTICAL:
				case NODE:
					position_dep=TRUE;
					break;
				case QW_ELECTRON:
				case QW_HOLE:
				case QUANTUM_WELL:
					qw_dep=TRUE;
					break;
				case CONTACT:
					contact_dep=TRUE;
					break;
				case SURFACE:
					surface_dep=TRUE;
					break;
				case MODE:
					mode_dep=TRUE;
					break;
				case CAVITY:
					cavity_dep=TRUE;
					break;
				case MIRROR:
					mirror_dep=TRUE;
					break;
				case DEVICE:
					device_dep=TRUE;
					break;
			}
		}
	}

	output_file.setf(ios::scientific,ios::floatfield);
	output_file.precision(4);

	if (device_dep) {
		test_flag=1;
		max_bit=bit_position(write_flags.get_max(DEVICE));
		valid_flag=write_flags.get_valid(DEVICE);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(DEVICE,test_flag))
				output_file << get_short_string(DEVICE,test_flag) << ',';
			test_flag<<=1;
		}

		output_file << '\n';

		test_flag=1;
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(DEVICE,test_flag)) {
				switch(test_flag) {
					case CURRENT_SOLUTION:
						output_file << get_solve_string((SolveType)get_value(DEVICE,CURRENT_SOLUTION)) << ',';
						break;
					case CURRENT_STATUS:
						output_file << get_status_string((StatusType)get_value(DEVICE,CURRENT_STATUS)) << ',';
						break;
					default:
						output_file << get_value(DEVICE,test_flag) << ',';
						break;
				}
			}
			test_flag<<=1;
		}
		output_file << '\n';
	}

	if (surface_dep) {
		write_flags.clear(SURFACE,POSITION);
		output_file << "Surface Number," << get_short_string(SURFACE,POSITION);

		test_flag=1;
		max_bit=bit_position(write_flags.get_max(SURFACE));
		valid_flag=write_flags.get_valid(SURFACE);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(SURFACE,test_flag))
				output_file << ',' << get_short_string(SURFACE,test_flag);
			test_flag<<=1;
		}

		output_file << '\n';

		for (i=0;i<number_surfaces;i++) {
			output_file << (i+1) << ',' << get_value(SURFACE,POSITION,i);

			test_flag=1;
			for (j=0;j<=max_bit;j++) {
				if ((valid_flag & test_flag) && write_flags.is_set(SURFACE,test_flag)) output_file << ',' <<
																					   get_value(SURFACE,test_flag,i);
				test_flag<<=1;
			}
			output_file << '\n';
		}
	}

	if (contact_dep) {
		write_flags.clear(CONTACT,POSITION);
		output_file << "Contact Number," << get_short_string(CONTACT,POSITION);

		test_flag=1;
		max_bit=bit_position(write_flags.get_max(CONTACT));
		valid_flag=write_flags.get_valid(CONTACT);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(CONTACT,test_flag))
				output_file << ',' << get_short_string(CONTACT,test_flag);
			test_flag<<=1;
		}

		output_file << '\n';

		for (i=0;i<number_contacts;i++) {
			output_file << (i+1) << ',' << get_value(CONTACT,POSITION,i);

			test_flag=1;
			for (j=0;j<=max_bit;j++) {
				if ((valid_flag & test_flag) && write_flags.is_set(CONTACT,test_flag))
					output_file << ',' << get_value(CONTACT,test_flag,i);
				test_flag<<=1;
			}
			output_file << '\n';
		}
	}

	if (cavity_dep) {
		test_flag=1;
		max_bit=bit_position(write_flags.get_max(CAVITY));
		valid_flag=write_flags.get_valid(CAVITY);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(CAVITY,test_flag)) output_file <<
																				  get_short_string(CAVITY,test_flag) << ',';
			test_flag<<=1;
		}
		output_file << '\n';

		test_flag=1;
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(CAVITY,test_flag)) {
				switch(test_flag) {
					case TYPE:
						output_file << get_cavity_string((CavityType)get_value(CAVITY,TYPE)) << ',';
						break;
					default:
						output_file << get_value(CAVITY,test_flag) << ',';
						break;
				}
			}
			test_flag<<=1;
		}
		output_file << '\n';
	}

	if (mirror_dep) {
		output_file << "Mirror Number";

		test_flag=1;
		max_bit=bit_position(write_flags.get_max(MIRROR));
		valid_flag=write_flags.get_valid(MIRROR);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(MIRROR,test_flag))
				output_file << ',' << get_short_string(MIRROR,test_flag);
			test_flag<<=1;
		}

		output_file << '\n';

		for (i=0;i<=1;i++) {
			output_file << (i+1);

			test_flag=1;
			for (j=0;j<=max_bit;j++) {
				if ((valid_flag & test_flag) && write_flags.is_set(MIRROR,test_flag)) {
					switch(test_flag) {
						case TYPE:
							output_file << ',' << get_mirror_string((MirrorType)get_value(MIRROR,TYPE,i));
							break;
						default:
							output_file << ',' << get_value(MIRROR,test_flag,i);
							break;
					}
				}
				test_flag<<=1;
			}
			output_file << '\n';
		}
	}

	if (mode_dep) {
		test_flag=1;
		max_bit=bit_position(write_flags.get_max(MODE));
		valid_flag=write_flags.get_valid(MODE);
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(MODE,test_flag))
				output_file << get_short_string(MODE,test_flag) << ',';
			test_flag<<=1;
		}

		output_file << '\n';

		test_flag=1;
		for (j=0;j<=max_bit;j++) {
			if ((valid_flag & test_flag) && write_flags.is_set(MODE,test_flag))
				output_file << get_value(MODE,test_flag) << ',';
			test_flag<<=1;
		}
		output_file << '\n';
	}

	if (qw_dep) {
		write_flags.clear(QUANTUM_WELL,POSITION);
		output_file << "QW Number," << get_short_string(QUANTUM_WELL,POSITION);

		for (flag_type=QW_ELECTRON;flag_type<=QUANTUM_WELL;flag_type++) {
			if (write_flags.any_set((FlagType)flag_type)) {
				test_flag=1;
				max_bit=bit_position(write_flags.get_max((FlagType)flag_type));
				valid_flag=write_flags.get_valid((FlagType)flag_type);
				for (j=0;j<=max_bit;j++) {
					if ((valid_flag & test_flag) && write_flags.is_set((FlagType)flag_type,test_flag))
						output_file << ',' << get_short_string((FlagType)flag_type,test_flag);
					test_flag<<=1;
				}
			}
		}

		output_file << '\n';

		for (i=0;i<quantum_wells;i++) {
			output_file << (i+1) << ',' << get_value(QUANTUM_WELL,POSITION,i);

			for (flag_type=(int)QW_ELECTRON;flag_type<=(int)QUANTUM_WELL;flag_type++) {
				if (write_flags.any_set((FlagType)flag_type)) {
					test_flag=1;
					max_bit=bit_position(write_flags.get_max((FlagType)flag_type));
					valid_flag=write_flags.get_valid((FlagType)flag_type);
					for (j=0;j<=max_bit;j++) {
						if ((valid_flag & test_flag) && write_flags.is_set((FlagType)flag_type,test_flag))
							output_file << ',' << get_value((FlagType)flag_type,test_flag,i);
						test_flag<<=1;
					}
				}
			}
			output_file << '\n';
		}
	}

	if (position_dep) {
		write_grid_multiplier=preferences.get_write_grid_multiplier();
		write_flags.clear(GRID_ELECTRICAL,POSITION);
		output_file << get_short_string(GRID_ELECTRICAL,POSITION);

		for (flag_type=FREE_ELECTRON;flag_type<=NODE;flag_type++) {
			if (write_flags.any_set((FlagType)flag_type)) {
				test_flag=1;
				max_bit=bit_position(write_flags.get_max((FlagType)flag_type));
				valid_flag=write_flags.get_valid((FlagType)flag_type);
				for (j=0;j<=max_bit;j++) {
					if ((valid_flag & test_flag) && write_flags.is_set((FlagType)flag_type,test_flag))
						output_file << ',' << get_short_string((FlagType)flag_type,test_flag);
					test_flag<<=1;
				}
			}
		}

		output_file << '\n';

		for (i=0;i<grid_points;i+=write_grid_multiplier) {
			output_file << get_value(GRID_ELECTRICAL,POSITION,i);

			for (flag_type=(int)FREE_ELECTRON;flag_type<=(int)NODE;flag_type++) {
				if (write_flags.any_set((FlagType)flag_type)) {
					test_flag=1;
					max_bit=bit_position(write_flags.get_max((FlagType)flag_type));
					valid_flag=write_flags.get_valid((FlagType)flag_type);
					for (j=0;j<=max_bit;j++) {
						if ((valid_flag & test_flag) && write_flags.is_set((FlagType)flag_type,test_flag)) {
							switch(flag_type) {
								case GRID_ELECTRICAL:
									if (test_flag==VACUUM_LEVEL) {
										output_file << ',' << (-get_value(GRID_ELECTRICAL,POTENTIAL,i)+reference_value);
										test_flag<<=1;
										continue;
									}

									if (test_flag==ALLOY_TYPE) {
										output_file << ',' << material_parameters.get_alloy_name(
																		(MaterialType)get_value(GRID_ELECTRICAL,MATERIAL,i),
																		(AlloyType)get_value(GRID_ELECTRICAL,ALLOY_TYPE,i));
										test_flag<<=1;
										continue;
									}

									if (test_flag==REGION_TYPE) {
										output_file << ',' << get_region_string(
																		(RegionType)get_value(GRID_ELECTRICAL,REGION_TYPE,i));
										test_flag<<=1;
										continue;
									}

									if (test_flag==MATERIAL) {
										output_file << ',' << material_parameters.get_material_name(
																		(MaterialType)get_value(GRID_ELECTRICAL,MATERIAL,i));
										test_flag<<=1;
										continue;
									}

									break;

								case ELECTRON:
									if (test_flag==BAND_EDGE) {
										output_file << ',' << (get_value(ELECTRON,BAND_EDGE,i)+reference_value);
										test_flag<<=1;
										continue;
									}

									if (test_flag==QUASI_FERMI) {
										output_file << ',' << (get_value(ELECTRON,BAND_EDGE,i)-
															   get_value(ELECTRON,QUASI_FERMI,i)+reference_value);
										test_flag<<=1;
										continue;
									}
									break;

								case HOLE:
									if (test_flag==BAND_EDGE) {
										output_file << ',' << (get_value(HOLE,BAND_EDGE,i)+reference_value);
										test_flag<<=1;
										continue;
									}

									if (test_flag==QUASI_FERMI) {
										output_file << ',' << (get_value(HOLE,BAND_EDGE,i)+
															   get_value(HOLE,QUASI_FERMI,i)+reference_value);
										test_flag<<=1;
										continue;
									}
									break;
							}
							output_file << ',' << get_value((FlagType)flag_type,test_flag,i);
						}
						test_flag<<=1;
					}
				}
			}
			output_file << '\n';
		}
	}
}

void TDevice::read_state_file(FILE *file_ptr)
{
	modified=FALSE;

	device_input.read_state_file(file_ptr);
	fread(&device_effects,sizeof(device_effects),1,file_ptr);

	fread(&current_status,sizeof(current_status),1,file_ptr);
	fread(&current_solution,sizeof(current_solution),1,file_ptr);

	fread(&curr_inner_elect_iter,sizeof(curr_inner_elect_iter),1,file_ptr);
	fread(&curr_inner_therm_iter,sizeof(curr_inner_therm_iter),1,file_ptr);
	fread(&curr_inner_mode_iter,sizeof(curr_inner_mode_iter),1,file_ptr);
	fread(&curr_outer_optic_iter,sizeof(curr_outer_optic_iter),1,file_ptr);
	fread(&curr_outer_therm_iter,sizeof(curr_outer_therm_iter),1,file_ptr);

	fread(&curr_elect_error,sizeof(curr_elect_error),1,file_ptr);
	fread(&curr_mode_error,sizeof(curr_mode_error),1,file_ptr);
	fread(&curr_optic_error,sizeof(curr_optic_error),1,file_ptr);
	fread(&curr_therm_error,sizeof(curr_therm_error),1,file_ptr);
}

void TDevice::write_state_file(FILE *file_ptr)
{
	int i;

	modified=FALSE;

	device_input.write_state_file(file_ptr);
	fwrite(&device_effects,sizeof(device_effects),1,file_ptr);

	fwrite(&current_status,sizeof(current_status),1,file_ptr);
	fwrite(&current_solution,sizeof(current_solution),1,file_ptr);

	fwrite(&curr_inner_elect_iter,sizeof(curr_inner_elect_iter),1,file_ptr);
	fwrite(&curr_inner_therm_iter,sizeof(curr_inner_therm_iter),1,file_ptr);
	fwrite(&curr_inner_mode_iter,sizeof(curr_inner_mode_iter),1,file_ptr);
	fwrite(&curr_outer_optic_iter,sizeof(curr_outer_optic_iter),1,file_ptr);
	fwrite(&curr_outer_therm_iter,sizeof(curr_outer_therm_iter),1,file_ptr);

	fwrite(&curr_elect_error,sizeof(curr_elect_error),1,file_ptr);
	fwrite(&curr_mode_error,sizeof(curr_mode_error),1,file_ptr);
	fwrite(&curr_optic_error,sizeof(curr_optic_error),1,file_ptr);
	fwrite(&curr_therm_error,sizeof(curr_therm_error),1,file_ptr);

	for (i=0;i<grid_points;i++)
		(*(grid_ptr+i))->write_state_file(file_ptr);

	for (i=0;i<quantum_wells;i++)
		(*(qw_ptr+i))->write_state_file(file_ptr);

	for (i=0;i<number_contacts;i++)
		(*(contact_ptr+i))->write_state_file(file_ptr);

	for (i=0;i<number_surfaces;i++)
		(*(surface_ptr+i))->write_state_file(file_ptr);

	if (cavity_ptr) cavity_ptr->write_state_file(file_ptr);
}

void TDevice::solve(void)
{
	prec max_elect_error, max_optic_error, max_therm_error;
	prec coarse_mode_error, fine_mode_error;
	prec curr_max_elect_error;
	SolveType solve_type, prev_solve_type;
	prec temp_bias_0,temp_bias_1;
	prec temp_temp_0,temp_temp_1;
	prec temp_electron_temp_0, temp_electron_temp_1;
	prec env_temp;
	flag temp_dev_effects, temp_env_effects, grid_effects, mode_effects;
	int max_inner_elect_iter, max_inner_therm_iter, max_inner_mode_iter;
	int max_outer_optic_iter, max_outer_therm_iter;

	temp_bias_0=get_value(CONTACT,APPLIED_BIAS,0);
	temp_bias_1=get_value(CONTACT,APPLIED_BIAS,1);
	temp_temp_0=get_value(SURFACE,TEMPERATURE,0);
	temp_temp_1=get_value(SURFACE,TEMPERATURE,1);
	temp_electron_temp_0=get_value(SURFACE,ELECTRON_TEMPERATURE,0);
	temp_electron_temp_1=get_value(SURFACE,ELECTRON_TEMPERATURE,1);
	env_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0);
	temp_env_effects=environment.get_value(ENVIRONMENT,EFFECTS);
	temp_dev_effects=device_effects;
	grid_effects=environment.get_value(GRID_ELECTRICAL,EFFECTS,0);
	if (device_effects & DEVICE_LASER) mode_effects=(flag)get_value(MODE,EFFECTS);
	max_inner_elect_iter=environment.get_value(ENVIRONMENT,MAX_INNER_ELECT_ITER);
	max_inner_therm_iter=environment.get_value(ENVIRONMENT,MAX_INNER_THERM_ITER);
	max_inner_mode_iter=environment.get_value(ENVIRONMENT,MAX_INNER_MODE_ITER);
	max_outer_optic_iter=environment.get_value(ENVIRONMENT,MAX_OUTER_OPTIC_ITER);
	max_outer_therm_iter=environment.get_value(ENVIRONMENT,MAX_OUTER_THERM_ITER);
	max_elect_error=environment.get_value(ENVIRONMENT,MAX_ELECTRICAL_ERROR);
	max_therm_error=environment.get_value(ENVIRONMENT,MAX_THERMAL_ERROR);
	max_optic_error=environment.get_value(ENVIRONMENT,MAX_OPTIC_ERROR);
	coarse_mode_error=environment.get_value(ENVIRONMENT,COARSE_MODE_ERROR);
	fine_mode_error=environment.get_value(ENVIRONMENT,FINE_MODE_ERROR);

	prev_solve_type=current_solution;

	if (((temp_bias_0-temp_bias_1)!=0) ||
		(temp_temp_0!=env_temp) ||
		(temp_temp_1!=env_temp) ||
		(temp_electron_temp_0!=env_temp) ||
		(temp_electron_temp_1!=env_temp) ||
		(temp_env_effects & ENV_OPTICAL_GEN)) {
		if (current_solution==CHARGE_NEUTRAL) {
			environment.put_value(ENVIRONMENT,EFFECTS, temp_env_effects&(~ENV_OPTICAL_GEN));
			environment.process_recompute_flags();
			put_value(CONTACT,APPLIED_BIAS,0.0,0);
			put_value(CONTACT,APPLIED_BIAS,0.0,1);
			put_value(SURFACE,TEMPERATURE,env_temp,0);
			put_value(SURFACE,TEMPERATURE,env_temp,1);
			put_value(SURFACE,ELECTRON_TEMPERATURE,env_temp,0);
			put_value(SURFACE,ELECTRON_TEMPERATURE,env_temp,1);
			solve();
			put_value(CONTACT,APPLIED_BIAS,temp_bias_0,0);
			put_value(CONTACT,APPLIED_BIAS,temp_bias_1,1);
			put_value(SURFACE,TEMPERATURE,temp_temp_0,0);
			put_value(SURFACE,TEMPERATURE,temp_temp_1,1);
			put_value(SURFACE,ELECTRON_TEMPERATURE,temp_electron_temp_0,0);
			put_value(SURFACE,ELECTRON_TEMPERATURE,temp_electron_temp_1,1);
			environment.put_value(ENVIRONMENT,EFFECTS,temp_env_effects);
			environment.process_recompute_flags();
            if (environment.do_stop_solution()) return;
		}
		solve_type=STEADY_STATE;
		if ((temp_temp_0!=env_temp) || (temp_temp_1!=env_temp)) {
			temp_dev_effects |= DEVICE_NON_ISOTHERMAL | DEVICE_SINGLE_TEMP;
			temp_dev_effects &= (~DEVICE_VARY_ELECTRON_TEMP);
			put_value(DEVICE,EFFECTS,temp_dev_effects);
            put_value(SURFACE,ELECTRON_TEMPERATURE,temp_temp_0,0);
            put_value(SURFACE,ELECTRON_TEMPERATURE,temp_temp_1,1);
			environment.process_recompute_flags();
		}
		else {
			if ((temp_electron_temp_0!=env_temp) || (temp_electron_temp_1!=env_temp)) {
				temp_dev_effects |= DEVICE_NON_ISOTHERMAL | DEVICE_VARY_ELECTRON_TEMP;
				temp_dev_effects &= (~DEVICE_SINGLE_TEMP);
				put_value(DEVICE,EFFECTS,temp_dev_effects);
				environment.process_recompute_flags();
			}
		}
	}
	else {
		solve_type=EQUILIBRIUM;
		if (prev_solve_type==STEADY_STATE) init_device();
	}

	solution_ptr->set_solution(solve_type);
	if (error_handler.fail()) return;

	modified=TRUE;
	current_solution=solve_type;
	solution_ptr->comp_thermal_boundary();
	solution_ptr->apply_thermal_boundary();

	curr_outer_therm_iter=0;
	do {
		solution_ptr->store_temperature();
		solution_ptr->comp_electrical_boundary();
		solution_ptr->apply_electrical_boundary();
		curr_outer_optic_iter=0;
		do {
			curr_inner_elect_iter=0;
			do {
				assert(!error_handler.fail());
				solution_ptr->electrical_iterate(curr_elect_error);
				curr_inner_elect_iter++;
				if (error_handler.fail()) return;

				out_elect_convergence(curr_inner_elect_iter,curr_elect_error);

				if (curr_elect_error.psi>curr_elect_error.eta_c) curr_max_elect_error=curr_elect_error.psi;
				else curr_max_elect_error=curr_elect_error.eta_c;
				if (curr_elect_error.eta_v>curr_max_elect_error) curr_max_elect_error=curr_elect_error.eta_v;
			}
			while ((curr_max_elect_error>=max_elect_error) &&
            	   (curr_inner_elect_iter <= max_inner_elect_iter-1) &&
                   (!environment.do_stop_solution()));

			switch (solve_type) {
				case EQUILIBRIUM:
					comp_value(ELECTRON,BAND_EDGE);
					comp_value(HOLE,BAND_EDGE);
					comp_value(ELECTRON,OPTICAL_GENERATION_REF);
					comp_value(HOLE,OPTICAL_GENERATION_REF);
					comp_value(ELECTRON,TOTAL_HEAT);
					if (device_effects & DEVICE_LASER) {
						comp_value(GRID_OPTICAL,MODE_GAIN);
						comp_value(MODE,MODE_GAIN);
					}
					break;
				case STEADY_STATE:
					comp_value(NODE,TOTAL_RADIATIVE_HEAT);
					comp_value(ELECTRON,SHR_HEAT);
					comp_value(ELECTRON,B_B_HEAT);
					comp_value(ELECTRON,STIM_HEAT);
                    comp_value(ELECTRON,AUGER_HEAT);
					comp_value(ELECTRON,RELAX_HEAT);
					comp_value(ELECTRON,OPTICAL_GENERATION_REF);
					comp_value(HOLE,OPTICAL_GENERATION_REF);
					comp_value(ELECTRON,TOTAL_HEAT);
					if (device_effects & DEVICE_LASER) {
						comp_value(GRID_OPTICAL,MODE_GAIN);
						comp_value(MODE,MODE_GAIN);
						comp_value(MODE,TOTAL_SPONTANEOUS);
					}
					break;
			}
			if (error_handler.fail()) return;

			if ((device_effects & DEVICE_LASER) && (solve_type!=EQUILIBRIUM)) {
				if (mode_effects & MODE_SEARCH_WAVELENGTH) {
					curr_inner_mode_iter=0;
                    if (!environment.do_stop_solution()) {
						do {
							assert(!error_handler.fail());
							cavity_ptr->field_iterate(curr_mode_error,coarse_mode_error,curr_inner_mode_iter);
							curr_inner_mode_iter++;
							if (error_handler.fail()) return;
							out_coarse_mode_convergence(curr_inner_mode_iter,curr_mode_error);
						}
						while ((curr_mode_error!=0.0) && (curr_inner_mode_iter<=max_inner_mode_iter) &&
        	                   (!environment.do_stop_solution()));
                    }
					curr_inner_mode_iter=0;
                    if (!environment.do_stop_solution()) {
						do {
							assert(!error_handler.fail());
							cavity_ptr->field_iterate(curr_mode_error,fine_mode_error,curr_inner_mode_iter);
							curr_inner_mode_iter++;
							if (error_handler.fail()) return;
							out_fine_mode_convergence(curr_inner_mode_iter,curr_mode_error);
						}
						while ((curr_mode_error!=0.0) && (curr_inner_mode_iter<=max_inner_mode_iter) &&
                        	   (!environment.do_stop_solution()));
                    }
				}

                if (!environment.do_stop_solution()) {
					assert(!error_handler.fail());
					cavity_ptr->photon_iterate(curr_optic_error);
					curr_outer_optic_iter++;
					if (error_handler.fail()) return;
					out_optic_convergence(curr_outer_optic_iter,curr_optic_error);
                }
			}
			else {
				curr_optic_error=0.0;
				curr_outer_optic_iter=2;
			}
		}
		while ((!environment.do_stop_solution()) &&
        	   (device_effects & DEVICE_LASER) && (solve_type!=EQUILIBRIUM) &&
			   (((curr_optic_error>=max_optic_error) || (curr_outer_optic_iter <2) ||
				 (curr_max_elect_error>=max_elect_error)) &&
				(curr_outer_optic_iter <= max_outer_optic_iter-1)));

		if ((device_effects & DEVICE_NON_ISOTHERMAL) && (solve_type!=EQUILIBRIUM) &&
        	(!environment.do_stop_solution())) {
			solution_ptr->comp_thermoelectric_param();
			curr_inner_therm_iter=0;
			do {
				assert(!error_handler.fail());
				solution_ptr->thermal_iterate(curr_therm_error);
				curr_inner_therm_iter++;
				if (error_handler.fail()) return;

				if (grid_effects & GRID_TEMP_THERMAL_COND) comp_value(GRID_ELECTRICAL,THERMAL_CONDUCT);

				out_therm_convergence(curr_inner_therm_iter,curr_therm_error);
			}
			while ((curr_therm_error>max_therm_error) && (curr_inner_therm_iter<=max_inner_therm_iter-1) &&
                   (!environment.do_stop_solution()));
			solution_ptr->outer_thermal_update_device();

			if (device_effects & (DEVICE_SINGLE_TEMP | DEVICE_VARY_LATTICE_TEMP))
				environment.set_update_flags(GRID_ELECTRICAL,TEMPERATURE);
			if (device_effects & (DEVICE_SINGLE_TEMP | DEVICE_VARY_ELECTRON_TEMP))
				environment.set_update_flags(ELECTRON,TEMPERATURE);
			if (device_effects & (DEVICE_SINGLE_TEMP | DEVICE_VARY_HOLE_TEMP))
				environment.set_update_flags(HOLE,TEMPERATURE);
			environment.process_recompute_flags();
			if (error_handler.fail()) return;
		}
		else curr_therm_error=0.0;

		curr_outer_therm_iter++;
	}
	while ((!environment.do_stop_solution()) &&
           (device_effects & DEVICE_NON_ISOTHERMAL) && (solve_type!=EQUILIBRIUM) &&
		   ((curr_inner_therm_iter>1) || (curr_inner_elect_iter>1) || (curr_outer_optic_iter>2)) &&
		   (curr_outer_therm_iter<=max_outer_therm_iter-1));

	if ((curr_max_elect_error<max_elect_error) &&
		(curr_optic_error<max_optic_error) &&
		(curr_therm_error<max_therm_error)) current_status=CONVERGED;
	else current_status=NOT_CONVERGED;

	comp_value(ELECTRON,CURRENT);
	comp_value(GRID_ELECTRICAL,FIELD);

	if (device_effects & DEVICE_LASER) {
		comp_value(MODE,MODE_GAIN);
		if (solve_type==STEADY_STATE) comp_value(MIRROR,POWER);
	}
}

void TDevice::establish_grid(void)
{
	int i,j, qw_count;
	int required_nodes;
	prec total_length, point_size, grid_length, grid_position;
	RegionType region_type, previous_region_type;
	TNode** temp_ptr;

	required_nodes=device_input.total_points+1;

// Increment number of grid points to include the last grid point and
// establish grid_ptr array.
	grid_ptr = new TNode*[required_nodes];
	if (!grid_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_GRID,0,"","");
		return;
	}

// Establish qw_ptr and create qw objects.
	if (device_input.number_qw) {
		qw_ptr= new TQuantumWell*[device_input.number_qw];
		if (!qw_ptr) {
			error_handler.set_error(ERROR_MEM_QW,0,"","");
			return;
		}
		for (i=0;i<device_input.number_qw;i++) {
			*(qw_ptr+quantum_wells)=new TQuantumWell(this,grid_ptr);
			if (!(*(qw_ptr+quantum_wells)))	{
				error_handler.set_error(ERROR_MEM_QW,0,"","");
				return;
			}
			quantum_wells++;
		}
	}

// Create either Bulk or QW nodes.
	temp_ptr=grid_ptr;
	qw_count=total_length=0;
	previous_region_type=BULK;
	for (i=0;i<device_input.number_grid;i++) {
		grid_length=(device_input.grid_ptr+i)->length;
		point_size=(device_input.grid_ptr+i)->length/
		   (double) (device_input.grid_ptr+i)->number_points;
		grid_position=total_length;

		for (j=0;j<((device_input.grid_ptr+i)->number_points);j++) {
			region_type=device_input.get_region_type(grid_position);
			switch(region_type) {
				case BULK:
					(*temp_ptr)=new TNode(grid_points,BULK);
					if (previous_region_type==QW) {
						(*(qw_ptr+qw_count))->put_node(NEXT_NODE,grid_points);
						qw_count++;
					}
					previous_region_type=BULK;
					break;
				case QW:
					(*temp_ptr)=new TNode(grid_points,QW,*(qw_ptr+qw_count));
					if (previous_region_type==BULK)
						(*(qw_ptr+qw_count))->put_node(PREVIOUS_NODE,grid_points-1);
					previous_region_type=QW;
					break;
			}
			if (!(*temp_ptr)) {
				error_handler.set_error(ERROR_MEM_DEVICE_GRID,0,"","");
				return;
			}
			else {
				grid_position+=point_size;
				temp_ptr++;
				grid_points++;
			}
		}
		total_length+=grid_length;
	}
	(*temp_ptr)=new TNode(grid_points,BULK);
	if (!(*temp_ptr)) {
		error_handler.set_error(ERROR_MEM_DEVICE_GRID,0,"","");
		return;
	}
	else grid_points++;

// Place Contacts at either end of the device.
	contact_ptr= new TContact*[number_contacts];
	if (!contact_ptr) {
		error_handler.set_error(ERROR_MEM_CONTACT,0,"","");
		return;
	}

	*contact_ptr= new TContact(*(grid_ptr),*(grid_ptr+1));
	*(contact_ptr+1)= new TContact(*(grid_ptr+grid_points-1),*(grid_ptr+grid_points-2));

	if ((!(*(contact_ptr))) || (!(*(contact_ptr+1)))) {
		error_handler.set_error(ERROR_MEM_CONTACT,0,"","");
		return;
	}

// Place Surfaces at either end of the device.
	surface_ptr= new TSurface*[number_surfaces];
	if (!surface_ptr) {
		error_handler.set_error(ERROR_MEM_SURFACE,0,"","");
		return;
	}

	*surface_ptr= new TSurface(*(grid_ptr),*(grid_ptr+1));
	*(surface_ptr+1)= new TSurface(*(grid_ptr+grid_points-1),*(grid_ptr+grid_points-2));

	if ((!(*(surface_ptr))) || (!(*(surface_ptr+1)))) {
		error_handler.set_error(ERROR_MEM_SURFACE,0,"","");
		return;
	}

// Create Cavity if one is present
	if (device_input.number_cavity) {
		cavity_ptr=new TCavity(this, grid_ptr);
		if (!cavity_ptr) {
			error_handler.set_error(ERROR_MEM_CAVITY,0,"","");
			return;
		}
	}
}

void TDevice::process_input_param(void)
{
	int i,j, grid_count, qw_count;
	prec total_length, point_size, grid_length, grid_position;
	flag grid_effects, qw_effects, env_effects, mode_effects;

	grid_effects=GRID_RECOMB_SHR | GRID_RECOMB_B_B | GRID_RECOMB_AUGER | /*GRID_QW_FREE_CARR |*/ GRID_RELAX |
				 GRID_THERMIONIC | GRID_THERMOELECTRIC_HEAT | GRID_JOULE_HEAT | GRID_LATERAL_HEAT;
	env_effects=environment.get_value(ENVIRONMENT,EFFECTS);
	if (env_effects & ENV_OPTICAL_GEN) grid_effects|=GRID_OPTICAL_GEN;
	if (env_effects & ENV_INCIDENT_REFLECTION) grid_effects|=GRID_INCIDENT_REFLECTION;
	qw_effects=QW_FINITE_SQRWELL;

// Store node parameters.
	grid_count=total_length=0;
	for (i=0;i<device_input.number_grid;i++) {
		grid_length=(device_input.grid_ptr+i)->length;
		point_size=(device_input.grid_ptr+i)->length/
		   (double) (device_input.grid_ptr+i)->number_points;
		grid_position=total_length;

		for (j=0;j<((device_input.grid_ptr+i)->number_points);j++) {
			put_value(GRID_ELECTRICAL,POSITION,grid_position,grid_count);
			put_value(GRID_ELECTRICAL,MATERIAL,device_input.get_material_type(grid_position),grid_count);
			put_value(GRID_ELECTRICAL,ALLOY_CONC,device_input.get_alloy_conc(grid_position),grid_count);
			put_value(GRID_ELECTRICAL,ALLOY_TYPE,device_input.get_alloy_type(grid_position),grid_count);
			put_value(ELECTRON,DOPING_CONC,device_input.get_doping_conc(grid_position,DONOR),grid_count);
			put_value(ELECTRON,DOPING_DEGENERACY,device_input.get_doping_degeneracy(grid_position,DONOR),grid_count);
			put_value(ELECTRON,DOPING_LEVEL,device_input.get_doping_level(grid_position,DONOR),grid_count);
			put_value(HOLE,DOPING_CONC,device_input.get_doping_conc(grid_position,ACCEPTOR),grid_count);
			put_value(HOLE,DOPING_DEGENERACY,device_input.get_doping_degeneracy(grid_position,ACCEPTOR),grid_count);
			put_value(HOLE,DOPING_LEVEL,device_input.get_doping_level(grid_position,ACCEPTOR),grid_count);
			put_value(GRID_ELECTRICAL,RADIUS,device_input.get_radius(grid_position),grid_count);
			grid_position+=point_size;
			grid_count++;
		}
		total_length+=grid_length;
	}
	put_value(GRID_ELECTRICAL,POSITION,total_length,grid_count);
	put_value(GRID_ELECTRICAL,MATERIAL,device_input.get_material_type(total_length),grid_count);
	put_value(GRID_ELECTRICAL,ALLOY_CONC,device_input.get_alloy_conc(total_length),grid_count);
	put_value(GRID_ELECTRICAL,ALLOY_TYPE,device_input.get_alloy_type(total_length),grid_count);
	put_value(ELECTRON,DOPING_CONC,device_input.get_doping_conc(total_length,DONOR),grid_count);
	put_value(ELECTRON,DOPING_DEGENERACY,device_input.get_doping_degeneracy(total_length,DONOR),grid_count);
	put_value(ELECTRON,DOPING_LEVEL,device_input.get_doping_level(total_length,DONOR),grid_count);
	put_value(HOLE,DOPING_CONC,device_input.get_doping_conc(total_length,ACCEPTOR),grid_count);
	put_value(HOLE,DOPING_DEGENERACY,device_input.get_doping_degeneracy(total_length,ACCEPTOR),grid_count);
	put_value(HOLE,DOPING_LEVEL,device_input.get_doping_level(total_length,ACCEPTOR),grid_count);
	put_value(GRID_ELECTRICAL,RADIUS,device_input.get_radius(total_length),grid_count);

	environment.set_update_flags(GRID_ELECTRICAL, MATERIAL | ALLOY_CONC | ALLOY_TYPE | RADIUS);
	environment.set_update_flags(ELECTRON, DOPING_CONC);
	environment.set_update_flags(HOLE, DOPING_CONC);

// Store qw parameters.
	if (quantum_wells) {
		qw_count=0;
		for (i=0;i<device_input.number_region;i++) {
			if (((device_input.region_ptr+i)->type)==QW) {
				put_value(QUANTUM_WELL,LENGTH,(device_input.region_ptr+i)->length,qw_count++);
			}
		}
		comp_value(QUANTUM_WELL,POSITION);
		comp_value(QUANTUM_WELL,MIDPOINT);
		comp_value(QW_ELECTRON,DOPING_CONC);
        comp_value(QW_HOLE,DOPING_CONC);
		put_value(QUANTUM_WELL,EFFECTS,qw_effects);
	}

// Store cavity parameters
	if (device_input.number_cavity) {
		put_value(CAVITY,TYPE,device_input.cavity_ptr->type);
		put_value(CAVITY,AREA,device_input.cavity_ptr->area);
		put_value(CAVITY,LENGTH,device_input.cavity_ptr->length);

		put_value(MODE,SPONT_FACTOR,0.01);
		put_value(MODE,MODE_PHOTON_WAVELENGTH,0.822);

		for (i=0;i<=1;i++) {
			put_value(MIRROR,TYPE,(device_input.mirror_ptr+i)->type,i);
			put_value(MIRROR,POSITION,(device_input.mirror_ptr+i)->position,i);
			put_value(MIRROR,REFLECTIVITY,(device_input.mirror_ptr+i)->reflectivity,i);
		}
		device_effects|=DEVICE_LASER;
		grid_effects|=GRID_RECOMB_STIM;
		mode_effects=0;

		environment.set_update_flags(MIRROR,REFLECTIVITY);
		environment.set_update_flags(MODE,SPONT_FACTOR | MODE_PHOTON_ENERGY);

		put_value(MODE,EFFECTS,mode_effects);
        environment.clear_effects_change_flags(MODE,MODE_EFFECTS_ALL);
	}

	put_value(GRID_ELECTRICAL,EFFECTS,grid_effects,0,grid_points-1);
	environment.clear_effects_change_flags(GRID_ELECTRICAL,GRID_EFFECTS_ALL);
}

void TDevice::update_solution_param(void)
{
	if (solution_ptr) solution_ptr->comp_independent_param();
}

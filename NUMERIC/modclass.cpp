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
#include "simmode.h"

/************************************** class TMode *******************************************

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
*/

TMode::TMode(TNode** grid)
{
	grid_ptr=grid;
	total_photons=0.0;
	previous_total_photons=0.0;
	mode_gain=0.0;
	previous_mode_gain=0.0;
	total_spont=0.0;
	previous_total_spont=0.0;
	photon_lifetime=0.0;
	spont_factor=0.0;
	group_velocity=0.0;
	mirror_loss=0.0;
	energy=0.0;
	waveguide_loss=0.0;
	effects=0.0;
}

void TMode::init(void)
{
	total_photons=0.0;
	previous_total_photons=0.0;
	previous_mode_gain=0.0;
	total_spont=0.0;
	previous_total_spont=0.0;
	environment.put_value(GRID_OPTICAL,MODE_TOTAL_PHOTONS,0.0,-1,-1,NORMALIZED);
}

void TMode::comp_group_velocity(int start_node, int end_node)
{
	int i;
	prec first_position;
	prec previous_value, previous_position;
	prec next_value, next_position;
	prec average_index=0;
	TNode** temp_ptr;

	assert(start_node<end_node);

	temp_ptr=grid_ptr+start_node;

	previous_value=(*temp_ptr)->get_value(GRID_OPTICAL,MODE_REFRACTIVE_INDEX,NORMALIZED);
	previous_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
	first_position=previous_position;

	for (i=start_node+1;i<=end_node;i++) {
		temp_ptr++;
		next_value=(*temp_ptr)->get_value(GRID_OPTICAL,MODE_REFRACTIVE_INDEX,NORMALIZED);
		next_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		average_index+=(next_value+previous_value)*(next_position-previous_position)/2.0;
		previous_value=next_value;
		previous_position=next_position;
	}

	average_index/=(next_position-first_position);

	group_velocity=SIM_c/(average_index*get_normalize_value(MODE,MODE_GROUP_VELOCITY));
}

void TMode::comp_mirror_loss(prec reflectivity_1, prec reflectivity_2,
							 prec length)
{
	mirror_loss=log(1.0/(reflectivity_1*reflectivity_2))/(2.0*length);
}

void TMode::comp_mode_gain(int start_node, int end_node, prec cavity_area)
{
	int i;
	prec previous_value, previous_position;
	prec next_value, next_position;
	TNode** temp_ptr;

	assert(start_node<end_node);

	mode_gain=0.0;

	temp_ptr=grid_ptr+start_node;

	previous_value=sq((*temp_ptr)->get_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,NORMALIZED))*
					  (*temp_ptr)->get_value(GRID_OPTICAL,MODE_GAIN,NORMALIZED);
	previous_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	for (i=start_node+1;i<=end_node;i++) {
		temp_ptr++;
		next_value=sq((*temp_ptr)->get_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,NORMALIZED))*
					  (*temp_ptr)->get_value(GRID_OPTICAL,MODE_GAIN,NORMALIZED);
		next_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		mode_gain+=(next_value+previous_value)*(next_position-previous_position)/2.0;
		previous_value=next_value;
		previous_position=next_position;
	}

	mode_gain*=cavity_area*sq(get_normalize_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG))*1e12*
							  pow(normalization.length,3.0);
}

void TMode::comp_mode_normalization(int start_node, int end_node, prec cavity_area)
{
	int i;
	prec normalization_value;
	prec previous_value, previous_position;
	prec next_value, next_position;
	TNode** temp_ptr;

	assert(start_node<end_node);

	temp_ptr=grid_ptr+start_node;

	normalization_value=0.0;

	previous_value=sq((*temp_ptr)->get_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,NORMALIZED));
	previous_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	for (i=start_node+1;i<=end_node;i++) {
		temp_ptr++;
		next_value=sq((*temp_ptr)->get_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,NORMALIZED));
		next_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		normalization_value+=(next_value+previous_value)*(next_position-previous_position)/2.0;
		previous_value=next_value;
		previous_position=next_position;
	}

	normalization_value*=cavity_area*sq(get_normalize_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG))*1e12*
									 pow(normalization.length,3.0);
	normalization_value=sqrt(normalization_value);

	if (normalization_value) {
		temp_ptr=grid_ptr+start_node;

		environment.put_value(SURFACE,MODE_TOTAL_FIELD_MAG,
							  environment.get_value(SURFACE,MODE_TOTAL_FIELD_MAG,0,NORMALIZED)/normalization_value,
							  0,0,NORMALIZED);

		environment.put_value(SURFACE,MODE_TOTAL_FIELD_MAG,
							  environment.get_value(SURFACE,MODE_TOTAL_FIELD_MAG,1,NORMALIZED)/normalization_value,
							  1,1,NORMALIZED);

		for (i=start_node;i<=end_node;i++) {
			(*temp_ptr)->put_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,
								   (*temp_ptr)->get_value(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG,NORMALIZED)/normalization_value,
								   NORMALIZED);
			temp_ptr++;
		}
	}
}

void TMode::comp_mode_optical_field(int start_node, int end_node)
{
	TNode** temp_grid_ptr;

	assert(start_node<end_node);

	environment.put_value(GRID_OPTICAL,MODE_PHOTON_ENERGY,energy,start_node, end_node, NORMALIZED);
	environment.comp_value(GRID_OPTICAL,MODE_ABSORPTION,start_node,end_node);
	environment.comp_value(GRID_OPTICAL,MODE_IMPEDANCE_REAL,start_node,end_node);

	environment.init_value(SURFACE,MODE_FORWARD_FIELD_REAL,0,1);

	environment.comp_value(SURFACE,MODE_INTERNAL_FIELD,1);
	for (temp_grid_ptr=grid_ptr+end_node;
		 temp_grid_ptr>=grid_ptr+start_node;
		 temp_grid_ptr--) {
		(*temp_grid_ptr)->comp_mode_optical_field(end_node);
	}
	environment.comp_value(SURFACE,MODE_SURFACE_FIELD,0);
}

void TMode::comp_photon_lifetime(void)
{
	photon_lifetime=1.0/(group_velocity*(mirror_loss+waveguide_loss));
}

void TMode::comp_total_spontaneous(int start_node, int end_node, prec cavity_area)
{
	int i;
	prec previous_value, previous_position;
	prec next_value, next_position;
	TNode** temp_ptr;

	assert(start_node<end_node);

	total_spont=0.0;

	temp_ptr=grid_ptr+start_node;

	previous_value=(*temp_ptr)->get_value(NODE,B_B_RECOMB,NORMALIZED);
	previous_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	for (i=start_node+1;i<=end_node;i++) {
		temp_ptr++;
		next_value=(*temp_ptr)->get_value(NODE,B_B_RECOMB,NORMALIZED);
		next_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		total_spont+=(next_value+previous_value)*(next_position-previous_position)/2.0;
		previous_value=next_value;
		previous_position=next_position;
	}

	total_spont*=spont_factor*cavity_area*normalization.conc*pow(normalization.length,3.0);
}

error TMode::field_iterate(prec& iteration_error, prec initial_error, int iteration_number)
{
	static error_sign;
	prec curr_wavelength;
	prec next_forward_poynting, prev_forward_poynting, curr_forward_poynting;

	if (iteration_number==0) {
		curr_wavelength=get_value(MODE_PHOTON_WAVELENGTH);
		curr_forward_poynting=environment.get_value(SURFACE,MODE_FORWARD_POYNTING,0);

		put_value(MODE_PHOTON_WAVELENGTH,curr_wavelength+initial_error);
		environment.process_recompute_flags();
		next_forward_poynting=environment.get_value(SURFACE,MODE_FORWARD_POYNTING,0);

		put_value(MODE_PHOTON_WAVELENGTH,curr_wavelength-initial_error);
		environment.process_recompute_flags();
		prev_forward_poynting=environment.get_value(SURFACE,MODE_FORWARD_POYNTING,0);

		if ((prev_forward_poynting>=curr_forward_poynting) &&
			(next_forward_poynting>=curr_forward_poynting))
			error_sign=0.0;
		else {
			if (prev_forward_poynting<curr_forward_poynting) error_sign=-1.0;
			else error_sign=1.0;
		}

		put_value(MODE_PHOTON_WAVELENGTH,curr_wavelength+error_sign*initial_error);
		environment.process_recompute_flags();
	}
	else {
		curr_wavelength=get_value(MODE_PHOTON_WAVELENGTH);
		curr_forward_poynting=environment.get_value(SURFACE,MODE_FORWARD_POYNTING,0);

		put_value(MODE_PHOTON_WAVELENGTH,curr_wavelength+error_sign*initial_error);
		environment.process_recompute_flags();
		next_forward_poynting=environment.get_value(SURFACE,MODE_FORWARD_POYNTING,0);

		if (next_forward_poynting>=curr_forward_poynting) {
			put_value(MODE_PHOTON_WAVELENGTH,curr_wavelength);
			environment.process_recompute_flags();
			error_sign=0.0;
		}
	}

	iteration_error=error_sign*initial_error;

	return(ERROR_NONE);
}

error TMode::photon_iterate(prec& iteration_error)
{
	prec rate_function;
	prec deriv_rate_function, deriv_gain, deriv_spont;
	prec photon_update;
	prec gain,loss;

	gain=group_velocity*mode_gain;
	loss=(1.0/photon_lifetime);

	if (total_photons!=previous_total_photons) {
		deriv_gain=group_velocity*(mode_gain-previous_mode_gain)/(total_photons-previous_total_photons);
		deriv_spont=(total_spont-previous_total_spont)/(total_photons-previous_total_photons);
	}
	else {
		deriv_gain=0.0;
		deriv_spont=0.0;
	}

	if (gain<0.9999*loss) {
		deriv_rate_function=deriv_gain*total_photons+gain-loss+deriv_spont;
		rate_function=(gain-loss)*total_photons+total_spont;
	}
	else {
		deriv_rate_function=deriv_gain;
		rate_function=gain-loss;
	}

	photon_update=rate_function/deriv_rate_function;

	if (total_photons!=0.0) iteration_error=fabs(photon_update/total_photons);
	else iteration_error=1.0;

	previous_total_spont=total_spont;
	previous_mode_gain=mode_gain;
	previous_total_photons=total_photons;
	total_photons-=photon_update;

	environment.put_value(GRID_OPTICAL, MODE_TOTAL_PHOTONS,total_photons,-1,-1,NORMALIZED);
	environment.comp_value(NODE,STIM_RECOMB);
	environment.comp_value(NODE,TOTAL_RECOMB);

	return(ERROR_NONE);
}

prec TMode::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EFFECTS: return_value=(prec)effects; break;
		case MODE_TOTAL_PHOTONS: return_value=total_photons; break;
		case PHOTON_LIFETIME: return_value=photon_lifetime; break;
		case MODE_GAIN: return_value=mode_gain; break;
		case SPONT_FACTOR: return_value=spont_factor; break;
		case MODE_GROUP_VELOCITY: return_value=group_velocity; break;
		case MODE_PHOTON_ENERGY: return_value=energy; break;
		case MODE_PHOTON_WAVELENGTH: return_value=(1.242/(energy*normalization.energy))*1e-4/normalization.length; break;
		case MIRROR_LOSS: return_value=mirror_loss; break;
		case WAVEGUIDE_LOSS: return_value=waveguide_loss; break;
		case TOTAL_SPONTANEOUS: return_value=total_spont; break;
		default: assert(FALSE); break;
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(MODE,flag_value);
	return(return_value);
}

void TMode::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(MODE, flag_value);

	switch(flag_value) {
		case MODE_TOTAL_PHOTONS: total_photons=value; return;
		case PHOTON_LIFETIME: photon_lifetime=value; return;
		case MODE_GAIN: mode_gain=value; return;
		case SPONT_FACTOR: spont_factor=value; return;
		case EFFECTS:
			environment.set_effects_change_flags(MODE,effects^(flag)value);
			effects=(flag)value;
			return;
		case MODE_GROUP_VELOCITY:
			if (group_velocity!=value) {
				group_velocity=value;
				environment.set_update_flags(MODE,MODE_GROUP_VELOCITY);
			}
			return;
		case MODE_PHOTON_ENERGY:
			if (energy!=value) {
				energy=value;
				environment.set_update_flags(MODE,MODE_PHOTON_ENERGY);
			}
			return;
		case MODE_PHOTON_WAVELENGTH:
			value=(1.242/(value*normalization.length*1e4))/normalization.energy;
			if (energy!=value) {
				energy=value;
				environment.set_update_flags(MODE,MODE_PHOTON_ENERGY);
			}
			return;
		case MIRROR_LOSS: mirror_loss=value; return;
		case WAVEGUIDE_LOSS:
			if (waveguide_loss!=value) {
				waveguide_loss=value;
				environment.set_update_flags(MODE,WAVEGUIDE_LOSS);
			}
			return;
		case TOTAL_SPONTANEOUS: total_spont=value; return;
		default: assert(FALSE); return;
	}
}

void TMode::read_state_file(FILE *file_ptr)
{
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&total_photons,sizeof(total_photons),1,file_ptr);
	fread(&previous_total_photons,sizeof(previous_total_photons),1,file_ptr);
	fread(&mode_gain,sizeof(mode_gain),1,file_ptr);
	fread(&previous_mode_gain,sizeof(previous_mode_gain),1,file_ptr);
	fread(&total_spont,sizeof(total_spont),1,file_ptr);
	fread(&previous_total_spont,sizeof(previous_total_spont),1,file_ptr);
	fread(&photon_lifetime,sizeof(photon_lifetime),1,file_ptr);
	fread(&spont_factor,sizeof(spont_factor),1,file_ptr);
	fread(&group_velocity,sizeof(group_velocity),1,file_ptr);
	fread(&mirror_loss,sizeof(mirror_loss),1,file_ptr);
	fread(&waveguide_loss,sizeof(waveguide_loss),1,file_ptr);
	fread(&energy,sizeof(energy),1,file_ptr);
}

void TMode::write_state_file(FILE *file_ptr)
{
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&total_photons,sizeof(total_photons),1,file_ptr);
	fwrite(&previous_total_photons,sizeof(previous_total_photons),1,file_ptr);
	fwrite(&mode_gain,sizeof(mode_gain),1,file_ptr);
	fwrite(&previous_mode_gain,sizeof(previous_mode_gain),1,file_ptr);
	fwrite(&total_spont,sizeof(total_spont),1,file_ptr);
	fwrite(&previous_total_spont,sizeof(previous_total_spont),1,file_ptr);
	fwrite(&photon_lifetime,sizeof(photon_lifetime),1,file_ptr);
	fwrite(&spont_factor,sizeof(spont_factor),1,file_ptr);
	fwrite(&group_velocity,sizeof(group_velocity),1,file_ptr);
	fwrite(&mirror_loss,sizeof(mirror_loss),1,file_ptr);
	fwrite(&waveguide_loss,sizeof(waveguide_loss),1,file_ptr);
	fwrite(&energy,sizeof(energy),1,file_ptr);
}

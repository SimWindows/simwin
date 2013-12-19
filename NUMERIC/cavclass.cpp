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
#include "simmode.h"
#include "simmir.h"
#include "simcav.h"

/*************************************** class TCavity ****************************************

class TCavity {
private:
	CavityType type;
	prec area;
	prec length;
	TMode mode;
	TMirror mirror_0;
	TMirror mirror_1;
public:
	TCavity(TDevice *ptr, TNode** grid);
	void init(void);

	error field_iterate(prec& iteration_error, prec initial_error, int iteration_number);
	error photon_iterate(prec& iteration_error);

	prec get_value(FlagType flag_type, flag flag_value, int object,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, int object, prec value,
				   ScaleType scale=UNNORMALIZED);

	void comp_value(FlagType flag_type, flag flag_value,
					int start_object=-1, int end_object=-1);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TCavity::TCavity(TDevice *ptr, TNode** grid)
	: mirror_0(ptr), mirror_1(ptr), mode(grid)
{
	type=(CavityType)0;
	area=0.0;
}

void TCavity::init(void)
{
	mirror_0.init();
	mirror_1.init();
	mode.init();
}

error TCavity::field_iterate(prec& iteration_error, prec initial_error, int iteration_number)
{
	return(mode.field_iterate(iteration_error,initial_error,iteration_number));
}

error TCavity::photon_iterate(prec& iteration_error)
{
	return(mode.photon_iterate(iteration_error));
}

prec TCavity::get_value(FlagType flag_type, flag flag_value, int object,
						ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case MODE: return(mode.get_value(flag_value,scale));
		case MIRROR:
			switch(object) {
				case 0: return(mirror_0.get_value(flag_value,scale));
				case 1: return(mirror_1.get_value(flag_value,scale));
				default: assert(FALSE); return(0.0);
			}
		case CAVITY:
			switch(flag_value) {
				case TYPE: return_value=(prec) type; break;
				case AREA: return_value=area; break;
				case LENGTH: return_value=length; break;
				default: assert(FALSE); return(0.0);
			}
			break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(CAVITY,flag_value);
	return(return_value);
}

void TCavity::put_value(FlagType flag_type, flag flag_value, int object,
						prec value, ScaleType scale)
{
	switch(flag_type) {
		case MODE: mode.put_value(flag_value,value,scale); return;
		case MIRROR:
			switch(object) {
				case 0: mirror_0.put_value(flag_value,value,scale); return;
				case 1: mirror_1.put_value(flag_value,value,scale); return;
				default: assert(FALSE); return;
			}
		case CAVITY:
			if (scale==UNNORMALIZED) value/=get_normalize_value(CAVITY,flag_value);
			switch(flag_value) {
				case TYPE: type=(CavityType)value; return;
				case AREA:
					if (area!=value) {
						area=value;
						environment.set_update_flags(CAVITY,AREA);
					}
					return;
				case LENGTH:
					if (length!=value) {
						length=value;
						environment.set_update_flags(CAVITY,LENGTH);
					}
                    return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

void TCavity::comp_value(FlagType flag_type, flag flag_value,
						 int start_object, int end_object)
{
	int start_node, end_node;

	switch(flag_type) {
		case MIRROR:
			if (flag_value==POWER) {
				if ((start_object==-1) || (start_object==0))
					mirror_0.comp_power(mode.get_value(MODE_TOTAL_PHOTONS,NORMALIZED),
										mode.get_value(MODE_PHOTON_ENERGY,NORMALIZED),
										mode.get_value(MODE_GROUP_VELOCITY,NORMALIZED),
										length);
				if ((end_object==-1) || (end_object!=0))
					mirror_1.comp_power(mode.get_value(MODE_TOTAL_PHOTONS,NORMALIZED),
										mode.get_value(MODE_PHOTON_ENERGY, NORMALIZED),
										mode.get_value(MODE_GROUP_VELOCITY,NORMALIZED),
										length);
			}
			else assert(FALSE);
			break;

		case MODE:
			start_node=mirror_0.get_value(NODE_NUMBER);
			end_node=mirror_1.get_value(NODE_NUMBER);
			if (start_node>end_node) swap(start_node,end_node);

			switch(flag_value) {
				case PHOTON_LIFETIME: mode.comp_photon_lifetime(); break;
				case MIRROR_LOSS: mode.comp_mirror_loss(mirror_0.get_value(REFLECTIVITY),
														mirror_1.get_value(REFLECTIVITY),
														length); break;
				case MODE_GROUP_VELOCITY: mode.comp_group_velocity(start_node,end_node); break;
				case MODE_GAIN: mode.comp_mode_gain(start_node,end_node,area); break;
				case TOTAL_SPONTANEOUS: mode.comp_total_spontaneous(start_node,end_node,area); break;
				case MODE_NORMALIZATION: mode.comp_mode_normalization(start_node,end_node,area); break;
				case MODE_TOTAL_FIELD_MAG: mode.comp_mode_optical_field(start_node,end_node); break;
				default: assert(FALSE); break;
			}
			break;
		case CAVITY: assert(FALSE);
		default: assert(FALSE);
	}
}

void TCavity::read_state_file(FILE *file_ptr)
{
	fread(&type,sizeof(type),1,file_ptr);
	fread(&area,sizeof(area),1,file_ptr);
	fread(&length,sizeof(length),1,file_ptr);

	mode.read_state_file(file_ptr);

	mirror_0.read_state_file(file_ptr);
	mirror_1.read_state_file(file_ptr);
}

void TCavity::write_state_file(FILE *file_ptr)
{
	fwrite(&type,sizeof(type),1,file_ptr);
	fwrite(&area,sizeof(area),1,file_ptr);
	fwrite(&length,sizeof(length),1,file_ptr);

	mode.write_state_file(file_ptr);

	mirror_0.write_state_file(file_ptr);
	mirror_1.write_state_file(file_ptr);
}

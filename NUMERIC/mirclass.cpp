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
#include "simmir.h"

/************************************** class TMirror *****************************************

class TMirror {
private:
	TDevice *device_ptr;
	MirrorType type;
	prec position;
	short node_number;
	prec reflectivity;
	float output_power;
public:
	TMirror(TDevice *ptr);
	void init(void);

	void comp_power(float photon_number, float photon_energy,
					float group_velocity, float cavity_length);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TMirror::TMirror(TDevice *ptr)
{
	device_ptr=ptr;
	type=(MirrorType) 0;
	reflectivity=0.0;
	position=0.0;
	node_number=0;
	output_power=0.0;
}

void TMirror::init(void)
{
	output_power=0.0;
}

void TMirror::comp_power(float photon_number, float photon_energy,
						 float group_velocity, float cavity_length)
{
	output_power=photon_energy*photon_number*group_velocity*log(1.0/reflectivity)/(2.0*cavity_length);
}

prec TMirror::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case TYPE: return_value=(prec) type; break;
		case REFLECTIVITY: return_value=reflectivity; break;
		case POSITION: return_value=position; break;
		case NODE_NUMBER: return_value=node_number; break;
		case POWER: return_value=output_power; break;
		default: assert(FALSE); break;
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(MIRROR,flag_value);
	return(return_value);
}

void TMirror::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(MIRROR,flag_value);

	switch(flag_value) {
		case TYPE: type=(MirrorType)value; return;
		case REFLECTIVITY:
			if (reflectivity!=value) {
				reflectivity=value;
				environment.set_update_flags(MIRROR,REFLECTIVITY);
			}
			return;
		case POSITION:
			if (value!=position) {
				position=value;
				node_number=device_ptr->get_node(position,-1,-1,NORMALIZED);
				environment.set_update_flags(MIRROR,POSITION);
			}
			return;
		case NODE_NUMBER:
			node_number=value;
			position=device_ptr->get_value(GRID_ELECTRICAL,POSITION,node_number,NORMALIZED);
			return;
		case POWER: output_power=value; return;
		default: assert(FALSE); return;
	}
}

void TMirror::read_state_file(FILE *file_ptr)
{
	fread(&type,sizeof(type),1,file_ptr);
	fread(&position,sizeof(position),1,file_ptr);
	fread(&node_number,sizeof(node_number),1,file_ptr);
	fread(&reflectivity,sizeof(reflectivity),1,file_ptr);
	fread(&output_power,sizeof(output_power),1,file_ptr);
}

void TMirror::write_state_file(FILE *file_ptr)
{
	fwrite(&type,sizeof(type),1,file_ptr);
	fwrite(&position,sizeof(position),1,file_ptr);
	fwrite(&node_number,sizeof(node_number),1,file_ptr);
	fwrite(&reflectivity,sizeof(reflectivity),1,file_ptr);
	fwrite(&output_power,sizeof(output_power),1,file_ptr);
}

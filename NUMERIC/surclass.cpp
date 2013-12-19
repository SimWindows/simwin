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
#include "simsurf.h"

/******************************* class TSurface ************************************************

class TSurface {
private:
	TNode *surface_node;
	int surface_node_number;
	TNode *second_node;
	int second_node_number;
	flag effects;
	prec temp;
	prec electron_temp;
	prec hole_temp;
	prec thermal_conduct;
	prec incident_refractive_index;
	OpticalField incident_field;
	prec mode_refractive_index;
	OpticalField mode_field;

public:
	TSurface(TNode* surface_node_ptr, TNode* next_node_ptr);
	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);
	void comp_incident_surface_field(void);
	void comp_incident_internal_field(void);
	void comp_emitted_total_poynting(prec multiplier);
	prec comp_input_total_poynting(int spectral_component);
	void comp_mode_surface_field(void);
	void comp_mode_internal_field(void);
	void comp_value(flag flag_value);
	void init_forward_incident_field(void);
	void init_reverse_incident_field(void);
	void init_forward_mode_field(void);
	void init_reverse_mode_field(void);
	void init_value(flag flag_value);
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TSurface::TSurface(TNode* surface_node_ptr, TNode* next_node_ptr)
{
	surface_node=surface_node_ptr;
	surface_node_number=surface_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	second_node=next_node_ptr;
	second_node_number=second_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	effects=SURFACE_HEAT_SINK;
	temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);
	electron_temp=temp;
	hole_temp=temp;
	thermal_conduct=0.0;
	incident_refractive_index=1.0;
	incident_field.forward_field=incident_field.forward_poynting=0.0;
	incident_field.reverse_field=incident_field.reverse_poynting=0.0;
	incident_field.total_poynting=incident_field.total_magnitude=0.0;
	mode_refractive_index=1.0;
	mode_field.forward_field=mode_field.forward_poynting=0.0;
	mode_field.reverse_field=mode_field.reverse_poynting=0.0;
	mode_field.total_poynting=mode_field.total_magnitude=0.0;
}

prec TSurface::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EFFECTS: return_value=(prec)effects; break;
		case NODE_NUMBER: return_value=surface_node_number; break;
		case POSITION: return_value=surface_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED); break;
		case THERMAL_CONDUCT: return_value=thermal_conduct; break;
		case TEMPERATURE: return_value=temp; break;
		case ELECTRON_TEMPERATURE: return_value=electron_temp; break;
		case HOLE_TEMPERATURE: return_value=hole_temp; break;
		case INCIDENT_TOTAL_POYNTING: return_value=incident_field.total_poynting; break;
		case INCIDENT_REFRACTIVE_INDEX: return_value=incident_refractive_index; break;
		case INCIDENT_FORWARD_FIELD_REAL: return_value=real(incident_field.forward_field); break;
		case INCIDENT_FORWARD_FIELD_IMAG: return_value=imag(incident_field.forward_field); break;
		case INCIDENT_FORWARD_POYNTING: return_value=incident_field.forward_poynting; break;
		case INCIDENT_REVERSE_FIELD_REAL: return_value=real(incident_field.reverse_field); break;
		case INCIDENT_REVERSE_FIELD_IMAG: return_value=imag(incident_field.reverse_field); break;
		case INCIDENT_REVERSE_POYNTING: return_value=incident_field.reverse_poynting; break;
		case INCIDENT_TOTAL_FIELD_MAG: return_value=incident_field.total_magnitude; break;
		case MODE_REFRACTIVE_INDEX: return_value=mode_refractive_index; break;
		case MODE_FORWARD_FIELD_REAL: return_value=real(mode_field.forward_field); break;
		case MODE_FORWARD_FIELD_IMAG: return_value=imag(mode_field.forward_field); break;
		case MODE_FORWARD_POYNTING: return_value=mode_field.forward_poynting; break;
		case MODE_REVERSE_FIELD_REAL: return_value=real(mode_field.reverse_field); break;
		case MODE_REVERSE_FIELD_IMAG: return_value=imag(mode_field.reverse_field); break;
		case MODE_REVERSE_POYNTING: return_value=mode_field.reverse_poynting; break;
		case MODE_TOTAL_FIELD_MAG: return_value=mode_field.total_magnitude; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(SURFACE,flag_value);
	return(return_value);
}

void TSurface::put_value(flag flag_value, prec value, ScaleType scale)
{
	prec prev_value;

	if (scale==UNNORMALIZED) value/=get_normalize_value(SURFACE,flag_value);

	switch(flag_value) {
		case EFFECTS:
			environment.set_effects_change_flags(SURFACE,effects^(flag)value);
			effects=(flag)value;
			return;
		case TEMPERATURE: temp=value; return;
		case ELECTRON_TEMPERATURE: electron_temp=value; return;
		case HOLE_TEMPERATURE: hole_temp=value; return;
		case INCIDENT_TOTAL_POYNTING: incident_field.total_poynting=value; return;
		case THERMAL_CONDUCT: thermal_conduct=value; return;
		case INCIDENT_REFRACTIVE_INDEX:
			prev_value=incident_refractive_index;
			incident_refractive_index=value;
			if (prev_value!=incident_refractive_index) environment.set_update_flags(SURFACE,INCIDENT_REFRACTIVE_INDEX);
			return;
		case MODE_REFRACTIVE_INDEX:
			prev_value=mode_refractive_index;
			mode_refractive_index=value;
			if (prev_value!=mode_refractive_index) environment.set_update_flags(SURFACE,MODE_REFRACTIVE_INDEX);
			return;
		case INCIDENT_FORWARD_FIELD_REAL:
			incident_field.forward_field=complex(value,imag(incident_field.forward_field));
			return;
		case INCIDENT_FORWARD_FIELD_IMAG:
			incident_field.forward_field=complex(real(incident_field.forward_field),value);
			return;
		case INCIDENT_FORWARD_POYNTING:	incident_field.forward_poynting=value; return;
		case INCIDENT_REVERSE_FIELD_REAL:
			incident_field.reverse_field=complex(value,imag(incident_field.reverse_field));
			return;
		case INCIDENT_REVERSE_FIELD_IMAG:
			incident_field.reverse_field=complex(real(incident_field.reverse_field),value);
			return;
		case INCIDENT_REVERSE_POYNTING:	incident_field.reverse_poynting=value; return;
		case INCIDENT_TOTAL_FIELD_MAG: incident_field.total_magnitude=value; return;
		case MODE_FORWARD_FIELD_REAL:
			mode_field.forward_field=complex(value,imag(mode_field.forward_field));
			return;
		case MODE_FORWARD_FIELD_IMAG:
			mode_field.forward_field=complex(real(mode_field.forward_field),value);
			return;
		case MODE_REVERSE_FIELD_REAL:
			mode_field.reverse_field=complex(value,imag(mode_field.reverse_field));
			return;
		case MODE_REVERSE_FIELD_IMAG:
			mode_field.reverse_field=complex(real(mode_field.reverse_field),value);
			return;
		case MODE_TOTAL_FIELD_MAG: mode_field.total_magnitude=value; return;
		default: assert(FALSE);
	}
}

void TSurface::comp_incident_surface_field(void)
{
	int start_grid_point;
	complex impedance_sum, impedance_diff;
	complex internal_forward_field, internal_reverse_field;

	start_grid_point=environment.get_node(environment.get_value(ENVIRONMENT,SPEC_START_POSITION));

	if ((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_INCIDENT_REFLECTION) {
		complex internal_impedance(environment.get_value(GRID_OPTICAL,INCIDENT_IMPEDANCE_REAL,start_grid_point,NORMALIZED),
								   environment.get_value(GRID_OPTICAL,INCIDENT_IMPEDANCE_IMAG,start_grid_point,NORMALIZED));
		complex surface_impedance(1.0/incident_refractive_index,0.0);
		impedance_sum=internal_impedance+surface_impedance;
		impedance_diff=internal_impedance-surface_impedance;

		internal_forward_field=complex(environment.get_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_REAL,
															 start_grid_point,NORMALIZED),
									   environment.get_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_IMAG,
															 start_grid_point,NORMALIZED));
		internal_reverse_field=complex(environment.get_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_REAL,
															 start_grid_point,NORMALIZED),
									   environment.get_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_IMAG,
															 start_grid_point,NORMALIZED));

		incident_field.forward_field=(impedance_sum*internal_forward_field+impedance_diff*internal_reverse_field)/
									 (2.0*internal_impedance);
		incident_field.reverse_field=(impedance_diff*internal_forward_field+impedance_sum*internal_reverse_field)/
									 (2.0*internal_impedance);

		incident_field.forward_poynting=0.5*norm(incident_field.forward_field)*incident_refractive_index;
		incident_field.reverse_poynting=0.5*norm(incident_field.reverse_field)*incident_refractive_index;
	}
	else {
		incident_field.forward_field=complex(environment.get_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_REAL,
																   start_grid_point,NORMALIZED),
											 environment.get_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_IMAG,
																   start_grid_point,NORMALIZED));
		incident_field.reverse_field=complex(environment.get_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_REAL,
																   start_grid_point,NORMALIZED),
											 environment.get_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_IMAG,
																   start_grid_point,NORMALIZED));

		incident_field.forward_poynting=0.5*norm(incident_field.forward_field);
		incident_field.reverse_poynting=0.5*norm(incident_field.reverse_field);
	}
	incident_field.total_magnitude=sqrt(norm(incident_field.forward_field+incident_field.reverse_field));
}

void TSurface::comp_incident_internal_field(void)
{
	int end_grid_point;
	complex impedance_sum, impedance_diff;
	complex internal_forward_field, internal_reverse_field;

	end_grid_point=environment.get_node(environment.get_value(ENVIRONMENT,SPEC_END_POSITION));

	if ((flag)environment.get_value(ENVIRONMENT,EFFECTS,0) & ENV_INCIDENT_REFLECTION) {
		complex internal_impedance(environment.get_value(GRID_OPTICAL,INCIDENT_IMPEDANCE_REAL,end_grid_point,NORMALIZED),
								   environment.get_value(GRID_OPTICAL,INCIDENT_IMPEDANCE_IMAG,end_grid_point,NORMALIZED));
		complex surface_impedance(1.0/incident_refractive_index,0.0);
		impedance_sum=surface_impedance+internal_impedance;
		impedance_diff=surface_impedance-internal_impedance;
		internal_forward_field=(impedance_sum*incident_field.forward_field+impedance_diff*incident_field.reverse_field)/
							   (2.0*surface_impedance);
		internal_reverse_field=(impedance_diff*incident_field.forward_field+impedance_sum*incident_field.reverse_field)/
							   (2.0*surface_impedance);

		environment.put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_REAL,real(internal_forward_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_IMAG,imag(internal_forward_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_REAL,real(internal_reverse_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_IMAG,imag(internal_reverse_field),
							  end_grid_point,end_grid_point,NORMALIZED);
	}
	else {
		environment.put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_REAL,real(incident_field.forward_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_FORWARD_FIELD_IMAG,imag(incident_field.forward_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_REAL,real(incident_field.reverse_field),
							  end_grid_point,end_grid_point,NORMALIZED);
		environment.put_value(GRID_OPTICAL,INCIDENT_REVERSE_FIELD_IMAG,imag(incident_field.reverse_field),
							  end_grid_point,end_grid_point,NORMALIZED);
	}
}

void TSurface::comp_emitted_total_poynting(prec multiplier)
{
	incident_field.forward_poynting*=multiplier;
	incident_field.reverse_poynting*=multiplier;

	if ((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_INCIDENT_REFLECTION) {
		if (norm(incident_field.forward_field)!=0.0)
			incident_field.forward_field*=sqrt(2.0*incident_field.forward_poynting/
											   (incident_refractive_index*norm(incident_field.forward_field)));

		if (norm(incident_field.reverse_field)!=0.0)
			incident_field.reverse_field*=sqrt(2.0*incident_field.reverse_poynting/
											   (incident_refractive_index*norm(incident_field.reverse_field)));
	}
	else {
		if (norm(incident_field.forward_field)!=0.0)
			incident_field.forward_field*=sqrt(2.0*incident_field.forward_poynting/
											   norm(incident_field.forward_field));

		if (norm(incident_field.reverse_field)!=0.0)
			incident_field.reverse_field*=sqrt(2.0*incident_field.reverse_poynting/
											   norm(incident_field.reverse_field));
	}
	incident_field.total_poynting=incident_field.forward_poynting-incident_field.reverse_poynting;
	incident_field.total_magnitude=sqrt(norm(incident_field.forward_field+incident_field.reverse_field));
}

prec TSurface::comp_input_total_poynting(int spectral_component)
{
	prec incident_input_intensity;
	prec multiplier;

	incident_input_intensity=environment.get_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,spectral_component,NORMALIZED);

	if (incident_field.reverse_poynting<incident_field.forward_poynting)
		multiplier=incident_input_intensity/incident_field.forward_poynting;
	else
		multiplier=incident_input_intensity/incident_field.reverse_poynting;

	incident_field.forward_poynting*=multiplier;
	incident_field.reverse_poynting*=multiplier;

	incident_field.total_poynting=incident_field.forward_poynting-incident_field.reverse_poynting;
	return(multiplier);
}

void TSurface::comp_mode_surface_field(void)
{
	int start_grid_point;
	complex impedance_sum, impedance_diff;
	complex internal_forward_field, internal_reverse_field;

	start_grid_point=environment.get_node(environment.get_value(MIRROR,POSITION,0));

	complex internal_impedance(environment.get_value(GRID_OPTICAL,MODE_IMPEDANCE_REAL,start_grid_point,NORMALIZED),
							   environment.get_value(GRID_OPTICAL,MODE_IMPEDANCE_IMAG,start_grid_point,NORMALIZED));
	complex surface_impedance(1.0/mode_refractive_index,0.0);
	impedance_sum=internal_impedance+surface_impedance;
	impedance_diff=internal_impedance-surface_impedance;

	internal_forward_field=complex(environment.get_value(GRID_OPTICAL,MODE_FORWARD_FIELD_REAL,start_grid_point,NORMALIZED),
								   environment.get_value(GRID_OPTICAL,MODE_FORWARD_FIELD_IMAG,start_grid_point,NORMALIZED));
	internal_reverse_field=complex(environment.get_value(GRID_OPTICAL,MODE_REVERSE_FIELD_REAL,start_grid_point,NORMALIZED),
								   environment.get_value(GRID_OPTICAL,MODE_REVERSE_FIELD_IMAG,start_grid_point,NORMALIZED));

	mode_field.forward_field=(impedance_sum*internal_forward_field+impedance_diff*internal_reverse_field)/
							 (2.0*internal_impedance);
	mode_field.reverse_field=(impedance_diff*internal_forward_field+impedance_sum*internal_reverse_field)/
							 (2.0*internal_impedance);

	mode_field.forward_poynting=0.5*norm(mode_field.forward_field)*mode_refractive_index;
	mode_field.reverse_poynting=0.5*norm(mode_field.reverse_field)*mode_refractive_index;
	mode_field.total_magnitude=sqrt(norm(mode_field.forward_field+mode_field.reverse_field));
}

void TSurface::comp_mode_internal_field(void)
{
	int end_grid_point;
	complex impedance_sum, impedance_diff;
	complex internal_forward_field, internal_reverse_field;

	end_grid_point=environment.get_node(environment.get_value(MIRROR,POSITION,1));

	complex internal_impedance(environment.get_value(GRID_OPTICAL,MODE_IMPEDANCE_REAL,end_grid_point,NORMALIZED),
							   environment.get_value(GRID_OPTICAL,MODE_IMPEDANCE_IMAG,end_grid_point,NORMALIZED));
	complex surface_impedance(1.0/mode_refractive_index,0.0);
	impedance_sum=surface_impedance+internal_impedance;
	impedance_diff=surface_impedance-internal_impedance;
	internal_forward_field=(impedance_sum*mode_field.forward_field+impedance_diff*mode_field.reverse_field)/
						   (2.0*surface_impedance);
	internal_reverse_field=(impedance_diff*mode_field.forward_field+impedance_sum*mode_field.reverse_field)/
						   (2.0*surface_impedance);

	environment.put_value(GRID_OPTICAL,MODE_FORWARD_FIELD_REAL,real(internal_forward_field),
						  end_grid_point,end_grid_point,NORMALIZED);
	environment.put_value(GRID_OPTICAL,MODE_FORWARD_FIELD_IMAG,imag(internal_forward_field),
						  end_grid_point,end_grid_point,NORMALIZED);
	environment.put_value(GRID_OPTICAL,MODE_REVERSE_FIELD_REAL,real(internal_reverse_field),
						  end_grid_point,end_grid_point,NORMALIZED);
	environment.put_value(GRID_OPTICAL,MODE_REVERSE_FIELD_IMAG,imag(internal_reverse_field),
						  end_grid_point,end_grid_point,NORMALIZED);
}

void TSurface::comp_value(flag flag_value)
{
	switch(flag_value) {
		case TEMPERATURE: temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED); return;
		case ELECTRON_TEMPERATURE: electron_temp=temp; return;
		case HOLE_TEMPERATURE: hole_temp=temp; return;
		case INCIDENT_SURFACE_FIELD: comp_incident_surface_field(); return;
		case INCIDENT_INTERNAL_FIELD: comp_incident_internal_field(); return;
		case MODE_SURFACE_FIELD: comp_mode_surface_field(); return;
		case MODE_INTERNAL_FIELD: comp_mode_internal_field(); return;
		default: assert(FALSE); return;
	}
}

void TSurface::init_forward_incident_field(void)
{
	incident_field.forward_field=complex(1.0,0.0);

	if ((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_INCIDENT_REFLECTION)
		incident_field.forward_poynting=0.5*incident_refractive_index;
	else
		incident_field.forward_poynting=0.5;

	incident_field.reverse_field=complex(0.0,0.0);
	incident_field.reverse_poynting=0.0;
	incident_field.total_magnitude=1.0;
}

void TSurface::init_reverse_incident_field(void)
{
	incident_field.forward_field=complex(0.0,0.0);
	incident_field.forward_poynting=0.0;
	incident_field.reverse_field=complex(1.0,0.0);

	if ((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_INCIDENT_REFLECTION)
		incident_field.reverse_poynting=0.5*incident_refractive_index;
	else
		incident_field.reverse_poynting=0.5;
	incident_field.total_magnitude=1.0;
}

void TSurface::init_forward_mode_field(void)
{
	mode_field.forward_field=complex(1.0,0.0);
	mode_field.forward_poynting=0.5*mode_refractive_index;
	mode_field.reverse_field=complex(0.0,0.0);
	mode_field.reverse_poynting=0.0;
	mode_field.total_magnitude=1.0;
}

void TSurface::init_reverse_mode_field(void)
{
	mode_field.forward_field=complex(0.0,0.0);
	mode_field.forward_poynting=0.0;
	mode_field.reverse_field=complex(1.0,0.0);
	mode_field.reverse_poynting=0.5*mode_refractive_index;
	mode_field.total_magnitude=1.0;
}

void TSurface::init_value(flag flag_value)
{
	switch(flag_value) {
		case INCIDENT_FORWARD_FIELD_REAL:
		case INCIDENT_FORWARD_FIELD_IMAG: init_forward_incident_field(); break;
		case INCIDENT_REVERSE_FIELD_REAL:
		case INCIDENT_REVERSE_FIELD_IMAG: init_reverse_incident_field(); break;
		case MODE_FORWARD_FIELD_REAL:
		case MODE_FORWARD_FIELD_IMAG: init_forward_mode_field(); break;
		case MODE_REVERSE_FIELD_REAL:
		case MODE_REVERSE_FIELD_IMAG: init_reverse_mode_field(); break;
		default: assert(FALSE);
	}
}

void TSurface::read_state_file(FILE *file_ptr)
{
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&temp,sizeof(temp),1,file_ptr);
	fread(&electron_temp,sizeof(electron_temp),1,file_ptr);
	fread(&hole_temp,sizeof(hole_temp),1,file_ptr);
	fread(&thermal_conduct,sizeof(thermal_conduct),1,file_ptr);
	fread(&incident_refractive_index,sizeof(incident_refractive_index),1,file_ptr);
	fread(&incident_field,sizeof(incident_field),1,file_ptr);
	fread(&mode_refractive_index,sizeof(mode_refractive_index),1,file_ptr);
	fread(&mode_field,sizeof(mode_field),1,file_ptr);
}

void TSurface::write_state_file(FILE *file_ptr)
{
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&temp,sizeof(temp),1,file_ptr);
	fwrite(&electron_temp,sizeof(electron_temp),1,file_ptr);
	fwrite(&hole_temp,sizeof(hole_temp),1,file_ptr);
	fwrite(&thermal_conduct,sizeof(thermal_conduct),1,file_ptr);
	fwrite(&incident_refractive_index,sizeof(incident_refractive_index),1,file_ptr);
	fwrite(&incident_field,sizeof(incident_field),1,file_ptr);
	fwrite(&mode_refractive_index,sizeof(mode_refractive_index),1,file_ptr);
	fwrite(&mode_field,sizeof(mode_field),1,file_ptr);
}

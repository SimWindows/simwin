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
#include "simcont.h"

/************************************** class TContact ****************************************

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
*/

TContact::TContact(TNode* contact_node_ptr, TNode* next_node_ptr)
{
	contact_node=contact_node_ptr;
	contact_node_number=contact_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	second_node=next_node_ptr;
	second_node_number=second_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	effects=CONTACT_IDEALOHMIC;
	bias=0.0;
	built_in_pot=0.0;
	electron_recomb_vel=0.0;
	equil_electron_conc=0.0;
	hole_recomb_vel=0.0;
	equil_hole_conc=0.0;
    barrier_height=0.0;
}

void TContact::init(void)
{
	bias=0.0;
}

void TContact::comp_built_pot(void)
{
	prec reference_quasi_fermi, current_quasi_fermi;

    reference_quasi_fermi=environment.get_value(ELECTRON,EQUIL_QUASI_FERMI,0,NORMALIZED);

	if (effects & CONTACT_SCHOTTKY)
    	current_quasi_fermi=barrier_height;
    else
    	current_quasi_fermi=environment.get_value(ELECTRON,EQUIL_QUASI_FERMI,contact_node_number,NORMALIZED);

	built_in_pot=reference_quasi_fermi+
				 environment.get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,0,NORMALIZED)-
				 environment.get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,contact_node_number,NORMALIZED)-
                 current_quasi_fermi;
}

void TContact::comp_electron_current(void)
{
	TNode *next_node, *prev_node;
	prec temp_next, temp_prev;
	prec mass_next, mass_prev;
	prec grad_band, grad_mass, grad_temp;
	prec current;
	prec planck_potential;
	prec mobility;
	prec element_length;
	long collision_factor;
	flag grid_effects;
	prec fermi_ratio_1_half_minus_1_half;
	prec fermi_ratio_3_half_1_half_col;
	prec bernoulli_param;

	if (second_node_number>contact_node_number) {
		next_node=second_node;
		prev_node=contact_node;
	}
	else {
		next_node=contact_node;
		prev_node=second_node;
	}

	element_length=next_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED)-
				   prev_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	grid_effects=(flag)next_node->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);

	temp_next=next_node->get_value(ELECTRON,TEMPERATURE,NORMALIZED);
	temp_prev=prev_node->get_value(ELECTRON,TEMPERATURE,NORMALIZED);

	mobility=contact_node->get_value(FREE_ELECTRON,MOBILITY,NORMALIZED);

	mass_next=next_node->get_value(ELECTRON,DOS_MASS,NORMALIZED);
	mass_prev=prev_node->get_value(ELECTRON,DOS_MASS,NORMALIZED);

	planck_potential=(next_node->get_value(ELECTRON,PLANCK_POT,NORMALIZED)+
					  prev_node->get_value(ELECTRON,PLANCK_POT,NORMALIZED))/2.0;

	collision_factor=(long)(prev_node->get_value(ELECTRON,COLLISION_FACTOR,NORMALIZED)*2.0);

	if (grid_effects & GRID_FERMI_DIRAC) {
		fermi_ratio_1_half_minus_1_half=fermi_integral_1_half(planck_potential)/
										fermi_integral_minus_1_half(planck_potential);

		switch(collision_factor) {
			case -1:
				fermi_ratio_3_half_1_half_col=fermi_integral_2_half(planck_potential)/
											  fermi_integral_0_half(planck_potential);
				break;
			case 0:
				fermi_ratio_3_half_1_half_col=fermi_integral_3_half(planck_potential)/
											  fermi_integral_1_half(planck_potential);
				break;
			case 1:
				fermi_ratio_3_half_1_half_col=fermi_integral_4_half(planck_potential)/
											  fermi_integral_2_half(planck_potential);
				break;
			case 3:
				fermi_ratio_3_half_1_half_col=fermi_integral_6_half(planck_potential)/
											  fermi_integral_4_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else {
		fermi_ratio_1_half_minus_1_half=1.0;
		fermi_ratio_3_half_1_half_col=1.0;
	}

	grad_temp=((2.5+(prec)collision_factor/2.0)*fermi_ratio_3_half_1_half_col/fermi_ratio_1_half_minus_1_half-1.5)*
			   (temp_next-temp_prev);

	grad_band=(next_node->get_value(ELECTRON,BAND_EDGE,NORMALIZED)-
			   prev_node->get_value(ELECTRON,BAND_EDGE,NORMALIZED))/fermi_ratio_1_half_minus_1_half;

	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	bernoulli_param=-grad_temp-grad_band+grad_mass;

	current=mobility*fermi_ratio_1_half_minus_1_half/element_length*
			(next_node->get_value(ELECTRON,CONCENTRATION,NORMALIZED)*temp_next*
			 bernoulli(bernoulli_param/temp_next)-
			 prev_node->get_value(ELECTRON,CONCENTRATION,NORMALIZED)*temp_prev*
			 bernoulli(-bernoulli_param/temp_prev));

	contact_node->put_value(ELECTRON,CURRENT,current,NORMALIZED);
}

void TContact::comp_equil_electron_conc(void)
{
	flag grid_effects=(flag)contact_node->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);
	prec equil_dos=contact_node->get_value(FREE_ELECTRON,EQUIL_DOS,NORMALIZED);
	prec equil_planck_potential=contact_node->get_value(ELECTRON,EQUIL_PLANCK_POT,NORMALIZED);

	if (grid_effects & GRID_FERMI_DIRAC)
		equil_electron_conc=equil_dos*fermi_integral_1_half(equil_planck_potential);
	else
		equil_electron_conc=equil_dos*exp(equil_planck_potential);
}

void TContact::comp_hole_current(void)
{
	TNode *next_node, *prev_node;
	prec temp_next, temp_prev;
	prec mass_next, mass_prev;
	prec grad_band, grad_mass, grad_temp;
	prec current;
	prec planck_potential;
	prec mobility;
	prec element_length;
	long collision_factor;
	flag grid_effects;
	prec fermi_ratio_1_half_minus_1_half;
	prec fermi_ratio_3_half_1_half_col;
	prec bernoulli_param;

	if (second_node_number>contact_node_number) {
		next_node=second_node;
		prev_node=contact_node;
	}
	else {
		next_node=contact_node;
		prev_node=second_node;
	}

	element_length=next_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED)-
				   prev_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	grid_effects=(flag)next_node->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);

	temp_next=next_node->get_value(HOLE,TEMPERATURE,NORMALIZED);
	temp_prev=prev_node->get_value(HOLE,TEMPERATURE,NORMALIZED);

	mobility=contact_node->get_value(FREE_HOLE,MOBILITY,NORMALIZED);

	mass_next=next_node->get_value(HOLE,DOS_MASS,NORMALIZED);
	mass_prev=prev_node->get_value(HOLE,DOS_MASS,NORMALIZED);

	planck_potential=(next_node->get_value(HOLE,PLANCK_POT,NORMALIZED)+
					  prev_node->get_value(HOLE,PLANCK_POT,NORMALIZED))/2.0;

	collision_factor=(long)(prev_node->get_value(HOLE,COLLISION_FACTOR,NORMALIZED)*2.0);

	if (grid_effects & GRID_FERMI_DIRAC) {
		fermi_ratio_1_half_minus_1_half=fermi_integral_1_half(planck_potential)/
										fermi_integral_minus_1_half(planck_potential);

		switch(collision_factor) {
			case -1:
				fermi_ratio_3_half_1_half_col=fermi_integral_2_half(planck_potential)/
											  fermi_integral_0_half(planck_potential);
				break;
			case 0:
				fermi_ratio_3_half_1_half_col=fermi_integral_3_half(planck_potential)/
											  fermi_integral_1_half(planck_potential);
				break;
			case 1:
				fermi_ratio_3_half_1_half_col=fermi_integral_4_half(planck_potential)/
											  fermi_integral_2_half(planck_potential);
				break;
			case 3:
				fermi_ratio_3_half_1_half_col=fermi_integral_6_half(planck_potential)/
											  fermi_integral_4_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else {
		fermi_ratio_1_half_minus_1_half=1.0;
		fermi_ratio_3_half_1_half_col=1.0;
	}

	grad_temp=((2.5+(prec)collision_factor/2.0)*fermi_ratio_3_half_1_half_col/fermi_ratio_1_half_minus_1_half-1.5)*
			   (temp_next-temp_prev);

	grad_band=(next_node->get_value(HOLE,BAND_EDGE,NORMALIZED)-
			   prev_node->get_value(HOLE,BAND_EDGE,NORMALIZED))/fermi_ratio_1_half_minus_1_half;

	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	bernoulli_param=-grad_temp+grad_band+grad_mass;

	current=-mobility*fermi_ratio_1_half_minus_1_half/element_length*
			 (next_node->get_value(HOLE,CONCENTRATION,NORMALIZED)*temp_next*
			  bernoulli(bernoulli_param/temp_next)-
			  prev_node->get_value(HOLE,CONCENTRATION,NORMALIZED)*temp_prev*
			  bernoulli(-bernoulli_param/temp_prev));

	contact_node->put_value(HOLE,CURRENT,current,NORMALIZED);
}

void TContact::comp_equil_hole_conc(void)
{
	flag grid_effects=(flag)contact_node->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);
	prec equil_dos=contact_node->get_value(FREE_HOLE,EQUIL_DOS,NORMALIZED);
	prec equil_planck_potential=contact_node->get_value(HOLE,EQUIL_PLANCK_POT,NORMALIZED);

	if (grid_effects & GRID_FERMI_DIRAC)
		equil_hole_conc=equil_dos*fermi_integral_1_half(equil_planck_potential);
	else
		equil_hole_conc=equil_dos*exp(equil_planck_potential);
}

void TContact::comp_contact_field(void)
{
	TNode *next_node, *prev_node;
	prec field;

	if (second_node_number>contact_node_number) {
		next_node=second_node;
		prev_node=contact_node;
	}
	else {
		next_node=contact_node;
		prev_node=second_node;
	}

	field=(prev_node->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED)-
		   next_node->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED))/
		  (next_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED)-
		   prev_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED));

	contact_node->put_value(GRID_ELECTRICAL,FIELD,field,NORMALIZED);
}

void TContact::comp_value(flag flag_value)
{
	switch(flag_value) {
		case ELECTRON_CURRENT: comp_electron_current(); return;
		case EQUIL_ELECTRON_CONC: comp_equil_electron_conc(); return;
		case HOLE_CURRENT: comp_hole_current(); return;
		case EQUIL_HOLE_CONC: comp_equil_hole_conc(); return;
		case FIELD: comp_contact_field(); return;
		case BUILT_IN_POT: comp_built_pot(); return;
		default: assert(FALSE); return;
	}
}

prec TContact::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EFFECTS: return_value=(prec)effects; break;
		case NODE_NUMBER: return_value=contact_node->get_value(GRID_ELECTRICAL,NODE_NUMBER,NORMALIZED); break;
		case POSITION: return_value=contact_node->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED); break;
		case APPLIED_BIAS: return_value=(prec)bias; break;
		case ELECTRON_RECOMB_VEL: return_value=electron_recomb_vel; break;
		case HOLE_RECOMB_VEL: return_value=hole_recomb_vel; break;
		case ELECTRON_CURRENT: return_value=contact_node->get_value(ELECTRON,CURRENT,NORMALIZED); break;
		case EQUIL_ELECTRON_CONC: return_value=equil_electron_conc; break;
		case HOLE_CURRENT: return_value=contact_node->get_value(HOLE,CURRENT,NORMALIZED); break;
		case EQUIL_HOLE_CONC: return_value=equil_hole_conc; break;
		case FIELD: return_value=contact_node->get_value(GRID_ELECTRICAL,FIELD,NORMALIZED);break;
		case TOTAL_CURRENT: return_value=contact_node->get_value(ELECTRON,CURRENT,NORMALIZED)+
										 contact_node->get_value(HOLE,CURRENT,NORMALIZED); break;
		case BUILT_IN_POT: return_value=built_in_pot; break;
        case BARRIER_HEIGHT: return_value=barrier_height; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(CONTACT,flag_value);
	return(return_value);
}

void TContact::put_value(flag flag_value, prec value, ScaleType scale)
{
	prec prev_value;
	if (scale==UNNORMALIZED) value/=get_normalize_value(CONTACT,flag_value);

	switch(flag_value) {
		case EFFECTS:
			environment.set_effects_change_flags(CONTACT,effects^(flag)value);
			effects=(flag)value;
			return;
		case APPLIED_BIAS: bias=(float)value; return;
		case ELECTRON_RECOMB_VEL: electron_recomb_vel=value; return;
		case HOLE_RECOMB_VEL: hole_recomb_vel=value; return;
		case ELECTRON_CURRENT: contact_node->put_value(ELECTRON,CURRENT,value,NORMALIZED); return;
		case EQUIL_ELECTRON_CONC: equil_electron_conc=value; return;
		case HOLE_CURRENT: contact_node->put_value(HOLE,CURRENT,value,NORMALIZED); return;
		case EQUIL_HOLE_CONC: equil_hole_conc=value; return;
		case FIELD: contact_node->put_value(GRID_ELECTRICAL,FIELD,value,NORMALIZED); return;
		case BUILT_IN_POT: built_in_pot=value; return;
        case BARRIER_HEIGHT:
        	prev_value=barrier_height;
        	barrier_height=value;
            if (prev_value!=value) environment.set_update_flags(CONTACT,BARRIER_HEIGHT);
            return;
		default: assert(FALSE); return;
	}
}

void TContact::read_state_file(FILE *file_ptr)
{
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&bias,sizeof(bias),1,file_ptr);
	fread(&built_in_pot,sizeof(built_in_pot),1,file_ptr);
	fread(&electron_recomb_vel,sizeof(electron_recomb_vel),1,file_ptr);
	fread(&equil_electron_conc,sizeof(equil_electron_conc),1,file_ptr);
	fread(&hole_recomb_vel,sizeof(hole_recomb_vel),1,file_ptr);
	fread(&equil_hole_conc,sizeof(equil_hole_conc),1,file_ptr);
    fread(&barrier_height,sizeof(barrier_height),1,file_ptr);
}

void TContact::write_state_file(FILE *file_ptr)
{
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&bias,sizeof(bias),1,file_ptr);
	fwrite(&built_in_pot,sizeof(built_in_pot),1,file_ptr);
	fwrite(&electron_recomb_vel,sizeof(electron_recomb_vel),1,file_ptr);
	fwrite(&equil_electron_conc,sizeof(equil_electron_conc),1,file_ptr);
	fwrite(&hole_recomb_vel,sizeof(hole_recomb_vel),1,file_ptr);
	fwrite(&equil_hole_conc,sizeof(equil_hole_conc),1,file_ptr);
    fwrite(&barrier_height,sizeof(barrier_height),1,file_ptr);
}

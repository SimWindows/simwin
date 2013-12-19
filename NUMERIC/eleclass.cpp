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
#include "simdev.h"
#include "simelem.h"

/************************************* class TElement *****************************************

class TElement {
protected:
	TDevice *device_ptr;
	RegionType type;
	flag grid_effects;
	flag device_effects;
	TNode* prev_node;
	TNode* next_node;

public:
	TElement(RegionType region_type, TDevice *device, TNode* node_1, TNode* node_2);
	void get_effects(void);
};
*/

TElement::TElement(RegionType region_type,TDevice *device, TNode* node_1, TNode* node_2)
{
	device_ptr=device;
	prev_node=node_1;
	next_node=node_2;

	type=region_type;
}

void TElement::get_effects(void)
{
	if (prev_node==NULL) grid_effects=(flag)next_node->get_value(GRID_ELECTRICAL,EFFECTS);
	else {
		if (next_node==NULL) grid_effects=(flag)prev_node->get_value(GRID_ELECTRICAL,EFFECTS);
		else
			grid_effects=(flag)prev_node->get_value(GRID_ELECTRICAL,EFFECTS) &
						 (flag)next_node->get_value(GRID_ELECTRICAL,EFFECTS);
	}
	device_effects=device_ptr->get_value(DEVICE,EFFECTS);
}

/**********************************  class TElectricalServices ******************************

class TElectricalServices {
protected:
	TDevice *device_ptr;
	TNode** grid_ptr;
	TNode* prev_node;
	TNode* next_node;
	prec length;
	prec electron_mobility_length;
	prec hole_mobility_length;
	logical elec_therm_emis_current;
	logical hole_therm_emis_current;
	prec elec_scat_length;
	prec hole_scat_length;
	int elec_transmit_max_node;
	int hole_transmit_max_node;
	ElecDriftDiffParam ec;
	ElecDriftDiffParam ev;
	ThermEmisParam ec_therm;
	ThermEmisParam ev_therm;
public:
	TElectricalServices(TDevice* device, TNode** grid, TNode* node_1, TNode* node_2);

	void comp_length(void);
	void comp_elec_mobil_length(void);
	void comp_hole_mobil_length(void);
	void comp_elec_scat_length(void);
	void comp_hole_scat_length(void);
	void comp_elec_transmit_max_node(void);
	void comp_hole_transmit_max_node(void);
	prec comp_elec_transmit_min_energy(void);
	prec comp_hole_transmit_min_energy(void);
	void comp_conduction_discont(void);
	void comp_conduction_richardson(void);
	prec comp_conduction_transmission(prec energy);
	void comp_valence_discont(void);
	void comp_valence_richardson(void);
	prec comp_valence_transmission(prec energy);
	void comp_cond_drift_diff_param(flag grid_effects);
	void comp_val_drift_diff_param(flag grid_effects);
	void comp_cond_therm_emis_param(flag grid_effects);
	void comp_val_therm_emis_param(flag grid_effects);
};
*/

TElectricalServices::TElectricalServices(TDevice* device, TNode** grid, TNode* node_1, TNode* node_2)
{
	device_ptr=device;
	grid_ptr=grid;
	prev_node=node_1;
	next_node=node_2;
	comp_length();
}

void TElectricalServices::comp_length(void)
{
	length=next_node->position-prev_node->position;
}

void TElectricalServices::comp_elec_mobil_length(void)
{
	electron_mobility_length=(next_node->TFreeElectron::mobility+prev_node->TFreeElectron::mobility)/
							 (2.0*length);
}

void TElectricalServices::comp_hole_mobil_length(void)
{
	hole_mobility_length=(next_node->TFreeHole::mobility+prev_node->TFreeHole::mobility)/
						 (2.0*length);
}

void TElectricalServices::comp_elec_scat_length(void)
{
	prec temp,mass;

	temp=(next_node->TElectron::temperature+prev_node->TElectron::temperature)*normalization.temp/2.0;
	mass=(next_node->TElectron::cond_mass+prev_node->TElectron::cond_mass)*SIM_mo/2.0;

	elec_scat_length=electron_mobility_length*length*normalization.mobility*1e-2*
					 sqrt(mass*SIM_k*temp)/SIM_q;

	elec_scat_length/=normalization.length;
}

void TElectricalServices::comp_hole_scat_length(void)
{
	prec temp,mass;

	temp=(next_node->THole::temperature+prev_node->THole::temperature)*normalization.temp/2.0;
	mass=(next_node->THole::cond_mass+prev_node->THole::cond_mass)*SIM_mo/2.0;

	hole_scat_length=hole_mobility_length*length*normalization.mobility*1e-2*
					 sqrt(mass*SIM_k*temp)/SIM_q;

	hole_scat_length/=normalization.length;
}

void TElectricalServices::comp_elec_transmit_max_node(void)
{
	if (ec_therm.band_discont>0) elec_transmit_max_node=device_ptr->get_node(prev_node->position-0.02/
																			 get_normalize_value(GRID_ELECTRICAL,POSITION),
																			 -1,-1,NORMALIZED);
	else elec_transmit_max_node=device_ptr->get_node(prev_node->position+0.02/
													 get_normalize_value(GRID_ELECTRICAL,POSITION),
													 -1,-1,NORMALIZED);

}

void TElectricalServices::comp_hole_transmit_max_node(void)
{
	if (ev_therm.band_discont>0) hole_transmit_max_node=device_ptr->get_node(prev_node->position+0.02/
																			 get_normalize_value(GRID_ELECTRICAL,POSITION),
																			 -1,-1,NORMALIZED);
	else hole_transmit_max_node=device_ptr->get_node(prev_node->position-0.02/
													 get_normalize_value(GRID_ELECTRICAL,POSITION),
													 -1,-1,NORMALIZED);

}

prec TElectricalServices::comp_elec_transmit_min_energy(void)
{
	TNode** temp_ptr;
	prec start_value,next_value;
	prec result;

	if (elec_transmit_max_node<=prev_node->node_number) {
		start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*prev_node->TElectron::stored_temperature;
		result=start_value;
		temp_ptr=grid_ptr+prev_node->node_number;
		do {
			next_value=(*temp_ptr)->TElectron::band_edge;
			if (next_value<result) result=next_value;
			temp_ptr--;
		} while ((temp_ptr>grid_ptr+elec_transmit_max_node) && (result>next_node->TElectron::band_edge));
		result-=start_value;
	}
	else {
		start_value=next_node->TElectron::band_edge+ec_therm.barrier_next*next_node->TElectron::stored_temperature;
		result=start_value;
		temp_ptr=grid_ptr+next_node->node_number;
		do {
			next_value=(*temp_ptr)->TElectron::band_edge;
			if (next_value<result) result=next_value;
			temp_ptr++;
		} while ((temp_ptr<grid_ptr+elec_transmit_max_node) && (result>prev_node->TElectron::band_edge));
		result-=start_value;
	}

	assert(result<=0.0);

	return(result);
}

prec TElectricalServices::comp_hole_transmit_min_energy(void)
{
	TNode** temp_ptr;
	prec start_value,next_value;
	prec result;

	if (hole_transmit_max_node<=prev_node->node_number) {
		start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*prev_node->THole::stored_temperature;
		result=start_value;
		temp_ptr=grid_ptr+prev_node->node_number;
		do {
			next_value=(*temp_ptr)->THole::band_edge;
			if (next_value>result) result=next_value;
			temp_ptr--;
		} while ((temp_ptr>grid_ptr+hole_transmit_max_node) && (result<next_node->THole::band_edge));
		result=start_value-result;
	}
	else {
		start_value=next_node->THole::band_edge-ev_therm.barrier_next*next_node->THole::stored_temperature;
		result=start_value;
		temp_ptr=grid_ptr+next_node->node_number;
		do {
			next_value=(*temp_ptr)->THole::band_edge;
			if (next_value>result) result=next_value;
			temp_ptr++;
		} while ((temp_ptr<grid_ptr+hole_transmit_max_node) && (result<prev_node->THole::band_edge));
		result=start_value-result;
	}

	assert(result<=0.0);

	return(result);
}

void TElectricalServices::comp_conduction_discont(void)
{
	ec_therm.band_discont=next_node->electron_affinity-prev_node->electron_affinity;
}

void TElectricalServices::comp_conduction_richardson(void)
{
	prec mass_next, mass_prev, light_mass;

	mass_next=next_node->TElectron::dos_mass;
	mass_prev=prev_node->TElectron::dos_mass;

	if (mass_next<mass_prev) light_mass=(prec) mass_next;
	else light_mass=(prec)mass_prev;

	ec_therm.richardson_const=(SIM_q*SIM_mo*light_mass*sq(SIM_k)/(2.0*sq(SIM_pi)*sq(SIM_hb)*SIM_hb*1e4))*sq(normalization.temp)
							  /normalization.current;
}

prec TElectricalServices::comp_conduction_transmission(prec energy)
{
	TNode** temp_ptr;
	prec prev_k_value,next_k_value;
	prec prev_position,next_position;
	prec prev_band_edge, next_band_edge;
	prec integral=0;
	prec result;

	if (elec_transmit_max_node<=prev_node->node_number) {
		prev_position=(prev_node->position+next_node->position)/2.0;
		prev_band_edge=prev_node->TElectron::band_edge+ec_therm.barrier_prev*prev_node->TElectron::stored_temperature;
		prev_k_value=sqrt(prev_node->TElectron::dos_mass*(prev_band_edge-energy));
		temp_ptr=grid_ptr+prev_node->node_number;
		while (((*temp_ptr)->TElectron::band_edge>=energy) && ((*temp_ptr)->node_number>=elec_transmit_max_node)) {
			next_position=(*temp_ptr)->position;
			next_band_edge=(*temp_ptr)->TElectron::band_edge;
			next_k_value=sqrt((*temp_ptr)->TElectron::dos_mass*(next_band_edge-energy));
			integral+=(next_k_value+prev_k_value)*(prev_position-next_position)/2.0;
			prev_band_edge=next_band_edge;
			prev_position=next_position;
			prev_k_value=next_k_value;
			temp_ptr--;
		}

		if ((*temp_ptr)->TElectron::band_edge<energy)
			integral+=0.5*prev_k_value*(prev_band_edge-energy)/(prev_band_edge-(*temp_ptr)->TElectron::band_edge)*
					  (prev_position-(*temp_ptr)->position);

	}
	else {
		prev_position=(prev_node->position+next_node->position)/2.0;
		prev_band_edge=prev_node->TElectron::band_edge+ec_therm.barrier_prev*prev_node->TElectron::stored_temperature;
		prev_k_value=sqrt(next_node->TElectron::dos_mass*(prev_band_edge-energy));
		temp_ptr=grid_ptr+next_node->node_number;
		while (((*temp_ptr)->TElectron::band_edge>=energy) && ((*temp_ptr)->node_number<=elec_transmit_max_node)) {
			next_position=(*temp_ptr)->position;
			next_band_edge=(*temp_ptr)->TElectron::band_edge;
			next_k_value=sqrt((*temp_ptr)->TElectron::dos_mass*(next_band_edge-energy));
			integral+=(next_k_value+prev_k_value)*(next_position-prev_position)/2.0;
			prev_position=next_position;
			prev_band_edge=next_band_edge;
			prev_k_value=next_k_value;
			temp_ptr++;
		}

		if ((*temp_ptr)->TElectron::band_edge<energy)
			integral+=0.5*prev_k_value*(prev_band_edge-energy)/(prev_band_edge-(*temp_ptr)->TElectron::band_edge)*
					  ((*temp_ptr)->position-prev_position);

	}

	result=exp(-2.0*integral*sqrt(2.0*SIM_mo*SIM_q*get_normalize_value(ELECTRON,BAND_EDGE))*
								  get_normalize_value(GRID_ELECTRICAL,POSITION)*1e-6/SIM_hb);
	assert((result<=1.0) && (result>=0.0));

	return(result);
}

void TElectricalServices::comp_valence_discont(void)
{
	ev_therm.band_discont=next_node->electron_affinity+next_node->band_gap-
						  prev_node->electron_affinity-prev_node->band_gap;
}

void TElectricalServices::comp_valence_richardson(void)
{
	prec mass_next,mass_prev,light_mass;

	mass_next=next_node->THole::dos_mass;
	mass_prev=prev_node->THole::dos_mass;

	if (mass_next<mass_prev) light_mass=(prec) mass_next;
	else light_mass=(prec)mass_prev;

	ev_therm.richardson_const=(SIM_q*SIM_mo*light_mass*sq(SIM_k)/(2.0*sq(SIM_pi)*sq(SIM_hb)*SIM_hb*1e4))*sq(normalization.temp)
							   /normalization.current;
}

prec TElectricalServices::comp_valence_transmission(prec energy)
{
	TNode** temp_ptr;
	prec prev_k_value,next_k_value;
	prec prev_band_edge, next_band_edge;
	prec prev_position,next_position;
	prec integral=0;
	prec result;

	if (hole_transmit_max_node<=prev_node->node_number) {
		prev_position=(prev_node->position+next_node->position)/2.0;
		prev_band_edge=prev_node->THole::band_edge-ev_therm.barrier_prev*prev_node->THole::stored_temperature;
		prev_k_value=sqrt(prev_node->THole::dos_mass*(energy-prev_band_edge));
		temp_ptr=grid_ptr+prev_node->node_number;
		while (((*temp_ptr)->THole::band_edge<=energy) && ((*temp_ptr)->node_number>=hole_transmit_max_node)) {
			next_position=(*temp_ptr)->position;
			next_band_edge=(*temp_ptr)->THole::band_edge;
			next_k_value=sqrt((*temp_ptr)->THole::dos_mass*(energy-next_band_edge));
			integral+=(next_k_value+prev_k_value)*(prev_position-next_position)/2.0;
			prev_position=next_position;
			prev_band_edge=next_band_edge;
			prev_k_value=next_k_value;
			temp_ptr--;
		}

		if ((*temp_ptr)->THole::band_edge>energy)
			integral+=0.5*prev_k_value*(energy-prev_band_edge)/((*temp_ptr)->THole::band_edge-prev_band_edge)*
					  (prev_position-(*temp_ptr)->position);
	}
	else {
		prev_position=(prev_node->position+next_node->position)/2.0;
		prev_band_edge=next_node->THole::band_edge-ev_therm.barrier_next*next_node->THole::stored_temperature;
		prev_k_value=sqrt(next_node->THole::dos_mass*(energy-prev_band_edge));
		temp_ptr=grid_ptr+next_node->node_number;
		while (((*temp_ptr)->THole::band_edge<=energy) && ((*temp_ptr)->node_number<=hole_transmit_max_node)) {
			next_position=(*temp_ptr)->position;
			next_band_edge=(*temp_ptr)->THole::band_edge;
			next_k_value=sqrt((*temp_ptr)->THole::dos_mass*(energy-next_band_edge));
			integral+=(next_k_value+prev_k_value)*(next_position-prev_position)/2.0;
			prev_position=next_position;
			prev_band_edge=next_band_edge;
			prev_k_value=next_k_value;
			temp_ptr++;
		}

		if ((*temp_ptr)->THole::band_edge>energy)
			integral+=0.5*prev_k_value*(energy-prev_band_edge)/((*temp_ptr)->THole::band_edge-prev_band_edge)*
					  ((*temp_ptr)->position-prev_position);
	}

	result=exp(-2.0*integral*sqrt(2.0*SIM_mo*SIM_q*get_normalize_value(HOLE,BAND_EDGE))*
							 get_normalize_value(GRID_ELECTRICAL,POSITION)*1e-6/SIM_hb);
	assert((result<=1.0) && (result>=0.0));

	return(result);
}

void TElectricalServices::comp_cond_drift_diff_param(flag grid_effects)
{
	prec grad_ec, grad_mass, grad_temp;
	prec temp_next=next_node->TElectron::temperature;
	prec temp_prev=prev_node->TElectron::temperature;
	prec mass_next=next_node->TElectron::dos_mass;
	prec mass_prev=prev_node->TElectron::dos_mass;
	prec planck_potential=(next_node->TElectron::planck_potential+prev_node->TElectron::planck_potential)/2.0;
	long collision_factor=prev_node->TElectron::collision_factor;

	if (grid_effects & GRID_FERMI_DIRAC) {
		ec.fermi_ratio_1_half_minus_1_half=fermi_integral_1_half(planck_potential)/
										   fermi_integral_minus_1_half(planck_potential);

		switch(collision_factor) {
			case -1:
				ec.fermi_ratio_3_half_1_half_col=fermi_integral_2_half(planck_potential)/
												 fermi_integral_0_half(planck_potential);
				break;
			case 0:
				ec.fermi_ratio_3_half_1_half_col=fermi_integral_3_half(planck_potential)/
												 fermi_integral_1_half(planck_potential);
				break;
			case 1:
				ec.fermi_ratio_3_half_1_half_col=fermi_integral_4_half(planck_potential)/
												 fermi_integral_2_half(planck_potential);
				break;
			case 3:
				ec.fermi_ratio_3_half_1_half_col=fermi_integral_6_half(planck_potential)/
												 fermi_integral_4_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else {
		ec.fermi_ratio_1_half_minus_1_half=1.0;
		ec.fermi_ratio_3_half_1_half_col=1.0;
	}

	grad_temp=((2.5+(prec)collision_factor/2.0)*ec.fermi_ratio_3_half_1_half_col/ec.fermi_ratio_1_half_minus_1_half-1.5)*
			   (temp_next-temp_prev);


	grad_ec=(next_node->TElectron::band_edge-prev_node->TElectron::band_edge)/ec.fermi_ratio_1_half_minus_1_half;
	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	ec.bernoulli_param=-grad_temp-grad_ec+grad_mass;

	ec.bernoulli_grad_temp_next=bernoulli(ec.bernoulli_param/temp_next);
	ec.bernoulli_grad_temp_prev=bernoulli(-ec.bernoulli_param/temp_prev);

	ec.deriv_bern_grad_temp_next=deriv_bernoulli(ec.bernoulli_param/temp_next);
	ec.deriv_bern_grad_temp_prev=deriv_bernoulli(-ec.bernoulli_param/temp_prev);
}

void TElectricalServices::comp_val_drift_diff_param(flag grid_effects)
{
	prec grad_ev, grad_mass, grad_temp;
	prec temp_next=next_node->THole::temperature;
	prec temp_prev=prev_node->THole::temperature;
	prec mass_next=next_node->THole::dos_mass;
	prec mass_prev=prev_node->THole::dos_mass;
	prec planck_potential=(next_node->THole::planck_potential+prev_node->THole::planck_potential)/2.0;
	long collision_factor=prev_node->THole::collision_factor;

	if (grid_effects & GRID_FERMI_DIRAC) {
		ev.fermi_ratio_1_half_minus_1_half=fermi_integral_1_half(planck_potential)/
										   fermi_integral_minus_1_half(planck_potential);

		switch(collision_factor) {
			case -1:
				ev.fermi_ratio_3_half_1_half_col=fermi_integral_2_half(planck_potential)/
												 fermi_integral_0_half(planck_potential);
				break;
			case 0:
				ev.fermi_ratio_3_half_1_half_col=fermi_integral_3_half(planck_potential)/
												 fermi_integral_1_half(planck_potential);
				break;
			case 1:
				ev.fermi_ratio_3_half_1_half_col=fermi_integral_4_half(planck_potential)/
												 fermi_integral_2_half(planck_potential);
				break;
			case 3:
				ev.fermi_ratio_3_half_1_half_col=fermi_integral_6_half(planck_potential)/
												 fermi_integral_4_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else {
		ev.fermi_ratio_1_half_minus_1_half=1.0;
		ev.fermi_ratio_3_half_1_half_col=1.0;
	}

	grad_temp=((2.5+(prec)collision_factor/2.0)*ev.fermi_ratio_3_half_1_half_col/ev.fermi_ratio_1_half_minus_1_half-1.5)*
			   (temp_next-temp_prev);


	grad_ev=(next_node->THole::band_edge-prev_node->THole::band_edge)/ev.fermi_ratio_1_half_minus_1_half;
	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	ev.bernoulli_param=-grad_temp+grad_ev+grad_mass;

	ev.bernoulli_grad_temp_next=bernoulli(ev.bernoulli_param/temp_next);
	ev.bernoulli_grad_temp_prev=bernoulli(-ev.bernoulli_param/temp_prev);

	ev.deriv_bern_grad_temp_next=deriv_bernoulli(ev.bernoulli_param/temp_next);
	ev.deriv_bern_grad_temp_prev=deriv_bernoulli(-ev.bernoulli_param/temp_prev);
}

void TElectricalServices::comp_cond_therm_emis_param(flag grid_effects)
{
	if (ec_therm.band_discont<0) {
		ec_therm.barrier_prev=-ec_therm.band_discont;
		ec_therm.barrier_next=0.0;
	}
	else {
		ec_therm.barrier_prev=0.0;
		ec_therm.barrier_next=ec_therm.band_discont;
	}

	ec_therm.barrier_prev-=0.5*(next_node->potential-prev_node->potential);
	ec_therm.barrier_next-=0.5*(prev_node->potential-next_node->potential);

	ec_therm.barrier_prev/=prev_node->TElectron::stored_temperature;
	ec_therm.barrier_next/=next_node->TElectron::stored_temperature;

	if (grid_effects & GRID_TUNNELING) ec_therm.min_transmit_energy=comp_elec_transmit_min_energy();
}

void TElectricalServices::comp_val_therm_emis_param(flag grid_effects)
{
	if (ev_therm.band_discont<0) {
		ev_therm.barrier_prev=0.0;
		ev_therm.barrier_next=-ev_therm.band_discont;
	}
	else {
		ev_therm.barrier_prev=ev_therm.band_discont;
		ev_therm.barrier_next=0.0;
	}

	ev_therm.barrier_prev+=0.5*(next_node->potential-prev_node->potential);
	ev_therm.barrier_next+=0.5*(prev_node->potential-next_node->potential);

	ev_therm.barrier_prev/=prev_node->THole::stored_temperature;
	ev_therm.barrier_next/=next_node->THole::stored_temperature;

	if (grid_effects & GRID_TUNNELING) ev_therm.min_transmit_energy=comp_hole_transmit_min_energy();
}




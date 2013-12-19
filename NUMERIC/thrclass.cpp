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
#include "simelem.h"
#include "simdev.h"
#include "simthele.h"

/********************************** class TThermalElement *************************************

class TThermalElement: public TElement {
public:
	TThermalElement(RegionType region_type,TDevice *device, TNode* node_1, TNode* node_2)
		: TElement(region_type,device,node_1,node_2) {}
	virtual ~TThermalElement(void) {}

	virtual void update(prec update_lattice_temp, prec update_electron_temp,
						prec update_hole_temp);
	virtual void outer_update(prec clamp_value, prec relaxation_value);
	virtual void update_sub_nodes(void) {}

	virtual void apply_boundary(void) {}
	virtual void comp_boundary_param(void) {}

	virtual void comp_dependent_param(void) {}
	virtual void comp_independent_param(void) {}

	virtual void comp_thermoelectric_param(void)=0;

	virtual prec comp_lat_heat_flow(void)=0;
	virtual prec comp_deriv_lat_heat_flow(NodeSide node)=0;
	virtual prec comp_trans_heat_flow(void)=0;
	virtual prec comp_deriv_trans_heat_flow(NodeSide node)=0;
	virtual prec comp_electron_heat_flow(void)=0;
	virtual prec comp_deriv_electron_heat_flow(NodeSide node)=0;
	virtual prec comp_hole_heat_flow(void)=0;
	virtual prec comp_deriv_hole_heat_flow(NodeSide node)=0;
	virtual prec comp_integral_rad_heat(ElementSide side)=0;
	virtual prec comp_integral_electron_hotcarriers(ElementSide side)=0;
	virtual prec comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node)=0;
};
*/

void TThermalElement::update(prec update_lattice_temp, prec update_electron_temp,
							 prec update_hole_temp)
{
	if (next_node->TElectron::temperature>update_electron_temp)
		next_node->TElectron::temperature-=update_electron_temp;
	else
		next_node->TElectron::temperature/=2.0;

	if (next_node->THole::temperature>update_hole_temp)
		next_node->THole::temperature-=update_hole_temp;
	else
		next_node->THole::temperature/=2.0;

	if (next_node->lattice_temp>update_lattice_temp)
		next_node->lattice_temp-=update_lattice_temp;
	else
		next_node->lattice_temp/2.0;
}

void TThermalElement::outer_update(prec clamp_value, prec relaxation_value)
{
	prec electron_update, hole_update, lattice_update;

	electron_update=next_node->TElectron::stored_temperature-next_node->TElectron::temperature;
	hole_update=next_node->THole::stored_temperature-next_node->THole::temperature;
	lattice_update=next_node->stored_lattice_temp-next_node->lattice_temp;

	if (clamp_value!=0.0) {
		if (fabs(electron_update)*relaxation_value<clamp_value)
			next_node->TElectron::temperature=next_node->TElectron::stored_temperature-electron_update*relaxation_value;
		else {
			if (electron_update>0) next_node->TElectron::temperature=next_node->TElectron::stored_temperature-clamp_value;
			else next_node->TElectron::temperature=next_node->TElectron::stored_temperature+clamp_value;
		}

		if (fabs(hole_update)*relaxation_value<clamp_value)
			next_node->THole::temperature=next_node->THole::stored_temperature-hole_update*relaxation_value;
		else {
			if (hole_update>0) next_node->THole::temperature=next_node->THole::stored_temperature-clamp_value;
			else next_node->THole::temperature=next_node->THole::stored_temperature+clamp_value;
		}

		if (fabs(lattice_update)*relaxation_value<clamp_value)
			next_node->lattice_temp=next_node->stored_lattice_temp-lattice_update*relaxation_value;
		else {
			if (lattice_update>0) next_node->lattice_temp=next_node->stored_lattice_temp-clamp_value;
			else next_node->lattice_temp=next_node->stored_lattice_temp+clamp_value;
		}
	}
	else {
		next_node->TElectron::temperature=next_node->TElectron::stored_temperature-electron_update*relaxation_value;
		next_node->THole::temperature=next_node->THole::stored_temperature-hole_update*relaxation_value;
		next_node->lattice_temp=next_node->stored_lattice_temp-lattice_update*relaxation_value;
	}
}

/********************************* class TBulkThermalElement *********************************

class TBulkThermalElement: public TThermalElement, public TElectricalServices {
private:
	TNode *prev_node;
	TNode *next_node;
	prec therm_cond_length;
	ThermDriftDiffParam therm_ec;
	ThermDriftDiffParam therm_ev;
	prec pos_electron_emis_curr;
	prec neg_electron_emis_curr;
	prec pos_hole_emis_curr;
	prec neg_hole_emis_curr;
	prec electron_current;
	prec hole_current;
public:
	TBulkThermalElement(TDevice *device_ptr, TNode** grid, TNode* node_1, TNode* node_2)
		: TThermalElement(BULK,device_ptr,node_1, node_2),
		  TElectricalServices(device_ptr, grid, node_1, node_2) { prev_node=node_1; next_node=node_2; comp_length(); }
	virtual ~TBulkThermalElement(void) {}

	virtual void comp_dependent_param(void);
	virtual void comp_independent_param(void);

	virtual void comp_thermoelectric_param(void);

private:
	void comp_therm_cond_length(void);
	void comp_therm_cond_drift_diff_param(void);
	void comp_therm_val_drift_diff_param(void);
	void comp_electron_current(void);
	void comp_hole_current(void);

public:
	virtual prec comp_lat_heat_flow(void);
	virtual prec comp_deriv_lat_heat_flow(NodeSide node);
	virtual prec comp_trans_heat_flow(void);
	virtual prec comp_deriv_trans_heat_flow(NodeSide node);
	virtual prec comp_electron_heat_flow(void);
	virtual prec comp_deriv_electron_heat_flow(NodeSide node);
	virtual prec comp_hole_heat_flow(void);
	virtual prec comp_deriv_hole_heat_flow(NodeSide node);
	virtual prec comp_integral_rad_heat(ElementSide side);
	virtual prec comp_integral_electron_hotcarriers(ElementSide side);
	virtual prec comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node);
};
*/

void TBulkThermalElement::comp_dependent_param(void)
{
	comp_therm_cond_length();
}

void TBulkThermalElement::comp_independent_param(void)
{
	comp_elec_mobil_length();
	comp_hole_mobil_length();
	comp_conduction_discont();
	if (ec_therm.band_discont) {
		comp_conduction_richardson();
		comp_elec_transmit_max_node();
	}
	comp_valence_discont();
	if (ev_therm.band_discont) {
		comp_valence_richardson();
		comp_hole_transmit_max_node();
	}
}

void TBulkThermalElement::comp_thermoelectric_param(void)
{

	prec test_length;

	elec_therm_emis_current=FALSE;
	hole_therm_emis_current=FALSE;

	if ((ec_therm.band_discont != 0) && ((grid_effects & GRID_THERMIONIC) || (grid_effects & GRID_TUNNELING))) {
		if (grid_effects & GRID_ABRUPT_MATERIALS) elec_therm_emis_current=TRUE;
		else {
			comp_elec_scat_length();
			test_length=(prev_node->TElectron::temperature+next_node->TElectron::temperature)*length/fabs(ec_therm.band_discont);
			if (test_length<elec_scat_length) {
				elec_therm_emis_current=TRUE;
			}
		}
	}

	if ((ev_therm.band_discont != 0) && ((grid_effects & GRID_THERMIONIC) || (grid_effects & GRID_TUNNELING))) {
		if (grid_effects & GRID_ABRUPT_MATERIALS) hole_therm_emis_current=TRUE;
		else {
			comp_hole_scat_length();
			test_length=(prev_node->THole::temperature+next_node->THole::temperature)*length/fabs(ev_therm.band_discont);
			if (test_length<hole_scat_length) {
				hole_therm_emis_current=TRUE;
			}
		}
	}

	if (elec_therm_emis_current) comp_cond_therm_emis_param(grid_effects);
	else {
		comp_cond_drift_diff_param(grid_effects);
		comp_therm_cond_drift_diff_param();
	}
	comp_electron_current();

	if (hole_therm_emis_current) comp_val_therm_emis_param(grid_effects);
	else {
		comp_val_drift_diff_param(grid_effects);
		comp_therm_val_drift_diff_param();
	}
	comp_hole_current();

}

void TBulkThermalElement::comp_therm_cond_length(void)
{
	therm_cond_length=(prev_node->thermal_conduct+next_node->thermal_conduct)/(2.0*length);
}

void TBulkThermalElement::comp_therm_cond_drift_diff_param(void)
{
	prec grad_ec, grad_mass, grad_temp;
	prec temp_next=next_node->TElectron::temperature;
	prec temp_prev=prev_node->TElectron::temperature;
	prec mass_next=next_node->TElectron::dos_mass;
	prec mass_prev=prev_node->TElectron::dos_mass;
	prec planck_potential=(next_node->TElectron::planck_potential+prev_node->TElectron::planck_potential)/2.0;
	long collision_factor=prev_node->TElectron::collision_factor;

	if (grid_effects & GRID_FERMI_DIRAC) {
		switch(collision_factor) {
			case -1:
				therm_ec.fermi_ratio_5_half_3_half_col=fermi_integral_4_half(planck_potential)/
													   fermi_integral_2_half(planck_potential);
				break;
			case 0:
				therm_ec.fermi_ratio_5_half_3_half_col=fermi_integral_5_half(planck_potential)/
													   fermi_integral_3_half(planck_potential);
				break;
			case 1:
				therm_ec.fermi_ratio_5_half_3_half_col=fermi_integral_6_half(planck_potential)/
													   fermi_integral_4_half(planck_potential);
				break;
			case 3:
				therm_ec.fermi_ratio_5_half_3_half_col=fermi_integral_8_half(planck_potential)/
													   fermi_integral_6_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else therm_ec.fermi_ratio_5_half_3_half_col=1.0;

	grad_temp=((3.5+(prec)collision_factor/2.0)*therm_ec.fermi_ratio_5_half_3_half_col/ec.fermi_ratio_1_half_minus_1_half-2.5)*
			   (temp_next-temp_prev);

	grad_ec=(next_node->TElectron::band_edge-prev_node->TElectron::band_edge)/ec.fermi_ratio_1_half_minus_1_half;

	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	therm_ec.bernoulli_param=-grad_temp-grad_ec+grad_mass;

	therm_ec.bernoulli_grad_temp_next=bernoulli(therm_ec.bernoulli_param/temp_next);
	therm_ec.bernoulli_grad_temp_prev=bernoulli(-therm_ec.bernoulli_param/temp_prev);
}

void TBulkThermalElement::comp_therm_val_drift_diff_param(void)
{
	prec grad_ev, grad_mass, grad_temp;
	prec temp_next=next_node->THole::temperature;
	prec temp_prev=prev_node->THole::temperature;
	prec mass_next=next_node->THole::dos_mass;
	prec mass_prev=prev_node->THole::dos_mass;
	prec planck_potential=(next_node->THole::planck_potential+prev_node->THole::planck_potential)/2.0;
	long collision_factor=prev_node->THole::collision_factor;

	if (grid_effects & GRID_FERMI_DIRAC) {
		switch(collision_factor) {
			case -1:
				therm_ev.fermi_ratio_5_half_3_half_col=fermi_integral_4_half(planck_potential)/
													   fermi_integral_2_half(planck_potential);
				break;
			case 0:
				therm_ev.fermi_ratio_5_half_3_half_col=fermi_integral_5_half(planck_potential)/
													   fermi_integral_3_half(planck_potential);
				break;
			case 1:
				therm_ev.fermi_ratio_5_half_3_half_col=fermi_integral_6_half(planck_potential)/
													   fermi_integral_4_half(planck_potential);
				break;
			case 3:
				therm_ev.fermi_ratio_5_half_3_half_col=fermi_integral_8_half(planck_potential)/
													   fermi_integral_6_half(planck_potential);
				break;
			default: assert(FALSE); break;
		}
	}
	else therm_ev.fermi_ratio_5_half_3_half_col=1.0;

	grad_temp=((3.5+(prec)collision_factor/2.0)*therm_ev.fermi_ratio_5_half_3_half_col/ev.fermi_ratio_1_half_minus_1_half-2.5)*
			   (temp_next-temp_prev);

	grad_ev=(next_node->THole::band_edge-prev_node->THole::band_edge)/ev.fermi_ratio_1_half_minus_1_half;

	grad_mass=1.5*(temp_next+temp_prev)*((mass_next-mass_prev)/(mass_next+mass_prev));

	therm_ev.bernoulli_param=-grad_temp+grad_ev+grad_mass;

	therm_ev.bernoulli_grad_temp_next=bernoulli(therm_ev.bernoulli_param/temp_next);
	therm_ev.bernoulli_grad_temp_prev=bernoulli(-therm_ev.bernoulli_param/temp_prev);
}

void TBulkThermalElement::comp_electron_current(void)
{
	prec temp_next, temp_prev;
	prec n_next, n_prev;
	prec planck_next, planck_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_pos_transmit_value, next_pos_transmit_value;
	prec prev_neg_transmit_value, next_neg_transmit_value;
	prec transmit_coef;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	if (elec_therm_emis_current) {

		planck_next=next_node->TElectron::planck_potential;
		planck_prev=prev_node->TElectron::planck_potential;

		pos_electron_emis_curr=0.0;
		neg_electron_emis_curr=0.0;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				neg_electron_emis_curr=-ec_therm.richardson_const*sq(temp_prev)*
													(0.5*pow(log_1_x(exp(planck_prev-ec_therm.barrier_prev)),2.0)
													 +dilog(1.0/(1.0+exp(ec_therm.barrier_prev-planck_prev))));
				pos_electron_emis_curr=-ec_therm.richardson_const*sq(temp_next)*
													(0.5*pow(log_1_x(exp(planck_next-ec_therm.barrier_next)),2.0)
													 +dilog(1.0/(1.0+exp(ec_therm.barrier_next-planck_next))));
			}
			else {
				neg_electron_emis_curr=-ec_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ec_therm.barrier_prev);
				pos_electron_emis_curr=-ec_therm.richardson_const*sq(temp_next)*exp(planck_next-ec_therm.barrier_next);
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ec_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				prev_neg_transmit_value=-temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev));
				prev_pos_transmit_value=-temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next));
				start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
				end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy>=end_value;
					 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
					transmit_coef=comp_conduction_transmission(transmit_energy);
					next_neg_transmit_value=-transmit_coef*temp_prev*log_1_x(exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/temp_prev));
					next_pos_transmit_value=-transmit_coef*temp_next*log_1_x(exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/temp_next));
					neg_electron_emis_curr+=ec_therm.richardson_const*(next_neg_transmit_value+prev_neg_transmit_value)/2.0*
																	  (-ec_therm.min_transmit_energy/25.0);
					pos_electron_emis_curr+=ec_therm.richardson_const*(next_pos_transmit_value+prev_pos_transmit_value)/2.0*
																	  (-ec_therm.min_transmit_energy/25.0);
					prev_pos_transmit_value=next_pos_transmit_value;
					prev_neg_transmit_value=next_neg_transmit_value;
				}
			}
			else {
				prev_neg_transmit_value=-temp_prev*exp(planck_prev-ec_therm.barrier_prev);
				prev_pos_transmit_value=-temp_next*exp(planck_next-ec_therm.barrier_next);
				start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
				end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy>=end_value;
					 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
					transmit_coef=comp_conduction_transmission(transmit_energy);
					next_neg_transmit_value=-transmit_coef*temp_prev*exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/temp_prev);
					next_pos_transmit_value=-transmit_coef*temp_next*exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/temp_next);
					neg_electron_emis_curr+=ec_therm.richardson_const*(next_neg_transmit_value+prev_neg_transmit_value)/2.0*
																	  (-ec_therm.min_transmit_energy/25.0);
					pos_electron_emis_curr+=ec_therm.richardson_const*(next_pos_transmit_value+prev_pos_transmit_value)/2.0*
																	  (-ec_therm.min_transmit_energy/25.0);
					prev_pos_transmit_value=next_pos_transmit_value;
					prev_neg_transmit_value=next_neg_transmit_value;
				}
			}
		}

		electron_current=neg_electron_emis_curr-pos_electron_emis_curr;
	}
	else {
		n_next=next_node->TElectron::total_conc;
		n_prev=prev_node->TElectron::total_conc;

		pos_electron_emis_curr=0.0;
		neg_electron_emis_curr=0.0;

		electron_current=electron_mobility_length*ec.fermi_ratio_1_half_minus_1_half*
						(n_next*temp_next*ec.bernoulli_grad_temp_next
						-n_prev*temp_prev*ec.bernoulli_grad_temp_prev);
	}
}

void TBulkThermalElement::comp_hole_current(void)
{
	prec temp_next, temp_prev;
	prec p_next, p_prev;
	prec planck_next, planck_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_pos_transmit_value, next_pos_transmit_value;
	prec prev_neg_transmit_value, next_neg_transmit_value;
	prec transmit_coef;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	if (hole_therm_emis_current) {

		planck_next=next_node->THole::planck_potential;
		planck_prev=prev_node->THole::planck_potential;

		neg_hole_emis_curr=0.0;
		pos_hole_emis_curr=0.0;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				neg_hole_emis_curr=ev_therm.richardson_const*sq(temp_prev)*
														(0.5*pow(log_1_x(exp(planck_prev-ev_therm.barrier_prev)),2.0)
														 +dilog(1.0/(1.0+exp(ev_therm.barrier_prev-planck_prev))));
				pos_hole_emis_curr=ev_therm.richardson_const*sq(temp_next)*
														(0.5*pow(log_1_x(exp(planck_next-ev_therm.barrier_next)),2.0)
														 +dilog(1.0/(1.0+exp(ev_therm.barrier_next-planck_next))));
			}
			else {
				neg_hole_emis_curr=ev_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ev_therm.barrier_prev);
				pos_hole_emis_curr=ev_therm.richardson_const*sq(temp_next)*exp(planck_next-ev_therm.barrier_next);
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ev_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				prev_neg_transmit_value=temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev));
				prev_pos_transmit_value=temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next));
				start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
				end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy<=end_value;
					 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
					transmit_coef=comp_valence_transmission(transmit_energy);
					next_neg_transmit_value=transmit_coef*temp_prev*log_1_x(exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/temp_prev));
					next_pos_transmit_value=transmit_coef*temp_next*log_1_x(exp(planck_next-(next_node->THole::band_edge-transmit_energy)/temp_next));
					neg_hole_emis_curr+=ev_therm.richardson_const*(next_neg_transmit_value+prev_neg_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
					pos_hole_emis_curr+=ev_therm.richardson_const*(next_pos_transmit_value+prev_pos_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
					prev_neg_transmit_value=next_neg_transmit_value;
					prev_pos_transmit_value=next_pos_transmit_value;
				}
			}
			else {
				prev_neg_transmit_value=temp_prev*exp(planck_prev-ev_therm.barrier_prev);
				prev_pos_transmit_value=temp_next*exp(planck_next-ev_therm.barrier_next);
				start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
				end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy<=end_value;
					 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
					transmit_coef=comp_valence_transmission(transmit_energy);
					next_neg_transmit_value=transmit_coef*temp_prev*exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/temp_prev);
					next_pos_transmit_value=transmit_coef*temp_next*exp(planck_next-(next_node->THole::band_edge-transmit_energy)/temp_next);
					neg_hole_emis_curr+=ev_therm.richardson_const*(next_neg_transmit_value+prev_neg_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
					pos_hole_emis_curr+=ev_therm.richardson_const*(next_pos_transmit_value+prev_pos_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
					prev_neg_transmit_value=next_neg_transmit_value;
					prev_pos_transmit_value=next_pos_transmit_value;
				}
			}
		}

		hole_current=neg_hole_emis_curr-pos_hole_emis_curr;
	}
	else {
		p_next=next_node->THole::total_conc;
		p_prev=prev_node->THole::total_conc;

		pos_hole_emis_curr=0.0;
		neg_hole_emis_curr=0.0;

		hole_current=-hole_mobility_length*ev.fermi_ratio_1_half_minus_1_half*
					  (p_next*temp_next*ev.bernoulli_grad_temp_next
					  -p_prev*temp_prev*ev.bernoulli_grad_temp_prev);
	}
}

prec TBulkThermalElement::comp_lat_heat_flow(void)
{
	return((prev_node->lattice_temp-next_node->lattice_temp)*therm_cond_length);
}

prec TBulkThermalElement::comp_deriv_lat_heat_flow(NodeSide node)
{
	prec result;

	result=0.0;

	switch(node) {
		case PREVIOUS_NODE:
			result=therm_cond_length;
			if (grid_effects & GRID_TEMP_THERMAL_COND) {
				result+=(prev_node->lattice_temp-next_node->lattice_temp)*prev_node->deriv_therm_cond_temp/(2.0*length);
			}
			break;
		case NEXT_NODE:
			result=-therm_cond_length;
			if (grid_effects & GRID_TEMP_THERMAL_COND) {
				result+=(prev_node->lattice_temp-next_node->lattice_temp)*next_node->deriv_therm_cond_temp/(2.0*length);
			}
			break;
	}

	return(result);
}

prec TBulkThermalElement::comp_trans_heat_flow(void)
{
	prec result;
	prec environ_temp;

	if (grid_effects & GRID_LATERAL_HEAT) {
		environ_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);
		result=length*((next_node->lateral_thermal_conduct+prev_node->lateral_thermal_conduct)/2.0)*
					 (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
	}
	else result=0.0;

	return(result);
}

prec TBulkThermalElement::comp_deriv_trans_heat_flow(NodeSide node)
{
	prec result,environ_temp;

	if (grid_effects & GRID_LATERAL_HEAT) {
		environ_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);

		result=length*((next_node->lateral_thermal_conduct+prev_node->lateral_thermal_conduct)/4.0);

		if (grid_effects & GRID_TEMP_THERMAL_COND) {
			if (node==PREVIOUS_NODE)
				result+=length*(prev_node->deriv_lateral_cond_temp/2.0)*
							   (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
			else
				result+=length*(next_node->deriv_lateral_cond_temp/2.0)*
							   (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
		}
	}
	else result=0.0;

	return(result);
}

prec TBulkThermalElement::comp_electron_heat_flow(void)
{
	prec electron_heat_flow;
	prec n_next,n_prev;
	prec temp_next, temp_prev;
	prec stored_temp_next, stored_temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_transmit_value, next_transmit_value;
	prec prev_exp_transmit_value, next_exp_transmit_value;
	long collision_factor;

	temp_prev=prev_node->TElectron::temperature;
	temp_next=next_node->TElectron::temperature;

	stored_temp_prev=prev_node->TElectron::stored_temperature;
	stored_temp_next=next_node->TElectron::stored_temperature;

	electron_heat_flow=0.0;

	if (elec_therm_emis_current) {
		if (grid_effects & GRID_JOULE_HEAT) {
			electron_heat_flow+=-prev_node->TElectron::band_edge*neg_electron_emis_curr
								+next_node->TElectron::band_edge*pos_electron_emis_curr;
		}

		if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
			planck_next=next_node->TElectron::planck_potential;
			planck_prev=prev_node->TElectron::planck_potential;

			exp_barrier_next=exp(ec_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			exp_barrier_prev=exp(ec_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			if (grid_effects & GRID_THERMIONIC) {
				if (grid_effects & GRID_FERMI_DIRAC) {
					electron_heat_flow+=ec_therm.richardson_const*sq(temp_prev)*temp_prev*
							(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
							 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_prev+
							 sq(ec_therm.barrier_prev)*ec_therm.barrier_prev/6.0+
							 ec_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
							 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
							 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
													 (1.5*ec_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
													 (ec_therm.barrier_prev*(ec_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

					electron_heat_flow-=ec_therm.richardson_const*sq(temp_next)*temp_next*
							(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
							 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_next+
							 sq(ec_therm.barrier_next)*ec_therm.barrier_next/6.0+
							 ec_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
							 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
							 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
													 (1.5*ec_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
													 (ec_therm.barrier_next*(ec_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));

				}
				else {
					electron_heat_flow+=ec_therm.richardson_const*sq(temp_prev)*temp_prev*
										(2.0+ec_therm.barrier_prev)/exp_barrier_prev;
					electron_heat_flow-=ec_therm.richardson_const*sq(temp_next)*temp_next*
										(2.0+ec_therm.barrier_next)/exp_barrier_next;
				}
			}

			if ((grid_effects & GRID_TUNNELING) && ec_therm.min_transmit_energy) {
				if (grid_effects & GRID_FERMI_DIRAC) {
					prev_transmit_value=+sq(temp_prev)*((planck_prev-log_1_x(1.0/exp_barrier_prev))*log_1_x(1.0/exp_barrier_prev)
														+dilog(1.0/(1.0+exp_barrier_prev))+log_1_div_1_x(exp_barrier_prev)*log_1_div_1_x(1.0/exp_barrier_prev))
										-sq(temp_next)*((planck_next-log_1_x(1.0/exp_barrier_next))*log_1_x(1.0/exp_barrier_next)
														+dilog(1.0/(1.0+exp_barrier_next))+log_1_div_1_x(exp_barrier_next)*log_1_div_1_x(1.0/exp_barrier_next));
					start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
					end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
					for (transmit_energy=start_value; transmit_energy>=end_value;
						 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
						prev_exp_transmit_value=exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev);
						next_exp_transmit_value=exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next);
						next_transmit_value=comp_conduction_transmission(transmit_energy)*
											(+sq(temp_prev)*((planck_prev-log_1_x(prev_exp_transmit_value))*log_1_x(prev_exp_transmit_value)
															 +dilog(1.0/(1.0+1.0/prev_exp_transmit_value))+log_1_div_1_x(1.0/prev_exp_transmit_value)*log_1_div_1_x(prev_exp_transmit_value))
											 -sq(temp_next)*((planck_next-log_1_x(next_exp_transmit_value))*log_1_x(next_exp_transmit_value)
															 +dilog(1.0/(1.0+1.0/next_exp_transmit_value))+log_1_div_1_x(1.0/next_exp_transmit_value)*log_1_div_1_x(next_exp_transmit_value)));
						electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
														   (-ec_therm.min_transmit_energy/25.0);
						prev_transmit_value=next_transmit_value;
					}
				}
				else {
					prev_transmit_value=+sq(temp_prev)*(1.0+ec_therm.barrier_prev)*exp(planck_prev-ec_therm.barrier_prev)
										-sq(temp_next)*(1.0+ec_therm.barrier_next)*exp(planck_next-ec_therm.barrier_next);
					start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
					end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
					for (transmit_energy=start_value; transmit_energy>=end_value;
						 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
						next_transmit_value=comp_conduction_transmission(transmit_energy)*
										   (+sq(temp_prev)*(1.0+(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev)*
														   (exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev))
											-sq(temp_next)*(1.0+(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next)*
														   (exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next)));
						electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
														   (-ec_therm.min_transmit_energy/25.0);
						prev_transmit_value=next_transmit_value;
					}
				}
			}
		}
	}
	else {
		n_next=next_node->TElectron::total_conc;
		n_prev=prev_node->TElectron::total_conc;

		collision_factor=prev_node->TElectron::collision_factor;

		if (grid_effects & GRID_JOULE_HEAT) {
			electron_heat_flow+=-(prev_node->TElectron::band_edge+next_node->TElectron::band_edge)*electron_current/2.0;
		}

		if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
			electron_heat_flow+=-electron_mobility_length*(2.5+(prec)collision_factor/2.0)*
								 ec.fermi_ratio_3_half_1_half_col*ec.fermi_ratio_1_half_minus_1_half*
								(n_next*sq(temp_next)*therm_ec.bernoulli_grad_temp_next
								-n_prev*sq(temp_prev)*therm_ec.bernoulli_grad_temp_prev);
		}
	}

	return(electron_heat_flow);
}

prec TBulkThermalElement::comp_deriv_electron_heat_flow(NodeSide node)
{
	prec deriv_electron_heat_flow=0.0;
	prec n_next,n_prev;
	prec temp_next, temp_prev;
	prec stored_temp_next, stored_temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_transmit_value, next_transmit_value;
	prec prev_exp_transmit_value, next_exp_transmit_value;
	long collision_factor=prev_node->TElectron::collision_factor;

	temp_prev=prev_node->TElectron::temperature;
	temp_next=next_node->TElectron::temperature;

	stored_temp_prev=prev_node->TElectron::stored_temperature;
	stored_temp_next=next_node->TElectron::stored_temperature;

	n_next=next_node->TElectron::total_conc;
	n_prev=prev_node->TElectron::total_conc;

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		if (elec_therm_emis_current) {
			planck_prev=prev_node->TElectron::planck_potential;
			planck_next=next_node->TElectron::planck_potential;

			exp_barrier_prev=exp(ec_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			exp_barrier_next=exp(ec_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			if (grid_effects & GRID_THERMIONIC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							deriv_electron_heat_flow=3.0*ec_therm.richardson_const*sq(temp_prev)*
								(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
								 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_prev+
								 sq(ec_therm.barrier_prev)*ec_therm.barrier_prev/6.0+
								 ec_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
								 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
								 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
													 (1.5*ec_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
													 (ec_therm.barrier_prev*(ec_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

						}
						else {
							deriv_electron_heat_flow=3.0*ec_therm.richardson_const*sq(temp_prev)*
													 (2.0+ec_therm.barrier_prev)/exp_barrier_prev;
						}

						break;

					case NEXT_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							deriv_electron_heat_flow=-3.0*ec_therm.richardson_const*sq(temp_next)*
								(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
								 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_next+
								 sq(ec_therm.barrier_next)*ec_therm.barrier_next/6.0+
								 ec_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
								 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
								 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
													 (1.5*ec_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
													 (ec_therm.barrier_next*(ec_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
						}
						else {
							deriv_electron_heat_flow=-3.0*ec_therm.richardson_const*sq(temp_next)*
													  (2.0+ec_therm.barrier_next)/exp_barrier_next;
						}
						break;
				}
			}

			if ((grid_effects & GRID_TUNNELING) && ec_therm.min_transmit_energy) {
				switch (node) {
					case PREVIOUS_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							prev_transmit_value=+2.0*temp_prev*((planck_prev-log_1_x(1.0/exp_barrier_prev))*log_1_x(1.0/exp_barrier_prev)
																+dilog(1.0/(1.0+exp_barrier_prev))+log_1_div_1_x(exp_barrier_prev)*log_1_div_1_x(1.0/exp_barrier_prev));
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								prev_exp_transmit_value=exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev);
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
													(+2.0*temp_prev*((planck_prev-log_1_x(prev_exp_transmit_value))*log_1_x(prev_exp_transmit_value)
																	 +dilog(1.0/(1.0+1.0/prev_exp_transmit_value))+log_1_div_1_x(1.0/prev_exp_transmit_value)*log_1_div_1_x(prev_exp_transmit_value)));
								deriv_electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																   (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						else {
							prev_transmit_value=+2.0*temp_prev*(1.0+ec_therm.barrier_prev)*exp(planck_prev-ec_therm.barrier_prev);
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
													2.0*temp_prev*(1.0+(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev)*
																  (exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/stored_temp_prev));
								deriv_electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																   (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
					case NEXT_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							prev_transmit_value=-2.0*temp_next*((planck_next-log_1_x(1.0/exp_barrier_next))*log_1_x(1.0/exp_barrier_next)
																+dilog(1.0/(1.0+exp_barrier_next))+log_1_div_1_x(exp_barrier_next)*log_1_div_1_x(1.0/exp_barrier_next));
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_exp_transmit_value=exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next);
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
													(-2.0*temp_next*((planck_next-log_1_x(next_exp_transmit_value))*log_1_x(next_exp_transmit_value)
																	 +dilog(1.0/(1.0+1.0/next_exp_transmit_value))+log_1_div_1_x(1.0/next_exp_transmit_value)*log_1_div_1_x(next_exp_transmit_value)));
								deriv_electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																   (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						else {
							prev_transmit_value=-2.0*temp_next*(1.0+ec_therm.barrier_next)*exp(planck_next-ec_therm.barrier_next);
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*stored_temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
													(-2.0*sq(temp_next)*(1.0+(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next)*
																   (exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/stored_temp_next)));
								deriv_electron_heat_flow+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																   (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
				}
			}
		}
		else {
			switch(node) {
				case PREVIOUS_NODE:
					deriv_electron_heat_flow=2.0*electron_mobility_length*(2.5+(prec)collision_factor/2.0)*
											 ec.fermi_ratio_3_half_1_half_col*ec.fermi_ratio_1_half_minus_1_half*
											(n_prev*temp_prev*therm_ec.bernoulli_grad_temp_prev);
					break;

				case NEXT_NODE:
					deriv_electron_heat_flow=-2.0*electron_mobility_length*(2.5+(prec)collision_factor/2.0)*
											  ec.fermi_ratio_3_half_1_half_col*ec.fermi_ratio_1_half_minus_1_half*
											 (n_next*temp_next*therm_ec.bernoulli_grad_temp_next);
					break;
			}
		}
	}

	return(deriv_electron_heat_flow);
}

prec TBulkThermalElement::comp_hole_heat_flow(void)
{
	prec hole_heat_flow;
	prec p_next,p_prev;
	prec temp_next, temp_prev;
	prec stored_temp_next, stored_temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_transmit_value, next_transmit_value;
	prec prev_exp_transmit_value, next_exp_transmit_value;
	long collision_factor;

	temp_prev=prev_node->THole::temperature;
	temp_next=next_node->THole::temperature;

	stored_temp_prev=prev_node->THole::stored_temperature;
	stored_temp_next=next_node->THole::stored_temperature;

	hole_heat_flow=0.0;

	if (hole_therm_emis_current) {
		if (grid_effects & GRID_JOULE_HEAT) {
			hole_heat_flow+=-prev_node->THole::band_edge*neg_hole_emis_curr
							+next_node->THole::band_edge*pos_hole_emis_curr;
		}

		if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
			planck_next=next_node->THole::planck_potential;
			planck_prev=prev_node->THole::planck_potential;

			exp_barrier_next=exp(ev_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			exp_barrier_prev=exp(ev_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			if (grid_effects & GRID_THERMIONIC) {
				if (grid_effects & GRID_FERMI_DIRAC) {

					hole_heat_flow+=ev_therm.richardson_const*sq(temp_prev)*temp_prev*
							(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
							 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_prev+
							 sq(ev_therm.barrier_prev)*ev_therm.barrier_prev/6.0+
							 ev_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
							 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
							 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
													 (1.5*ev_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
													 (ev_therm.barrier_prev*(ev_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

					hole_heat_flow-=ev_therm.richardson_const*sq(temp_next)*temp_next*
							(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
							 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_next+
							 sq(ev_therm.barrier_next)*ev_therm.barrier_next/6.0+
							 ev_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
							 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
							 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
													 (1.5*ev_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
													 (ev_therm.barrier_next*(ev_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
				}
				else {
					hole_heat_flow+=ev_therm.richardson_const*sq(temp_prev)*temp_prev*
									(2.0+ev_therm.barrier_prev)/exp_barrier_prev;
					hole_heat_flow-=ev_therm.richardson_const*sq(temp_next)*temp_next*
									(2.0+ev_therm.barrier_next)/exp_barrier_next;
				}
			}

			if ((grid_effects & GRID_TUNNELING) && ev_therm.min_transmit_energy) {
				if (grid_effects & GRID_FERMI_DIRAC) {
					prev_transmit_value=+sq(temp_prev)*((planck_prev-log_1_x(1.0/exp_barrier_prev))*log_1_x(1.0/exp_barrier_prev)
														+dilog(1.0/(1.0+exp_barrier_prev))+log_1_div_1_x(exp_barrier_prev)*log_1_div_1_x(1.0/exp_barrier_prev))
										-sq(temp_next)*((planck_next-log_1_x(1.0/exp_barrier_next))*log_1_x(1.0/exp_barrier_next)
														+dilog(1.0/(1.0+exp_barrier_next))+log_1_div_1_x(exp_barrier_next)*log_1_div_1_x(1.0/exp_barrier_next));
					start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
					end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
					for (transmit_energy=start_value; transmit_energy<=end_value;
						 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
						prev_exp_transmit_value=exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev);
						next_exp_transmit_value=exp(planck_next-(next_node->THole::band_edge-transmit_energy)/stored_temp_next);
						next_transmit_value=comp_valence_transmission(transmit_energy)*
											(+sq(temp_prev)*((planck_prev-log_1_x(prev_exp_transmit_value))*log_1_x(prev_exp_transmit_value)
															 +dilog(1.0/(1.0+1.0/prev_exp_transmit_value))+log_1_div_1_x(1.0/prev_exp_transmit_value)*log_1_div_1_x(prev_exp_transmit_value))
											 -sq(temp_next)*((planck_next-log_1_x(next_exp_transmit_value))*log_1_x(next_exp_transmit_value)
															 +dilog(1.0/(1.0+1.0/next_exp_transmit_value))+log_1_div_1_x(1.0/next_exp_transmit_value)*log_1_div_1_x(next_exp_transmit_value)));
						hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
						prev_transmit_value=next_transmit_value;
					}
				}
				else {
					prev_transmit_value=+sq(temp_prev)*(1.0+ev_therm.barrier_prev)*exp(planck_prev-ev_therm.barrier_prev)
										-sq(temp_next)*(1.0+ev_therm.barrier_next)*exp(planck_next-ev_therm.barrier_next);
					start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
					end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
					for (transmit_energy=start_value; transmit_energy<=end_value;
						 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
						next_transmit_value=comp_valence_transmission(transmit_energy)*
										   (+sq(temp_prev)*(1.0+(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev)*
														   (exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev))
											-sq(temp_next)*(1.0+(next_node->THole::band_edge-transmit_energy)/stored_temp_next)*
														   (exp(planck_next-(next_node->THole::band_edge-transmit_energy)/stored_temp_next)));
						hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																  (-ev_therm.min_transmit_energy/25.0);
						prev_transmit_value=next_transmit_value;
					}
				}
			}
		}
	}
	else {
		p_next=next_node->THole::total_conc;
		p_prev=prev_node->THole::total_conc;

		collision_factor=prev_node->THole::collision_factor;

		if (grid_effects & GRID_JOULE_HEAT) {
			hole_heat_flow+=-(prev_node->THole::band_edge+next_node->THole::band_edge)*hole_current/2.0;
		}

		if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
			hole_heat_flow+=(-2.0*hole_mobility_length)*(2.5+(prec)collision_factor/2.0)*
							 ev.fermi_ratio_3_half_1_half_col*ev.fermi_ratio_1_half_minus_1_half*
							(p_next*sq(temp_next)*therm_ev.bernoulli_grad_temp_next
							-p_prev*sq(temp_prev)*therm_ev.bernoulli_grad_temp_prev);
		}
	}

	return(hole_heat_flow);
}

prec TBulkThermalElement::comp_deriv_hole_heat_flow(NodeSide node)
{
	prec deriv_hole_heat_flow=0.0;
	prec p_next,p_prev;
	prec temp_next, temp_prev;
	prec stored_temp_next, stored_temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;
	prec start_value, end_value;
	prec transmit_energy;
	prec prev_transmit_value, next_transmit_value;
	prec prev_exp_transmit_value, next_exp_transmit_value;
	long collision_factor=prev_node->THole::collision_factor;

	temp_prev=prev_node->THole::temperature;
	temp_next=next_node->THole::temperature;

	stored_temp_prev=prev_node->THole::stored_temperature;
	stored_temp_next=next_node->THole::stored_temperature;

	p_next=next_node->THole::total_conc;
	p_prev=prev_node->THole::total_conc;

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		if (hole_therm_emis_current) {
			planck_prev=prev_node->THole::planck_potential;
			planck_next=next_node->THole::planck_potential;

			exp_barrier_prev=exp(ev_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			exp_barrier_next=exp(ev_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			if (grid_effects & GRID_THERMIONIC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							deriv_hole_heat_flow=3.0*ev_therm.richardson_const*sq(temp_prev)*
								(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
								 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_prev+
								 sq(ev_therm.barrier_prev)*ev_therm.barrier_prev/6.0+
								 ev_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
								 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
								 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
													 (1.5*ev_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
													 (ev_therm.barrier_prev*(ev_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));
						}
						else {
							deriv_hole_heat_flow=3.0*ev_therm.richardson_const*sq(temp_prev)*
												 (2.0+ev_therm.barrier_prev)/exp_barrier_prev;
						}
						break;

					case NEXT_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							deriv_hole_heat_flow=-3.0*ev_therm.richardson_const*sq(temp_next)*
									(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
									 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_next+
									 sq(ev_therm.barrier_next)*ev_therm.barrier_next/6.0+
									 ev_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
									 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
									 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
													 (1.5*ev_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
													 (ev_therm.barrier_next*(ev_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
						}
						else {
							deriv_hole_heat_flow=-3.0*ev_therm.richardson_const*sq(temp_next)*
												  (2.0+ev_therm.barrier_next)/exp_barrier_next;
						}
						break;
				}
			}

			if ((grid_effects & GRID_TUNNELING) && ev_therm.min_transmit_energy) {
				switch (node) {
					case PREVIOUS_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							prev_transmit_value=+2.0*temp_prev*((planck_prev-log_1_x(1.0/exp_barrier_prev))*log_1_x(1.0/exp_barrier_prev)
																+dilog(1.0/(1.0+exp_barrier_prev))+log_1_div_1_x(exp_barrier_prev)*log_1_div_1_x(1.0/exp_barrier_prev));
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								prev_exp_transmit_value=exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev);
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													(+2.0*temp_prev*((planck_prev-log_1_x(prev_exp_transmit_value))*log_1_x(prev_exp_transmit_value)
																	 +dilog(1.0/(1.0+1.0/prev_exp_transmit_value))+log_1_div_1_x(1.0/prev_exp_transmit_value)*log_1_div_1_x(prev_exp_transmit_value)));
								deriv_hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																		  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						else {
							prev_transmit_value=2.0*temp_prev*(1.0+ev_therm.barrier_prev)*exp(planck_prev-ev_therm.barrier_prev);
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													2.0*temp_prev*(1.0+(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev)*
																  (exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/stored_temp_prev));
								deriv_hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																				(-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
					case NEXT_NODE:
						if (grid_effects & GRID_FERMI_DIRAC) {
							prev_transmit_value=-2.0*temp_next*((planck_next-log_1_x(1.0/exp_barrier_next))*log_1_x(1.0/exp_barrier_next)
																+dilog(1.0/(1.0+exp_barrier_next))+log_1_div_1_x(exp_barrier_next)*log_1_div_1_x(1.0/exp_barrier_next));
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_exp_transmit_value=exp(planck_next-(next_node->THole::band_edge-transmit_energy)/stored_temp_next);
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													(-2.0*temp_next*((planck_next-log_1_x(next_exp_transmit_value))*log_1_x(next_exp_transmit_value)
																	 +dilog(1.0/(1.0+1.0/next_exp_transmit_value))+log_1_div_1_x(1.0/next_exp_transmit_value)*log_1_div_1_x(next_exp_transmit_value)));
								deriv_hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																		  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						else {
							prev_transmit_value=-2.0*temp_next*(1.0+ev_therm.barrier_next)*exp(planck_next-ev_therm.barrier_next);
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*stored_temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													(-2.0*temp_next*(1.0+(next_node->THole::band_edge-transmit_energy)/stored_temp_next)*
																	(exp(planck_next-(next_node->THole::band_edge-transmit_energy)/stored_temp_next)));
								deriv_hole_heat_flow+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																		  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
				}
			}
		}
		else {
			switch (node) {
				case PREVIOUS_NODE:
					deriv_hole_heat_flow=2.0*hole_mobility_length*(2.5+(prec)collision_factor/2.0)*
										 ev.fermi_ratio_3_half_1_half_col*ev.fermi_ratio_1_half_minus_1_half*
										 (p_prev*temp_prev*therm_ev.bernoulli_grad_temp_prev);
					break;

				case NEXT_NODE:
					deriv_hole_heat_flow=-2.0*hole_mobility_length*(2.5+(prec)collision_factor/2.0)*
										 ev.fermi_ratio_3_half_1_half_col*ev.fermi_ratio_1_half_minus_1_half*
										 (p_next*temp_next*therm_ev.bernoulli_grad_temp_next);
					break;
			}
		}
	}

	return(deriv_hole_heat_flow);
}

prec TBulkThermalElement::comp_integral_rad_heat(ElementSide side)
{
	switch(side) {
		case FIRSTHALF:
			return(prev_node->radiative_heat.total*length/2.0);
		case SECONDHALF:
			return(next_node->radiative_heat.total*length/2.0);
		default: return(0);
	}
}

prec TBulkThermalElement::comp_integral_electron_hotcarriers(ElementSide side)
{
	switch(side) {
		case FIRSTHALF:
			return(prev_node->TElectron::hotcarriers.total*length/2.0);
		case SECONDHALF:
			return(next_node->TElectron::hotcarriers.total*length/2.0);
		default: return(0);
	}

}
prec TBulkThermalElement::comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node)
{
	TNode* node_ptr;
	prec integral_constant;

	if (node==PREVIOUS_NODE) node_ptr=prev_node;
	else node_ptr=next_node;

	if (((node==PREVIOUS_NODE) && (side==FIRSTHALF)) ||
		((node==NEXT_NODE) && (side==SECONDHALF)))
		integral_constant=length/2.0;
	else
		integral_constant=0.0;

	return(node_ptr->TElectron::total_deriv_hotcarriers*integral_constant);

}

/********************************** class TQWThermalElement **********************************

class TQWThermalElement: public TThermalElement, public TElectricalServices {
private:
	TNode** device_grid_ptr;
	TNode* qw_node;
	int qw_node_number;
	TNode* prev_node;
	int prev_node_number;
	TNode* next_node;
	int next_node_number;
	int mid_node_number;
	int number_sub_elements;
	prec therm_cond_length;
	prec *sub_length;
	prec length_fraction;
	prec pos_electron_emis_curr;
	prec neg_electron_emis_curr;
	prec pos_hole_emis_curr;
	prec neg_hole_emis_curr;
public:
	TQWThermalElement(TDevice *device_ptr, TNode** device_grid,
					  TNode* node_1, TNode* node_2);
	virtual ~TQWThermalElement(void);

	virtual void update_sub_nodes(void);

	virtual void comp_dependent_param(void);
	virtual void comp_independent_param(void);

	virtual void comp_thermoelectric_param(void);

private:
	void comp_therm_cond_length(void);
	void comp_sub_length(void);
	void comp_length_fraction(void);
	void comp_cond_therm_emis_param(void);
	void comp_val_therm_emis_param(void);
	void comp_electron_current(void);
	void comp_hole_current(void);

public:
	virtual prec comp_lat_heat_flow(void);
	virtual prec comp_deriv_lat_heat_flow(NodeSide node);
	virtual prec comp_trans_heat_flow(void);
	virtual prec comp_deriv_trans_heat_flow(NodeSide node);
	virtual prec comp_electron_heat_flow(void);
	virtual prec comp_deriv_electron_heat_flow(NodeSide node);
	virtual prec comp_hole_heat_flow(void);
	virtual prec comp_deriv_hole_heat_flow(NodeSide node);
	virtual prec comp_integral_rad_heat(ElementSide side);
	virtual prec comp_integral_electron_hotcarriers(ElementSide side);
	virtual prec comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node);
};
*/

TQWThermalElement::TQWThermalElement(TDevice *device_ptr, TNode** device_grid,
									 TNode* node_1, TNode* node_2)
	: TThermalElement(QW, device_ptr, node_1, node_2),
	  TElectricalServices(device_ptr, device_grid,node_1,node_2)
{
	device_grid_ptr=device_grid;

	prev_node=node_1;
	next_node=node_2;

	if (node_1->TGrid::region_type==QW)	qw_node=node_1;
	else qw_node=node_2;

	comp_length();

	qw_node_number=qw_node->node_number;
	prev_node_number=prev_node->node_number;
	next_node_number=next_node->node_number;
	mid_node_number=device_ptr->get_node((prev_node->position+next_node->position)/2.0,-1,-1,NORMALIZED);

	number_sub_elements=next_node_number-prev_node_number;

	sub_length=new prec[number_sub_elements];
}

TQWThermalElement::~TQWThermalElement(void)
{
	delete[] sub_length;
}

void TQWThermalElement::update_sub_nodes(void)
{
	int i;
	TNode **temp_ptr;
	prec temp_difference, start_temp;
	prec position, start_position;

	start_position=prev_node->position;
	start_temp=prev_node->lattice_temp;

	if (device_effects & DEVICE_SINGLE_TEMP) {
		temp_difference=next_node->lattice_temp-prev_node->lattice_temp;
		temp_ptr=device_grid_ptr+prev_node_number+1;
		for (i=prev_node_number+1;i<=next_node_number-1;i++) {
			position=(*temp_ptr)->position;
			(*temp_ptr)->lattice_temp=start_temp+temp_difference*(position-start_position)/length;
			(*temp_ptr)->TElectron::temperature=(*temp_ptr)->lattice_temp;
			(*temp_ptr)->THole::temperature=(*temp_ptr)->lattice_temp;
			temp_ptr++;
		}
	}
	else {
		if (device_effects & DEVICE_VARY_ELECTRON_TEMP) {
			temp_ptr=device_grid_ptr+prev_node_number+1;
			for (i=prev_node_number+1;i<=next_node_number-1;i++) {
				(*temp_ptr)->TElectron::temperature=(qw_node)->TElectron::temperature;
				temp_ptr++;
			}
		}
	}

	temp_ptr=device_grid_ptr+prev_node_number+1;
	for (i=prev_node_number+1;i<=next_node_number-1;i++) {
		(*temp_ptr)->init_value(ELECTRON,PLANCK_POT,qw_node,NULL);
		(*temp_ptr)->init_value(HOLE,PLANCK_POT,qw_node,NULL);
	}
}

void TQWThermalElement::comp_dependent_param(void)
{
	comp_therm_cond_length();
}

void TQWThermalElement::comp_independent_param(void)
{
	comp_sub_length();
	comp_length_fraction();
	comp_conduction_discont();
	comp_conduction_richardson();
	comp_valence_discont();
	comp_valence_richardson();
}

void TQWThermalElement::comp_thermoelectric_param(void)
{
	comp_cond_therm_emis_param();
	comp_electron_current();
	comp_val_therm_emis_param();
	comp_hole_current();
}

void TQWThermalElement::comp_therm_cond_length(void)
{
	therm_cond_length=qw_node->thermal_conduct/length;
}

void TQWThermalElement::comp_sub_length(void)
{
	int i;
	TNode** temp_ptr;

	temp_ptr=device_grid_ptr+prev_node_number;
	for (i=0;i<number_sub_elements;i++) {
		sub_length[i]=(*(temp_ptr+1))->position-(*temp_ptr)->position;
		temp_ptr++;
	}
}

void TQWThermalElement::comp_length_fraction(void)
{
	if (next_node_number==qw_node_number)
		length_fraction=1.0-sub_length[0]/length;
	else
		length_fraction=1.0-sub_length[number_sub_elements-1]/length;
//	length_fraction=0.5;
}

void TQWThermalElement::comp_cond_therm_emis_param(void)
{
	if (ec_therm.band_discont<0) {
		ec_therm.barrier_prev=-ec_therm.band_discont-length_fraction*(next_node->potential-prev_node->potential);
		ec_therm.barrier_next=-(1.0-length_fraction)*(prev_node->potential-next_node->potential);
	}
	else {
		ec_therm.barrier_prev=-(1.0-length_fraction)*(next_node->potential-prev_node->potential);
		ec_therm.barrier_next=ec_therm.band_discont-length_fraction*(prev_node->potential-next_node->potential);
	}

	ec_therm.barrier_prev/=prev_node->TElectron::temperature;
	ec_therm.barrier_next/=next_node->TElectron::temperature;
}

void TQWThermalElement::comp_val_therm_emis_param(void)
{
	if (ev_therm.band_discont<0) {
		ev_therm.barrier_prev=(1.0-length_fraction)*(next_node->potential-prev_node->potential);
		ev_therm.barrier_next=-ev_therm.band_discont+length_fraction*(prev_node->potential-next_node->potential);
	}
	else {
		ev_therm.barrier_prev=ev_therm.band_discont+length_fraction*(next_node->potential-prev_node->potential);
		ev_therm.barrier_next=(1.0-length_fraction)*(prev_node->potential-next_node->potential);
	}

	ev_therm.barrier_prev/=prev_node->THole::temperature;
	ev_therm.barrier_next/=next_node->THole::temperature;
}

void TQWThermalElement::comp_electron_current(void)
{
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	planck_next=next_node->TElectron::planck_potential;
	planck_prev=prev_node->TElectron::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		neg_electron_emis_curr=-ec_therm.richardson_const*sq(temp_prev)*
											(0.5*pow(log_1_x(exp(planck_prev-ec_therm.barrier_prev)),2.0)
											 +dilog(1.0/(1.0+exp(ec_therm.barrier_prev-planck_prev))));
		pos_electron_emis_curr=-ec_therm.richardson_const*sq(temp_next)*
											(0.5*pow(log_1_x(exp(planck_next-ec_therm.barrier_next)),2.0)
											 +dilog(1.0/(1.0+exp(ec_therm.barrier_next-planck_next))));
	}
	else {
		neg_electron_emis_curr=-ec_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ec_therm.barrier_prev);
		pos_electron_emis_curr=-ec_therm.richardson_const*sq(temp_next)*exp(planck_next-ec_therm.barrier_next);
	}
}

void TQWThermalElement::comp_hole_current(void)
{
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	planck_next=next_node->THole::planck_potential;
	planck_prev=prev_node->THole::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		neg_hole_emis_curr=ev_therm.richardson_const*sq(temp_prev)*
												(0.5*pow(log_1_x(exp(planck_prev-ev_therm.barrier_prev)),2.0)
												 +dilog(1.0/(1.0+exp(ev_therm.barrier_prev-planck_prev))));
		pos_hole_emis_curr=ev_therm.richardson_const*sq(temp_next)*
												(0.5*pow(log_1_x(exp(planck_next-ev_therm.barrier_next)),2.0)
												 +dilog(1.0/(1.0+exp(ev_therm.barrier_next-planck_next))));
	}
	else {
		neg_hole_emis_curr=ev_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ev_therm.barrier_prev);
		pos_hole_emis_curr=ev_therm.richardson_const*sq(temp_next)*exp(planck_next-ev_therm.barrier_next);
	}
}

prec TQWThermalElement::comp_lat_heat_flow(void)
{
	return((prev_node->lattice_temp-next_node->lattice_temp)*therm_cond_length);
}

prec TQWThermalElement::comp_deriv_lat_heat_flow(NodeSide node)
{
	prec result;

	result=0.0;

	switch(node) {
		case PREVIOUS_NODE:
			result=therm_cond_length;
			if ((grid_effects & GRID_TEMP_THERMAL_COND) && (prev_node==qw_node)) {
				result+=(prev_node->lattice_temp-next_node->lattice_temp)*qw_node->deriv_therm_cond_temp/length;
			}
			break;
		case NEXT_NODE:
			result=-therm_cond_length;
			if ((grid_effects & GRID_TEMP_THERMAL_COND) && (next_node==qw_node)) {
				result+=(prev_node->lattice_temp-next_node->lattice_temp)*qw_node->deriv_therm_cond_temp/length;
			}
			break;
	}

	return(result);
}

prec TQWThermalElement::comp_trans_heat_flow(void)
{
	prec result;
	prec environ_temp;

	if (grid_effects & GRID_LATERAL_HEAT) {
		environ_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);
		result=length*((next_node->lateral_thermal_conduct+prev_node->lateral_thermal_conduct)/2.0)*
					 (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
	}
	else result=0.0;

	return(result);
}

prec TQWThermalElement::comp_deriv_trans_heat_flow(NodeSide node)
{
	prec result,environ_temp;

	if (grid_effects & GRID_LATERAL_HEAT) {
		environ_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);

		result=length*((next_node->lateral_thermal_conduct+prev_node->lateral_thermal_conduct)/4.0);

		if (grid_effects & GRID_TEMP_THERMAL_COND) {
			if (node==PREVIOUS_NODE)
				result+=length*(prev_node->deriv_lateral_cond_temp/2.0)*
							   (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
			else
				result+=length*(next_node->deriv_lateral_cond_temp/2.0)*
							   (((next_node->lattice_temp+prev_node->lattice_temp)/2.0)-environ_temp);
		}
	}
	else result=0.0;

	return(result);
}

prec TQWThermalElement::comp_electron_heat_flow(void)
{
	prec electron_heat_flow;
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;

	electron_heat_flow=0.0;

	temp_prev=prev_node->TElectron::temperature;
	temp_next=next_node->TElectron::temperature;

	if (grid_effects & GRID_JOULE_HEAT) {
		electron_heat_flow+=-prev_node->TElectron::band_edge*neg_electron_emis_curr
							+next_node->TElectron::band_edge*pos_electron_emis_curr;
	}

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		if (grid_effects & GRID_FERMI_DIRAC) {
			planck_next=next_node->TElectron::planck_potential;
			planck_prev=prev_node->TElectron::planck_potential;

			exp_barrier_next=exp(ec_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			exp_barrier_prev=exp(ec_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			electron_heat_flow+=ec_therm.richardson_const*sq(temp_prev)*temp_prev*
					(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
					 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_prev+
					 sq(ec_therm.barrier_prev)*ec_therm.barrier_prev/6.0+
					 ec_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
					 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
					 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
											 (1.5*ec_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
											 (ec_therm.barrier_prev*(ec_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

			electron_heat_flow-=ec_therm.richardson_const*sq(temp_next)*temp_next*
					(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
					 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_next+
					 sq(ec_therm.barrier_next)*ec_therm.barrier_next/6.0+
					 ec_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
					 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
					 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
											 (1.5*ec_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
											 (ec_therm.barrier_next*(ec_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));

		}
		else {
			electron_heat_flow+=-(2.0+ec_therm.barrier_prev)*temp_prev*neg_electron_emis_curr
								+(2.0+ec_therm.barrier_next)*temp_next*pos_electron_emis_curr;
		}
	}

	return(electron_heat_flow);
}

prec TQWThermalElement::comp_deriv_electron_heat_flow(NodeSide node)
{
	prec deriv_electron_heat_flow;
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;

	temp_prev=prev_node->TElectron::temperature;
	temp_next=next_node->TElectron::temperature;

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		switch(node) {
			case PREVIOUS_NODE:
				if (grid_effects & GRID_FERMI_DIRAC) {
					planck_prev=prev_node->TElectron::planck_potential;

					exp_barrier_prev=exp(ec_therm.barrier_prev-planck_prev);
					log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

					deriv_electron_heat_flow=3.0*ec_therm.richardson_const*sq(temp_prev)*
						(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
						 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_prev+
						 sq(ec_therm.barrier_prev)*ec_therm.barrier_prev/6.0+
						 ec_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
						 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
						 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
											 (1.5*ec_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
											 (ec_therm.barrier_prev*(ec_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

				}
				else {
					deriv_electron_heat_flow=-(2.0+ec_therm.barrier_prev)*neg_electron_emis_curr;
				}
				break;
			case NEXT_NODE:
				if (grid_effects & GRID_FERMI_DIRAC) {
					planck_next=next_node->TElectron::planck_potential;

					exp_barrier_next=exp(ec_therm.barrier_next-planck_next);
					log_1_exp_barrier_next=log_1_x(exp_barrier_next);

					deriv_electron_heat_flow=-3.0*ec_therm.richardson_const*sq(temp_next)*
						(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
						 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ec_therm.barrier_next+
						 sq(ec_therm.barrier_next)*ec_therm.barrier_next/6.0+
						 ec_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
						 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
						 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
											 (1.5*ec_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
											 (ec_therm.barrier_next*(ec_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
				}
				else {
					deriv_electron_heat_flow=(2.0+ec_therm.barrier_next)*pos_electron_emis_curr;
				}
				break;
		}
	}
	return(deriv_electron_heat_flow);
}

prec TQWThermalElement::comp_hole_heat_flow(void)
{
	prec hole_heat_flow;
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;

	temp_prev=prev_node->THole::temperature;
	temp_next=next_node->THole::temperature;

	hole_heat_flow=0.0;

	if (grid_effects & GRID_JOULE_HEAT) {
		hole_heat_flow+=-prev_node->THole::band_edge*neg_hole_emis_curr
						+next_node->THole::band_edge*pos_hole_emis_curr;
	}

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		if (grid_effects & GRID_FERMI_DIRAC) {
			planck_next=next_node->THole::planck_potential;
			planck_prev=prev_node->THole::planck_potential;

			exp_barrier_next=exp(ev_therm.barrier_next-planck_next);
			log_1_exp_barrier_next=log_1_x(exp_barrier_next);

			exp_barrier_prev=exp(ev_therm.barrier_prev-planck_prev);
			log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

			hole_heat_flow+=ev_therm.richardson_const*sq(temp_prev)*temp_prev*
					(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
					 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_prev+
					 sq(ev_therm.barrier_prev)*ev_therm.barrier_prev/6.0+
					 ev_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
					 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
					 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
											 (1.5*ev_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
											 (ev_therm.barrier_prev*(ev_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));

			hole_heat_flow-=ev_therm.richardson_const*sq(temp_next)*temp_next*
					(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
					 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_next+
					 sq(ev_therm.barrier_next)*ev_therm.barrier_next/6.0+
					 ev_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
					 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
					 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
											 (1.5*ev_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
											 (ev_therm.barrier_next*(ev_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
		}
		else {
			hole_heat_flow+=(2.0+ev_therm.barrier_prev)*temp_prev*neg_hole_emis_curr
						   -(2.0+ev_therm.barrier_next)*temp_next*pos_hole_emis_curr;
		}
	}

	return(hole_heat_flow);
}

prec TQWThermalElement::comp_deriv_hole_heat_flow(NodeSide node)
{
	prec deriv_hole_heat_flow;
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec exp_barrier_next, exp_barrier_prev;
	prec log_1_exp_barrier_next, log_1_exp_barrier_prev;

	temp_prev=prev_node->THole::temperature;
	temp_next=next_node->THole::temperature;

	if (grid_effects & GRID_THERMOELECTRIC_HEAT) {
		switch(node) {
			case PREVIOUS_NODE:
				if (grid_effects & GRID_FERMI_DIRAC) {
					planck_prev=prev_node->THole::planck_potential;

					exp_barrier_prev=exp(ev_therm.barrier_prev-planck_prev);
					log_1_exp_barrier_prev=log_1_x(exp_barrier_prev);

					deriv_hole_heat_flow=3.0*ev_therm.richardson_const*sq(temp_prev)*
						(planck_prev*(sq(planck_prev)+sq(SIM_pi))/3.0-
						 (sq(planck_prev)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_prev+
						 sq(ev_therm.barrier_prev)*ev_therm.barrier_prev/6.0+
						 ev_therm.barrier_prev*dilog(1.0/(1.0+exp_barrier_prev))+
						 2.0*(trilog(1.0/(1.0+exp_barrier_prev))+trilog(1.0/(1.0+1.0/exp_barrier_prev))-SIM_trilog_1)-
						 log_1_exp_barrier_prev*(2.0*sq(log_1_exp_barrier_prev)/3.0-
											 (1.5*ev_therm.barrier_prev-planck_prev)*log_1_exp_barrier_prev+
											 (ev_therm.barrier_prev*(ev_therm.barrier_prev-planck_prev)-sq(SIM_pi)/3.0)));
				}
				else {
					deriv_hole_heat_flow=(2.0+ev_therm.barrier_prev)*neg_hole_emis_curr;
				}
				break;
			case NEXT_NODE:
				if (grid_effects & GRID_FERMI_DIRAC) {
					planck_next=next_node->THole::planck_potential;

					exp_barrier_next=exp(ev_therm.barrier_next-planck_next);
					log_1_exp_barrier_next=log_1_x(exp_barrier_next);

					deriv_hole_heat_flow=-3.0*ev_therm.richardson_const*sq(temp_next)*
							(planck_next*(sq(planck_next)+sq(SIM_pi))/3.0-
							 (sq(planck_next)/2.0+sq(SIM_pi)/3.0)*ev_therm.barrier_next+
							 sq(ev_therm.barrier_next)*ev_therm.barrier_next/6.0+
							 ev_therm.barrier_next*dilog(1.0/(1.0+exp_barrier_next))+
							 2.0*(trilog(1.0/(1.0+exp_barrier_next))+trilog(1.0/(1.0+1.0/exp_barrier_next))-SIM_trilog_1)-
							 log_1_exp_barrier_next*(2.0*sq(log_1_exp_barrier_next)/3.0-
											 (1.5*ev_therm.barrier_next-planck_next)*log_1_exp_barrier_next+
											 (ev_therm.barrier_next*(ev_therm.barrier_next-planck_next)-sq(SIM_pi)/3.0)));
				}
				else {
					deriv_hole_heat_flow=-(2.0+ev_therm.barrier_next)*pos_hole_emis_curr;
				}

				break;
		}
	}

	return(deriv_hole_heat_flow);
}

prec TQWThermalElement::comp_integral_rad_heat(ElementSide side)
{
	int i;
	prec result;
	TNode** temp_ptr;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				result=prev_node->radiative_heat.total*sub_length[0]/2.0;
				break;
			case SECONDHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->radiative_heat.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=next_node->radiative_heat.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	else {
		switch(side) {
			case FIRSTHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->radiative_heat.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=prev_node->radiative_heat.total*sub_length[number_sub_elements-1]/2.0;
				break;
			case SECONDHALF:
				result=next_node->radiative_heat.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	return(result);
}

prec TQWThermalElement::comp_integral_electron_hotcarriers(ElementSide side)
{
	int i;
	prec result;
	TNode** temp_ptr;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				result=prev_node->TElectron::hotcarriers.total*sub_length[0]/2.0;
				break;
			case SECONDHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->TElectron::hotcarriers.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=next_node->TElectron::hotcarriers.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	else {
		switch(side) {
			case FIRSTHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->TElectron::hotcarriers.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=prev_node->TElectron::hotcarriers.total*sub_length[number_sub_elements-1]/2.0;
				break;
			case SECONDHALF:
				result=next_node->TElectron::hotcarriers.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	return(result);
}
prec TQWThermalElement::comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node)
{
	int i;
	prec result=0.0;
	TNode** temp_ptr;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				switch(node) {
					case PREVIOUS_NODE:
						result=prev_node->TElectron::total_deriv_hotcarriers*sub_length[0]/2.0;
						break;

					case NEXT_NODE:
						result=0.0;
						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						result=0.0;

						break;
					case NEXT_NODE:
						temp_ptr=device_grid_ptr+prev_node_number+1;
						result=0.0;
						for (i=prev_node_number+1;i<next_node_number;i++) {
							result+=(*temp_ptr)->TElectron::total_deriv_hotcarriers*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
							temp_ptr++;
						}
						result+=next_node->TElectron::total_deriv_hotcarriers*sub_length[number_sub_elements-1]/2.0;

						break;
				}
				break;
		}
	}
	else {
		switch(side) {
			case FIRSTHALF:
				switch(node) {
					case PREVIOUS_NODE:
						temp_ptr=device_grid_ptr+prev_node_number+1;
						result=0.0;
						for (i=prev_node_number+1;i<next_node_number;i++) {
							result+=(*temp_ptr)->TElectron::total_deriv_hotcarriers*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
							temp_ptr++;
						}
						result+=prev_node->TElectron::total_deriv_hotcarriers*sub_length[number_sub_elements-1]/2.0;

						break;
					case NEXT_NODE:
						result=0.0;
						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						result=0.0;

						break;
					case NEXT_NODE:
						result=next_node->TElectron::total_deriv_hotcarriers*sub_length[number_sub_elements-1]/2.0;
						break;
				}
				break;
		}
	}
	return(result);
}

/***************************** class TBoundaryThermalElement **********************************

class TBoundaryThermalElement: public TThermalElement {
private:
	TNode *surface_node;
	int surface_node_number;
	int surface_number;

	prec thermal_conduct;
	prec surface_temp;
	prec surface_electron_temp;
	prec surface_hole_temp;
public:
	TBoundaryThermalElement(TDevice *device, TNode* node_1, TNode* node_2, int surface);
	virtual ~TBoundaryThermalElement(void) {}

	virtual void apply_boundary(void);
	virtual void comp_boundary_param(void);

	virtual void comp_thermoelectric_param(void) {}

	virtual prec comp_trans_heat_flow(void) { return(0.0); }
	virtual prec comp_deriv_trans_heat_flow(NodeSide node) { return(0.0); }

	virtual prec comp_lat_heat_flow(void);
	virtual prec comp_deriv_lat_heat_flow(NodeSide node);
	virtual prec comp_electron_heat_flow(void) { return(0.0); }
	virtual prec comp_deriv_electron_heat_flow(NodeSide node) { return(0.0); }
	virtual prec comp_hole_heat_flow(void) { return(0.0); }
	virtual prec comp_deriv_hole_heat_flow(NodeSide node) { return(0.0); }
	virtual prec comp_integral_rad_heat(ElementSide side) { return(0.0); }
	virtual prec comp_integral_electron_hotcarriers(ElementSide side) { return(0.0); }
	virtual prec comp_deriv_electron_hotcarriers(ElementSide side, NodeSide node) { return(0.0); }
};
*/
TBoundaryThermalElement::TBoundaryThermalElement(TDevice *device, TNode* node_1, TNode* node_2, int surface)
	: TThermalElement(BOUNDARY,device,node_1,node_2)
{
	if (node_1) surface_node=node_1;
	else surface_node=node_2;

	surface_node_number=surface_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	surface_number=surface;
}


void TBoundaryThermalElement::apply_boundary(void)
{
	if (device_effects & DEVICE_SINGLE_TEMP) {
		if ((flag)device_ptr->get_value(SURFACE,EFFECTS,surface_number) & SURFACE_HEAT_SINK) {
			surface_node->put_value(GRID_ELECTRICAL,TEMPERATURE,surface_temp,NORMALIZED);
			surface_node->put_value(ELECTRON,TEMPERATURE,surface_temp,NORMALIZED);
			surface_node->put_value(HOLE,TEMPERATURE,surface_temp,NORMALIZED);
		}
	}
	else {
		if (device_effects & DEVICE_VARY_ELECTRON_TEMP) {
			surface_node->put_value(GRID_ELECTRICAL,TEMPERATURE,surface_temp,NORMALIZED);
			surface_node->put_value(ELECTRON,TEMPERATURE,surface_electron_temp,NORMALIZED);
			surface_node->put_value(HOLE,TEMPERATURE,surface_temp,NORMALIZED);
		}
	}
}

void TBoundaryThermalElement::comp_boundary_param(void)
{
	thermal_conduct=device_ptr->get_value(SURFACE,THERMAL_CONDUCT,surface_number,NORMALIZED);
	surface_temp=device_ptr->get_value(SURFACE,TEMPERATURE,surface_number,NORMALIZED);
	surface_electron_temp=device_ptr->get_value(SURFACE,ELECTRON_TEMPERATURE,surface_number,NORMALIZED);
	surface_hole_temp=device_ptr->get_value(SURFACE,HOLE_TEMPERATURE,surface_number,NORMALIZED);
}

prec TBoundaryThermalElement::comp_lat_heat_flow(void)
{
	if (prev_node)
		return((prev_node->lattice_temp-surface_temp)*thermal_conduct);
	else
		return((surface_temp-next_node->lattice_temp)*thermal_conduct);
}

prec TBoundaryThermalElement::comp_deriv_lat_heat_flow(NodeSide node)
{
	switch(node) {
		case PREVIOUS_NODE:
			if (prev_node) return(thermal_conduct);
			break;
		case NEXT_NODE:
			if (next_node) return(-thermal_conduct);
			break;
	}
	return(0.0);
}


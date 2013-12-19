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
#include "simecele.h"

/*********************************** class TElectricalElement ********************************

class TElectricalElement: public TElement {
public:
	TElectricalElement(RegionType region_type,TDevice *device, TNode* node_1, TNode* node_2)
		: TElement(region_type,device,node_1,node_2) {}
	virtual ~TElectricalElement(void) {}

	virtual void update(FundamentalParam update);
	virtual void update_sub_nodes(void) {}

	virtual void apply_boundary(void) {}
	virtual void comp_boundary_param(void) {}

	virtual void comp_dependent_param(SolveType solve) {};
	virtual void comp_independent_param(void) {};

	virtual prec comp_field(void)=0;
	virtual FundamentalParam comp_deriv_field(NodeSide node, int return_flag)=0;
	virtual prec comp_electron_current(void)=0;
	virtual FundamentalParam comp_deriv_electron_current(NodeSide node,int return_flag)=0;
	virtual prec comp_hole_current(void)=0;
	virtual FundamentalParam comp_deriv_hole_current(NodeSide node,int return_flag)=0;
	virtual prec comp_integral_charge(ElementSide side)=0;
	virtual FundamentalParam comp_deriv_integral_charge(ElementSide side, NodeSide node,
														int return_flag, SolveType solve)=0;
	virtual prec comp_integral_recomb(ElementSide side)=0;
	virtual FundamentalParam comp_deriv_integral_recomb(ElementSide side, NodeSide node,
														int return_flag)=0;
};
*/

void TElectricalElement::update(FundamentalParam update)
{
	next_node->potential-=update.psi;

	if (update.eta_c) next_node->TElectron::planck_potential-=update.eta_c;
	if (update.eta_v) next_node->THole::planck_potential-=update.eta_v;
}

/******************************  class TBulkElectricalElement ********************************

class TBulkElectricalElement: public TElectricalElement, public TElectricalServices {
protected:
	TNode *prev_node;
	TNode *next_node;
	prec permitivity_length;
public:
	TBulkElectricalElement(TDevice *device_ptr, TNode** grid, TNode* node_1, TNode* node_2)
		: TElectricalElement(BULK,device_ptr,node_1, node_2),
		  TElectricalServices(device_ptr, grid, node_1,node_2) { prev_node=node_1; next_node=node_2; }
	virtual ~TBulkElectricalElement(void) {}

	virtual void comp_dependent_param(SolveType solve);
	virtual void comp_independent_param(void);

private:
	void comp_permit_length(void);

public:
	virtual prec comp_field(void);
	virtual FundamentalParam comp_deriv_field(NodeSide node, int return_flag);
	virtual prec comp_electron_current(void);
	virtual FundamentalParam comp_deriv_electron_current(NodeSide node,int return_flag);
	virtual prec comp_hole_current(void);
	virtual FundamentalParam comp_deriv_hole_current(NodeSide node,int return_flag);
	virtual prec comp_integral_charge(ElementSide side);
	virtual FundamentalParam comp_deriv_integral_charge(ElementSide side, NodeSide node,
														int return_flag, SolveType solve);
	virtual prec comp_integral_recomb(ElementSide side);
	virtual FundamentalParam comp_deriv_integral_recomb(ElementSide side, NodeSide node,
														int return_flag);
};
*/

void TBulkElectricalElement::comp_dependent_param(SolveType solve)
{
	prec test_length;

	if (solve==STEADY_STATE) {
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
		else comp_cond_drift_diff_param(grid_effects);

		if (hole_therm_emis_current) comp_val_therm_emis_param(grid_effects);
		else comp_val_drift_diff_param(grid_effects);
	}
}

void TBulkElectricalElement::comp_independent_param(void)
{
	comp_permit_length();
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

void TBulkElectricalElement::comp_permit_length(void)
{
	permitivity_length=((prec) next_node->permitivity+(prec) prev_node->permitivity)/(2.0*length);
}

prec TBulkElectricalElement::comp_field(void)
{
	return((prev_node->potential-next_node->potential)*permitivity_length);
}

FundamentalParam TBulkElectricalElement::comp_deriv_field(NodeSide node,int return_flag)
{
	FundamentalParam result;

	result.psi=result.eta_c=result.eta_v=0.0;

	if (return_flag & D_PSI) {
		switch(node) {
			case PREVIOUS_NODE:
				result.psi=permitivity_length;
				break;
			case NEXT_NODE:
				result.psi=-permitivity_length;
				break;
		}
	}
	return(result);
}

prec TBulkElectricalElement::comp_electron_current(void)
{
	prec temp_next, temp_prev;
	prec n_next, n_prev;
	prec prev_transmit_value, next_transmit_value;
	prec start_value, end_value;
	prec transmit_energy;
	prec planck_next, planck_prev;
	prec current=0.0;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	if (elec_therm_emis_current) {

		planck_next=next_node->TElectron::planck_potential;
		planck_prev=prev_node->TElectron::planck_potential;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				current=-ec_therm.richardson_const*sq(temp_prev)*(0.5*pow(log_1_x(exp(planck_prev-ec_therm.barrier_prev)),2.0)
																		   +dilog(1.0/(1.0+exp(ec_therm.barrier_prev-planck_prev))))
						+ec_therm.richardson_const*sq(temp_next)*(0.5*pow(log_1_x(exp(planck_next-ec_therm.barrier_next)),2.0)
																		   +dilog(1.0/(1.0+exp(ec_therm.barrier_next-planck_next))));
			}
			else {
				current=-ec_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ec_therm.barrier_prev)
						+ec_therm.richardson_const*sq(temp_next)*exp(planck_next-ec_therm.barrier_next);
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ec_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				prev_transmit_value=-temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))
									+temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next));
				start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
				end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy>=end_value;
					 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
					next_transmit_value=comp_conduction_transmission(transmit_energy)*
									   (-temp_prev*(log_1_x(exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/temp_prev)))
										+temp_next*(log_1_x(exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/temp_next))));
					current+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
													   (-ec_therm.min_transmit_energy/25.0);
					prev_transmit_value=next_transmit_value;
				}
			}
			else {
				prev_transmit_value=-temp_prev*exp(planck_prev-ec_therm.barrier_prev)
									+temp_next*exp(planck_next-ec_therm.barrier_next);
				start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
				end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy>=end_value;
					 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
					next_transmit_value=comp_conduction_transmission(transmit_energy)*
									   (-temp_prev*(exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/temp_prev))
										+temp_next*(exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/temp_next)));
					current+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
													   (-ec_therm.min_transmit_energy/25.0);
					prev_transmit_value=next_transmit_value;
				}
			}
		}
	}
	else {
		n_next=next_node->TElectron::total_conc;
		n_prev=prev_node->TElectron::total_conc;

		current=electron_mobility_length*ec.fermi_ratio_1_half_minus_1_half*
				(n_next*temp_next*ec.bernoulli_grad_temp_next
				-n_prev*temp_prev*ec.bernoulli_grad_temp_prev);
	}

	return(current);
}

FundamentalParam TBulkElectricalElement::comp_deriv_electron_current(NodeSide node,
																	  int return_flag)
{
	prec n_next, n_prev;
	prec temp_next, temp_prev;
	FundamentalParam return_value;
	prec deriv_n_eta_c_next, deriv_n_eta_c_prev;
	prec planck_next,planck_prev;
	prec prev_transmit_value,next_transmit_value;
	prec transmit_energy;
	prec start_value,end_value;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

	if (elec_therm_emis_current) {

		planck_next=next_node->TElectron::planck_potential;
		planck_prev=prev_node->TElectron::planck_potential;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI)
							return_value.psi=0.5*ec_therm.richardson_const*
												  (temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
												   temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));

						if (return_flag & D_ETA_C)
							return_value.eta_c=-ec_therm.richardson_const*sq(temp_prev)*
												 log_1_x(exp(planck_prev-ec_therm.barrier_prev));

						break;

					case NEXT_NODE:
						if (return_flag & D_PSI)
							return_value.psi=-0.5*ec_therm.richardson_const*
												  (temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
												   temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));

						if (return_flag & D_ETA_C)
							return_value.eta_c=ec_therm.richardson_const*sq(temp_next)*
												 log_1_x(exp(planck_next-ec_therm.barrier_next));

						break;
				}
			}
			else {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI)
							return_value.psi=0.5*ec_therm.richardson_const*
												  (temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
												   temp_next*exp(planck_next-ec_therm.barrier_next));

						if (return_flag & D_ETA_C)
							return_value.eta_c=-ec_therm.richardson_const*sq(temp_prev)*
												 exp(planck_prev-ec_therm.barrier_prev);


						break;

					case NEXT_NODE:
						if (return_flag & D_PSI)
							return_value.psi=-0.5*ec_therm.richardson_const*
												  (temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
												   temp_next*exp(planck_next-ec_therm.barrier_next));

						if (return_flag & D_ETA_C)
							return_value.eta_c=ec_therm.richardson_const*sq(temp_next)*
												 exp(planck_next-ec_therm.barrier_next);

						break;
				}
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ec_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_C) {
							prev_transmit_value=-temp_prev/(1.0+exp(-planck_prev+ec_therm.barrier_prev));
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
												   (-temp_prev/(1.0+exp(-planck_prev+(transmit_energy-prev_node->TElectron::band_edge)/temp_prev)));
								return_value.eta_c+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}

						break;

					case NEXT_NODE:
						if (return_flag & D_ETA_C) {
							prev_transmit_value=temp_next/(1.0+exp(-planck_next+ec_therm.barrier_next));
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
												   (temp_next/(1.0+exp(-planck_next+(transmit_energy-next_node->TElectron::band_edge)/temp_next)));
								return_value.eta_c+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}

						break;
				}
			}
			else {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_C) {
							prev_transmit_value=-temp_prev*exp(planck_prev-ec_therm.barrier_prev);
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
												   (-temp_prev*(exp(planck_prev-(transmit_energy-prev_node->TElectron::band_edge)/temp_prev)));
								return_value.eta_c+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;

					case NEXT_NODE:
						if (return_flag & D_ETA_C) {
							prev_transmit_value=temp_next*exp(planck_next-ec_therm.barrier_next);
							start_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy/25.0;
							end_value=prev_node->TElectron::band_edge+ec_therm.barrier_prev*temp_prev+ec_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy>=end_value;
								 transmit_energy+=ec_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_conduction_transmission(transmit_energy)*
												   (temp_next*(exp(planck_next-(transmit_energy-next_node->TElectron::band_edge)/temp_next)));
								return_value.eta_c+=ec_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ec_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
				}
			}
		}
	}
	else {

		n_next=next_node->TElectron::total_conc;
		n_prev=prev_node->TElectron::total_conc;

		deriv_n_eta_c_next=next_node->TElectron::total_deriv_conc_eta_c;
		deriv_n_eta_c_prev=prev_node->TElectron::total_deriv_conc_eta_c;

		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI)
					return_value.psi=-electron_mobility_length*
									  (n_next*ec.deriv_bern_grad_temp_next+
									   n_prev*ec.deriv_bern_grad_temp_prev);

				if (return_flag & D_ETA_C)
					return_value.eta_c=-electron_mobility_length*ec.fermi_ratio_1_half_minus_1_half*
										deriv_n_eta_c_prev*temp_prev*ec.bernoulli_grad_temp_prev;

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI)
					return_value.psi=electron_mobility_length*
									 (n_next*ec.deriv_bern_grad_temp_next+
									  n_prev*ec.deriv_bern_grad_temp_prev);

				if (return_flag & D_ETA_C)
					return_value.eta_c=electron_mobility_length*ec.fermi_ratio_1_half_minus_1_half*
									   deriv_n_eta_c_next*temp_next*ec.bernoulli_grad_temp_next;

				break;
		}
	}

	return(return_value);
}

prec TBulkElectricalElement::comp_hole_current(void)
{
	prec temp_next, temp_prev;
	prec p_next, p_prev;
	prec planck_next, planck_prev;
	prec prev_transmit_value, next_transmit_value;
	prec start_value, end_value;
	prec transmit_energy;
	prec current=0.0;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	if (hole_therm_emis_current) {

		planck_next=next_node->THole::planck_potential;
		planck_prev=prev_node->THole::planck_potential;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				current=ev_therm.richardson_const*sq(temp_prev)*(0.5*pow(log_1_x(exp(planck_prev-ev_therm.barrier_prev)),2.0)
																  +dilog(1.0/(1.0+exp(ev_therm.barrier_prev-planck_prev))))
					   -ev_therm.richardson_const*sq(temp_next)*(0.5*pow(log_1_x(exp(planck_next-ev_therm.barrier_next)),2.0)
																  +dilog(1.0/(1.0+exp(ev_therm.barrier_next-planck_next))));
			}
			else {
				current=ev_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ev_therm.barrier_prev)
					   -ev_therm.richardson_const*sq(temp_next)*exp(planck_next-ev_therm.barrier_next);
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ev_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				prev_transmit_value=temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))
								   -temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next));
				start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
				end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy<=end_value;
					 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
					next_transmit_value=comp_valence_transmission(transmit_energy)*
									   (temp_prev*(log_1_x(exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/temp_prev)))
									   -temp_next*(log_1_x(exp(planck_next-(next_node->THole::band_edge-transmit_energy)/temp_next))));
					current+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
													   (-ev_therm.min_transmit_energy/25.0);
					prev_transmit_value=next_transmit_value;
				}
			}
			else {
				prev_transmit_value=temp_prev*exp(planck_prev-ev_therm.barrier_prev)
								   -temp_next*exp(planck_next-ev_therm.barrier_next);
				start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
				end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
				for (transmit_energy=start_value; transmit_energy<=end_value;
					 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
					next_transmit_value=comp_valence_transmission(transmit_energy)*
									   (temp_prev*(exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/temp_prev))
									   -temp_next*(exp(planck_next-(next_node->THole::band_edge-transmit_energy)/temp_next)));
					current+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
													   (-ev_therm.min_transmit_energy/25.0);
					prev_transmit_value=next_transmit_value;
				}
			}
		}
	}
	else {
		p_next=next_node->THole::total_conc;
		p_prev=prev_node->THole::total_conc;

		current=-hole_mobility_length*ev.fermi_ratio_1_half_minus_1_half*
				 (p_next*temp_next*ev.bernoulli_grad_temp_next
				 -p_prev*temp_prev*ev.bernoulli_grad_temp_prev);
	}
	return(current);
}

FundamentalParam TBulkElectricalElement::comp_deriv_hole_current(NodeSide node,
																  int return_flag)
{
	prec p_next, p_prev;
	prec temp_next, temp_prev;
	FundamentalParam return_value;
	prec deriv_p_eta_v_next, deriv_p_eta_v_prev;
	prec prev_transmit_value, next_transmit_value;
	prec start_value, end_value;
	prec transmit_energy;
	prec planck_next,planck_prev;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

	if (hole_therm_emis_current) {

		planck_next=next_node->THole::planck_potential;
		planck_prev=prev_node->THole::planck_potential;

		if (grid_effects & GRID_THERMIONIC) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI)
							return_value.psi=0.5*ev_therm.richardson_const*
												  (temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
												   temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));

						if (return_flag & D_ETA_V)
							return_value.eta_v=ev_therm.richardson_const*sq(temp_prev)*
												 log_1_x(exp(planck_prev-ev_therm.barrier_prev));

						break;

					case NEXT_NODE:
						if (return_flag & D_PSI)
							return_value.psi=-0.5*ev_therm.richardson_const*
												  (temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
												   temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));

						if (return_flag & D_ETA_V)
							return_value.eta_v=-ev_therm.richardson_const*sq(temp_next)*
												 log_1_x(exp(planck_next-ev_therm.barrier_next));

						break;
				}
			}
			else {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI)
							return_value.psi=0.5*ev_therm.richardson_const*
												  (temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
												   temp_next*exp(planck_next-ev_therm.barrier_next));

						if (return_flag & D_ETA_V)
							return_value.eta_v=ev_therm.richardson_const*sq(temp_prev)*
												 exp(planck_prev-ev_therm.barrier_prev);

						break;

					case NEXT_NODE:
						if (return_flag & D_PSI)
							return_value.psi=-0.5*ev_therm.richardson_const*
												  (temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
												   temp_next*exp(planck_next-ev_therm.barrier_next));

						if (return_flag & D_ETA_V)
							return_value.eta_v=-ev_therm.richardson_const*sq(temp_next)*
												 exp(planck_next-ev_therm.barrier_next);

						break;
				}
			}
		}

		if ((grid_effects & GRID_TUNNELING) && ev_therm.min_transmit_energy) {
			if (grid_effects & GRID_FERMI_DIRAC) {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_V) {
							prev_transmit_value=temp_prev/(1.0+exp(-planck_prev+ev_therm.barrier_prev));
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													temp_prev/(1.0+exp(-planck_prev+(prev_node->THole::band_edge-transmit_energy)/temp_prev));
								return_value.eta_v+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;

					case NEXT_NODE:
						if (return_flag & D_ETA_V) {
							prev_transmit_value=-temp_next/(1.0+exp(-planck_next+ev_therm.barrier_next));
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													(-temp_next/(1.0+exp(-planck_next+(next_node->THole::band_edge-transmit_energy)/temp_next)));
								return_value.eta_v+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
				}
			}
			else {
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_V) {
							prev_transmit_value=temp_prev*exp(planck_prev-ev_therm.barrier_prev);
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													temp_prev*(exp(planck_prev-(prev_node->THole::band_edge-transmit_energy)/temp_prev));
								return_value.eta_v+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;

					case NEXT_NODE:
						if (return_flag & D_ETA_V) {
							prev_transmit_value=-temp_next*exp(planck_next-ev_therm.barrier_next);
							start_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy/25.0;
							end_value=prev_node->THole::band_edge-ev_therm.barrier_prev*temp_prev-ev_therm.min_transmit_energy;
							for (transmit_energy=start_value; transmit_energy<=end_value;
								 transmit_energy-=ev_therm.min_transmit_energy/25.0) {
								next_transmit_value=comp_valence_transmission(transmit_energy)*
													(-temp_next*(exp(planck_next-(next_node->THole::band_edge-transmit_energy)/temp_next)));
								return_value.eta_v+=ev_therm.richardson_const*(next_transmit_value+prev_transmit_value)/2.0*
																			  (-ev_therm.min_transmit_energy/25.0);
								prev_transmit_value=next_transmit_value;
							}
						}
						break;
				}
			}
		}
	}
	else {
		p_next=next_node->THole::total_conc;
		p_prev=prev_node->THole::total_conc;

		deriv_p_eta_v_next=next_node->THole::total_deriv_conc_eta_v;
		deriv_p_eta_v_prev=prev_node->THole::total_deriv_conc_eta_v;

		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI)
					return_value.psi=-hole_mobility_length*
									  (p_next*ev.deriv_bern_grad_temp_next
									  +p_prev*ev.deriv_bern_grad_temp_prev);

				if (return_flag & D_ETA_V)
					return_value.eta_v=hole_mobility_length*ev.fermi_ratio_1_half_minus_1_half*
									   deriv_p_eta_v_prev*temp_prev*ev.bernoulli_grad_temp_prev;

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI)
					return_value.psi=hole_mobility_length*
									 (p_next*ev.deriv_bern_grad_temp_next
									 +p_prev*ev.deriv_bern_grad_temp_prev);

				if (return_flag & D_ETA_V)
					return_value.eta_v=-hole_mobility_length*ev.fermi_ratio_1_half_minus_1_half*
										deriv_p_eta_v_next*temp_next*ev.bernoulli_grad_temp_next;

				break;
		}
	}

	return(return_value);
}

prec TBulkElectricalElement::comp_integral_charge(ElementSide side)
{
	switch(side) {
		case FIRSTHALF:
			return(prev_node->total_charge*length/2.0);
		case SECONDHALF:
			return(next_node->total_charge*length/2.0);
		default: return(0);
	}
}

FundamentalParam TBulkElectricalElement::comp_deriv_integral_charge(ElementSide side,
																	 NodeSide node,
																	 int return_flag,
																	 SolveType solve)
{
	FundamentalParam result;
	TNode* node_ptr;
	prec integral_constant;

	result.psi=result.eta_c=result.eta_v=0;

	if (node==PREVIOUS_NODE) node_ptr=prev_node;
	else node_ptr=next_node;

	if (((node==PREVIOUS_NODE) && (side==FIRSTHALF)) ||
		((node==NEXT_NODE) && (side==SECONDHALF)))
		integral_constant=length/2.0;
	else
		integral_constant=0.0;

	if (return_flag & D_PSI) {
		if (solve==EQUILIBRIUM) {
			result.psi=(-node_ptr->TElectron::total_deriv_conc_eta_c
						+node_ptr->TElectron::total_deriv_ionized_doping_eta_c
						-node_ptr->THole::total_deriv_conc_eta_v
						+node_ptr->THole::total_deriv_ionized_doping_eta_v)*integral_constant;
		}
	}

	if (return_flag & D_ETA_C)
		result.eta_c=(-node_ptr->TElectron::total_deriv_conc_eta_c
					  +node_ptr->TElectron::total_deriv_ionized_doping_eta_c)*integral_constant;

	if (return_flag & D_ETA_V)
		result.eta_v=(+node_ptr->THole::total_deriv_conc_eta_v
					  -node_ptr->THole::total_deriv_ionized_doping_eta_v)*integral_constant;

	return(result);
}


prec TBulkElectricalElement::comp_integral_recomb(ElementSide side)
{
	switch(side) {
		case FIRSTHALF:
			return(prev_node->recombination.total*length/2.0);
		case SECONDHALF:
			return(next_node->recombination.total*length/2.0);
		default: return(0);
	}
}

FundamentalParam TBulkElectricalElement::comp_deriv_integral_recomb(ElementSide side,
																	 NodeSide node,
																	 int return_flag)
{
	FundamentalParam result;
	TNode* node_ptr;
	prec integral_constant;

	result.psi=result.eta_c=result.eta_v=0;

	if (node==PREVIOUS_NODE) node_ptr=prev_node;
	else node_ptr=next_node;

	if (((node==PREVIOUS_NODE) && (side==FIRSTHALF)) ||
		((node==NEXT_NODE) && (side==SECONDHALF)))
		integral_constant=length/2.0;
	else
		integral_constant=0.0;

	if (return_flag & D_ETA_C)
		result.eta_c=node_ptr->deriv_recomb.eta_c*integral_constant;

	if (return_flag & D_ETA_V)
		result.eta_v=node_ptr->deriv_recomb.eta_v*integral_constant;

	return(result);
}

/********************************* class TQWElectricalElement *********************************

class TQWElectricalElement: public TElectricalElement, public TElectricalServices {
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
	prec *sub_length;
	prec length_fraction;
	prec permitivity_length;
public:
	TQWElectricalElement(TDevice *device_ptr, TNode** device_grid,
						 TNode* node_1, TNode* node_2);
	virtual ~TQWElectricalElement(void) { delete[] sub_length; }

	virtual void update_sub_nodes(void);

	virtual void comp_dependent_param(SolveType solve);
	virtual void comp_independent_param(void);

private:
	void comp_sub_length(void);
	void comp_permit_length(void);
	void comp_length_fraction(void);
	void comp_cond_therm_emis_param(void);
	void comp_val_therm_emis_param(void);

public:
	virtual prec comp_field(void);
	virtual FundamentalParam comp_deriv_field(NodeSide node, int return_flag);
	virtual prec comp_electron_current(void);
	virtual FundamentalParam comp_deriv_electron_current(NodeSide node,int return_flag);
	virtual prec comp_hole_current(void);
	virtual FundamentalParam comp_deriv_hole_current(NodeSide node,int return_flag);
	virtual prec comp_integral_charge(ElementSide side);
	virtual FundamentalParam comp_deriv_integral_charge(ElementSide side, NodeSide node,
														int return_flag, SolveType solve);
	virtual prec comp_integral_recomb(ElementSide side);
	virtual FundamentalParam comp_deriv_integral_recomb(ElementSide side, NodeSide node,
														int return_flag);
};
*/

TQWElectricalElement::TQWElectricalElement(TDevice *device_ptr,
										   TNode** device_grid,
										   TNode* node_1, TNode* node_2)
	: TElectricalElement(QW, device_ptr, node_1, node_2),
	  TElectricalServices(device_ptr, device_grid, node_1, node_2)
{
	device_grid_ptr=device_grid;

	prev_node=node_1;
	next_node=node_2;

	if (node_1->TGrid::region_type==QW)
		qw_node=node_1;
	else qw_node=node_2;

	qw_node_number=qw_node->node_number;
	prev_node_number=prev_node->node_number;
	next_node_number=next_node->node_number;
	mid_node_number=device_ptr->get_node((prev_node->position+next_node->position)/2.0,-1,-1,NORMALIZED);

	number_sub_elements=next_node_number-prev_node_number;

	sub_length=new prec[number_sub_elements];
}

void TQWElectricalElement::update_sub_nodes(void)
{
	int i;
	prec integrated_charge;
	prec prev_charge, prev_position, prev_field, prev_potential, prev_permitivity;
	prec next_charge, next_position, next_field, next_potential, next_permitivity;
	prec charge, position, field, potential, permitivity;
	TNode **temp_ptr;

	if (next_node_number==qw_node_number) {
		prev_field=((*(device_grid_ptr+prev_node_number-1))->potential-prev_node->potential)/
					(prev_node->position-(*(device_grid_ptr+prev_node_number-1))->position);
		prev_permitivity=prev_node->permitivity;
		prev_potential=prev_node->potential;
		prev_position=prev_node->position;
		prev_charge=prev_node->total_charge;

		temp_ptr=device_grid_ptr+prev_node_number+1;

		for (i=prev_node_number+1;i<=next_node_number-1;i++) {
			position=(*temp_ptr)->position;
			potential=prev_potential-prev_field*(position-prev_position);
			(*temp_ptr)->potential=potential;
			(*temp_ptr)->init_value(ELECTRON,PLANCK_POT,qw_node,NULL);
			(*temp_ptr)->init_value(HOLE,PLANCK_POT,qw_node,NULL);

			permitivity=(*temp_ptr)->permitivity;
			charge=(*temp_ptr)->total_charge;
			integrated_charge=(charge+prev_charge)*
							  (position-prev_position)/2.0;
			field=(prev_permitivity*prev_field+integrated_charge)/permitivity;

			prev_permitivity=permitivity;
			prev_potential=potential;
			prev_charge=charge;
			prev_position=position;
			prev_field=field;

			temp_ptr++;
		}
	}
	else {
		next_field=(next_node->potential-(*(device_grid_ptr+next_node_number+1))->potential)/
				   ((*(device_grid_ptr+next_node_number+1))->position-next_node->position);
		next_permitivity=next_node->permitivity;
		next_potential=next_node->potential;
		next_position=next_node->position;
		next_charge=next_node->total_charge;

		temp_ptr=device_grid_ptr+next_node_number-1;

		for (i=next_node_number-1;i>=prev_node_number+1;i--) {
			position=(*temp_ptr)->position;
			potential=next_potential+next_field*(next_position-position);
			(*temp_ptr)->potential=potential;
			(*temp_ptr)->init_value(ELECTRON,PLANCK_POT,qw_node,NULL);
			(*temp_ptr)->init_value(HOLE,PLANCK_POT,qw_node,NULL);

			permitivity=(*temp_ptr)->permitivity;
			charge=(*temp_ptr)->total_charge;
			integrated_charge=(next_charge+charge)*
							  (next_position-position)/2.0;
			field=(next_permitivity*next_field-integrated_charge)/permitivity;

			next_permitivity=permitivity;
			next_potential=potential;
			next_charge=charge;
			next_position=position;
			next_field=field;

			temp_ptr--;
		}
	}
}

void TQWElectricalElement::comp_dependent_param(SolveType solve)
{
	if (solve==STEADY_STATE) {
		comp_cond_therm_emis_param();
		comp_val_therm_emis_param();
	}
}

void TQWElectricalElement::comp_independent_param(void)
{
	comp_permit_length();
	comp_sub_length();
	comp_length_fraction();
	comp_conduction_discont();
	comp_conduction_richardson();
	comp_valence_discont();
	comp_valence_richardson();
}

void TQWElectricalElement::comp_sub_length(void)
{
	int i;
	TNode** temp_ptr;

	temp_ptr=device_grid_ptr+prev_node_number;
	for (i=0;i<number_sub_elements;i++) {
		sub_length[i]=(*(temp_ptr+1))->position-(*temp_ptr)->position;
		temp_ptr++;
	}
}

void TQWElectricalElement::comp_permit_length(void)
{
	permitivity_length=qw_node->permitivity/length;
}

void TQWElectricalElement::comp_length_fraction(void)
{
	if (next_node_number==qw_node_number)
		length_fraction=1.0-sub_length[0]/length;
	else
		length_fraction=1.0-sub_length[number_sub_elements-1]/length;
//	length_fraction=0.5;
}

void TQWElectricalElement::comp_cond_therm_emis_param(void)
{
	if (ec_therm.band_discont<0) {
		ec_therm.barrier_prev=-ec_therm.band_discont-length_fraction*(next_node->potential-prev_node->potential);
		ec_therm.barrier_next=-(1.0-length_fraction)*(prev_node->potential-next_node->potential);
	}
	else {
		ec_therm.barrier_prev=-(1.0-length_fraction)*(next_node->potential-prev_node->potential);
		ec_therm.barrier_next=ec_therm.band_discont-length_fraction*(prev_node->potential-next_node->potential);
	}

//	ec_therm.barrier_prev/=prev_node->TElectron::stored_temperature;
//	ec_therm.barrier_next/=next_node->TElectron::stored_temperature;

	ec_therm.barrier_prev/=prev_node->TElectron::temperature;
	ec_therm.barrier_next/=next_node->TElectron::temperature;

}

void TQWElectricalElement::comp_val_therm_emis_param(void)
{
	if (ev_therm.band_discont<0) {
		ev_therm.barrier_prev=(1.0-length_fraction)*(next_node->potential-prev_node->potential);
		ev_therm.barrier_next=-ev_therm.band_discont+length_fraction*(prev_node->potential-next_node->potential);
	}
	else {
		ev_therm.barrier_prev=ev_therm.band_discont+length_fraction*(next_node->potential-prev_node->potential);
		ev_therm.barrier_next=(1.0-length_fraction)*(prev_node->potential-next_node->potential);
	}

//	ev_therm.barrier_prev/=prev_node->THole::stored_temperature;
//	ev_therm.barrier_next/=next_node->THole::stored_temperature;

	ev_therm.barrier_prev/=prev_node->THole::temperature;
	ev_therm.barrier_next/=next_node->THole::temperature;

}

prec TQWElectricalElement::comp_field(void)
{
	return((prev_node->potential-next_node->potential)*permitivity_length);
}

FundamentalParam TQWElectricalElement::comp_deriv_field(NodeSide node,int return_flag)
{
	FundamentalParam result;

	result.psi=result.eta_c=result.eta_v=0.0;

	if (return_flag & D_PSI) {
		switch(node) {
			case PREVIOUS_NODE:
				result.psi=permitivity_length;
				break;
			case NEXT_NODE:
				result.psi=-permitivity_length;
				break;
		}
	}
	return(result);
}

prec TQWElectricalElement::comp_electron_current(void)
{
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec current;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	planck_next=next_node->TElectron::planck_potential;
	planck_prev=prev_node->TElectron::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		current=+ec_therm.richardson_const*sq(temp_next)*(0.5*pow(log_1_x(exp(planck_next-ec_therm.barrier_next)),2.0)
														  +dilog(1.0/(1.0+exp(ec_therm.barrier_next-planck_next))))
				-ec_therm.richardson_const*sq(temp_prev)*(0.5*pow(log_1_x(exp(planck_prev-ec_therm.barrier_prev)),2.0)
														  +dilog(1.0/(1.0+exp(ec_therm.barrier_prev-planck_prev))));
	}
	else {

		current=+ec_therm.richardson_const*sq(temp_next)*exp(planck_next-ec_therm.barrier_next)
				-ec_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ec_therm.barrier_prev);
	}

	return(current);
}

FundamentalParam TQWElectricalElement::comp_deriv_electron_current(NodeSide node,
																	int return_flag)
{
	prec temp_next, temp_prev;
	FundamentalParam return_value;
	prec planck_next,planck_prev;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

	temp_next=next_node->TElectron::temperature;
	temp_prev=prev_node->TElectron::temperature;

	planck_next=next_node->TElectron::planck_potential;
	planck_prev=prev_node->TElectron::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI) {
					if (ec_therm.band_discont<0)
						return_value.psi=ec_therm.richardson_const*
										 (length_fraction*temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
										  (1.0-length_fraction)*temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));
					else
						return_value.psi=ec_therm.richardson_const*
										 ((1.0-length_fraction)*temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
										  length_fraction*temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));
				}

				if (return_flag & D_ETA_C)
					return_value.eta_c=-ec_therm.richardson_const*sq(temp_prev)*
										log_1_x(exp(planck_prev-ec_therm.barrier_prev));

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI) {
					if (ec_therm.band_discont<0)
						return_value.psi=-ec_therm.richardson_const*
										   (length_fraction*temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
											(1.0-length_fraction)*temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));
					else
						return_value.psi=-ec_therm.richardson_const*
										   ((1.0-length_fraction)*temp_prev*log_1_x(exp(planck_prev-ec_therm.barrier_prev))+
											length_fraction*temp_next*log_1_x(exp(planck_next-ec_therm.barrier_next)));
				}

				if (return_flag & D_ETA_C)
					return_value.eta_c=ec_therm.richardson_const*sq(temp_next)*
									   log_1_x(exp(planck_next-ec_therm.barrier_next));

				break;
		}
	}
	else {
		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI) {
					if (ec_therm.band_discont<0)
						return_value.psi=ec_therm.richardson_const*
										 (length_fraction*temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
										  (1.0-length_fraction)*temp_next*exp(planck_next-ec_therm.barrier_next));
					else
						return_value.psi=ec_therm.richardson_const*
										 ((1.0-length_fraction)*temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
										  length_fraction*temp_next*exp(planck_next-ec_therm.barrier_next));
				}

				if (return_flag & D_ETA_C)
					return_value.eta_c=-ec_therm.richardson_const*sq(temp_prev)*
										 exp(planck_prev-ec_therm.barrier_prev);

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI) {
					if (ec_therm.band_discont<0)
						return_value.psi=-ec_therm.richardson_const*
										   (length_fraction*temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
											(1.0-length_fraction)*temp_next*exp(planck_next-ec_therm.barrier_next));
					else
						return_value.psi=-ec_therm.richardson_const*
										   ((1.0-length_fraction)*temp_prev*exp(planck_prev-ec_therm.barrier_prev)+
											length_fraction*temp_next*exp(planck_next-ec_therm.barrier_next));
				}

				if (return_flag & D_ETA_C)
					return_value.eta_c=ec_therm.richardson_const*sq(temp_next)*
									   exp(planck_next-ec_therm.barrier_next);

				break;
		}
	}

	return(return_value);
}

prec TQWElectricalElement::comp_hole_current(void)
{
	prec temp_next, temp_prev;
	prec planck_next, planck_prev;
	prec current;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	planck_next=next_node->THole::planck_potential;
	planck_prev=prev_node->THole::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		current=ev_therm.richardson_const*sq(temp_prev)*(0.5*pow(log_1_x(exp(planck_prev-ev_therm.barrier_prev)),2.0)
														  +dilog(1.0/(1.0+exp(ev_therm.barrier_prev-planck_prev))))
			   -ev_therm.richardson_const*sq(temp_next)*(0.5*pow(log_1_x(exp(planck_next-ev_therm.barrier_next)),2.0)
														  +dilog(1.0/(1.0+exp(ev_therm.barrier_next-planck_next))));
	}
	else {
		current=ev_therm.richardson_const*sq(temp_prev)*exp(planck_prev-ev_therm.barrier_prev)
			   -ev_therm.richardson_const*sq(temp_next)*exp(planck_next-ev_therm.barrier_next);
	}

	return(current);
}

FundamentalParam TQWElectricalElement::comp_deriv_hole_current(NodeSide node,
																int return_flag)
{
	prec temp_next, temp_prev;
	FundamentalParam return_value;
	prec planck_next,planck_prev;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

	temp_next=next_node->THole::temperature;
	temp_prev=prev_node->THole::temperature;

	planck_next=next_node->THole::planck_potential;
	planck_prev=prev_node->THole::planck_potential;

	if (grid_effects & GRID_FERMI_DIRAC) {
		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI) {
					if (ev_therm.band_discont<0)
						return_value.psi=ev_therm.richardson_const*
										   ((1.0-length_fraction)*temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
											length_fraction*temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));
					else
						return_value.psi=ev_therm.richardson_const*
										   (length_fraction*temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
											(1.0-length_fraction)*temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));
				}

				if (return_flag & D_ETA_V)
					return_value.eta_v=ev_therm.richardson_const*sq(temp_prev)*
									   log_1_x(exp(planck_prev-ev_therm.barrier_prev));

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI) {
					if (ev_therm.band_discont<0)
						return_value.psi=-ev_therm.richardson_const*
											((1.0-length_fraction)*temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
											 length_fraction*temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));
					else
						return_value.psi=-ev_therm.richardson_const*
											(length_fraction*temp_prev*log_1_x(exp(planck_prev-ev_therm.barrier_prev))+
											 (1.0-length_fraction)*temp_next*log_1_x(exp(planck_next-ev_therm.barrier_next)));
				}

				if (return_flag & D_ETA_V)
					return_value.eta_v=-ev_therm.richardson_const*sq(temp_next)*
										log_1_x(exp(planck_next-ev_therm.barrier_next));

				break;
		}
	}
	else {
		switch(node) {
			case PREVIOUS_NODE:
				if (return_flag & D_PSI) {
					if (ev_therm.band_discont<0)
						return_value.psi=ev_therm.richardson_const*
										   ((1.0-length_fraction)*temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
											length_fraction*temp_next*exp(planck_next-ev_therm.barrier_next));
					else
						return_value.psi=ev_therm.richardson_const*
										   (length_fraction*temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
											(1.0-length_fraction)*temp_next*exp(planck_next-ev_therm.barrier_next));
				}

				if (return_flag & D_ETA_V)
					return_value.eta_v=ev_therm.richardson_const*sq(temp_prev)*
										 exp(planck_prev-ev_therm.barrier_prev);

				break;

			case NEXT_NODE:
				if (return_flag & D_PSI) {
					if (ev_therm.band_discont<0)
						return_value.psi=-ev_therm.richardson_const*
											((1.0-length_fraction)*temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
											 length_fraction*temp_next*exp(planck_next-ev_therm.barrier_next));
					else
						return_value.psi=-ev_therm.richardson_const*
											(length_fraction*temp_prev*exp(planck_prev-ev_therm.barrier_prev)+
											 (1.0-length_fraction)*temp_next*exp(planck_next-ev_therm.barrier_next));
				}

				if (return_flag & D_ETA_V)
					return_value.eta_v=-ev_therm.richardson_const*sq(temp_next)*
										 exp(planck_next-ev_therm.barrier_next);

				break;
		}
	}

	return(return_value);
}

prec TQWElectricalElement::comp_integral_charge(ElementSide side)
{
// Still using trapezoid integration rule
	int i,start_node, end_node;
	prec result;
	TNode** temp_ptr;

	switch(side) {
		case FIRSTHALF:
			start_node=prev_node_number;
			end_node=mid_node_number;
			break;
		case SECONDHALF:
			start_node=mid_node_number;
			end_node=next_node_number;
			break;
	}

	temp_ptr=device_grid_ptr+start_node;
	result=0;
	for (i=start_node;i<end_node;i++) {
		result+=((*temp_ptr)->total_charge+(*(temp_ptr+1))->total_charge)*sub_length[i-prev_node_number]/2.0;
		temp_ptr++;
	}
	return(result);
}

FundamentalParam TQWElectricalElement::comp_deriv_integral_charge(ElementSide side,
																   NodeSide node,
																   int return_flag,
																   SolveType solve)
{

	int i,start_node, end_node;
	FundamentalParam result;
	TNode** temp_ptr;

	result.psi=result.eta_c=result.eta_v=0.0;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								result.psi=(-prev_node->TElectron::total_deriv_conc_eta_c
											+prev_node->TElectron::total_deriv_ionized_doping_eta_c
											-prev_node->THole::total_deriv_conc_eta_v
											+prev_node->THole::total_deriv_ionized_doping_eta_v)*sub_length[0]/2.0;
							}
						}

						if (return_flag & D_ETA_C)
							result.eta_c=(-prev_node->TElectron::total_deriv_conc_eta_c
										  +prev_node->TElectron::total_deriv_ionized_doping_eta_c)*sub_length[0]/2.0;

						if (return_flag & D_ETA_V)
							result.eta_v=(+prev_node->THole::total_deriv_conc_eta_v
										  -prev_node->THole::total_deriv_ionized_doping_eta_v)*sub_length[0]/2.0;

						break;

					case NEXT_NODE:
						start_node=prev_node_number+1;
						end_node=mid_node_number;

						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								temp_ptr=device_grid_ptr+start_node;
								result.psi=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											+(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											-(*temp_ptr)->THole::total_deriv_conc_eta_v
											+(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v)*sub_length[0]/2.0;
								temp_ptr++;
								for (i=start_node+1;i<=end_node;i++) {
									result.psi+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
												 +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
												 -(*temp_ptr)->THole::total_deriv_conc_eta_v
												 +(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
												 -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
												 +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c
												 -(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
												 +(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[i-prev_node_number-1]/2.0;
									temp_ptr++;
								}
							}
						}

						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+start_node;
							result.eta_c=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
										  +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c)*sub_length[0]/2.0;
							temp_ptr++;
							for (i=start_node+1;i<=end_node;i++) {
								result.eta_c+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											   +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											   -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
											   +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c)
																		*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+start_node;
							result.eta_v=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
										  -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v)*sub_length[0]/2.0;
							temp_ptr++;
							for (i=start_node+1;i<=end_node;i++) {
								result.eta_v+=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
											   -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
											   +(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
											   -(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_PSI)
							if (solve==EQUILIBRIUM) result.psi=0.0;

						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
					case NEXT_NODE:
						start_node=mid_node_number+1;
						end_node=next_node_number;

						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								temp_ptr=device_grid_ptr+start_node;
								for (i=start_node;i<=end_node;i++) {
									result.psi+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
												 +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
												 -(*temp_ptr)->THole::total_deriv_conc_eta_v
												 +(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
												 -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
												 +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c
												 -(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
												 +(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																	*sub_length[i-prev_node_number-1]/2.0;
									temp_ptr++;
								}
							}
						}

						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node;i++) {
								result.eta_c+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											   +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											   -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
											   +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c)
																	*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node;i++) {
								result.eta_v+=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
											   -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
											   +(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
											   -(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																	*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

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
						start_node=prev_node_number+1;
						end_node=mid_node_number;

						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								temp_ptr=device_grid_ptr+start_node;
								for (i=start_node;i<=end_node;i++) {
									result.psi+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
												 +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
												 -(*temp_ptr)->THole::total_deriv_conc_eta_v
												 +(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
												 -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
												 +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c
												 -(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
												 +(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																	*sub_length[i-prev_node_number-1]/2.0;
									temp_ptr++;
								}
							}
						}

						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node;i++) {
								result.eta_c+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											   +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											   -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
											   +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c)
																	*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node;i++) {
								result.eta_v+=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
											   -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
											   +(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
											   -(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																	*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
						}

						break;
					case NEXT_NODE:
						if (return_flag & D_PSI)
							if (solve==EQUILIBRIUM) result.psi=0.0;

						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						start_node=mid_node_number+1;
						end_node=next_node_number;

						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								temp_ptr=device_grid_ptr+start_node;
								for (i=start_node;i<=end_node-1;i++) {
									result.psi+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
												 +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
												 -(*temp_ptr)->THole::total_deriv_conc_eta_v
												 +(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
												 -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
												 +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c
												 -(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
												 +(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[i-prev_node_number-1]/2.0;
									temp_ptr++;
								}
								result.psi+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											 +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											 -(*temp_ptr)->THole::total_deriv_conc_eta_v
											 +(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[number_sub_elements-1]/2.0;
							}
						}

						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node-1;i++) {
								result.eta_c+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
											   +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c
											   -(*(temp_ptr-1))->TElectron::total_deriv_conc_eta_c
											   +(*(temp_ptr-1))->TElectron::total_deriv_ionized_doping_eta_c)*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
							result.eta_c+=(-(*temp_ptr)->TElectron::total_deriv_conc_eta_c
										   +(*temp_ptr)->TElectron::total_deriv_ionized_doping_eta_c)
																		*sub_length[number_sub_elements-1]/2.0;
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+start_node;
							for (i=start_node;i<=end_node-1;i++) {
								result.eta_v+=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
											   -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v
											   +(*(temp_ptr-1))->THole::total_deriv_conc_eta_v
											   -(*(temp_ptr-1))->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[i-prev_node_number-1]/2.0;
								temp_ptr++;
							}
							result.eta_v+=(+(*temp_ptr)->THole::total_deriv_conc_eta_v
										   -(*temp_ptr)->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[number_sub_elements-1]/2.0;
						}

						break;
					case NEXT_NODE:
						if (return_flag & D_PSI) {
							if (solve==EQUILIBRIUM) {
								result.psi=(-next_node->TElectron::total_deriv_conc_eta_c
											+next_node->TElectron::total_deriv_ionized_doping_eta_c
											-next_node->THole::total_deriv_conc_eta_v
											+next_node->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[number_sub_elements-1]/2.0;
							}
						}

						if (return_flag & D_ETA_C)
							result.eta_c=(-next_node->TElectron::total_deriv_conc_eta_c
										  +next_node->TElectron::total_deriv_ionized_doping_eta_c)
																		*sub_length[number_sub_elements-1]/2.0;

						if (return_flag & D_ETA_V)
							result.eta_v=(+next_node->THole::total_deriv_conc_eta_v
										  -next_node->THole::total_deriv_ionized_doping_eta_v)
																		*sub_length[number_sub_elements-1]/2.0;

						break;
				}
				break;
		}

	}
	return(result);
}

prec TQWElectricalElement::comp_integral_recomb(ElementSide side)
{
	int i;
	prec result;
	TNode** temp_ptr;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				result=prev_node->recombination.total*sub_length[0]/2.0;
				break;
			case SECONDHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->recombination.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=next_node->recombination.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	else {
		switch(side) {
			case FIRSTHALF:
				temp_ptr=device_grid_ptr+prev_node_number+1;
				result=0;
				for (i=prev_node_number+1;i<next_node_number;i++) {
					result+=(*temp_ptr)->recombination.total*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
					temp_ptr++;
				}
				result+=prev_node->recombination.total*sub_length[number_sub_elements-1]/2.0;
				break;
			case SECONDHALF:
				result=next_node->recombination.total*sub_length[number_sub_elements-1]/2.0;
				break;
		}
	}
	return(result);
}

FundamentalParam TQWElectricalElement::comp_deriv_integral_recomb(ElementSide side,
																   NodeSide node,
																   int return_flag)
{
	int i;
	FundamentalParam result;
	TNode** temp_ptr;

	result.psi=result.eta_c=result.eta_v=0.0;

	if (next_node_number==qw_node_number) {
		switch(side) {
			case FIRSTHALF:
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_C)
							result.eta_c=prev_node->deriv_recomb.eta_c*sub_length[0]/2.0;

						if (return_flag & D_ETA_V)
							result.eta_v=prev_node->deriv_recomb.eta_v*sub_length[0]/2.0;

						break;

					case NEXT_NODE:
						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
					case NEXT_NODE:
						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+prev_node_number+1;
							result.eta_c=0.0;
							for (i=prev_node_number+1;i<next_node_number;i++) {
								result.eta_c+=(*temp_ptr)->deriv_recomb.eta_c*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
								temp_ptr++;
							}
							result.eta_c+=next_node->deriv_recomb.eta_c*sub_length[number_sub_elements-1]/2.0;
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+prev_node_number+1;
							result.eta_v=0.0;
							for (i=prev_node_number+1;i<next_node_number;i++) {
								result.eta_v+=(*temp_ptr)->deriv_recomb.eta_v*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
								temp_ptr++;
							}
							result.eta_v+=next_node->deriv_recomb.eta_v*sub_length[number_sub_elements-1]/2.0;
						}

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
						if (return_flag & D_ETA_C) {
							temp_ptr=device_grid_ptr+prev_node_number+1;
							result.eta_c=0.0;
							for (i=prev_node_number+1;i<next_node_number;i++) {
								result.eta_c+=(*temp_ptr)->deriv_recomb.eta_c*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
								temp_ptr++;
							}
							result.eta_c+=prev_node->deriv_recomb.eta_c*sub_length[number_sub_elements-1]/2.0;
						}

						if (return_flag & D_ETA_V) {
							temp_ptr=device_grid_ptr+prev_node_number+1;
							result.eta_v=0.0;
							for (i=prev_node_number+1;i<next_node_number;i++) {
								result.eta_v+=(*temp_ptr)->deriv_recomb.eta_v*(sub_length[i-prev_node_number-1]+sub_length[i-prev_node_number]);
								temp_ptr++;
							}
							result.eta_v+=prev_node->deriv_recomb.eta_v*sub_length[number_sub_elements-1]/2.0;
						}

						break;
					case NEXT_NODE:
						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
				}
				break;
			case SECONDHALF:
				switch(node) {
					case PREVIOUS_NODE:
						if (return_flag & D_ETA_C) result.eta_c=0.0;

						if (return_flag & D_ETA_V) result.eta_v=0.0;

						break;
					case NEXT_NODE:
						if (return_flag & D_ETA_C)
							result.eta_c=next_node->deriv_recomb.eta_c*sub_length[number_sub_elements-1]/2.0;

						if (return_flag & D_ETA_V)
							result.eta_v=next_node->deriv_recomb.eta_v*sub_length[number_sub_elements-1]/2.0;

						break;
				}
				break;
		}
	}

	return(result);
}

/********************************** class TOhmicBoundaryElement ******************************

class TOhmicBoundaryElement: public TElectricalElement {
private:
	TNode *contact_node;
	int contact_node_number;
	int contact_number;
	flag contact_flags;

	prec bias;
	prec built_in_pot;

	prec electron_recomb_vel;
	prec equil_electron_conc;
	prec hole_recomb_vel;
	prec equil_hole_conc;
    prec electron_richardson_const;
    prec hole_richardson_const;
    prec electron_barrier;
    prec hole_barrier;
public:
	TOhmicBoundaryElement(TDevice *device, TNode* node_1, TNode* node_2, int contact);
	virtual ~TOhmicBoundaryElement(void) {}

	virtual void apply_boundary(void);
	virtual void comp_boundary_param(void);

	virtual void comp_independent_param(void);

private:
	void comp_conduction_discont(void);
	void comp_conduction_richardson(void);
	void comp_valence_discont(void);
	void comp_valence_richardson(void);

public:
	virtual prec comp_field(void);
	virtual FundamentalParam comp_deriv_field(NodeSide node, int return_flag);
	virtual prec comp_electron_current(void);
	virtual FundamentalParam comp_deriv_electron_current(NodeSide node,int return_flag);
	virtual prec comp_hole_current(void);
	virtual FundamentalParam comp_deriv_hole_current(NodeSide node,int return_flag);
	virtual prec comp_integral_charge(ElementSide side) { return(0.0); }
	virtual FundamentalParam comp_deriv_integral_charge(ElementSide side, NodeSide node,
														int return_flag, SolveType solve);
	virtual prec comp_integral_recomb(ElementSide side) { return(0.0); }
	virtual FundamentalParam comp_deriv_integral_recomb(ElementSide side, NodeSide node,
														int return_flag);
};
*/

TOhmicBoundaryElement::TOhmicBoundaryElement(TDevice *device, TNode* node_1, TNode* node_2,
											 int contact)
	: TElectricalElement(BOUNDARY,device,node_1,node_2)
{
	if (node_1) contact_node=node_1;
	else contact_node=node_2;

	contact_node_number=contact_node->get_value(GRID_ELECTRICAL,NODE_NUMBER);
	contact_number=contact;
}

void TOhmicBoundaryElement::apply_boundary(void)
{
	if (contact_flags & CONTACT_IDEALOHMIC) {
		contact_node->comp_value(ELECTRON,PLANCK_POT);
		contact_node->comp_value(HOLE,PLANCK_POT);
		device_ptr->init_value(ELECTRON,CONCENTRATION,0,contact_node_number,contact_node_number);
		device_ptr->init_value(HOLE,CONCENTRATION,0,contact_node_number,contact_node_number);
		contact_node->comp_value(ELECTRON,IONIZED_DOPING);
		contact_node->comp_value(HOLE,IONIZED_DOPING);
		contact_node->comp_value(NODE,TOTAL_CHARGE);
    }
    contact_node->put_value(GRID_ELECTRICAL,POTENTIAL,bias+built_in_pot,NORMALIZED);
    contact_node->comp_value(ELECTRON,BAND_EDGE);
    contact_node->comp_value(HOLE,BAND_EDGE);
}

void TOhmicBoundaryElement::comp_boundary_param(void)
{
	contact_flags=(flag)device_ptr->get_value(CONTACT,EFFECTS,contact_number);
	electron_recomb_vel=device_ptr->get_value(CONTACT,ELECTRON_RECOMB_VEL,contact_number,NORMALIZED);
	equil_electron_conc=device_ptr->get_value(CONTACT,EQUIL_ELECTRON_CONC,contact_number,NORMALIZED);
	hole_recomb_vel=device_ptr->get_value(CONTACT,HOLE_RECOMB_VEL,contact_number,NORMALIZED);
	equil_hole_conc=device_ptr->get_value(CONTACT,EQUIL_HOLE_CONC,contact_number,NORMALIZED);
	bias=device_ptr->get_value(CONTACT,APPLIED_BIAS,contact_number,NORMALIZED);
	built_in_pot=device_ptr->get_value(CONTACT,BUILT_IN_POT,contact_number,NORMALIZED);
}

void TOhmicBoundaryElement::comp_independent_param(void)
{
	comp_conduction_discont();
    comp_conduction_richardson();
    comp_valence_discont();
    comp_valence_richardson();
}

void TOhmicBoundaryElement::comp_conduction_discont(void)
{
	prec temp;

    if (next_node) temp=next_node->TElectron::temperature;
    else temp=prev_node->TElectron::temperature;

    electron_barrier=device_ptr->get_value(CONTACT,BARRIER_HEIGHT,contact_number,NORMALIZED)/temp;
}

void TOhmicBoundaryElement::comp_conduction_richardson(void)
{
	prec mass;

    if (next_node) mass=next_node->TElectron::dos_mass;
    else mass=prev_node->TElectron::dos_mass;

	electron_richardson_const=(SIM_q*SIM_mo*mass*sq(SIM_k)/(2.0*sq(SIM_pi)*sq(SIM_hb)*SIM_hb*1e4))*sq(normalization.temp)
							  /normalization.current;
}

void TOhmicBoundaryElement::comp_valence_discont(void)
{
	prec temp, band_gap;

    if (next_node) {
    	temp=next_node->TElectron::temperature;
        band_gap=next_node->band_gap;
    }
    else {
    	temp=prev_node->TElectron::temperature;
        band_gap=prev_node->band_gap;
    }

    hole_barrier=(band_gap-device_ptr->get_value(CONTACT,BARRIER_HEIGHT,contact_number,NORMALIZED))/temp;
}

void TOhmicBoundaryElement::comp_valence_richardson(void)
{
	prec mass;

	if (next_node) mass=next_node->THole::dos_mass;
    else mass=prev_node->THole::dos_mass;

	hole_richardson_const=(SIM_q*SIM_mo*mass*sq(SIM_k)/(2.0*sq(SIM_pi)*sq(SIM_hb)*SIM_hb*1e4))*sq(normalization.temp)
							   /normalization.current;
}

prec TOhmicBoundaryElement::comp_field(void)
{
	return(0.0);
}

FundamentalParam TOhmicBoundaryElement::comp_deriv_field(NodeSide /*node*/, int /*return_flag*/)
{
	FundamentalParam result={0,0,0};
	return(result);
}

prec TOhmicBoundaryElement::comp_electron_current(void)
{
	prec current;
    prec planck_prev, planck_next;
    prec temp;

	if (prev_node) {
    	if (contact_flags & CONTACT_FINITERECOMB)
	    	current=-electron_recomb_vel*(prev_node->TElectron::total_conc-equil_electron_conc);
    	else {
			planck_prev=prev_node->TElectron::planck_potential;
            temp=prev_node->TElectron::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
				current=-electron_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(planck_prev)),2.0)
																		   +dilog(1.0/(1.0+exp(-planck_prev))))
						+electron_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(-electron_barrier)),2.0)
																		   +dilog(1.0/(1.0+exp(electron_barrier))));
			}
			else {
				current=-electron_richardson_const*sq(temp)*exp(planck_prev)
						+electron_richardson_const*sq(temp)*exp(-electron_barrier);
			}
        }
    }
    else {
    	if (contact_flags & CONTACT_FINITERECOMB)
		   	current=electron_recomb_vel*(next_node->TElectron::total_conc-equil_electron_conc);
		else {
			planck_next=next_node->TElectron::planck_potential;
            temp=next_node->TElectron::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
				current=-electron_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(-electron_barrier)),2.0)
																		   +dilog(1.0/(1.0+exp(electron_barrier))))
						+electron_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(planck_next)),2.0)
																		   +dilog(1.0/(1.0+exp(-planck_next))));
			}
			else {
				current=-electron_richardson_const*sq(temp)*exp(-electron_barrier)
						+electron_richardson_const*sq(temp)*exp(planck_next);
			}
        }
    }

    return(current);
}

FundamentalParam TOhmicBoundaryElement::comp_deriv_electron_current(NodeSide node,
																	int return_flag)
{
	FundamentalParam return_value;
    prec planck_prev, planck_next;
    prec temp;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

    if (prev_node && (node==PREVIOUS_NODE)) {
    	if (contact_flags & CONTACT_FINITERECOMB) {
        	if (return_flag & D_ETA_C)
            	return_value.eta_c=-electron_recomb_vel*prev_node->TElectron::total_deriv_conc_eta_c;
        }
        else {
			planck_prev=prev_node->TElectron::planck_potential;
            temp=prev_node->TElectron::temperature;

        	if (grid_effects & GRID_FERMI_DIRAC) {
	            if (return_flag & D_ETA_C)
    	        	return_value.eta_c=-electron_richardson_const*sq(temp)*
        	        						log_1_x(exp(planck_prev));
            }
            else {
                if (return_flag & D_ETA_C)
                	return_value.eta_c=-electron_richardson_const*sq(temp)*
                    						exp(planck_prev);

            }
        }
    }

    if (next_node && (node==NEXT_NODE)) {
    	if (contact_flags & CONTACT_FINITERECOMB) {
			if (return_flag & D_ETA_C)
				return_value.eta_c=electron_recomb_vel*next_node->TElectron::total_deriv_conc_eta_c;
        }
        else {
			planck_next=next_node->TElectron::planck_potential;
            temp=next_node->TElectron::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
                if (return_flag & D_ETA_C)
            		return_value.eta_c=electron_richardson_const*sq(temp)*
                    						log_1_x(exp(planck_next));
            }
            else {
                if (return_flag & D_ETA_C)
            		return_value.eta_c=electron_richardson_const*sq(temp)*
                    						exp(planck_next);
            }
        }
    }

	return(return_value);
}

prec TOhmicBoundaryElement::comp_hole_current(void)
{
	prec current;
    prec planck_prev, planck_next;
    prec temp;

	if (prev_node) {
    	if (contact_flags & CONTACT_FINITERECOMB)
			current=hole_recomb_vel*(prev_node->THole::total_conc-equil_hole_conc);
    	else {
			planck_prev=prev_node->THole::planck_potential;
            temp=prev_node->THole::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
				current=hole_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(planck_prev)),2.0)
																  +dilog(1.0/(1.0+exp(-planck_prev))))
					   -hole_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(-hole_barrier)),2.0)
																  +dilog(1.0/(1.0+exp(hole_barrier))));
			}
			else {
				current=hole_richardson_const*sq(temp)*exp(planck_prev)
					   -hole_richardson_const*sq(temp)*exp(-hole_barrier);
			}
        }
    }
    else {
    	if (contact_flags & CONTACT_FINITERECOMB)
			current=-hole_recomb_vel*(next_node->THole::total_conc-equil_hole_conc);
		else {
			planck_next=next_node->THole::planck_potential;
            temp=next_node->THole::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
				current=hole_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(-hole_barrier)),2.0)
																  +dilog(1.0/(1.0+exp(hole_barrier))))
					   -hole_richardson_const*sq(temp)*(0.5*pow(log_1_x(exp(planck_next)),2.0)
																  +dilog(1.0/(1.0+exp(-planck_next))));
			}
			else {
				current=hole_richardson_const*sq(temp)*exp(-hole_barrier)
					   -hole_richardson_const*sq(temp)*exp(planck_next);
			}
        }
    }

    return(current);
}

FundamentalParam TOhmicBoundaryElement::comp_deriv_hole_current(NodeSide node,
																int return_flag)
{
	FundamentalParam return_value;
    prec planck_prev, planck_next;
    prec temp;

	return_value.psi=return_value.eta_c=return_value.eta_v=0.0;

    if (prev_node && (node==PREVIOUS_NODE)) {
    	if (contact_flags & CONTACT_FINITERECOMB) {
        	if (return_flag & D_ETA_V)
				return_value.eta_v=hole_recomb_vel*prev_node->THole::total_deriv_conc_eta_v;
        }
        else {
			planck_prev=prev_node->THole::planck_potential;
            temp=prev_node->THole::temperature;

			if (grid_effects & GRID_FERMI_DIRAC) {
                if (return_flag & D_ETA_V)
                	return_value.eta_v=hole_richardson_const*sq(temp)*
                    					log_1_x(exp(planck_prev));
            }
            else {
                if (return_flag & D_ETA_V)
                	return_value.eta_v=hole_richardson_const*sq(temp)*
                    					exp(planck_prev);
            }
        }
    }

    if (next_node && (node==NEXT_NODE)) {
    	if (contact_flags & CONTACT_FINITERECOMB) {
			if (return_flag & D_ETA_V)
				return_value.eta_v=-hole_recomb_vel*next_node->THole::total_deriv_conc_eta_v;
        }
        else {
			planck_next=next_node->THole::planck_potential;
            temp=next_node->THole::temperature;

            if (grid_effects & GRID_FERMI_DIRAC) {
                if (return_flag & D_ETA_V)
                	return_value.eta_v=-hole_richardson_const*sq(temp)*
                    						log_1_x(exp(planck_next));
        	}
            else {
                if (return_flag & D_ETA_V)
                	return_value.eta_v=-hole_richardson_const*sq(temp)*
                    						exp(planck_next);
            }
        }
    }

	return(return_value);
}

FundamentalParam TOhmicBoundaryElement::comp_deriv_integral_charge(ElementSide /*side*/,
																		NodeSide /*node*/,
																		int /*return_flag*/,
																		SolveType /*solve*/)
{
	FundamentalParam result={0,0,0};
	return(result);
}

FundamentalParam TOhmicBoundaryElement::comp_deriv_integral_recomb(ElementSide /*side*/,
																		NodeSide /*node*/,
																		int /*return_flag*/)
{
	FundamentalParam result={0,0,0};
	return(result);
}


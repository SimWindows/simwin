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
#include "simelem.h"
#include "simecele.h"
#include "simthele.h"
#include "simdev.h"
#include "simsol.h"

/************************************** class TSolution ***************************************

class TSolution {
private:
	SolveType solve_type;
	flag device_effects;
	flag contact_flag_0;
	flag contact_flag_1;
	flag surface_flag_0;
	flag surface_flag_1;
	TDevice *device_ptr;
	int quantum_wells;
	TQuantumWell **qw_ptr;
	int device_grid_points;
	TNode **device_grid_ptr;
	int solution_grid_points;
	TNode **solution_grid_ptr;
	int elements;
	int electrical_start_node;
	int electrical_end_node;
	int thermal_start_node;
	int thermal_end_node;
	int electrical_unknown_nodes;
	int thermal_unknown_nodes;
	TElectricalElement **electrical_element_ptr;
	TThermalElement **thermal_element_ptr;
	prec **electrical_jacobian;
	prec *electrical_solution[3];
	int number_elect_variables;
	prec **thermal_jacobian;
	prec *thermal_solution;
public:
	TSolution(TDevice *device, TNode** grd_ptr,
			  TQuantumWell **qwell_ptr);
	~TSolution(void);
	void apply_electrical_boundary(void);
	void apply_thermal_boundary(void);
	void comp_independent_param(void);
	void comp_thermoelectric_param(void);
	void comp_electrical_boundary(void);
	void comp_thermal_boundary(void);
	void store_temperature(void);
	void set_solution(SolveType type);
	void electrical_iterate(FundamentalParam& iteration_error);
	void thermal_iterate(prec& iteration_error);
	void outer_thermal_update_device(void);
private:
	void establish_elements(void);
	void comp_electrical_dep_param(SolveType solve);
	void comp_thermal_dep_param(void);
	void comp_deriv_recomb(void);
	void comp_deriv_conc(void);
	void comp_deriv_thermal_conduct(void);
	void comp_deriv_electron_hotcarriers(void);
	void comp_electrical_jacobian(void);
	void comp_electrical_solution(void);
	void comp_thermal_jacobian(void);
	void comp_thermal_solution(void);
	void factor_jacobian(prec **jacobian, int variables, int unknown_nodes);
	void solve_electrical_jacobian(void);
	void solve_thermal_jacobian(void);
	void electrical_update_device(void);
	void thermal_update_device(void);
	void electrical_update_sub_nodes(void);
	void thermal_update_sub_nodes(void);
	FundamentalParam comp_electrical_error(void);
	prec comp_thermal_error(void);
	FundamentalParam comp_deriv_poisson(int i, NodeSide node, int return_flag);
	FundamentalParam comp_deriv_electron_rate(int i, NodeSide node, int return_flag);
	FundamentalParam comp_deriv_hole_rate(int i, NodeSide node, int return_flag);
	prec comp_deriv_heat_rate(int i, NodeSide node);
	prec comp_deriv_rate_hotcarriers_electron(int i, NodeSide node);
};
*/

TSolution::TSolution(TDevice *device, TNode** grd_ptr,
					 TQuantumWell **qwell_ptr)
{
	device_ptr=device;
	device_grid_points=device_ptr->get_number_objects(NODE);
	device_grid_ptr=grd_ptr;
	quantum_wells=device_ptr->get_number_objects(QUANTUM_WELL);
	qw_ptr=qwell_ptr;
	solve_type=(SolveType)device_ptr->get_value(DEVICE,CURRENT_SOLUTION,0);

	contact_flag_0=contact_flag_1=(flag)0;
	surface_flag_0=surface_flag_1=(flag)0;
	solution_grid_points=0;
	elements=0;
	electrical_start_node=0;
	electrical_end_node=0;
	thermal_start_node=0;
	thermal_end_node=0;
	electrical_unknown_nodes=0;
	thermal_unknown_nodes=0;
	device_effects=0;
	electrical_solution[0]=(prec *)0;
	electrical_solution[1]=(prec *)0;
	electrical_solution[2]=(prec *)0;
	electrical_jacobian=(prec **)0;
	thermal_solution=(prec *)0;
	thermal_jacobian=(prec **)0;
	electrical_element_ptr=(TElectricalElement **)0;
	thermal_element_ptr=(TThermalElement **)0;
	solution_grid_ptr=(TNode **)0;
	number_elect_variables=0;

	establish_elements();
}

TSolution::~TSolution(void)
{
	int i;

	if (electrical_solution[0]) delete[] electrical_solution[0];
	if (electrical_solution[1]) delete[] electrical_solution[1];
	if (electrical_solution[2]) delete[] electrical_solution[2];

	if (electrical_jacobian) {
		for (i=0;i<solution_grid_points;i++) delete[] electrical_jacobian[i];
		delete[] electrical_jacobian;
	}

	if (thermal_solution) delete[] thermal_solution;

	if (thermal_jacobian) {
		for (i=0;i<solution_grid_points;i++) delete[] thermal_jacobian[i];
		delete[] thermal_jacobian;
	}

	if (electrical_element_ptr) {
		for (i=0;i<elements;i++) delete electrical_element_ptr[i];
		delete[] electrical_element_ptr;
	}

	if (thermal_element_ptr) {
		for (i=0;i<elements;i++) delete thermal_element_ptr[i];
		delete[] thermal_element_ptr;
	}

	if (solution_grid_ptr) delete[] solution_grid_ptr;
}

void TSolution::apply_electrical_boundary(void)
{
	(*electrical_element_ptr)->apply_boundary();
	(*(electrical_element_ptr+elements-1))->apply_boundary();
}

void TSolution::apply_thermal_boundary(void)
{
	(*thermal_element_ptr)->apply_boundary();
	(*(thermal_element_ptr+elements-1))->apply_boundary();
}

void TSolution::comp_electrical_dep_param(SolveType solve)
{
	int i;
	TElectricalElement **temp_ele_ptr;

	temp_ele_ptr=electrical_element_ptr;
	for (i=0;i<elements;i++) (*(temp_ele_ptr++))->comp_dependent_param(solve);
}

void TSolution::comp_thermal_dep_param(void)
{
	int i;
	TThermalElement **temp_therm_ptr;

	temp_therm_ptr=thermal_element_ptr;
	for (i=0;i<elements;i++) (*(temp_therm_ptr++))->comp_dependent_param();
}

void TSolution::comp_independent_param(void)
{
	int i;
	TElectricalElement **temp_ele_ptr;
	TThermalElement **temp_therm_ptr;

	temp_ele_ptr=electrical_element_ptr;
	for (i=0;i<elements;i++) (*(temp_ele_ptr++))->comp_independent_param();

	temp_therm_ptr=thermal_element_ptr;
	for (i=0;i<elements;i++) (*(temp_therm_ptr++))->comp_independent_param();
}

void TSolution::store_temperature(void)
{
	int i;
	TNode** temp_ptr;

	temp_ptr=device_grid_ptr;
	for (i=0;i<device_grid_points;i++) (*(temp_ptr++))->store_temperature();
}

void TSolution::comp_electrical_boundary(void)
{
	(*electrical_element_ptr)->comp_boundary_param();
	(*(electrical_element_ptr+elements-1))->comp_boundary_param();
}

void TSolution::comp_thermal_boundary(void)
{
	(*thermal_element_ptr)->comp_boundary_param();
	(*(thermal_element_ptr+elements-1))->comp_boundary_param();
}

void TSolution::comp_thermoelectric_param(void)
{
	int i;
	TThermalElement **temp_therm_ptr;

	temp_therm_ptr=thermal_element_ptr;
	for (i=0;i<elements;i++) (*(temp_therm_ptr++))->comp_thermoelectric_param();
}

void TSolution::establish_elements(void)
{
	int i, req_elements, req_solution_grid_points;
	TNode** temp_solution_grid_ptr;
	TNode** temp_device_grid_ptr;
	TQuantumWell **temp_qw_ptr;

	req_solution_grid_points=device_grid_points;
	for (i=0;i<quantum_wells;i++) {
		req_solution_grid_points-=((*(qw_ptr+i))->get_value(QUANTUM_WELL,NUMBER_NODES)-1);
	}

	solution_grid_ptr=new TNode*[req_solution_grid_points];
	if (!solution_grid_ptr) {
		error_handler.set_error(ERROR_MEM_SOLUTION_GRID,0,"","");
		return;
	}
	temp_solution_grid_ptr=solution_grid_ptr;
	temp_qw_ptr=qw_ptr;
	i=0;
	do {
		temp_device_grid_ptr=device_grid_ptr+i;
		if ((RegionType)(*temp_device_grid_ptr)->get_value(GRID_ELECTRICAL,REGION_TYPE,NORMALIZED)==BULK) {
			*(temp_solution_grid_ptr++)=*temp_device_grid_ptr;
			i++;
		}
		else {
			*(temp_solution_grid_ptr++)=*(device_grid_ptr+(*temp_qw_ptr)->get_node(CURRENT_NODE));
			solution_grid_points++;
			i=(*(temp_qw_ptr++))->get_node(NEXT_NODE);
			temp_device_grid_ptr=device_grid_ptr+i;
			*(temp_solution_grid_ptr++)=*temp_device_grid_ptr;
			i++;
		}
		solution_grid_points++;
	}
	while (i<device_grid_points);

	req_elements=req_solution_grid_points+1;
	electrical_element_ptr=new TElectricalElement*[req_elements];
	thermal_element_ptr=new TThermalElement*[req_elements];
	if ((!electrical_element_ptr) || (!thermal_element_ptr)) {
		error_handler.set_error(ERROR_MEM_SOLUTION_ELEMENT,0,"","");
		return;
	}

	temp_solution_grid_ptr=solution_grid_ptr;

	*electrical_element_ptr=new TOhmicBoundaryElement(device_ptr,NULL,*temp_solution_grid_ptr,0);
	*thermal_element_ptr=new TBoundaryThermalElement(device_ptr,NULL,*temp_solution_grid_ptr,0);
	if ((!(*thermal_element_ptr)) || (!(*electrical_element_ptr))) {
		error_handler.set_error(ERROR_MEM_SOLUTION_ELEMENT,0,"","");
		return;
	}

	elements++;

	for (i=1;i<req_elements-1;i++) {
		if ((RegionType)(*(temp_solution_grid_ptr+1))->get_value(GRID_ELECTRICAL,REGION_TYPE,NORMALIZED)==QW) {
			*(electrical_element_ptr+i)=new TQWElectricalElement(device_ptr, device_grid_ptr,
																 *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
			*(thermal_element_ptr+i)=new TQWThermalElement(device_ptr,device_grid_ptr,
														   *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
		}
		else {
			if ((RegionType)(*(temp_solution_grid_ptr))->get_value(GRID_ELECTRICAL,REGION_TYPE,NORMALIZED)==QW) {
				*(electrical_element_ptr+i)=new TQWElectricalElement(device_ptr, device_grid_ptr,
																	 *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
				*(thermal_element_ptr+i)=new TQWThermalElement(device_ptr, device_grid_ptr,
															   *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
			}
			else {
				*(electrical_element_ptr+i)=new TBulkElectricalElement(device_ptr, device_grid_ptr,
																	   *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
				*(thermal_element_ptr+i)=new TBulkThermalElement(device_ptr, device_grid_ptr,
																 *(temp_solution_grid_ptr),*(temp_solution_grid_ptr+1));
			}
		}
		if ((!(*(electrical_element_ptr+i))) || (!(*(thermal_element_ptr+i)))) {
			error_handler.set_error(ERROR_MEM_SOLUTION_ELEMENT,0,"","");
			return;
		}

		elements++;
		temp_solution_grid_ptr++;
	}

	*(electrical_element_ptr+req_elements-1)=new TOhmicBoundaryElement(device_ptr,*temp_solution_grid_ptr,NULL,1);
	*(thermal_element_ptr+req_elements-1)=new TBoundaryThermalElement(device_ptr,*temp_solution_grid_ptr,NULL,1);
	if ((!(*(thermal_element_ptr+req_elements-1))) || (!(*(electrical_element_ptr+req_elements-1)))) {
		error_handler.set_error(ERROR_MEM_SOLUTION_ELEMENT,0,"","");
		return;
	}
	elements++;
}

void TSolution::set_solution(SolveType type)
{
	int i;

	device_effects=(flag)device_ptr->get_value(DEVICE,EFFECTS,0);
	for (i=0;i<elements;i++) (*(electrical_element_ptr+i))->get_effects();
	for (i=0;i<elements;i++) (*(thermal_element_ptr+i))->get_effects();

	solve_type=type;

	if (!electrical_jacobian) {
		electrical_jacobian = new prec*[3*solution_grid_points];
		if (!electrical_jacobian) {
			error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
			return;
		}
		for (i=0;i<3*solution_grid_points;i++) {
			*(electrical_jacobian+i)=new prec[9];
			if (!(*(electrical_jacobian+i))) {
				error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
				return;
			}
		}
	}

	if (!electrical_solution[0]) {
		electrical_solution[0] = new prec[solution_grid_points];
		if (!electrical_solution[0]) {
			error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
			return;
		}
	}

	if (!electrical_solution[1]) {
		electrical_solution[1] = new prec[solution_grid_points];
		if (!electrical_solution[1]) {
			error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
			return;
		}
	}

	if (!electrical_solution[2]) {
		electrical_solution[2] = new prec[solution_grid_points];
		if (!electrical_solution[2]) {
			error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
			return;
		}
	}

	if ((device_effects & DEVICE_NON_ISOTHERMAL) &&	(solve_type!=EQUILIBRIUM)) {
		if (!thermal_jacobian) {
			thermal_jacobian = new prec*[solution_grid_points];
			if (!thermal_jacobian) {
				error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
				return;
			}
			for (i=0;i<solution_grid_points;i++) {
				*(thermal_jacobian+i)=new prec[3];
				if (!(*(electrical_jacobian+i))) {
					error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
					return;
				}
			}
		}

		if (!thermal_solution) {
			thermal_solution= new prec[solution_grid_points];
			if (!thermal_solution) {
				error_handler.set_error(ERROR_MEM_SOLUTION_ARRAYS,0,"","");
				return;
			}
		}
	}

	contact_flag_0=(flag)device_ptr->get_value(CONTACT,EFFECTS,0);
	contact_flag_1=(flag)device_ptr->get_value(CONTACT,EFFECTS,1);
	surface_flag_0=(flag)device_ptr->get_value(SURFACE,EFFECTS,0);
	surface_flag_1=(flag)device_ptr->get_value(SURFACE,EFFECTS,1);

	switch(solve_type) {
		case EQUILIBRIUM:
			number_elect_variables=1;
			electrical_start_node=1;
			electrical_end_node=solution_grid_points-2;
			break;
		case STEADY_STATE:
			number_elect_variables=3;
			if (contact_flag_0 & CONTACT_IDEALOHMIC) electrical_start_node=1;
			else electrical_start_node=0;
			if (contact_flag_1 & CONTACT_IDEALOHMIC) electrical_end_node=solution_grid_points-2;
			else electrical_end_node=solution_grid_points-1;
			if (device_effects & (DEVICE_SINGLE_TEMP |DEVICE_VARY_LATTICE_TEMP)){
				if (surface_flag_0 & SURFACE_HEAT_SINK)thermal_start_node=1;
				else thermal_start_node=0;
				if (surface_flag_1 & SURFACE_HEAT_SINK) thermal_end_node=solution_grid_points-2;
				else thermal_end_node=solution_grid_points-1;
			}
			else {
				thermal_start_node=1;
				thermal_end_node=solution_grid_points-2;
			}
			thermal_unknown_nodes=thermal_end_node-thermal_start_node+1;
			break;
		default: break;
	}
	electrical_unknown_nodes=electrical_end_node-electrical_start_node+1;
}

void TSolution::comp_deriv_recomb(void)
{
	int i;

	TNode**temp_ptr;

	for (i=0;i<quantum_wells;i++) (*(qw_ptr+i))->comp_deriv_recomb();

	temp_ptr=device_grid_ptr;
	for (i=0;i<device_grid_points;i++) (*temp_ptr++)->comp_deriv_recomb();
}

void TSolution::comp_deriv_conc(void)
{
	int i;
	TNode** temp_ptr;

	for (i=0;i<quantum_wells;i++) (*(qw_ptr+i))->comp_deriv_conc();

	temp_ptr=device_grid_ptr;
	for (i=0;i<device_grid_points;i++) (*(temp_ptr++))->comp_deriv_conc();
}

void TSolution::comp_deriv_thermal_conduct(void)
{
	int i;

	TNode**temp_ptr;

	temp_ptr=device_grid_ptr;
	for (i=0;i<device_grid_points;i++) (*(temp_ptr++))->comp_deriv_thermal_conduct();
}

void TSolution::comp_deriv_electron_hotcarriers(void)
{
	int i;

	TNode**temp_ptr;

	temp_ptr=device_grid_ptr;
	for (i=0;i<device_grid_points;i++) (*temp_ptr++)->comp_deriv_electron_hotcarriers();
}

void TSolution::comp_electrical_jacobian(void)
{
	int i,k,l;
	prec **jacobian_ptr;
	prec *row_ptr;
	int required_deriv;

	FundamentalParam poisson_deriv_prev,
					  electron_rate_deriv_prev,
					  hole_rate_deriv_prev;

	FundamentalParam poisson_deriv_curr,
					  electron_rate_deriv_curr,
					  hole_rate_deriv_curr;

	FundamentalParam poisson_deriv_next,
					  electron_rate_deriv_next,
					  hole_rate_deriv_next;

	jacobian_ptr=electrical_jacobian;

	switch(solve_type) {
		case EQUILIBRIUM:

			for (i=electrical_start_node;i<=electrical_end_node;i++) {
				row_ptr=*(jacobian_ptr);

				// Previous element value

				if (i==electrical_start_node) *(row_ptr++)=0;
				else {
					poisson_deriv_prev=comp_deriv_poisson(i,PREVIOUS_NODE,D_PSI);
					*(row_ptr++)=poisson_deriv_prev.psi;
				}

				// Current element value

				poisson_deriv_curr=comp_deriv_poisson(i,CURRENT_NODE,D_PSI);
				*(row_ptr++)=poisson_deriv_curr.psi;

				// Next element value

				if (i==electrical_end_node) *(row_ptr)=0;
				else {
					poisson_deriv_next=comp_deriv_poisson(i,NEXT_NODE,D_PSI);
					*(row_ptr)=poisson_deriv_next.psi;
				}
				jacobian_ptr++;
			}
			break;
		case STEADY_STATE:

			required_deriv=ALL_DERIV;

			for (i=electrical_start_node;i<=electrical_end_node;i++) {
				for (k=0;k<3;k++) {
					row_ptr=*(jacobian_ptr);
					switch(k) {
						case 0:
							if (i==electrical_start_node) {
								for (l=0;l<3;l++) *(row_ptr++)=0;
							}
							else {
								electron_rate_deriv_prev=comp_deriv_electron_rate(i,PREVIOUS_NODE,required_deriv);

								*(row_ptr++)=electron_rate_deriv_prev.eta_c;
								*(row_ptr++)=electron_rate_deriv_prev.psi;
								*(row_ptr++)=electron_rate_deriv_prev.eta_v;
							}

							electron_rate_deriv_curr=comp_deriv_electron_rate(i,CURRENT_NODE,required_deriv);

							*(row_ptr++)=electron_rate_deriv_curr.eta_c;
							*(row_ptr++)=electron_rate_deriv_curr.psi;
							*(row_ptr++)=electron_rate_deriv_curr.eta_v;

							if (i==electrical_end_node) {
								for (l=0;l<3;l++) *(row_ptr++)=0;
							}
							else {
								electron_rate_deriv_next=comp_deriv_electron_rate(i,NEXT_NODE,required_deriv);

								*(row_ptr++)=electron_rate_deriv_next.eta_c;
								*(row_ptr++)=electron_rate_deriv_next.psi;
								*(row_ptr)=electron_rate_deriv_next.eta_v;
							}

							break;
						case 1:
							if (i==electrical_start_node) {
								*(row_ptr++)=0.0;
								*(row_ptr++)=0.0;
								*(row_ptr++)=0.0;

                                if (!(contact_flag_0 & CONTACT_IDEALOHMIC)) {
                                	*(row_ptr++)=0.0;
                                    *(row_ptr++)=1.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr)=0.0;
                                }
                                else {
									poisson_deriv_curr=comp_deriv_poisson(i,CURRENT_NODE,required_deriv);

									*(row_ptr++)=poisson_deriv_curr.eta_c;
									*(row_ptr++)=poisson_deriv_curr.psi;
									*(row_ptr++)=poisson_deriv_curr.eta_v;

									poisson_deriv_next=comp_deriv_poisson(i,NEXT_NODE,required_deriv);

									*(row_ptr++)=poisson_deriv_next.eta_c;
									*(row_ptr++)=poisson_deriv_next.psi;
									*(row_ptr)=poisson_deriv_next.eta_v;
                                }
                                break;

							}

                        	if (i==electrical_end_node) {
                            	if (!(contact_flag_1 & CONTACT_IDEALOHMIC)) {
                                	*(row_ptr++)=0.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr++)=0.0;
                                    *(row_ptr++)=1.0;
                                    *(row_ptr++)=0.0;
                                }
                                else {
 									poisson_deriv_prev=comp_deriv_poisson(i,PREVIOUS_NODE,required_deriv);

									*(row_ptr++)=poisson_deriv_prev.eta_c;
									*(row_ptr++)=poisson_deriv_prev.psi;
									*(row_ptr++)=poisson_deriv_prev.eta_v;

		 							poisson_deriv_curr=comp_deriv_poisson(i,CURRENT_NODE,required_deriv);

									*(row_ptr++)=poisson_deriv_curr.eta_c;
									*(row_ptr++)=poisson_deriv_curr.psi;
									*(row_ptr++)=poisson_deriv_curr.eta_v;
                                }

								*(row_ptr++)=0.0;
								*(row_ptr++)=0.0;
								*(row_ptr)=0.0;

                                break;
							}

                        	poisson_deriv_prev=comp_deriv_poisson(i,PREVIOUS_NODE,required_deriv);

                            *(row_ptr++)=poisson_deriv_prev.eta_c;
                            *(row_ptr++)=poisson_deriv_prev.psi;
                        	*(row_ptr++)=poisson_deriv_prev.eta_v;

                            poisson_deriv_curr=comp_deriv_poisson(i,CURRENT_NODE,required_deriv);

                            *(row_ptr++)=poisson_deriv_curr.eta_c;
                            *(row_ptr++)=poisson_deriv_curr.psi;
                            *(row_ptr++)=poisson_deriv_curr.eta_v;

                            poisson_deriv_next=comp_deriv_poisson(i,NEXT_NODE,required_deriv);

                            *(row_ptr++)=poisson_deriv_next.eta_c;
                            *(row_ptr++)=poisson_deriv_next.psi;
                            *(row_ptr)=poisson_deriv_next.eta_v;

                            break;

						case 2:
							if (i==electrical_start_node) {
								for (l=0;l<3;l++) *(row_ptr++)=0;
							}
							else {
								hole_rate_deriv_prev=comp_deriv_hole_rate(i,PREVIOUS_NODE,required_deriv);

								*(row_ptr++)=hole_rate_deriv_prev.eta_c;
								*(row_ptr++)=hole_rate_deriv_prev.psi;
								*(row_ptr++)=hole_rate_deriv_prev.eta_v;
							}

							hole_rate_deriv_curr=comp_deriv_hole_rate(i,CURRENT_NODE,required_deriv);

							*(row_ptr++)=hole_rate_deriv_curr.eta_c;
							*(row_ptr++)=hole_rate_deriv_curr.psi;
							*(row_ptr++)=hole_rate_deriv_curr.eta_v;

							if (i==electrical_end_node) {
								for (l=0;l<3;l++) *(row_ptr++)=0;
							}
							else {
								hole_rate_deriv_next=comp_deriv_hole_rate(i,NEXT_NODE,required_deriv);

								*(row_ptr++)=hole_rate_deriv_next.eta_c;
								*(row_ptr++)=hole_rate_deriv_next.psi;
								*(row_ptr)=hole_rate_deriv_next.eta_v;
							}

							break;
						default: break;
					}
					jacobian_ptr++;
				}
			}
		default: break;
	}
}

void TSolution::comp_electrical_solution(void)
{
	int i;

	prec electron_current_0, electron_current_1;
	prec hole_current_0, hole_current_1;
	prec recombination_integral;
	prec field_0, field_1;

	TElectricalElement** temp_ele_ptr;
	TElectricalElement* prev_elem, *next_elem;
	prec *solution_ptr_0, *solution_ptr_1, *solution_ptr_2;

	solution_ptr_0=electrical_solution[0];
	solution_ptr_1=electrical_solution[1];
	solution_ptr_2=electrical_solution[2];
	temp_ele_ptr=electrical_element_ptr+electrical_start_node;

	prev_elem=*(temp_ele_ptr++);

	switch(solve_type) {
		case EQUILIBRIUM:

			field_0=prev_elem->comp_field();

			for (i=electrical_start_node;i<=electrical_end_node;i++) {
				next_elem=*temp_ele_ptr++;

				field_1=next_elem->comp_field();

				*(solution_ptr_0++)=field_1-field_0-
									next_elem->comp_integral_charge(FIRSTHALF)-
									prev_elem->comp_integral_charge(SECONDHALF);

				field_0=field_1;

				prev_elem=next_elem;
			}
			break;
		case STEADY_STATE:
			electron_current_0=prev_elem->comp_electron_current();
			hole_current_0=prev_elem->comp_hole_current();
			field_0=prev_elem->comp_field();

			for (i=electrical_start_node;i<=electrical_end_node;i++) {
				next_elem=*temp_ele_ptr;

				electron_current_1=next_elem->comp_electron_current();
				hole_current_1=next_elem->comp_hole_current();
				field_1=next_elem->comp_field();

				recombination_integral=prev_elem->comp_integral_recomb(SECONDHALF)+
									   next_elem->comp_integral_recomb(FIRSTHALF);

            	*(solution_ptr_0++)=electron_current_1-electron_current_0-
                			 		recombination_integral;

                if ( ((i==electrical_start_node) && !(contact_flag_0 & CONTACT_IDEALOHMIC)) ||
                	 ((i==electrical_end_node) && !(contact_flag_1 & CONTACT_IDEALOHMIC)) ){
                	*(solution_ptr_1++)=0.0;
                }
                else {
                	*(solution_ptr_1++)=field_1-field_0-
                    					next_elem->comp_integral_charge(FIRSTHALF)-
                                        prev_elem->comp_integral_charge(SECONDHALF);
                }

                *(solution_ptr_2++)=hole_current_1-hole_current_0+
                					recombination_integral;

				electron_current_0=electron_current_1;
				hole_current_0=hole_current_1;
				field_0=field_1;

				prev_elem=next_elem;

				temp_ele_ptr++;
			}
			break;
	}
}

void TSolution::comp_thermal_jacobian(void)
{
	int i;
	prec **jacobian_ptr;
	prec *row_ptr;

	jacobian_ptr=thermal_jacobian;

	if (device_effects & (DEVICE_SINGLE_TEMP | (DEVICE_VARY_LATTICE_TEMP))) {
		for (i=thermal_start_node;i<=thermal_end_node;i++) {
			row_ptr=*(jacobian_ptr);

			// Previous element value

			if (i==thermal_start_node) *(row_ptr++)=0;
			else *(row_ptr++)=comp_deriv_heat_rate(i,PREVIOUS_NODE);

			// Current element value

			*(row_ptr++)=comp_deriv_heat_rate(i,CURRENT_NODE);

			// Next element value

			if (i==thermal_end_node) *(row_ptr)=0;
			else *(row_ptr)=comp_deriv_heat_rate(i,NEXT_NODE);

			jacobian_ptr++;
		}
	}
	else {
		if (device_effects & (DEVICE_VARY_ELECTRON_TEMP)) {
			for (i=thermal_start_node;i<=thermal_end_node;i++) {
				row_ptr=*(jacobian_ptr);

				// Previous element value

				if (i==thermal_start_node) *(row_ptr++)=0;
				else *(row_ptr++)=comp_deriv_rate_hotcarriers_electron(i,PREVIOUS_NODE);

				// Current element value

				*(row_ptr++)=comp_deriv_rate_hotcarriers_electron(i,CURRENT_NODE);

				// Next element value

				if (i==thermal_end_node) *(row_ptr)=0;
				else *(row_ptr)=comp_deriv_rate_hotcarriers_electron(i,NEXT_NODE);

				jacobian_ptr++;
			}
		}
	}
}

void TSolution::comp_thermal_solution(void)
{
	int i;
	TThermalElement **temp_ele_ptr;
	TThermalElement *prev_elem, *next_elem;
	prec *solution_ptr;
	prec lat_heat_flow_0, lat_heat_flow_1;
	prec electron_heat_flow_0, electron_heat_flow_1;
	prec hole_heat_flow_0, hole_heat_flow_1;
	prec trans_heat_flow_0, trans_heat_flow_1;

	solution_ptr=thermal_solution;
	temp_ele_ptr=thermal_element_ptr+thermal_start_node;

	prev_elem=*(temp_ele_ptr++);

	if (device_effects & (DEVICE_SINGLE_TEMP | (DEVICE_VARY_LATTICE_TEMP)))	{
		lat_heat_flow_0=prev_elem->comp_lat_heat_flow();
		electron_heat_flow_0=prev_elem->comp_electron_heat_flow();
		hole_heat_flow_0=prev_elem->comp_hole_heat_flow();
		trans_heat_flow_0=prev_elem->comp_trans_heat_flow();

		for (i=thermal_start_node;i<=thermal_end_node;i++) {
			next_elem=*temp_ele_ptr++;

			lat_heat_flow_1=next_elem->comp_lat_heat_flow();
			electron_heat_flow_1=next_elem->comp_electron_heat_flow();
			hole_heat_flow_1=next_elem->comp_hole_heat_flow();
			trans_heat_flow_1=next_elem->comp_trans_heat_flow();

			if ( ((i==thermal_start_node) && !(surface_flag_0 & SURFACE_HEAT_SINK)) ||
				 ((i==thermal_end_node) &&   !(surface_flag_1 & SURFACE_HEAT_SINK)) ) {
				*(solution_ptr)=lat_heat_flow_1-lat_heat_flow_0;
			}
			else {
				*(solution_ptr)=lat_heat_flow_1+electron_heat_flow_1+hole_heat_flow_1-
								lat_heat_flow_0-electron_heat_flow_0-hole_heat_flow_0+
								prev_elem->comp_integral_rad_heat(SECONDHALF)+
								next_elem->comp_integral_rad_heat(FIRSTHALF);
			}

			*(solution_ptr)+=(trans_heat_flow_0+trans_heat_flow_1)/2.0;

			solution_ptr++;
			lat_heat_flow_0=lat_heat_flow_1;
			electron_heat_flow_0=electron_heat_flow_1;
			hole_heat_flow_0=hole_heat_flow_1;
			trans_heat_flow_0=trans_heat_flow_1;

			prev_elem=next_elem;
		}
	}
	else {
		if(device_effects & DEVICE_VARY_ELECTRON_TEMP) {
			electron_heat_flow_0=prev_elem->comp_electron_heat_flow();

			for (i=thermal_start_node;i<=thermal_end_node;i++) {
				next_elem=*temp_ele_ptr++;

				electron_heat_flow_1=next_elem->comp_electron_heat_flow();

				*(solution_ptr)=electron_heat_flow_1
								-electron_heat_flow_0+
								prev_elem->comp_integral_electron_hotcarriers(SECONDHALF)+
								next_elem->comp_integral_electron_hotcarriers(FIRSTHALF);

				solution_ptr++;
				electron_heat_flow_0=electron_heat_flow_1;
				prev_elem=next_elem;
			}
		}
	}
}

void TSolution::factor_jacobian(prec **jacobian, int variables, int unknown_nodes)
{
	int i,k,l,m;
	prec **current_row_ptr;
	prec *diagonal_ptr;
	prec **reduce_row_ptr;
	prec *reduce_element_ptr;

	current_row_ptr=jacobian;

	for (i=0;i<unknown_nodes;i++) {

		for (l=0;l<variables;l++) {
			diagonal_ptr=*(current_row_ptr)+variables+l;
			reduce_row_ptr=current_row_ptr+1;

			for (k=l+1;k<variables;k++) {
				reduce_element_ptr=*(reduce_row_ptr)+variables+l;

				*reduce_element_ptr/=(*(diagonal_ptr));

				for(m=1;m<variables-l;m++) {
					*(reduce_element_ptr+m)-=(*(reduce_element_ptr))*(*(diagonal_ptr+m));
				}

				if (i!=unknown_nodes-1) {
					for(m=0;m<variables;m++) {
						*(reduce_element_ptr+variables-l+m)-=
							(*(reduce_element_ptr))*(*(diagonal_ptr+variables-l+m));
					}
				}
				reduce_row_ptr++;
			}

			if (i!=unknown_nodes-1) {

				for (k=0;k<variables;k++) {
					reduce_element_ptr=*(reduce_row_ptr)+l;

					*(reduce_element_ptr)/=(*(diagonal_ptr));

					for(m=1;m<variables-l;m++) {
						*(reduce_element_ptr+m)-=(*(reduce_element_ptr))*(*(diagonal_ptr+m));
					}

					for(m=0;m<variables;m++) {
						*(reduce_element_ptr+variables-l+m)-=
							(*(reduce_element_ptr))*(*(diagonal_ptr+variables-l+m));
					}
					reduce_row_ptr++;
				}
			}
			current_row_ptr++;
		}
	}
}

void TSolution::solve_electrical_jacobian(void)
{
	int i,k,l;
	prec** jacobian_ptr;
	prec *diagonal_ptr;
	prec *solution_ptr[3];

//	Forward Solve

	jacobian_ptr=electrical_jacobian;
	solution_ptr[0]=electrical_solution[0];
	solution_ptr[1]=electrical_solution[1];
	solution_ptr[2]=electrical_solution[2];

	for (i=1;i<=electrical_unknown_nodes;i++) {
		for (k=0;k<number_elect_variables;k++) {
			diagonal_ptr=*(jacobian_ptr)+number_elect_variables+k;

			if (i!=1) {
				for (l=-number_elect_variables-k;l<=-1-k;l++)
					*(solution_ptr[k])-=*(diagonal_ptr+l)*(*(solution_ptr[l+k+number_elect_variables]-1));
			}

			for (l=-k;l<=-1;l++)
				*(solution_ptr[k])-=*(diagonal_ptr+l)*(*(solution_ptr[k+l]));

			jacobian_ptr++;
		}
		solution_ptr[0]++;
		solution_ptr[1]++;
		solution_ptr[2]++;
	}

//	Backward Solve

	jacobian_ptr=electrical_jacobian+number_elect_variables*electrical_unknown_nodes-1;
	solution_ptr[0]=electrical_solution[0]+electrical_unknown_nodes-1;
	solution_ptr[1]=electrical_solution[1]+electrical_unknown_nodes-1;
	solution_ptr[2]=electrical_solution[2]+electrical_unknown_nodes-1;

	for (i=electrical_unknown_nodes;i>=1;i--) {
		for (k=0;k<number_elect_variables;k++) {
			diagonal_ptr=*(jacobian_ptr)+2*number_elect_variables-(k+1);

			if (i!=electrical_unknown_nodes) {
				for (l=number_elect_variables+k;l>=1+k;l--)
					*(solution_ptr[number_elect_variables-1-k])-=*(diagonal_ptr+l)*(*(solution_ptr[l-k-1]+1));
			}

			for (l=k;l>=1;l--)
				*(solution_ptr[number_elect_variables-1-k])-=*(diagonal_ptr+l)*(*(solution_ptr[number_elect_variables-1-k+l]));

			*(solution_ptr[number_elect_variables-1-k])/=*(diagonal_ptr);

			jacobian_ptr--;
		}
		solution_ptr[0]--;
		solution_ptr[1]--;
		solution_ptr[2]--;
	}
}

void TSolution::solve_thermal_jacobian(void)
{
	int i,k,l;
	prec** jacobian_ptr;
	prec *diagonal_ptr;
	prec *solution_ptr;

//	Forward Solve

	jacobian_ptr=thermal_jacobian;
	solution_ptr=thermal_solution;

	for (i=1;i<=thermal_unknown_nodes;i++) {
		for (k=0;k<1;k++) {
			diagonal_ptr=*(jacobian_ptr)+1+k;

			if (i!=1) {
				for (l=-1-k;l<=-1-k;l++)
					*(solution_ptr)-=*(diagonal_ptr+l)*(*(solution_ptr+l));
			}

			for (l=-k;l<=-1;l++)
				*(solution_ptr)-=*(diagonal_ptr+l)*(*(solution_ptr+l));

			solution_ptr++;
			jacobian_ptr++;
		}
	}

//	Backward Solve

	jacobian_ptr=thermal_jacobian+1*thermal_unknown_nodes-1;
	solution_ptr=thermal_solution+1*thermal_unknown_nodes-1;

	for (i=thermal_unknown_nodes;i>=1;i--) {
		for (k=0;k<1;k++) {
			diagonal_ptr=*(jacobian_ptr)+2*1-(k+1);

			if (i!=thermal_unknown_nodes) {
				for (l=1+k;l>=1+k;l--)
					*(solution_ptr)-=*(diagonal_ptr+l)*(*(solution_ptr+l));
			}

			for (l=k;l>=1;l--)
				*(solution_ptr)-=*(diagonal_ptr+l)*(*(solution_ptr+l));

			*(solution_ptr)/=*(diagonal_ptr);

			solution_ptr--;
			jacobian_ptr--;
		}
	}
}

void TSolution::electrical_update_device(void)
{
	int i;
	prec *solution_ptr_0, *solution_ptr_1, *solution_ptr_2;
	prec clamp_value;
	TElectricalElement **temp_ptr;
	FundamentalParam update_amount;
	logical should_clamp;

	solution_ptr_0=electrical_solution[0];
	solution_ptr_1=electrical_solution[1];
	solution_ptr_2=electrical_solution[2];
	temp_ptr=electrical_element_ptr+electrical_start_node;

	clamp_value=environment.get_value(ENVIRONMENT,POT_CLAMP_VALUE);
	should_clamp=((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_CLAMP_POTENTIAL)!=0;

	switch(solve_type) {
		case EQUILIBRIUM:
			update_amount.eta_c=update_amount.eta_v=0.0;

			for (i=0;i<electrical_unknown_nodes;i++) {
				if (should_clamp) {
					if (fabs(*(solution_ptr_0))<clamp_value) update_amount.psi=*(solution_ptr_0);
					else {
						if (*(solution_ptr_0)>0) update_amount.psi=clamp_value;
						else update_amount.psi=-clamp_value;
					}
				}
				else update_amount.psi=*(solution_ptr_0);

				solution_ptr_0++;
				(*(temp_ptr++))->update(update_amount);
			}
			break;
		case STEADY_STATE:
			for (i=0;i<electrical_unknown_nodes;i++) {

				if (fabs(*(solution_ptr_0))<clamp_value) update_amount.eta_c=*(solution_ptr_0);
				else {
					if (*(solution_ptr_0)>0) update_amount.eta_c=clamp_value;
					else update_amount.eta_c=-clamp_value;
				}

				if (should_clamp) {
					if (fabs(*(solution_ptr_1))<clamp_value) update_amount.psi=*(solution_ptr_1);
					else {
						if (*(solution_ptr_1)>0) update_amount.psi=clamp_value;
						else update_amount.psi=-clamp_value;
					}
				}
				else update_amount.psi=*(solution_ptr_1);

				if (fabs(*(solution_ptr_2))<clamp_value) update_amount.eta_v=*(solution_ptr_2);
				else {
					if (*(solution_ptr_2)>0) update_amount.eta_v=clamp_value;
					else update_amount.eta_v=-clamp_value;
				}

				(*(temp_ptr++))->update(update_amount);

				solution_ptr_0++;
				solution_ptr_1++;
				solution_ptr_2++;
			}
			break;
		default: break;
	}
}

void TSolution::thermal_update_device(void)
{
	int i;
	prec *solution_ptr;
	TThermalElement **temp_ptr;
	prec update_amount;
	prec clamp_value;
	logical should_clamp;
	prec relaxation_value;

	solution_ptr=thermal_solution;
	temp_ptr=thermal_element_ptr+thermal_start_node;

	clamp_value=environment.get_value(ENVIRONMENT,TEMP_CLAMP_VALUE);
	relaxation_value=environment.get_value(ENVIRONMENT,TEMP_RELAX_VALUE);
	should_clamp=((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_CLAMP_TEMPERATURE)!=0;

	if (device_effects & (DEVICE_SINGLE_TEMP | (DEVICE_VARY_LATTICE_TEMP))) {
		for (i=0;i<thermal_unknown_nodes;i++) {
			if (should_clamp) {
				if (fabs(*solution_ptr)*relaxation_value<clamp_value) update_amount=(*solution_ptr)*relaxation_value;
				else {
					if (*solution_ptr>0) update_amount=clamp_value;
					else update_amount=-clamp_value;
				}
			}
			else update_amount=(*solution_ptr)*relaxation_value;
			(*(temp_ptr++))->update(update_amount,update_amount,update_amount);
			solution_ptr++;
		}
	}
	else {
		if(device_effects & DEVICE_VARY_ELECTRON_TEMP) {
			for (i=0;i<thermal_unknown_nodes;i++) {
				if (should_clamp) {
					if (fabs(*solution_ptr)*relaxation_value<clamp_value) update_amount=(*solution_ptr)*relaxation_value;
					else {
						if (*solution_ptr>0) update_amount=clamp_value;
						else update_amount=-clamp_value;
					}
				}
				else update_amount=(*solution_ptr)*relaxation_value;
				(*(temp_ptr++))->update(0.0,update_amount,0.0);
				solution_ptr++;
			}
		}
	}
}

void TSolution::outer_thermal_update_device(void)
{
	int i;
	TThermalElement **temp_therm_ptr;
	logical should_clamp;
	prec relaxation_value;
	prec clamp_value;

	should_clamp=((flag)environment.get_value(ENVIRONMENT,EFFECTS) & ENV_CLAMP_TEMPERATURE)!=0;
	relaxation_value=environment.get_value(ENVIRONMENT,TEMP_RELAX_VALUE);

	if (should_clamp) clamp_value=environment.get_value(ENVIRONMENT,TEMP_CLAMP_VALUE);
	else clamp_value=0.0;

	temp_therm_ptr=thermal_element_ptr+thermal_start_node;
	for (i=0;i<thermal_unknown_nodes;i++) {
		(*(temp_therm_ptr++))->outer_update(clamp_value,relaxation_value);
	}
	thermal_update_sub_nodes();
}

void TSolution::electrical_update_sub_nodes(void)
{
	int i;
	TElectricalElement **temp_ele_ptr;

	if (qw_ptr) {
		temp_ele_ptr=electrical_element_ptr;
		for (i=0;i<elements;i++) (*(temp_ele_ptr++))->update_sub_nodes();
	}
}

void TSolution::thermal_update_sub_nodes(void)
{
	int i;
	TThermalElement **temp_ele_ptr;

	if (qw_ptr) {
		temp_ele_ptr=thermal_element_ptr;
		for (i=0;i<elements;i++) (*(temp_ele_ptr++))->update_sub_nodes();
	}
}

FundamentalParam TSolution::comp_electrical_error(void)
{
	int i;
	FundamentalParam update, variable, error;
	TNode**temp_ptr;
	prec *solution_ptr_0, *solution_ptr_1, *solution_ptr_2;

	update.psi=variable.psi=error.psi=0;
	update.eta_c=variable.eta_c=error.eta_c=0;
	update.eta_v=variable.eta_v=error.eta_v=0;

	solution_ptr_0=electrical_solution[0];
	solution_ptr_1=electrical_solution[1];
	solution_ptr_2=electrical_solution[2];
	temp_ptr=solution_grid_ptr+electrical_start_node;

	switch(solve_type) {
		case EQUILIBRIUM:
			for (i=0;i<electrical_unknown_nodes;i++) {
				update.psi+=fabs(*(solution_ptr_0++));
				variable.psi+=fabs((*(temp_ptr++))->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED));
			}
			if (variable.psi==0) {
				if (update.psi==0) error.psi=0.0;
				else error.psi=1.0;
			}
			else error.psi=update.psi/variable.psi;
			break;
		case STEADY_STATE:
			for (i=0;i<electrical_unknown_nodes;i++) {
				update.eta_c+=fabs(*(solution_ptr_0++));
				update.psi+=fabs(*(solution_ptr_1++));
				update.eta_v+=fabs(*(solution_ptr_2++));
				variable.eta_c+=fabs((*temp_ptr)->get_value(ELECTRON,PLANCK_POT,NORMALIZED));
				variable.psi+=fabs((*temp_ptr)->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED));
				variable.eta_v+=fabs((*temp_ptr)->get_value(HOLE,PLANCK_POT,NORMALIZED));
				temp_ptr++;
			}
			if (variable.psi==0) {
				if (update.psi==0) error.psi=0.0;
				else error.psi=1.0;
			}
			else error.psi=update.psi/variable.psi;

			error.eta_c=update.eta_c/variable.eta_c;
			error.eta_v=update.eta_v/variable.eta_v;
	}
	return(error);
}

prec TSolution::comp_thermal_error(void)
{
	int i;
	prec update, variable;
	TNode **temp_ptr;
	prec *solution_ptr;

	solution_ptr=thermal_solution;
	temp_ptr=solution_grid_ptr+thermal_start_node;

	update=variable=0.0;

	if (device_effects & (DEVICE_SINGLE_TEMP | (DEVICE_VARY_LATTICE_TEMP))) {
		for (i=0;i<thermal_unknown_nodes;i++) {
			update+=fabs(*(solution_ptr++));
			variable+=fabs((*(temp_ptr++))->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED));
		}
	}
	else {
		if(device_effects & (DEVICE_VARY_ELECTRON_TEMP)) {
			for (i=0;i<thermal_unknown_nodes;i++) {
				update+=fabs(*(solution_ptr++));
				variable+=fabs((*(temp_ptr++))->get_value(ELECTRON,TEMPERATURE,NORMALIZED));
			}
		}
	}
	return(update/variable);
}

FundamentalParam TSolution::comp_deriv_poisson(int i, NodeSide node,
										  int return_flag)
{
	FundamentalParam result;
	TElectricalElement *prev_elem, *next_elem;
	FundamentalParam deriv_field_0, deriv_field_1;
	FundamentalParam deriv_integral_charge_0, deriv_integral_charge_1;

	result.psi=result.eta_c=result.eta_v=0;

	prev_elem=*(electrical_element_ptr+i);
	next_elem=*(electrical_element_ptr+i+1);

	switch(node) {
		case CURRENT_NODE:
			deriv_field_0=next_elem->comp_deriv_field(PREVIOUS_NODE,return_flag);
			deriv_field_1=prev_elem->comp_deriv_field(NEXT_NODE,return_flag);
			deriv_integral_charge_0=next_elem->comp_deriv_integral_charge(FIRSTHALF,PREVIOUS_NODE,return_flag,solve_type);
			deriv_integral_charge_1=prev_elem->comp_deriv_integral_charge(SECONDHALF,NEXT_NODE,return_flag,solve_type);

			if (return_flag & D_PSI)
				result.psi=deriv_field_0.psi-deriv_field_1.psi-
						   deriv_integral_charge_0.psi-
						   deriv_integral_charge_1.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=-deriv_integral_charge_0.eta_c
							 -deriv_integral_charge_1.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_charge_0.eta_v
							 -deriv_integral_charge_1.eta_v;

			return(result);

		case PREVIOUS_NODE:
			deriv_field_0=prev_elem->comp_deriv_field(PREVIOUS_NODE,return_flag);
			deriv_integral_charge_0=prev_elem->comp_deriv_integral_charge(SECONDHALF,PREVIOUS_NODE,return_flag,solve_type);

			if (return_flag & D_PSI)
				result.psi=-deriv_field_0.psi
						   -deriv_integral_charge_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=-deriv_integral_charge_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_charge_0.eta_v;

			return(result);

		case NEXT_NODE:
			deriv_field_0=next_elem->comp_deriv_field(NEXT_NODE,return_flag);
			deriv_integral_charge_0=next_elem->comp_deriv_integral_charge(FIRSTHALF,NEXT_NODE,return_flag,solve_type);

			if (return_flag & D_PSI)
				result.psi=deriv_field_0.psi
						   -deriv_integral_charge_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=-deriv_integral_charge_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_charge_0.eta_v;

			return(result);

		default: return(result);
	}
}

FundamentalParam TSolution::comp_deriv_electron_rate(int i, NodeSide node, int return_flag)
{
	FundamentalParam result,deriv_current_1, deriv_current_0;
	FundamentalParam deriv_integral_recomb_1, deriv_integral_recomb_0;
	TElectricalElement *next_elem, *prev_elem;

	result.eta_c=result.psi=result.eta_v=0;

	prev_elem=*(electrical_element_ptr+i);
	next_elem=*(electrical_element_ptr+i+1);

	switch(node) {
		case CURRENT_NODE:

			deriv_current_0=next_elem->comp_deriv_electron_current(PREVIOUS_NODE,return_flag);
			deriv_current_1=prev_elem->comp_deriv_electron_current(NEXT_NODE,return_flag);
			deriv_integral_recomb_0=next_elem->comp_deriv_integral_recomb(FIRSTHALF,PREVIOUS_NODE,return_flag);
			deriv_integral_recomb_1=prev_elem->comp_deriv_integral_recomb(SECONDHALF,NEXT_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=deriv_current_0.psi-deriv_current_1.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=deriv_current_0.eta_c-deriv_current_1.eta_c-
							 deriv_integral_recomb_0.eta_c-
							 deriv_integral_recomb_1.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_recomb_0.eta_v-
							  deriv_integral_recomb_1.eta_v;

			return(result);

		case PREVIOUS_NODE:

			deriv_current_0=prev_elem->comp_deriv_electron_current(PREVIOUS_NODE,return_flag);
			deriv_integral_recomb_0=prev_elem->comp_deriv_integral_recomb(SECONDHALF,PREVIOUS_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=-deriv_current_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=-deriv_current_0.eta_c
							 -deriv_integral_recomb_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_recomb_0.eta_v;

			return(result);

		case NEXT_NODE:

			deriv_current_0=next_elem->comp_deriv_electron_current(NEXT_NODE,return_flag);
			deriv_integral_recomb_0=next_elem->comp_deriv_integral_recomb(FIRSTHALF,NEXT_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=deriv_current_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=deriv_current_0.eta_c-
							 deriv_integral_recomb_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_integral_recomb_0.eta_v;

			return(result);

		default: return(result);

	}
}

FundamentalParam TSolution::comp_deriv_hole_rate(int i, NodeSide node, int return_flag)
{
	FundamentalParam result,deriv_current_0, deriv_current_1;
	FundamentalParam deriv_integral_recomb_0, deriv_integral_recomb_1;
	TElectricalElement *next_elem, *prev_elem;

	result.psi=result.eta_c=result.eta_v=0;

	prev_elem=*(electrical_element_ptr+i);
	next_elem=*(electrical_element_ptr+i+1);

	switch(node) {
		case CURRENT_NODE:

			deriv_current_0=next_elem->comp_deriv_hole_current(PREVIOUS_NODE,return_flag);
			deriv_current_1=prev_elem->comp_deriv_hole_current(NEXT_NODE,return_flag);
			deriv_integral_recomb_0=next_elem->comp_deriv_integral_recomb(FIRSTHALF,PREVIOUS_NODE,return_flag);
			deriv_integral_recomb_1=prev_elem->comp_deriv_integral_recomb(SECONDHALF,NEXT_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=deriv_current_0.psi-deriv_current_1.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=deriv_integral_recomb_0.eta_c+
							 deriv_integral_recomb_1.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=deriv_current_0.eta_v-deriv_current_1.eta_v+
							 deriv_integral_recomb_0.eta_v+
							 deriv_integral_recomb_1.eta_v;

			return(result);

		case PREVIOUS_NODE:

			deriv_current_0=prev_elem->comp_deriv_hole_current(PREVIOUS_NODE,return_flag);
			deriv_integral_recomb_0=prev_elem->comp_deriv_integral_recomb(SECONDHALF,PREVIOUS_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=-deriv_current_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=deriv_integral_recomb_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=-deriv_current_0.eta_v+
							  deriv_integral_recomb_0.eta_v;

			return(result);

		case NEXT_NODE:

			deriv_current_0=next_elem->comp_deriv_hole_current(NEXT_NODE,return_flag);
			deriv_integral_recomb_0=next_elem->comp_deriv_integral_recomb(FIRSTHALF,NEXT_NODE,return_flag);

			if (return_flag & D_PSI)
				result.psi=deriv_current_0.psi;

			if (return_flag & D_ETA_C)
				result.eta_c=deriv_integral_recomb_0.eta_c;

			if (return_flag & D_ETA_V)
				result.eta_v=deriv_current_0.eta_v+
							 deriv_integral_recomb_0.eta_v;

			return(result);

		default: return(result);
	}
}

prec TSolution::comp_deriv_heat_rate(int i, NodeSide node)
{
	prec result;
	TThermalElement *prev_elem, *next_elem;
	prec deriv_lat_heat_flow_0, deriv_lat_heat_flow_1;
	prec deriv_electron_heat_flow_0, deriv_electron_heat_flow_1;
	prec deriv_hole_heat_flow_0, deriv_hole_heat_flow_1;
	prec deriv_trans_heat_flow_0, deriv_trans_heat_flow_1;

	prev_elem=*(thermal_element_ptr+i);
	next_elem=*(thermal_element_ptr+i+1);

	switch(node) {
		case CURRENT_NODE:
			deriv_lat_heat_flow_0=next_elem->comp_deriv_lat_heat_flow(PREVIOUS_NODE);
			deriv_lat_heat_flow_1=prev_elem->comp_deriv_lat_heat_flow(NEXT_NODE);
			deriv_electron_heat_flow_0=next_elem->comp_deriv_electron_heat_flow(PREVIOUS_NODE);
			deriv_electron_heat_flow_1=prev_elem->comp_deriv_electron_heat_flow(NEXT_NODE);
			deriv_hole_heat_flow_0=next_elem->comp_deriv_hole_heat_flow(PREVIOUS_NODE);
			deriv_hole_heat_flow_1=prev_elem->comp_deriv_hole_heat_flow(NEXT_NODE);
			deriv_trans_heat_flow_0=next_elem->comp_deriv_trans_heat_flow(PREVIOUS_NODE);
			deriv_trans_heat_flow_1=prev_elem->comp_deriv_trans_heat_flow(NEXT_NODE);

			result=deriv_lat_heat_flow_0-deriv_lat_heat_flow_1+
				   deriv_electron_heat_flow_0-deriv_electron_heat_flow_1+
				   deriv_hole_heat_flow_0-deriv_hole_heat_flow_1;

			result+=(deriv_trans_heat_flow_0+deriv_trans_heat_flow_1)/2.0;

			return(result);

		case PREVIOUS_NODE:
			deriv_lat_heat_flow_0=prev_elem->comp_deriv_lat_heat_flow(PREVIOUS_NODE);
			deriv_electron_heat_flow_0=prev_elem->comp_deriv_electron_heat_flow(PREVIOUS_NODE);
			deriv_hole_heat_flow_0=prev_elem->comp_deriv_hole_heat_flow(PREVIOUS_NODE);
			deriv_trans_heat_flow_0=prev_elem->comp_deriv_trans_heat_flow(PREVIOUS_NODE);

			result=-deriv_lat_heat_flow_0-deriv_electron_heat_flow_0-deriv_hole_heat_flow_0;
			result+=deriv_trans_heat_flow_0/2.0;

			return(result);

		case NEXT_NODE:
			deriv_lat_heat_flow_0=next_elem->comp_deriv_lat_heat_flow(NEXT_NODE);
			deriv_electron_heat_flow_0=next_elem->comp_deriv_electron_heat_flow(NEXT_NODE);
			deriv_hole_heat_flow_0=next_elem->comp_deriv_hole_heat_flow(NEXT_NODE);
			deriv_trans_heat_flow_0=next_elem->comp_deriv_trans_heat_flow(NEXT_NODE);

			result=deriv_lat_heat_flow_0+deriv_electron_heat_flow_0+deriv_hole_heat_flow_0;
			result+=deriv_trans_heat_flow_0/2.0;

			return(result);

		default: return(0.0);
	}
}

prec TSolution::comp_deriv_rate_hotcarriers_electron(int i, NodeSide node)
{
	prec result;
	TThermalElement *prev_elem, *next_elem;
	prec deriv_electron_heat_flow_0, deriv_electron_heat_flow_1;
	prec deriv_electron_hotcarriers_0, deriv_electron_hotcarriers_1;

	prev_elem=*(thermal_element_ptr+i);
	next_elem=*(thermal_element_ptr+i+1);

	switch(node) {
		case CURRENT_NODE:
			deriv_electron_heat_flow_0=next_elem->comp_deriv_electron_heat_flow(PREVIOUS_NODE);
			deriv_electron_heat_flow_1=prev_elem->comp_deriv_electron_heat_flow(NEXT_NODE);
			deriv_electron_hotcarriers_0=next_elem->comp_deriv_electron_hotcarriers(FIRSTHALF,PREVIOUS_NODE);
			deriv_electron_hotcarriers_1=prev_elem->comp_deriv_electron_hotcarriers(SECONDHALF,NEXT_NODE);
			result=+deriv_electron_heat_flow_0-deriv_electron_heat_flow_1+
					deriv_electron_hotcarriers_0+deriv_electron_hotcarriers_1;
			return(result);
		case PREVIOUS_NODE:
			deriv_electron_heat_flow_1=prev_elem->comp_deriv_electron_heat_flow(PREVIOUS_NODE);
			deriv_electron_hotcarriers_1=prev_elem->comp_deriv_electron_hotcarriers(SECONDHALF,PREVIOUS_NODE);
			result=-deriv_electron_heat_flow_1+deriv_electron_hotcarriers_1;
			return(result);

		case NEXT_NODE:
			deriv_electron_heat_flow_0=next_elem->comp_deriv_electron_heat_flow(NEXT_NODE);
			deriv_electron_hotcarriers_0=next_elem->comp_deriv_electron_hotcarriers(FIRSTHALF,NEXT_NODE);
			result=+deriv_electron_heat_flow_0+deriv_electron_hotcarriers_0;
			return(result);

		default: return(0.0);
	}
}

void TSolution::electrical_iterate(FundamentalParam& iteration_error)
{
	comp_electrical_dep_param(solve_type);

	comp_deriv_conc();
	if (solve_type==STEADY_STATE) comp_deriv_recomb();

	comp_electrical_solution();
	comp_electrical_jacobian();
	factor_jacobian(electrical_jacobian,number_elect_variables,electrical_unknown_nodes);
	solve_electrical_jacobian();
	iteration_error=comp_electrical_error();
	electrical_update_device();
	electrical_update_sub_nodes();

	if (solve_type==EQUILIBRIUM) {
		device_ptr->init_value(ELECTRON,PLANCK_POT);
		device_ptr->init_value(HOLE,PLANCK_POT);
	}

	device_ptr->comp_value(ELECTRON,CONCENTRATION);
	device_ptr->comp_value(HOLE,CONCENTRATION);
	device_ptr->comp_value(ELECTRON,IONIZED_DOPING);
    device_ptr->comp_value(HOLE,IONIZED_DOPING);
	device_ptr->comp_value(NODE,TOTAL_CHARGE);
	if (solve_type==STEADY_STATE) {
		device_ptr->comp_value(NODE,SHR_RECOMB);
		device_ptr->comp_value(NODE,B_B_RECOMB);
        device_ptr->comp_value(NODE,AUGER_RECOMB);
		device_ptr->comp_value(NODE,STIM_RECOMB);
		device_ptr->comp_value(NODE,TOTAL_RECOMB);
		device_ptr->comp_value(ELECTRON,BAND_EDGE);
		device_ptr->comp_value(HOLE,BAND_EDGE);
	}
}

void TSolution::thermal_iterate(prec& iteration_error)
{
	comp_thermal_dep_param();

	if (device_effects & (DEVICE_SINGLE_TEMP | (DEVICE_VARY_LATTICE_TEMP)))comp_deriv_thermal_conduct();

	if (device_effects & (DEVICE_VARY_ELECTRON_TEMP))comp_deriv_electron_hotcarriers();

	comp_thermal_solution();
	comp_thermal_jacobian();
	factor_jacobian(thermal_jacobian,1,thermal_unknown_nodes);
	solve_thermal_jacobian();
	iteration_error=comp_thermal_error();
	thermal_update_device();
	thermal_update_sub_nodes();

	if (device_effects & (DEVICE_VARY_ELECTRON_TEMP)){
		device_ptr->comp_value(ELECTRON,SHR_HEAT);
		device_ptr->comp_value(ELECTRON,B_B_HEAT);
		device_ptr->comp_value(ELECTRON,STIM_HEAT);
		device_ptr->comp_value(ELECTRON,RELAX_HEAT);
		device_ptr->comp_value(ELECTRON,AUGER_HEAT);
		device_ptr->comp_value(ELECTRON,TOTAL_HEAT);
	}
}



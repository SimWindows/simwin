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


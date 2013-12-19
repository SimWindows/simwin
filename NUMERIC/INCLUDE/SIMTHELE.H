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



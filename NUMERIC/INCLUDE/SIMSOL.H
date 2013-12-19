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




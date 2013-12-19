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

class TDeviceFileInput {
public:
	short total_points;
	prec total_length;
	short number_grid;
	GridInput *grid_ptr;
	short number_doping;
	DopingInput *doping_ptr;
	short number_structure;
	StructureInput *structure_ptr;
	short number_region;
	short number_qw;
	RegionInput *region_ptr;
	short number_cavity;
	CavityInput *cavity_ptr;
	short number_mirror;
	MirrorInput *mirror_ptr;
	prec radius;
	short number_material_param[MAT_MAX_NUMBER_PARAMETERS];
	MaterialParamInput *material_param_input[MAT_MAX_NUMBER_PARAMETERS];

public:
	TDeviceFileInput(void)
		{ clear_contents(); }
	TDeviceFileInput(const TDeviceFileInput& new_device_input)
		{ clear_contents(); copy_contents(new_device_input); }
	~TDeviceFileInput(void) { delete_contents(); }
	void add_grid(GridInput new_grid);
	void add_doping(DopingInput new_doping);
	void add_material_param(MaterialParam param_number, MaterialParamInput new_param);
	void add_structure(StructureInput new_structure);
	void add_region(RegionInput new_region);
	void add_cavity(CavityInput new_cavity);
	void add_mirror(MirrorInput new_mirror);
	void add_radius(prec new_radius) { radius=new_radius; }
	void apply_defaults(void);
	void check_device(void);
	void delete_contents(void);
	RegionType get_region_type(prec position);
	AlloyType get_alloy_type(prec position);
	MaterialType get_material_type(prec position);
	prec get_material_param(MaterialParam param_number,prec *values);
	prec get_doping_conc(prec position, DopingType type);
	prec get_alloy_conc(prec position);
	long get_doping_degeneracy(prec position, DopingType type);
	prec get_doping_level(prec position, DopingType type);
	prec get_radius(prec position) { return(radius); }
	logical material_param_entered(MaterialParam param_number)
		{ assert((param_number>=1) && (param_number<=MAT_MAX_NUMBER_PARAMETERS));
		  return(number_material_param[param_number-1]!=0); }
	TDeviceFileInput& operator=(const TDeviceFileInput& new_device_input)
		{ clear_contents(); copy_contents(new_device_input); return(*this); }
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
private:
	void clear_contents(void);
	void copy_contents(const TDeviceFileInput& new_device_input);
};



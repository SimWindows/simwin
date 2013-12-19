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

/******************************* class TDeviceFileInput ***************************************

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
	long get_doping_degeneracy(prec position, DopingType type);
	prec get_doping_level(prec position, DopingType type);
	prec get_alloy_conc(prec position);
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

***********************************************************************************************/

void TDeviceFileInput::add_grid(GridInput new_grid)
{
	GridInput *temp_ptr;

	temp_ptr=(GridInput *)realloc(grid_ptr,(number_grid+1)*sizeof(GridInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	grid_ptr=temp_ptr;
	(grid_ptr+number_grid)->length=new_grid.length;
	(grid_ptr+number_grid)->number_points=new_grid.number_points;
	number_grid++;
	total_points+=new_grid.number_points;
}

void TDeviceFileInput::add_doping(DopingInput new_doping)
{
	DopingInput *temp_ptr;

	assert(new_doping.acceptor_function!=(TFunction *)NULL);
	assert(new_doping.donor_function!=(TFunction *)NULL);

	temp_ptr=(DopingInput *)realloc(doping_ptr,(number_doping+1)*sizeof(DopingInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	doping_ptr=temp_ptr;
	(doping_ptr+number_doping)->length=new_doping.length;
	(doping_ptr+number_doping)->acceptor_function=new_doping.acceptor_function;
	(doping_ptr+number_doping)->donor_function=new_doping.donor_function;
	(doping_ptr+number_doping)->acceptor_degeneracy=new_doping.acceptor_degeneracy;
	(doping_ptr+number_doping)->acceptor_level=new_doping.acceptor_level;
	(doping_ptr+number_doping)->donor_degeneracy=new_doping.donor_degeneracy;
	(doping_ptr+number_doping)->donor_level=new_doping.donor_level;
	number_doping++;
}

void TDeviceFileInput::add_material_param(MaterialParam param_number,
										  MaterialParamInput new_param)
{
	MaterialParamInput *temp_ptr;

	assert((param_number>=1) && (param_number<=MAT_MAX_NUMBER_PARAMETERS));
	temp_ptr=(MaterialParamInput *)realloc(material_param_input[param_number-1],
										   (number_material_param[param_number-1]+1)*sizeof(MaterialParamInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	material_param_input[param_number-1]=temp_ptr;
	(material_param_input[param_number-1]+number_material_param[param_number-1])->length=
										new_param.length;
	(material_param_input[param_number-1]+number_material_param[param_number-1])->material_model=new_param.material_model;
	number_material_param[param_number-1]++;
}

void TDeviceFileInput::add_structure(StructureInput new_structure)
{
	StructureInput *temp_ptr;

	assert(new_structure.alloy_function!=(TFunction *)NULL);

	temp_ptr=(StructureInput *)realloc(structure_ptr,(number_structure+1)*sizeof(StructureInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	structure_ptr=temp_ptr;
	(structure_ptr+number_structure)->material_type=new_structure.material_type;
	(structure_ptr+number_structure)->length=new_structure.length;
	(structure_ptr+number_structure)->alloy_type=new_structure.alloy_type;
	(structure_ptr+number_structure)->alloy_function=new_structure.alloy_function;
	total_length+=new_structure.length;
	number_structure++;
}

void TDeviceFileInput::add_region(RegionInput new_region)
{
	RegionInput *temp_ptr;

	temp_ptr=(RegionInput *)realloc(region_ptr,(number_region+1)*sizeof(RegionInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	region_ptr=temp_ptr;
	(region_ptr+number_region)->type=new_region.type;
	(region_ptr+number_region)->length=new_region.length;
	if (new_region.type==QW) number_qw++;
	number_region++;
}

void TDeviceFileInput::add_cavity(CavityInput new_cavity)
{
	CavityInput *temp_ptr;

	if (number_cavity==1) {
		error_handler.set_error(ERROR_DEVICE_NUMBER_CAVITY,0,"","");
		return;
	}

	temp_ptr=(CavityInput *)realloc(cavity_ptr,(number_cavity+1)*sizeof(CavityInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	cavity_ptr=temp_ptr;
	(cavity_ptr+number_cavity)->type=new_cavity.type;
	(cavity_ptr+number_cavity)->length=new_cavity.length;
	(cavity_ptr+number_cavity)->area=new_cavity.area;
	number_cavity++;
}

void TDeviceFileInput::add_mirror(MirrorInput new_mirror)
{
	MirrorInput *temp_ptr;

	if (number_mirror==2) {
		error_handler.set_error(ERROR_DEVICE_NUMBER_MIRROR,0,"","");
		return;
	}

	if (new_mirror.reflectivity<0 || new_mirror.reflectivity>1) {
		error_handler.set_error(ERROR_DEVICE_MIRROR_REF,number_mirror+1,"","");
		return;
	}

	temp_ptr=(MirrorInput *)realloc(mirror_ptr,(number_mirror+1)*sizeof(MirrorInput));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_DEVICE_INPUT,0,"","");
		return;
	}

	mirror_ptr=temp_ptr;
	(mirror_ptr+number_mirror)->type=new_mirror.type;
	(mirror_ptr+number_mirror)->position=new_mirror.position;
	(mirror_ptr+number_mirror)->reflectivity=new_mirror.reflectivity;
	number_mirror++;
}

void TDeviceFileInput::check_device(void)
{
	int i;
	MaterialParam j;
	prec grid_total_length=0, region_total_length=0, doping_total_length=0;
	prec material_total_length;

	if (!number_grid) {
		error_handler.set_error(ERROR_DEVICE_NO_GRID,0,"","");
		return;
	}

	if (!number_structure) {
		error_handler.set_error(ERROR_DEVICE_NO_STRUCTURE,0,"","");
		return;
	}

	for (i=0;i<number_grid;i++) grid_total_length+=(grid_ptr+i)->length;
	for (i=0;i<number_region;i++) region_total_length+=(region_ptr+i)->length;
	for (i=0;i<number_doping;i++) doping_total_length+=(doping_ptr+i)->length;

	if (fabs(grid_total_length-total_length)>=1e-6) {
		error_handler.set_error(ERROR_DEVICE_GRID_LENGTH,0,"","");
		return;
	}

	if (number_region && (fabs(region_total_length-total_length)>=1e-6)) {
		error_handler.set_error(ERROR_DEVICE_REGION_LENGTH,0,"","");
		return;
	}

	if (number_doping && (fabs(doping_total_length-total_length)>=1e-6)) {
		error_handler.set_error(ERROR_DEVICE_DOPING_LENGTH,0,"","");
		return;
	}

	for (j=1;j<MAT_MAX_NUMBER_PARAMETERS;j++) {
		if (number_material_param[j]!=0) {
			material_total_length=0.0;
			for (i=0;i<number_material_param[j];i++)
				material_total_length+=(material_param_input[j]+i)->length;

			if (fabs(material_total_length-total_length)>=1e-6) {
				error_handler.set_error(ERROR_DEVICE_MATERIAL_PARAM_LENGTH,0,"","");
				return;
			}
		}
	}

	if (number_mirror>0 && number_cavity!=1) {
		error_handler.set_error(ERROR_DEVICE_NUMBER_CAVITY,0,"","");
		return;
	}

	if (number_cavity>0 && number_mirror!=2) {
		error_handler.set_error(ERROR_DEVICE_NUMBER_MIRROR,0,"","");
		return;
	}
}

void TDeviceFileInput::delete_contents(void)
{
	int i,j;

	if(number_grid) free(grid_ptr);
	if(number_doping) {
		for (i=0;i<number_doping;i++) {
			delete (doping_ptr+i)->acceptor_function;
			delete (doping_ptr+i)->donor_function;
		}
		free(doping_ptr);
	}

	if(number_structure) {
		for (i=0;i<number_structure;i++) delete (structure_ptr+i)->alloy_function;
		free(structure_ptr);
	}

	if(number_region) free(region_ptr);
	if(number_cavity) free(cavity_ptr);
	if(number_mirror) free(mirror_ptr);

	for (i=0;i<MAT_MAX_NUMBER_PARAMETERS;i++) {
		if (number_material_param[i]) {
			for (j=0;j<number_material_param[i];j++) {
				delete (material_param_input[i]+j)->material_model;
			}
			free(material_param_input[i]);
		}
	}
	clear_contents();
}


RegionType TDeviceFileInput::get_region_type(prec position)
{
	int i=0;
	RegionType result=(RegionType)-1;
	prec next_length=0;

	do {
		if (position<=(region_ptr+i)->length+next_length)
			result=(region_ptr+i)->type;
		else {
			next_length+=(region_ptr+i)->length;
			i++;
		}
	}
	while ((result==-1) && (i<number_region));

	if (result==-1) result=(region_ptr+i-1)->type;

	return(result);
}

AlloyType TDeviceFileInput::get_alloy_type(prec position)
{
	int i=0;
	AlloyType alloy_type=(AlloyType)-1;
	prec next_length=0;

	do {
		if (position<=(structure_ptr+i)->length+next_length) {
			alloy_type=(structure_ptr+i)->alloy_type;
		}
		else {
			next_length+=(structure_ptr+i)->length;
			i++;
		}
	}
	while ((alloy_type==-1) && (i<number_structure));

	if (alloy_type==-1) alloy_type=(structure_ptr+i-1)->alloy_type;

	return(alloy_type);
}

MaterialType TDeviceFileInput::get_material_type(prec position)
{
	int i=0;
	MaterialType material_type=(MaterialType)-1;
	prec next_length=0;

	do {
		if (position<=(structure_ptr+i)->length+next_length) {
			material_type=(structure_ptr+i)->material_type;
		}
		else {
			next_length+=(structure_ptr+i)->length;
			i++;
		}
	}
	while ((material_type==-1) && (i<number_structure));

	if (material_type==-1) material_type=(structure_ptr+i-1)->material_type;

	return(material_type);
}

prec TDeviceFileInput::get_material_param(MaterialParam param_number,prec *values)
{
	int i=0;
	logical result_obtained=FALSE;
	prec result, next_length=0;
	extern char *material_parameters_variables[];

	assert(material_param_entered(param_number));

	string variable_string(material_parameters_variables[param_number-1]);
	prec &position=values[(int)variable_string.length()];

	do {
		if (position<=(material_param_input[param_number-1]+i)->length+next_length) {
			position-=next_length;
			result=(material_param_input[param_number-1]+i)->material_model->evaluate(values);
			result_obtained=TRUE;
		}
		else {
			next_length+=(material_param_input[param_number-1]+i)->length;
			i++;
		}
	}
	while ((!result_obtained) && (i<number_material_param[param_number-1]));

	if (!result_obtained) {
		position-=(next_length-(material_param_input[param_number-1]+i-1)->length);
		result=(material_param_input[param_number-1]+i-1)->material_model->evaluate(values);
	}

	return(result);
}

prec TDeviceFileInput::get_doping_conc(prec position, DopingType type)
{
	int i=0;
	logical result_obtained=FALSE;
	prec result;
	prec next_length=0;

	do {
		if (position<=(doping_ptr+i)->length+next_length) {
			position-=next_length;
			if (type==ACCEPTOR)	result=(doping_ptr+i)->acceptor_function->evaluate(&position);
			else result=(doping_ptr+i)->donor_function->evaluate(&position);
			result_obtained=TRUE;
		}
		else {
			next_length+=(doping_ptr+i)->length;
			i++;
		}
	}
	while (!result_obtained && (i<number_doping));

	if (!result_obtained) {
		position-=(next_length-(doping_ptr+i-1)->length);
		if (type==ACCEPTOR)	result=(doping_ptr+i-1)->acceptor_function->evaluate(&position);
		else result=(doping_ptr+i-1)->donor_function->evaluate(&position);
	}
	return(result);
}

long TDeviceFileInput::get_doping_degeneracy(prec position, DopingType type)
{
	int i=0;
	logical result_obtained=FALSE;
	long result;
	prec next_length=0;

	do {
		if (position<=(doping_ptr+i)->length+next_length) {
			position-=next_length;
			if (type==ACCEPTOR)	result=(doping_ptr+i)->acceptor_degeneracy;
			else result=(doping_ptr+i)->donor_degeneracy;
			result_obtained=TRUE;
		}
		else {
			next_length+=(doping_ptr+i)->length;
			i++;
		}
	}
	while (!result_obtained && (i<number_doping));

	if (!result_obtained) {
		if (type==ACCEPTOR)	result=(doping_ptr+i-1)->acceptor_degeneracy;
		else result=(doping_ptr+i-1)->donor_degeneracy;
	}
	return(result);
}

prec TDeviceFileInput::get_doping_level(prec position, DopingType type)
{
	int i=0;
	logical result_obtained=FALSE;
	prec result;
	prec next_length=0;

	do {
		if (position<=(doping_ptr+i)->length+next_length) {
			position-=next_length;
			if (type==ACCEPTOR)	result=(doping_ptr+i)->acceptor_level;
			else result=(doping_ptr+i)->donor_level;
			result_obtained=TRUE;
		}
		else {
			next_length+=(doping_ptr+i)->length;
			i++;
		}
	}
	while (!result_obtained && (i<number_doping));

	if (!result_obtained) {
		if (type==ACCEPTOR)	result=(doping_ptr+i-1)->acceptor_level;
		else result=(doping_ptr+i-1)->donor_level;
	}
	return(result);
}

prec TDeviceFileInput::get_alloy_conc(prec position)
{
	int i=0;
	logical result_obtained=FALSE;
	prec result;
	prec next_length=0;

	do {
		if (position<=(structure_ptr+i)->length+next_length) {
			position-=next_length;
			result=(structure_ptr+i)->alloy_function->evaluate(&position);
			result_obtained=TRUE;
		}
		else {
			next_length+=(structure_ptr+i)->length;
			i++;
		}
	}
	while (!result_obtained && (i<number_structure));

	if (!result_obtained) {
		position-=(next_length-(structure_ptr+i-1)->length);
		result=(structure_ptr+i-1)->alloy_function->evaluate(&position);
	}

	return(result);
}

void TDeviceFileInput::read_state_file(FILE *file_ptr)
{
	MaterialParam i;
	int j;

	fread(&total_points,sizeof(total_points),1,file_ptr);
	fread(&total_length,sizeof(total_length),1,file_ptr);
	fread(&number_grid,sizeof(number_grid),1,file_ptr);
	fread(&number_doping,sizeof(number_doping),1,file_ptr);
	fread(&number_structure,sizeof(number_structure),1,file_ptr);
	fread(&number_region,sizeof(number_region),1,file_ptr);
	fread(&number_qw,sizeof(number_qw),1,file_ptr);
	fread(&number_cavity,sizeof(number_cavity),1,file_ptr);
	fread(&number_mirror,sizeof(number_mirror),1,file_ptr);
	fread(number_material_param,sizeof(number_material_param),1,file_ptr);

	grid_ptr=(GridInput *)malloc(number_grid*sizeof(GridInput));
	doping_ptr=(DopingInput *)malloc(number_doping*sizeof(DopingInput));
	structure_ptr=(StructureInput *)malloc(number_structure*sizeof(StructureInput));
	region_ptr=(RegionInput *)malloc(number_region*sizeof(RegionInput));
	if (number_cavity) cavity_ptr=(CavityInput *)malloc(number_cavity*sizeof(CavityInput));
	if (number_mirror) mirror_ptr=(MirrorInput *)malloc(number_mirror*sizeof(MirrorInput));

	fread(grid_ptr,sizeof(GridInput),number_grid,file_ptr);

	for (j=0;j<number_doping;j++) {
		fread(&(doping_ptr+j)->length,sizeof((doping_ptr+j)->length),1,file_ptr);
		(doping_ptr+j)->acceptor_function=TFunction::create_copy(file_ptr);
		(doping_ptr+j)->donor_function=TFunction::create_copy(file_ptr);
		fread(&(doping_ptr+j)->acceptor_degeneracy,sizeof((doping_ptr+j)->acceptor_degeneracy),1,file_ptr);
		fread(&(doping_ptr+j)->acceptor_level,sizeof((doping_ptr+j)->acceptor_level),1,file_ptr);
		fread(&(doping_ptr+j)->donor_degeneracy,sizeof((doping_ptr+j)->donor_degeneracy),1,file_ptr);
		fread(&(doping_ptr+j)->donor_level,sizeof((doping_ptr+j)->donor_level),1,file_ptr);
	}

	for (j=0;j<number_structure;j++) {
		fread(&(structure_ptr+j)->length,sizeof((structure_ptr+j)->length),1,file_ptr);
		fread(&(structure_ptr+j)->alloy_type,sizeof((structure_ptr+j)->alloy_type),1,file_ptr);
		fread(&(structure_ptr+j)->material_type,sizeof((structure_ptr+j)->material_type),1,file_ptr);
		(structure_ptr+j)->alloy_function=TFunction::create_copy(file_ptr);
	}

	fread(region_ptr,sizeof(RegionInput),number_region,file_ptr);
	fread(cavity_ptr,sizeof(CavityInput),number_cavity,file_ptr);
	fread(mirror_ptr,sizeof(MirrorInput),number_mirror,file_ptr);
	fread(&radius,sizeof(radius),1,file_ptr);

	for (i=1;i<=MAT_MAX_NUMBER_PARAMETERS;i++) {
		if (number_material_param[i-1]!=0) {
			material_param_input[i-1]=(MaterialParamInput *)malloc(number_material_param[i-1]*
																   sizeof(MaterialParamInput));
			for (j=0;j<number_material_param[i-1];j++) {
				fread(&(material_param_input[i-1]+j)->length,sizeof((material_param_input[i-1]+j)->length),1,file_ptr);
				(material_param_input[i-1]+j)->material_model=new TMaterialParamModel(file_ptr);
			}
		}
	}
}

void TDeviceFileInput::write_state_file(FILE *file_ptr)
{
	MaterialParam i;
	int j;

	fwrite(&total_points,sizeof(total_points),1,file_ptr);
	fwrite(&total_length,sizeof(total_length),1,file_ptr);
	fwrite(&number_grid,sizeof(number_grid),1,file_ptr);
	fwrite(&number_doping,sizeof(number_doping),1,file_ptr);
	fwrite(&number_structure,sizeof(number_structure),1,file_ptr);
	fwrite(&number_region,sizeof(number_region),1,file_ptr);
	fwrite(&number_qw,sizeof(number_qw),1,file_ptr);
	fwrite(&number_cavity,sizeof(number_cavity),1,file_ptr);
	fwrite(&number_mirror,sizeof(number_mirror),1,file_ptr);
	fwrite(number_material_param,sizeof(number_material_param),1,file_ptr);

	fwrite(grid_ptr,sizeof(GridInput),number_grid,file_ptr);

	for (j=0;j<number_doping;j++) {
		fwrite(&(doping_ptr+j)->length,sizeof((doping_ptr+j)->length),1,file_ptr);
		(doping_ptr+j)->acceptor_function->write_state_file(file_ptr);
		(doping_ptr+j)->donor_function->write_state_file(file_ptr);
		fwrite(&(doping_ptr+j)->acceptor_degeneracy,sizeof((doping_ptr+j)->acceptor_degeneracy),1,file_ptr);
		fwrite(&(doping_ptr+j)->acceptor_level,sizeof((doping_ptr+j)->acceptor_level),1,file_ptr);
		fwrite(&(doping_ptr+j)->donor_degeneracy,sizeof((doping_ptr+j)->donor_degeneracy),1,file_ptr);
		fwrite(&(doping_ptr+j)->donor_level,sizeof((doping_ptr+j)->donor_level),1,file_ptr);
	}

	for (j=0;j<number_structure;j++) {
		fwrite(&(structure_ptr+j)->length,sizeof((structure_ptr+j)->length),1,file_ptr);
		fwrite(&(structure_ptr+j)->alloy_type,sizeof((structure_ptr+j)->alloy_type),1,file_ptr);
		fwrite(&(structure_ptr+j)->material_type,sizeof((structure_ptr+j)->material_type),1,file_ptr);
		(structure_ptr+j)->alloy_function->write_state_file(file_ptr);
	}

	fwrite(region_ptr,sizeof(RegionInput),number_region,file_ptr);
	fwrite(cavity_ptr,sizeof(CavityInput),number_cavity,file_ptr);
	fwrite(mirror_ptr,sizeof(MirrorInput),number_mirror,file_ptr);
	fwrite(&radius,sizeof(radius),1,file_ptr);

	for (i=1;i<=MAT_MAX_NUMBER_PARAMETERS;i++) {
		for (j=0;j<number_material_param[i-1];j++) {
			fwrite(&(material_param_input[i-1]+j)->length,sizeof((material_param_input[i-1]+j)->length),1,file_ptr);
			(material_param_input[i-1]+j)->material_model->write_state_file(file_ptr);
		}
	}
}

void TDeviceFileInput::clear_contents(void)
{
	MaterialParam i;

	total_points=0;
	total_length=0.0;
	number_grid=0;
	grid_ptr=(GridInput *)0;
	number_doping=0;
	doping_ptr=(DopingInput *)0;
	number_structure=0;
	structure_ptr=(StructureInput *)0;
	number_region=0;
	number_qw=0;
	region_ptr=(StructureInput *)0;
	number_cavity=0;
	cavity_ptr=(CavityInput *)0;
	number_mirror=0;
	mirror_ptr=(MirrorInput *)0;
	radius=0.0;

	for (i=1;i<=MAT_MAX_NUMBER_PARAMETERS;i++) {
		number_material_param[i-1]=0;
		material_param_input[i-1]=(MaterialParamInput *)0;
	}
}

void TDeviceFileInput::copy_contents(const TDeviceFileInput& new_device_input)
{
	int i;
	MaterialParam j;
	MaterialParamInput new_param_input;
	DopingInput new_doping;
	StructureInput new_structure;

	for (i=0;i<new_device_input.number_grid;i++) add_grid(*(new_device_input.grid_ptr+i));
	for (i=0;i<new_device_input.number_doping;i++) {
		new_doping.length=(new_device_input.doping_ptr+i)->length;
		new_doping.acceptor_function=(new_device_input.doping_ptr+i)->acceptor_function->create_copy();
		new_doping.donor_function=(new_device_input.doping_ptr+i)->donor_function->create_copy();
		new_doping.acceptor_degeneracy=(new_device_input.doping_ptr+i)->acceptor_degeneracy;
		new_doping.acceptor_level=(new_device_input.doping_ptr+i)->acceptor_level;
		new_doping.donor_degeneracy=(new_device_input.doping_ptr+i)->donor_degeneracy;
		new_doping.donor_level=(new_device_input.doping_ptr+i)->donor_level;
		add_doping(new_doping);
	}

	for (i=0;i<new_device_input.number_structure;i++) {
		new_structure.material_type=(new_device_input.structure_ptr+i)->material_type;
		new_structure.alloy_type=(new_device_input.structure_ptr+i)->alloy_type;
		new_structure.length=(new_device_input.structure_ptr+i)->length;
		new_structure.alloy_function=(new_device_input.structure_ptr+i)->alloy_function->create_copy();
		add_structure(new_structure);
	}

	for (i=0;i<new_device_input.number_region;i++) add_region(*(new_device_input.region_ptr+i));
	for (i=0;i<new_device_input.number_cavity;i++) add_cavity(*(new_device_input.cavity_ptr+i));
	for (i=0;i<new_device_input.number_mirror;i++) add_mirror(*(new_device_input.mirror_ptr+i));
	radius=new_device_input.radius;

	for (j=1;j<=MAT_MAX_NUMBER_PARAMETERS;j++) {
		for (i=0;i<new_device_input.number_material_param[j-1];i++) {
			new_param_input.length=(new_device_input.material_param_input[j-1]+i)->length;
			new_param_input.material_model=
				new TMaterialParamModel(*((new_device_input.material_param_input[j-1]+i)->material_model));
			add_material_param(j,new_param_input);
		}
	}

}


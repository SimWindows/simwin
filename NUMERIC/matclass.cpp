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

//*********************************** class TMaterialParamModel *****************************
/*
class TMaterialParamModel {
protected:
	short number_functions;
	TPieceWiseFunction **function;
public:
	TMaterialParamModel(void) : number_functions(0), function((TPieceWiseFunction **)NULL) {}
	TMaterialParamModel(TPieceWiseFunction *new_function)
		: number_functions(0), function((TPieceWiseFunction **)NULL) { add_function(new_function); }
	TMaterialParamModel(const TMaterialParamModel& new_model);
	TMaterialParamModel(FILE *file_ptr) { read_state_file(file_ptr); }
	~TMaterialParamModel(void);
	void add_function(TPieceWiseFunction *new_function);
	prec evaluate(prec *values);
	void write_state_file(FILE *file_ptr);
	void read_state_file(FILE *file_ptr);
};
*/

TMaterialParamModel::TMaterialParamModel(const TMaterialParamModel& new_model)
{
	int i;

	number_functions=new_model.number_functions;

	function=(TPieceWiseFunction **)malloc(number_functions*sizeof(TPieceWiseFunction *));
	if (!function) {
		error_handler.set_error(ERROR_MEM_MATERIAL_MODEL,0,"","");
		return;
	}

	for (i=0;i<number_functions;i++) function[i]=new TPieceWiseFunction(*new_model.function[i]);
}

TMaterialParamModel::~TMaterialParamModel(void)
{
	int i;

	if (number_functions) {
		for (i=0;i<number_functions;i++) delete function[i];
		free(function);
	}
}

void TMaterialParamModel::add_function(TPieceWiseFunction *new_function)
{
	TPieceWiseFunction **temp_ptr;

	assert(new_function!=(TPieceWiseFunction *)NULL);

	temp_ptr=(TPieceWiseFunction **)realloc(function,(number_functions+1)*sizeof(TPieceWiseFunction *));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_MATERIAL_MODEL,0,"","");
		return;
	}
	function=temp_ptr;
	function[number_functions]=new_function;
	number_functions++;
}

prec TMaterialParamModel::evaluate(prec *values)
{
	int i;

	for (i=0;i<number_functions;i++) {
		if (function[i]->is_valid(values)) return(function[i]->evaluate(values));
	}
	return(0.0);
}

void TMaterialParamModel::write_state_file(FILE *file_ptr)
{
	short i;

	fwrite(&number_functions,sizeof(number_functions),1,file_ptr);
	for (i=0;i<number_functions;i++) function[i]->write_state_file(file_ptr);
}

void TMaterialParamModel::read_state_file(FILE *file_ptr)
{
	short i;

	fread(&number_functions,sizeof(number_functions),1,file_ptr);

	function=(TPieceWiseFunction **)malloc(number_functions*sizeof(TPieceWiseFunction *));
	if (!function) {
		error_handler.set_error(ERROR_MEM_MATERIAL_MODEL,0,"","");
		return;
	}
	for (i=0;i<number_functions;i++) function[i]=new TPieceWiseFunction(file_ptr);
}

/********************************* class TAlloy *********************************************

class TAlloy {
public:
	string name;
private:
	TMaterialParamModel *parameters[MAT_MAX_NUMBER_PARAMETERS];
public:
	TAlloy(string& alloy_name);
	~TAlloy(void);
	void add_model(MaterialParam param, TMaterialParamModel *new_model)
		{ assert((param>=1) && (param<=MAT_MAX_NUMBER_PARAMETERS));
		  parameters[param-1]=new_model; }
	prec evaluate(MaterialParam param, prec *values)
		{ return(parameters[param-1]->evaluate(values)); }
};
*/

TAlloy::TAlloy(string& alloy_name)
	: name(alloy_name)
{
	int i;
	for (i=0;i<MAT_MAX_NUMBER_PARAMETERS;i++) parameters[i]=(TMaterialParamModel *)NULL;
}


TAlloy::~TAlloy(void)
{
	int i;

	for (i=0;i<MAT_MAX_NUMBER_PARAMETERS;i++)
		if (parameters[i]!=(TMaterialParamModel *)NULL) delete parameters[i];
}

/********************************** class TMaterial *****************************************

class TMaterial {
public:
	string name;
private:
	int number_alloys;
	TAlloy **alloys;
public:
	TMaterial(string& material_name) : name(material_name) { number_alloys=0; alloys=(TAlloy **)0; }
	~TMaterial(void);
	void add_alloy(string& alloy_name);
	logical valid_alloy(string& alloy_name);
	logical valid_alloy(AlloyType alloy_type)
		{ return((alloy_type>=1) && (alloy_type<=number_alloys)); }
	void add_model(AlloyType alloy_type, MaterialParam param, TMaterialParamModel *new_model)
		{ assert(valid_alloy(alloy_type));
		  alloys[alloy_type-1]->add_model(param, new_model); }
	prec evaluate(AlloyType alloy_type, MaterialParam param, prec *values)
		{ assert(valid_alloy(alloy_type));
		  return(alloys[alloy_type-1]->evaluate(param,values)); }
	AlloyType get_alloy_type(string& alloy_name);
	string get_alloy_name(AlloyType alloy_type);
};

*/

TMaterial::~TMaterial(void)
{
	int i;

	for (i=0;i<number_alloys;i++) delete alloys[i];
	free (alloys);
}

void TMaterial::add_alloy(string& alloy_name)
{
	TAlloy **temp_ptr;

	temp_ptr=(TAlloy **)realloc(alloys,(number_alloys+1)*sizeof(TAlloy *));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_ALLOY,0,"","");
		return;
	}
	alloys=temp_ptr;
	alloys[number_alloys]=new TAlloy(alloy_name);
	number_alloys++;
}

logical TMaterial::valid_alloy(string& alloy_name)
{
	int i=0;

	while((i<number_alloys) && (alloys[i]->name!=alloy_name)) i++;
	return(i!=number_alloys);
}

AlloyType TMaterial::get_alloy_type(string& alloy_name)
{
	AlloyType i=1;

	if (valid_alloy(alloy_name)) {
		while(alloys[i-1]->name!=alloy_name) i++;
		return(i);
	}
	else return((AlloyType) MAT_NO_ALLOY);
}

string TMaterial::get_alloy_name(AlloyType alloy_type)
{
	if (valid_alloy(alloy_type))
		return(alloys[alloy_type-1]->name);
	else return("");
}

/****************************** class TMaterialStorage ****************************************

class TMaterialStorage {
private:
	logical ready;
	int number_materials;
	TMaterial **materials;
	TDeviceFileInput *device_file;
public:
	TMaterialStorage(void);
	~TMaterialStorage(void) { clear(); }
	void clear(void);
	void add_material(string& material_name);
	void add_alloy(string& material_name, string& alloy_name)
		{ assert(valid_material(material_name));
		  add_alloy(get_material_type(material_name),alloy_name); }
	void add_alloy(MaterialType material_type, string& alloy_name)
		{ assert(valid_material(material_type));
		  materials[material_type-1]->add_alloy(alloy_name); }
	void add_model(MaterialType material_type, AlloyType alloy_type, MaterialParam param,
				   TMaterialParamModel *new_model)
		{ assert(valid_material(material_type));
		  materials[material_type-1]->add_model(alloy_type,param,new_model); }
	prec evaluate(MaterialParam param, MaterialType material_type,
				  AlloyType alloy_type, prec *values);

	MaterialType get_material_type(string& material_name);
	AlloyType get_alloy_type(string& material_name, string& alloy_name);
	AlloyType get_alloy_type(MaterialType material_type, string& alloy_name);
	string get_material_name(MaterialType material_type);
	string get_alloy_name(string& material_name, AlloyType alloy_type);
	string get_alloy_name(MaterialType material_type, AlloyType alloy_type);

	logical valid_material(string& material_name);
	logical valid_material(MaterialType material_type)
		{ return((material_type>=1) && (material_type<=number_materials)); }
	logical valid_alloy(MaterialType material_type, string& alloy_name)
		{ assert(valid_material(material_type));
		  return(materials[material_type-1]->valid_alloy(alloy_name)); }
	logical valid_alloy(MaterialType material_type, AlloyType alloy_type)
		{ assert(valid_material(material_type));
		  return(materials[material_type-1]->valid_alloy(alloy_type)); }
	void put_device_file(TDeviceFileInput *new_device) { device_file=new_device; }

	void set_ready(logical value) { ready=value; }
	logical is_ready(void) { return(ready); }

	void set_normalization(MaterialType material_type);
};

*/

TMaterialStorage::TMaterialStorage(void)
{
	ready=FALSE;
	number_materials=0;
	materials=(TMaterial **)0;
	device_file=(TDeviceFileInput *)0;
}

void TMaterialStorage::clear(void)
{
	int i;

	for (i=0;i<number_materials;i++) delete materials[i];
	free(materials);

	number_materials=0;
	materials=(TMaterial **)0;
	device_file=(TDeviceFileInput *)0;
	ready=FALSE;
}

void TMaterialStorage::add_material(string& material_name)
{
	TMaterial **temp_ptr;

	temp_ptr=(TMaterial **)realloc(materials,(number_materials+1)*sizeof(TMaterial *));
	if (!temp_ptr) {
		error_handler.set_error(ERROR_MEM_MATERIAL,0,"","");
		return;
	}
	materials=temp_ptr;
	materials[number_materials]=new TMaterial(material_name);
	number_materials++;
}

prec TMaterialStorage::evaluate(MaterialParam param, MaterialType material_type,
								AlloyType alloy_type, prec *values)
{
	assert(valid_material(material_type));
	if (device_file->material_param_entered(param))
		return(device_file->get_material_param(param,values));
	else return(materials[material_type-1]->evaluate(alloy_type,param,values));
}

MaterialType TMaterialStorage::get_material_type(string& material_name)
{
	MaterialType i=1;

	if (valid_material(material_name)) {
		while(materials[i-1]->name!=material_name) i++;
		return(i);
	}
	else return((MaterialType) MAT_NO_MATERIAL);
}

AlloyType TMaterialStorage::get_alloy_type(string& material_name, string& alloy_name)
{
	assert(valid_material(material_name));
	return(get_alloy_type(get_material_type(material_name),alloy_name));
}

AlloyType TMaterialStorage::get_alloy_type(MaterialType material_type, string& alloy_name)
{
	assert(valid_material(material_type));
	return(materials[material_type-1]->get_alloy_type(alloy_name));
}

string TMaterialStorage::get_material_name(MaterialType material_type)
{
	if (valid_material(material_type))
		return(materials[material_type-1]->name);
	else return("");
}
string TMaterialStorage::get_alloy_name(string& material_name, AlloyType alloy_type)
{
	if (valid_material(material_name))
		return(get_alloy_name(get_material_type(material_name), alloy_type));
	else return("");
}
string TMaterialStorage::get_alloy_name(MaterialType material_type, AlloyType alloy_type)
{
	if (valid_material(material_type))
		return(materials[material_type-1]->get_alloy_name(alloy_type));
	else return("");
}

logical TMaterialStorage::valid_material(string& material_name)
{
	int i=0;

	while((i<number_materials) && (materials[i]->name!=material_name)) i++;
	return(i!=number_materials);
}

void TMaterialStorage::set_normalization(MaterialType material_type)
{
	prec electron_dos_mass, hole_dos_mass;
	prec alloy_conc=0.0;

	if (valid_material(material_type)) {
		electron_dos_mass=evaluate(MAT_ELECTRON_DOS_MASS,material_type,(AlloyType)1,&alloy_conc);
		hole_dos_mass=evaluate(MAT_HOLE_DOS_MASS,material_type,(AlloyType)1,&alloy_conc);

		normalization.charge=NORM_charge;
		normalization.temp=NORM_temp;
		normalization.pot=SIM_k_eV*normalization.temp;
		normalization.energy=normalization.pot;
		normalization.conc=2.0*pow((double) 2.0*SIM_pi*SIM_mo*SIM_k*normalization.temp/sq(SIM_h),
								   (double) 1.5)*pow((double) electron_dos_mass*hole_dos_mass, (double) 0.75)/1E6;
		normalization.length=sqrt(SIM_eo*normalization.pot/
								 (normalization.charge*normalization.conc));
		normalization.mobility=NORM_mobility;
		normalization.time=sq(normalization.length)/(normalization.mobility*normalization.pot);
		normalization.recomb=normalization.conc/normalization.time;
		normalization.therm_cond=normalization.charge*normalization.pot*normalization.conc*sq(normalization.length)/
								(normalization.temp*normalization.time);
		normalization.field=normalization.pot/normalization.length;
		normalization.current=normalization.charge*normalization.mobility*normalization.conc*normalization.field;
		normalization.intensity=normalization.energy*normalization.length*normalization.conc/normalization.time;
	}
}



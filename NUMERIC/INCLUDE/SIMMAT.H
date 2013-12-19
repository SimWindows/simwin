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



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

class TParse {
protected:
	int line_number;
	ifstream input_file_stream;
	int number_string;
	string file_name;
public:
	TParse(const char *file);
	~TParse(void) { input_file_stream.close(); }
	void set_filename(const char *file);
protected:
	string get_string(void);
	string get_name(string& line_string, string value_string);
	FunctionType get_function_type(string line_string, string value_string);
	prec get_float(string& line_string, string value_string);
	long get_long(string& line_string, string value_string);
	int get_terms(string& line_string, string value_string, prec*& terms);
	void get_function_limit(string& line_string, string limit_variable, string function_variables,
							TFunction* &lower_limit, TFunction* &upper_limit);
	TPolynomial *get_polynomial(string& line_string, string value_string, string variable_string);
	TUserFunction *get_function(string& line_string, string value_string, string variable_string);
	TFunction *get_general_function(string& line_string, string value_string, string variable_string);
	void test_string(const string& line_string);
};

class TParseMaterialParam: public TParse {
public:
	TParseMaterialParam(const char *file) : TParse(file) {}
protected:
	TMaterialParamModel *get_material_parameter(MaterialParam param,
												string parameter_line,
												string function_variables,
												string limit_variables);
private:
	TFunction *get_material_param_function(MaterialParam param,
										   string parameter_line,
										   string function_variables);
};

class TParseDevice: public TParseMaterialParam {
private:
	TDeviceFileInput device_input;
	streampos start_repeat_pos;
	prec structure_total_length;
	logical doping_entered;
	logical region_entered;
	logical radius_entered;
public:
	TParseDevice(const char *file);
	~TParseDevice(void) {}
	TDeviceFileInput parse_device(void);
private:
	void process_line(string line_string);
	void process_grid(string line_string);
	void process_doping(string line_string);
	void process_structure(string line_string);
	void process_region(string line_string);
	void process_cavity(string line_string);
	void process_mirror(string line_string);
	void process_repeat(string line_string);
	void process_radius(string line_string);
	void process_material_param(string line_string, MaterialParam param);
	void apply_defaults(void);
};

class TParseMaterial: public TParseMaterialParam {
private:
	MaterialType current_material_type;
	AlloyType current_alloy_type;
public:
	TParseMaterial(const char *file) : TParseMaterialParam(file)
		{ current_material_type=(MaterialType)0; current_alloy_type=(AlloyType)0; }
	void parse_material(void);
private:
	void process_material(string line_string);
	void process_alloy(string line_string);
	void init(void) { material_parameters.clear(); }
};


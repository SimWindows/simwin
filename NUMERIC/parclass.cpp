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
#include "simparse.h"

/*************************************** class TParse ****************************************

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
*/

TParse::TParse(const char *file)
{
	set_filename(file);
	line_number=0;
	number_string=0;
}

void TParse::set_filename(const char *file)
{
	file_name=file;
	input_file_stream.close();
	input_file_stream.open(file,ios::in);
	if (input_file_stream.fail()) error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",file);
}

string TParse::get_string(void)
{
	string new_line="";
	logical valid_line=FALSE;

	while (!valid_line) {
		line_number++;
		if (input_file_stream.eof()) new_line="";
		else {
			new_line.read_to_delim(input_file_stream);
			new_line=new_line.strip(string::Leading);
			if (new_line.is_null()) continue;
			if (new_line.find_first_of("#\n")==0) continue;
			new_line.to_upper();
		}
		valid_line=TRUE;
	}
	return(new_line);
}

string TParse::get_name(string& line_string, string value_string)
{
	string result_string;
	size_t token_position,name_start,name_end;

	value_string.append("=");
	token_position=line_string.find(value_string);
	if (token_position==NPOS) {
		error_handler.set_error(ERROR_PARSE_NAME_STRING,line_number,value_string,file_name);
		return("");
	}
	else {
		if (token_position!=0) {
			if ((line_string.get_at(token_position-1)!=' ') &&
				(line_string.get_at(token_position-1)!='\t'))
				error_handler.set_error(ERROR_PARSE_NAME_STRING,line_number,value_string,file_name);
		}
	}
	name_start=token_position+value_string.length();
	name_end=line_string.find_first_of(" \n\t",name_start);
	if (name_end==name_start) {
		error_handler.set_error(ERROR_PARSE_NAME,line_number,value_string,file_name);
		return("");
	}

	result_string=line_string.substr(name_start,name_end-name_start);
	line_string.remove(token_position,name_end-token_position);
	return(result_string);
}

FunctionType TParse::get_function_type(string line_string, string value_string)
{
	string test_string(get_name(line_string,value_string));
	if (error_handler.fail()) return(NON_FUNCTION);
	if (test_string.find_first_not_of("0123456789.eE+-")==NPOS) return(CONSTANT);
	if (test_string.find_first_not_of("0123456789.eE+-,")==NPOS) return(POLYNOMIAL);
	return(USER_FUNCTION);
}

prec TParse::get_float(string& line_string, string value_string)
{
	string number_string(get_name(line_string,value_string));
	if(error_handler.fail()) return(0);
	if (number_string.find_first_not_of("0123456789.eE+-")!=NPOS) {
		error_handler.set_error(ERROR_PARSE_FLOAT,line_number,value_string,file_name);
		return(0);
	}
	return(atof(number_string.c_str()));
}

long TParse::get_long(string& line_string, string value_string)
{
	string number_string(get_name(line_string,value_string));
	if(error_handler.fail()) return(0);
	if (number_string.find_first_not_of("0123456789")!=NPOS) {
		error_handler.set_error(ERROR_PARSE_LONG,line_number,value_string,file_name);
		return(0);
	}
	return(atol(number_string.c_str()));
}


void TParse::get_function_limit(string& line_string, string limit_variable, string function_variables,
								TFunction* &lower_limit, TFunction* &upper_limit)
{
	logical lower_entered, upper_entered;

	string lower_string("START_"+limit_variable);
	lower_limit=get_general_function(line_string,lower_string,function_variables);
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		lower_entered=FALSE;
	}
	else lower_entered=TRUE;

	string upper_string("END_"+limit_variable);
	upper_limit=get_general_function(line_string,upper_string,function_variables);
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		upper_entered=FALSE;
	}
	else upper_entered=TRUE;

	if ((!lower_entered) && (!upper_entered)) {
		lower_limit=new TConstant(0.0,function_variables.length());
		upper_limit=new TConstant(0.0,function_variables.length());
	}
	else {
		if (!lower_entered) {
			error_handler.set_error(ERROR_PARSE_NAME,line_number,lower_string,file_name);
			return;
		}
		if (!upper_entered) {
			error_handler.set_error(ERROR_PARSE_NAME,line_number,upper_string,file_name);
			return;
		}
	}
}

int TParse::get_terms(string& line_string, string value_string, prec* &terms)
{
	int i;
	string term_string;
	size_t comma_position;
	int number_terms=0;
	int number_start, number_end;

	term_string=get_name(line_string, value_string);
	if (error_handler.fail()) return(0);

	comma_position=term_string.find_first_of(",",0);
	while (comma_position!=NPOS) {
		number_terms++;
		if (comma_position==term_string.length()) {
			error_handler.set_error(ERROR_PARSE_TERMS,line_number,value_string,file_name);
			return(0);
		}
		comma_position=term_string.find_first_of(",",comma_position+1);
	}
	term_string+=",";
	terms=new prec[++number_terms];
	if (!terms) {
		error_handler.set_error(ERROR_MEM_PARSE_TERMS,0,"","");
		return(0);
	}

	number_start=0;
	for (i=0;i<number_terms;i++) {
		number_end=term_string.find_first_of(",",number_start);
		terms[i]=atof(term_string.substr(number_start,number_start-number_end).c_str());
		number_start=number_end+1;
	}
	return(number_terms);
}

TPolynomial *TParse::get_polynomial(string& line_string, string value_string, string variable_string)
{
	TPolynomial *return_function;
	string function_string;
	prec *terms;
	int number_terms;

	number_terms=get_terms(line_string,value_string,terms);
	if (error_handler.fail()) return(NULL);
	return_function=new TPolynomial(number_terms, terms, variable_string.length());
	return(return_function);
}

TUserFunction *TParse::get_function(string& line_string, string value_string, string variable_string)
{
	TUserFunction *return_function;
	string function_string;

	function_string=get_name(line_string,value_string);
	if(error_handler.fail()) return(NULL);
	else {
		return_function=new TUserFunction(function_string,variable_string);
		return_function->translate();
		if (error_handler.fail()) {
			delete return_function;
			error_handler.clear();
			error_handler.set_error(ERROR_PARSE_FUNCTION,line_number,value_string,file_name);
			return(NULL);
		}
	}
	return(return_function);
}

TFunction *TParse::get_general_function(string& line_string, string value_string,
										string variable_string)
{
	prec constant_value;
	TPolynomial *user_polynomial;
	TUserFunction *user_function;
	FunctionType function_type;

	function_type=get_function_type(line_string,value_string);
	if (!error_handler.fail()) {
		switch(function_type) {
			case CONSTANT:
				constant_value=get_float(line_string,value_string);
				if (error_handler.fail()) return(NULL);
				return(new TConstant(constant_value,variable_string.length()));
			case POLYNOMIAL:
				user_polynomial=get_polynomial(line_string,value_string,variable_string);
				if (error_handler.fail()) return(NULL);
				return(user_polynomial);
			case USER_FUNCTION:
				user_function=get_function(line_string,value_string,variable_string);
				if (error_handler.fail()) return(NULL);
				return(user_function);
			default: assert(FALSE); return(NULL);
		}
	}
	else return(NULL);
}

void TParse::test_string(const string& line_string)
{
	if (line_string.find_first_not_of(" \n\t")!=NPOS)
		error_handler.set_error(ERROR_PARSE_UNKNOWN_SYMBOL,line_number,"",file_name);
}

/******************************* class TParseMaterialParam ************************************

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
*/

TMaterialParamModel *TParseMaterialParam::get_material_parameter(MaterialParam param,
																 string parameter_line,
																 string function_variables,
																 string limit_variables)
{
	int i,j,segments;
	int number_limit_variables;
	TMaterialParamModel *return_model;
	TFunction *general_function;
	TFunction *lower_limit[MAT_MAX_NUMBER_PARAMETER_VAR];
	TFunction *upper_limit[MAT_MAX_NUMBER_PARAMETER_VAR];
	string lower_string, upper_string;

	number_limit_variables=limit_variables.length();

#ifndef NDEBUG
	int number_function_variables=function_variables.length();
	assert(number_limit_variables<=number_function_variables);
#endif

	segments=(int)get_long(parameter_line,"SEGMENTS");
	if (error_handler.fail()) {
// Single Function
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return(NULL);
		error_handler.clear();

		general_function=get_material_param_function(param,parameter_line,function_variables);
		if (error_handler.fail()) return(NULL);
		return(new TMaterialParamModel(new TPieceWiseFunction(general_function,NULL,NULL)));
	}

// PieceWise Function
	i=0;
	return_model=new TMaterialParamModel;
	while (!input_file_stream.eof() && !error_handler.fail() && (i<segments)) {
		i++;
		string segment_line(get_string());
		if (error_handler.fail()) return(NULL);

		for (j=0;j<MAT_MAX_NUMBER_PARAMETER_VAR;j++) {
			lower_limit[j]=(TFunction *)NULL;
			upper_limit[j]=(TFunction *)NULL;
		}

		for (j=0;j<number_limit_variables;j++) {
			get_function_limit(segment_line,function_variables.substr(j,1),limit_variables,
							   lower_limit[j],upper_limit[j]);
			if (error_handler.fail()) return(NULL);
		}

		general_function=get_material_param_function(param,segment_line,function_variables);
		if (!error_handler.fail()) {
			return_model->add_function(new TPieceWiseFunction(general_function,lower_limit,upper_limit));
		}
		else return(NULL);
	}
	return(return_model);
}

TFunction *TParseMaterialParam::get_material_param_function(MaterialParam param,
															string parameter_line,
															string function_variables)
{
	string model_name;
	TFunction *general_function;
	TFunction *model_function;
	int number_terms;
	prec *terms;

// Single Function

	general_function=get_general_function(parameter_line,"VALUE",function_variables);
	if (!error_handler.fail()) return(general_function);
	else {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return(NULL);
		error_handler.clear();

		model_name=get_name(parameter_line,"MODEL");
		if (error_handler.fail()) return(NULL);

		if (model_name=="BAND_GAP") {
			if (!TModelBandGap::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelBandGap::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelBandGap(terms);
			delete[] terms;
			return(model_function);
		}

		if (model_name=="THERMAL_CONDUCT") {
			if (!TModelThermalConductivity::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelThermalConductivity::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelThermalConductivity(terms);
			delete[] terms;
			return(model_function);
		}

		if (model_name=="POWER_ABSORPTION") {
			if (!TModelPowerAbsorption::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelPowerAbsorption::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelPowerAbsorption(terms);
			delete[] terms;
			return(model_function);
		}

		if (model_name=="POWER_BAND_GAP_ABSORPTION") {
			if (!TModelPowerAbsorptionBandGap::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelPowerAbsorptionBandGap::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelPowerAbsorptionBandGap(terms);
			delete[] terms;
			return(model_function);
		}

		if (model_name=="EXP_ABSORPTION") {
			if (!TModelExpAbsorption::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelExpAbsorption::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelExpAbsorption(terms);
			delete[] terms;
			return(model_function);
		}

		if (model_name=="EXP_BAND_GAP_ABSORPTION") {
			if (!TModelExpAbsorptionBandGap::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelExpAbsorptionBandGap::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelExpAbsorptionBandGap(terms);
			delete[] terms;
			return(model_function);
		}
/*
		if (model_name=="OSCILLATOR_ABSORPTION") {
			if (!TModelAlGaAsAbsorption::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			return(new TModelAlGaAsAbsorption);
		}
*/
		if (model_name=="OSCILLATOR_REFRACTIVE_INDEX") {
			if (!TModelAlGaAsRefractiveIndex::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			return(new TModelAlGaAsRefractiveIndex);
		}

		if (model_name=="MOBILITY") {
			if (!TModelMobility::valid_model(param)) {
				error_handler.set_error(ERROR_PARSE_MODEL_INVALID,line_number,model_name,file_name);
				return(NULL);
			}

			number_terms=get_terms(parameter_line,"TERMS",terms);
			if (error_handler.fail()) return(NULL);

			if (number_terms!=TModelMobility::get_required_terms()) {
				error_handler.set_error(ERROR_PARSE_MODEL_INCORRECT_TERMS,line_number,"",file_name);
				delete[] terms;
				return(NULL);
			}

			model_function=new TModelMobility(terms);
			delete[] terms;
			return(model_function);
		}


		error_handler.set_error(ERROR_PARSE_MODEL_UNKNOWN,line_number,model_name,file_name);
		return(NULL);
	}
}

/************************************** class TParseDevice ************************************

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
*/

TParseDevice::TParseDevice(const char *file)
	: TParseMaterialParam(file)
{
	structure_total_length=0.0;
	start_repeat_pos=0;
	doping_entered=FALSE;
	region_entered=FALSE;
	radius_entered=FALSE;
}

TDeviceFileInput TParseDevice::parse_device(void)
{
	string new_line;

	device_input.delete_contents();
	while (!input_file_stream.eof() && !error_handler.fail()) {
		new_line=get_string();
		if (!new_line.is_null()) process_line(new_line);
	}
	if (!error_handler.fail()) apply_defaults();
	return(device_input);
}

void TParseDevice::process_line(string line_string)
{
	MaterialParam i;

	assert(!line_string.is_null());

	if (line_string.find("GRID",0)==0) {
		process_grid(line_string);
		return;
	}
	if (line_string.find("DOPING",0)==0) {
		process_doping(line_string);
		return;
	}
	if (line_string.find("STRUCTURE",0)==0) {
		process_structure(line_string);
		return;
	}
	if (line_string.find("REGION",0)==0) {
		process_region(line_string);
		return;
	}
	if (line_string.find("CAVITY",0)==0) {
		process_cavity(line_string);
		return;
	}
	if (line_string.find("MIRROR",0)==0) {
		process_mirror(line_string);
		return;
	}
	if (line_string.find("REPEAT",0)==0) {
		process_repeat(line_string);
		return;
	}
	if (line_string.find("RADIUS",0)==0) {
		process_radius(line_string);
		return;
	}

	for (i=1;i<=MAT_MAX_NUMBER_PARAMETERS;i++) {
		if (line_string.find(material_value_to_string(i),0)==0) {
			process_material_param(line_string,i);
			return;
		}
	}
	error_handler.set_error(ERROR_PARSE_KEYWORD,line_number,"",file_name);
}

void TParseDevice::process_grid(string line_string)
{
	prec size;
	GridInput new_grid;

	new_grid.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	new_grid.number_points=(short)get_long(line_string,"POINTS");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()==ERROR_PARSE_NAME_STRING) {
			error_handler.clear();
			size=get_float(line_string,"SIZE");
			if (error_handler.fail()) return;
			new_grid.number_points=new_grid.length/size;
		}
		else return;
	}

	device_input.add_grid(new_grid);
}

void TParseDevice::process_doping(string line_string)
{
	TFunction *donor_function, *acceptor_function;
	logical donor_entered=FALSE, acceptor_entered=FALSE;
	DopingInput new_doping;

	new_doping.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	donor_function=get_general_function(line_string,"ND","D");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
	}
	else donor_entered=TRUE;

	new_doping.donor_degeneracy=get_long(line_string,"ND_DEG");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		new_doping.donor_degeneracy=0;
	}

	new_doping.donor_level=get_float(line_string,"ND_LEVEL");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		new_doping.donor_level=0.0;
	}

	acceptor_function=get_general_function(line_string,"NA","D");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
	}
	else acceptor_entered=TRUE;

	new_doping.acceptor_degeneracy=get_long(line_string,"NA_DEG");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		new_doping.acceptor_degeneracy=0;
	}

	new_doping.acceptor_level=get_float(line_string,"NA_LEVEL");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()!=ERROR_PARSE_NAME_STRING) return;
		error_handler.clear();
		new_doping.acceptor_level=0.0;
	}

	if (donor_entered) new_doping.donor_function=donor_function;
	else new_doping.donor_function=new TConstant(0.0,1);

	if (acceptor_entered) new_doping.acceptor_function=acceptor_function;
	else new_doping.acceptor_function=new TConstant(0.0,1);

	device_input.add_doping(new_doping);
	doping_entered=TRUE;
}

void TParseDevice::process_structure(string line_string)
{
	string material_name, alloy_name;
	StructureInput new_structure;

	new_structure.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	material_name=get_name(line_string,"MATERIAL");
	if (error_handler.fail()) return;
	new_structure.material_type=material_parameters.get_material_type(material_name);
	if (new_structure.material_type==MAT_NO_MATERIAL) {
		error_handler.set_error(ERROR_MAT_INVALID_NAME,line_number,material_name,file_name);
		return;
	}

	alloy_name=get_name(line_string,"ALLOY");
	if (error_handler.fail()) {
		if (error_handler.get_error_number()==ERROR_PARSE_NAME) return;
		error_handler.clear();
		alloy_name="DEFAULT";
		new_structure.alloy_function=new TConstant(0.0,1);
	}
	else {
		new_structure.alloy_function=get_general_function(line_string,"CONC","D");
		if (error_handler.fail()) {
			delete new_structure.alloy_function;
			return;
		}
	}

	new_structure.alloy_type=material_parameters.get_alloy_type(material_name,alloy_name);
	if (new_structure.alloy_type==MAT_NO_ALLOY) {
		error_handler.set_error(ERROR_ALLOY_INVALID_NAME,line_number,alloy_name,file_name);
		delete new_structure.alloy_function;
		return;
	}
	device_input.add_structure(new_structure);
	structure_total_length+=new_structure.length;
}

void TParseDevice::process_region(string line_string)
{
	RegionInput new_region;

	new_region.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	if (line_string.find("BULK")!=NPOS) new_region.type=BULK;
	else {
		if (line_string.find("QW")!=NPOS) new_region.type=QW;
		else {
			error_handler.set_error(ERROR_PARSE_REGION_TYPE,line_number,"",file_name);
			return;
		}
	}

	device_input.add_region(new_region);
	region_entered=TRUE;
}

void TParseDevice::process_cavity(string line_string)
{
	CavityInput new_cavity;

	new_cavity.area=get_float(line_string,"AREA");
	if (error_handler.fail()) return;

	new_cavity.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	if (line_string.find("SURFACE")!=NPOS) new_cavity.type=SURFACE_CAVITY;
	else {
		if (line_string.find("EDGE")!=NPOS) new_cavity.type=EDGE_CAVITY;
		else {
			error_handler.set_error(ERROR_PARSE_CAVITY_TYPE,line_number,"",file_name);
			return;
		}
	}

	device_input.add_cavity(new_cavity);
}

void TParseDevice::process_mirror(string line_string)
{
	MirrorInput new_mirror;

	new_mirror.position=get_float(line_string,"POSITION");
	if (error_handler.fail()) return;

	new_mirror.reflectivity=get_float(line_string,"REF");
	if (error_handler.fail()) return;

	if (line_string.find("METAL")!=NPOS) new_mirror.type=METAL_MIRROR;
	else {
		if (line_string.find("DBR")!=NPOS) new_mirror.type=DBR_MIRROR;
		else {
			error_handler.set_error(ERROR_PARSE_MIRROR_TYPE,line_number,"",file_name);
			return;
		}
	}

	device_input.add_mirror(new_mirror);
}

void TParseDevice::process_repeat(string line_string)
{
	int i,repeat_times;
	string new_line;

	string current_repeat_string(line_string);

	if (line_string.find("START")!=NPOS) {
		if (start_repeat_pos!=0) {
			error_handler.set_error(ERROR_PARSE_DOUBLE_REPEAT,line_number,"",file_name);
			return;
		}
		else start_repeat_pos=input_file_stream.rdbuf()->seekoff(0,ios::cur,ios::in);
	}
	else {
		repeat_times=(int)get_long(line_string,"REPEAT");
		if (error_handler.fail()) return;

		if (start_repeat_pos==0) {
			error_handler.set_error(ERROR_PARSE_NO_REPEAT_START,line_number,"",file_name);
			return;
		}
		for (i=1;i<repeat_times;i++) {
			input_file_stream.rdbuf()->seekoff(start_repeat_pos,ios::beg,ios::in);
			new_line=get_string();
			while (new_line!=current_repeat_string) {
				process_line(new_line);
				new_line=get_string();
			}
		}
		start_repeat_pos=0;
	}
}

void TParseDevice::process_radius(string line_string)
{
	prec new_radius;

	if (radius_entered) {
		error_handler.set_error(ERROR_PARSE_RADIUS,line_number,"",file_name);
		return;
	}

	new_radius=get_float(line_string,"RADIUS");
	if (error_handler.fail()) return;

	device_input.add_radius(new_radius);
	radius_entered=TRUE;
}

void TParseDevice::process_material_param(string line_string, MaterialParam param)
{
	MaterialParamInput new_param_input;
	extern char *material_parameters_variables[];

	new_param_input.length=get_float(line_string,"LENGTH");
	if (error_handler.fail()) return;

	string function_variables(string(material_parameters_variables[param-1])+"D");
	string limit_variables(material_parameters_variables[param-1]);

	new_param_input.material_model=get_material_parameter(param,line_string,function_variables,limit_variables);
	if (error_handler.fail()) {
		delete new_param_input.material_model;
		return;
	}
	else device_input.add_material_param(param,new_param_input);
}

void TParseDevice::apply_defaults(void)
{
	RegionInput new_region;
	DopingInput new_doping;

	if (!region_entered) {
		new_region.type=BULK;
		new_region.length=structure_total_length;
		device_input.add_region(new_region);
	}

	if (!doping_entered) {
		new_doping.length=structure_total_length;
		new_doping.acceptor_function=new TConstant(0.0,1);
		new_doping.donor_function=new TConstant(0.0,1);
		new_doping.acceptor_degeneracy=0;
		new_doping.acceptor_level=0.0;
		new_doping.donor_degeneracy=0;
		new_doping.donor_level=0.0;
		device_input.add_doping(new_doping);
	}

	if (!radius_entered) device_input.add_radius(1.0);
}

/*********************************** class TParseMaterial ************************************

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
*/

void TParseMaterial::parse_material(void)
{
	string line_string;

	init();

	while (!input_file_stream.eof() && !error_handler.fail()) {
		line_string=get_string();

		if (line_string.find("MATERIAL",0)==0) {
			process_material(line_string);
			continue;
		}
		if (line_string.find("ALLOY",0)==0) {
			process_alloy(line_string);
			continue;
		}
		if (!line_string.is_null())
			error_handler.set_error(ERROR_PARSE_UNKNOWN_SYMBOL,line_number,"",file_name);
	}
	if (!error_handler.fail()) material_parameters.set_ready(TRUE);
}

void TParseMaterial::process_material(string line_string)
{
	string material_name;

	material_name=get_name(line_string,"MATERIAL");
	if (error_handler.fail()) return;

	material_parameters.add_material(material_name);
	if (error_handler.fail()) return;
	current_material_type=material_parameters.get_material_type(material_name);
}

void TParseMaterial::process_alloy(string line_string)
{
	TMaterialParamModel *material_param_model;
	MaterialParam i,j;
	logical mat_processed;
	string alloy_name, parameter_line;
	extern char *material_parameters_variables[];

	alloy_name=get_name(line_string,"ALLOY");
	if (error_handler.fail()) return;

	material_parameters.add_alloy(current_material_type, alloy_name);
	if (error_handler.fail()) return;
	current_alloy_type=material_parameters.get_alloy_type(current_material_type,alloy_name);

	i=0;
	while (!input_file_stream.eof() && !error_handler.fail() && (i<MAT_MAX_NUMBER_PARAMETERS)) {
		i++;
		parameter_line=get_string();

		mat_processed=FALSE;
		for (j=1;j<=MAT_MAX_NUMBER_PARAMETERS;j++) {
			if (parameter_line.find(material_value_to_string(j),0)==0) {
				material_param_model=get_material_parameter(j,parameter_line,
															material_parameters_variables[j-1],
															material_parameters_variables[j-1]);
				if (error_handler.fail()) return;
				material_parameters.add_model(current_material_type,current_alloy_type,j,material_param_model);
				mat_processed=TRUE;
				break;
			}
		}
		if (mat_processed) continue;

		if (!parameter_line.is_null())
			error_handler.set_error(ERROR_PARSE_UNKNOWN_SYMBOL,line_number,"",file_name);
	}
}


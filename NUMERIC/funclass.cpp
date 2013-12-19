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

//********************************* class TFunction ********************************************
/*
class TFunction {
protected:
	FunctionType function_type;
	short number_variables;
public:
	TFunction(FunctionType new_function_type, short new_number_variables)
		: function_type(new_function_type), number_variables(new_number_variables) {}
	TFunction(const TFunction& new_function)
		: function_type(new_function.function_type), number_variables(new_function.number_variables) {}
	TFunction(FILE *file_ptr) { read_contents(file_ptr); }
	virtual ~TFunction(void) {}
	virtual TFunction *create_copy(void)=0;
	static TFunction *create_copy(FILE *file_ptr);
	virtual prec evaluate(prec *values)=0;
	virtual void translate(void) {}
	virtual void write_state_file(FILE *file_ptr) { write_contents(file_ptr); }
	short get_number_variables(void) { return(number_variables); }
protected:
	void read_contents(FILE *file_ptr);
	void write_contents(FILE *file_ptr);
};
*/

TFunction* TFunction::create_copy(FILE *file_ptr)
{
	long file_pos;
	FunctionType function_type;
	TFunction *return_function;

	file_pos=ftell(file_ptr);
	fread(&function_type,sizeof(function_type),1,file_ptr);
	fseek(file_ptr,file_pos,SEEK_SET);

	switch(function_type) {
		case CONSTANT: return_function=new TConstant(file_ptr); break;
		case USER_FUNCTION: return_function=new TUserFunction(file_ptr); break;
		case POLYNOMIAL: return_function=new TPolynomial(file_ptr); break;
		case BAND_GAP_MODEL: return_function=new TModelBandGap(file_ptr); break;
		case THERM_CONDUCT_MODEL: return_function=new TModelThermalConductivity(file_ptr); break;
		case POWER_ABSORPTION_MODEL: return_function=new TModelPowerAbsorption(file_ptr); break;
		case POWER_ABSORPTION_BAND_GAP_MODEL: return_function=new TModelPowerAbsorptionBandGap(file_ptr); break;
		case EXP_ABSORPTION_MODEL: return_function=new TModelExpAbsorption(file_ptr); break;
		case EXP_ABSORPTION_BAND_GAP_MODEL: return_function=new TModelExpAbsorptionBandGap(file_ptr); break;
		case ALGAAS_REFRACTIVE_INDEX_MODEL: return_function=new TModelAlGaAsRefractiveIndex(file_ptr); break;
		case ALGAAS_ABSORPTION_MODEL: return_function=new TModelAlGaAsAbsorption(file_ptr); break;
		case MOBILITY_MODEL: return_function=new TModelMobility(file_ptr); break;
		default: assert(FALSE); break;
	}
	return(return_function);
}

void TFunction::read_contents(FILE *file_ptr)
{
	fread(&function_type,sizeof(function_type),1,file_ptr);
	fread(&number_variables,sizeof(number_variables),1,file_ptr);
}

void TFunction::write_contents(FILE *file_ptr)
{
	fwrite(&function_type,sizeof(function_type),1,file_ptr);
	fwrite(&number_variables,sizeof(number_variables),1,file_ptr);
}

//********************************** class TConstant *******************************************
/*
class TConstant: public TFunction {
private:
	prec term;
public:
	TConstant(prec new_term, prec number_variables)
		: TFunction(CONSTANT, number_variables), term(new_term) {}
	TConstant(const TConstant& new_constant)
		: TFunction(new_constant) { term=new_constant.term; }
	TConstant(FILE *file_ptr)
		: TFunction(file_ptr) { read_contents(file_ptr); }
	virtual TFunction *create_copy(void) { return(new TConstant(*this)); }
	virtual prec evaluate(prec *values) { return(term); }
	virtual void write_state_file(FILE *file_ptr) { write_contents(file_ptr); }
protected:
	void read_contents(FILE *file_ptr);
	void write_contents(FILE *file_ptr);
};
*/

void TConstant::read_contents(FILE *file_ptr)
{
	fread(&term,sizeof(term),1,file_ptr);
}

void TConstant::write_contents(FILE *file_ptr)
{
	TFunction::write_contents(file_ptr);
	fwrite(&term,sizeof(term),1,file_ptr);
}

//********************************** class TUserFunction ***************************************
/*
class TUserFunction: public TFunction {
private:
	char *function_string;
	char *variable_string;
	formu function;
	int translate_error;
public:
	TUserFunction(string new_function_string, string new_variables);
	TUserFunction(const TUserFunction& new_user_function);
	TUserFunction(FILE *file_ptr) : TFunction(file_ptr) { read_contents(file_ptr); }
	virtual ~TUserFunction(void)
		{ destrf(function); delete[] function_string; delete[] variable_string; }
	virtual TFunction *create_copy(void) { return(new TUserFunction(*this)); }
	virtual void translate(void);
	virtual prec evaluate(prec *values);
	virtual void write_state_file(FILE *file_ptr) { write_contents(file_ptr); }
private:
	void function_fix_up(void);
	void read_contents(FILE *file_ptr);
	void write_contents(FILE *file_ptr);
};
*/
TUserFunction::TUserFunction(string new_function_string, string new_variables)
	: TFunction(USER_FUNCTION,(short)strlen(new_variables.c_str()))
{
	function_string=new char[strlen(new_function_string.c_str())+1];
	strcpy(function_string,new_function_string.c_str());
	strlwr(function_string);

	variable_string=new char[strlen(new_variables.c_str())+1];
	strcpy(variable_string,new_variables.c_str());
	strlwr(variable_string);

	translate_error=-1;

	make_empty(&function);
}

TUserFunction::TUserFunction(const TUserFunction& new_user_function)
	: TFunction(new_user_function)
{
	function_string=new char[strlen(new_user_function.function_string)+1];
	variable_string=new char[strlen(new_user_function.variable_string)+1];

	strcpy(function_string,new_user_function.function_string);
	strcpy(variable_string,new_user_function.variable_string);

	translate_error=new_user_function.translate_error;
	if (fnot_empty(new_user_function.function)) translate();
}

void TUserFunction::translate(void)
{
	int length;
	string temp_string;

	function_fix_up();
	function=::translate(function_string,variable_string,&length,&translate_error);
	if (translate_error!=-1) {
		error_handler.set_error(ERROR_FUNCTION_TRANSLATE,0,"","");
		return;
	}
}

prec TUserFunction::evaluate(prec *values)
{
	assert(translate_error==-1);
	return(fval(function,variable_string,values));
}

void TUserFunction::function_fix_up(void)
{
	size_t start_search=0;
	char prior_char;

	string temp_string(function_string);

	while (start_search!=NPOS) {
		start_search=temp_string.find("e",start_search);
		if ((start_search>0) && (start_search!=NPOS)) {
			prior_char=temp_string.get_at(start_search-1);
			if ((prior_char>='0') && (prior_char<='9'))	temp_string.replace(start_search,1,"E");
			start_search++;
		}
	}
	strcpy(function_string,temp_string.c_str());
}

void TUserFunction::read_contents(FILE *file_ptr)
{
	short function_length, variable_length;

	fread(&function_length,sizeof(function_length),1,file_ptr);
	fread(&variable_length,sizeof(variable_length),1,file_ptr);
	function_string=new char[function_length];
	variable_string=new char[variable_length];
	fread(function_string,sizeof(char),function_length,file_ptr);
	fread(variable_string,sizeof(char),variable_length,file_ptr);

	translate();
}

void TUserFunction::write_contents(FILE *file_ptr)
{
	short function_length, variable_length;

	function_length=(short)(strlen(function_string)+1);
	variable_length=(short)(strlen(variable_string)+1);

	TFunction::write_contents(file_ptr);
	fwrite(&function_length,sizeof(function_length),1,file_ptr);
	fwrite(&variable_length,sizeof(variable_length),1,file_ptr);
	fwrite(function_string,sizeof(char),function_length,file_ptr);
	fwrite(variable_string,sizeof(char),variable_length,file_ptr);
}

//********************************** class TTermsFunction **************************************
/*
class TTermsFunction: public TFunction {
protected:
	short number_terms;
	prec *terms;
public:
	TTermsFunction(FunctionType function_type, int new_number_terms, prec *new_terms,
				   prec number_variables)
		: TFunction(function_type, number_variables) { copy_terms(new_number_terms,new_terms); }
	TTermsFunction(const TTermsFunction& new_function)
		: TFunction(new_function)
		  { copy_terms(new_function.number_terms, new_function.terms); }
	TTermsFunction(FILE *file_ptr) : TFunction(file_ptr) { read_contents(file_ptr); }
	virtual ~TTermsFunction(void) { delete[] terms; }
	virtual void write_state_file(FILE *file_ptr) { write_contents(file_ptr); }
protected:
	void read_contents(FILE *file_ptr);
	void write_contents(FILE *file_ptr);
private:
	void copy_terms(int number_terms, prec *new_terms);
};
*/

void TTermsFunction::read_contents(FILE *file_ptr)
{
	fread(&number_terms,sizeof(number_terms),1,file_ptr);
	terms=new prec[number_terms];
	fread(terms,sizeof(prec),number_terms,file_ptr);
}

void TTermsFunction::write_contents(FILE *file_ptr)
{
	TFunction::write_contents(file_ptr);
	fwrite(&number_terms,sizeof(number_terms),1,file_ptr);
	fwrite(terms,sizeof(prec),number_terms,file_ptr);
}

void TTermsFunction::copy_terms(int new_number_terms, prec *new_terms)
{
	int i;

	number_terms=(short)new_number_terms;
	terms=new prec[number_terms];
	for (i=0;i<number_terms;i++) terms[i]=new_terms[i];
}

//********************************** class TPolynomial *****************************************
/*
class TPolynomial: public TTermsFunction {
public:
	TPolynomial(int new_number_terms, prec *new_terms, prec number_variables)
		: TTermsFunction(POLYNOMIAL, new_number_terms, new_terms, number_variables) {}
	TPolynomial(const TPolynomial& new_polynomial)
		: TTermsFunction(new_polynomial) {}
	TPolynomial(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TPolynomial(void) {}
	virtual TFunction *create_copy(void) { return(new TPolynomial(*this)); }
	virtual prec evaluate(prec *values);
};
*/

prec TPolynomial::evaluate(prec *values)
{
	int i;
	prec result, power_value;

	result=terms[0];
	power_value=*values;
	for (i=1;i<number_terms;i++) {
		result+=terms[i]*power_value;
		power_value*=(*values);
	}
	return(result);
}

//********************************** class TModelBandGap ***************************************
/*
class TModelBandGap: public TTermsFunction {
public:
	TModelBandGap(prec *new_terms) : TTermsFunction(BAND_GAP_MODEL,5,new_terms,2) {}
	TModelBandGap(const TModelBandGap& new_model) : TTermsFunction(new_model) {}
	TModelBandGap(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelBandGap(void) {}
	virtual TFunction *create_copy(void) { return(new TModelBandGap(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(5); }
	static logical valid_model(MaterialParam param)
		{ return((param==MAT_BAND_GAP) || (param==MAT_ELECTRON_AFFINITY)); }
};
*/
prec TModelBandGap::evaluate(prec *values)
{
	return(terms[0]+
		   terms[1]*values[0]+
		   terms[2]*sq(values[0])+
		   terms[3]*(sq(values[1])/(terms[4]+values[1])-sq(300.0)/(terms[4]+300.0)));
}

//********************************** class TModelThermalConductivity ***************************
/*
class TModelThermalConductivity: public TTermsFunction {
public:
	TModelThermalConductivity(prec *new_terms)
		: TTermsFunction(THERM_CONDUCT_MODEL,5,new_terms,2) {}
	TModelThermalConductivity(const TModelThermalConductivity& new_model)
		: TTermsFunction(new_model) {}
	TModelThermalConductivity(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelThermalConductivity(void) {}
	virtual TFunction *create_copy(void) { return(new TModelThermalConductivity(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(5); }
	static logical valid_model(MaterialParam param)
		{ return((param==MAT_THERMAL_CONDUCTIVITY) || (param==MAT_DERIV_THERMAL_CONDUCT)); }
};
*/
prec TModelThermalConductivity::evaluate(prec *values)
{
	return(terms[0]*pow(values[1],terms[4])/
		   (terms[1]+terms[2]*values[0]+terms[3]*sq(values[0])));
}

/************************************ class TModelPowerAbsorption ******************************

class TModelPowerAbsorption: public TTermsFunction {
public:
	TModelPowerAbsorption(prec *new_terms)
		: TTermsFunction(POWER_ABSORPTION_MODEL,5,new_terms,4) {}
	TModelPowerAbsorption(const TModelPowerAbsorption& new_model)
		: TTermsFunction(new_model) {}
	TModelPowerAbsorption(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelPowerAbsorption(void) {}
	virtual TFunction *create_copy(void) { return(new TModelPowerAbsorption(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(5); }
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_ABSORPTION); }
};
*/
prec TModelPowerAbsorption::evaluate(prec *values)
{
	return((terms[0]+terms[1]*values[0]+terms[2]*sq(values[0]))*pow(values[2]-terms[3],terms[4]));
}

/****************************** class TModelPowerAbsorptionBandGap *****************************

class TModelPowerAbsorptionBandGap: public TTermsFunction {
public:
	TModelPowerAbsorptionBandGap(prec *new_terms)
		: TTermsFunction(POWER_ABSORPTION_BAND_GAP_MODEL,5,new_terms,4) {}
	TModelPowerAbsorptionBandGap(const TModelPowerAbsorptionBandGap& new_model)
		: TTermsFunction(new_model) {}
	TModelPowerAbsorptionBandGap(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelPowerAbsorptionBandGap(void) {}
	virtual TFunction *create_copy(void) { return(new TModelPowerAbsorptionBandGap(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(5); }
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_ABSORPTION); }
};
*/
prec TModelPowerAbsorptionBandGap::evaluate(prec *values)
{
	return((terms[0]+terms[1]*values[0]+terms[2]*sq(values[0]))*pow(values[2]-values[3]-terms[3],terms[4]));
}

/************************************ class TModelExpAbsorption ********************************

class TModelExpAbsorption: public TTermsFunction {
public:
	TModelExpAbsorption(prec *new_terms)
		: TTermsFunction(EXP_ABSORPTION_MODEL,3,new_terms,4) {}
	TModelExpAbsorption(const TModelExpAbsorption& new_model)
		: TTermsFunction(new_model) {}
	TModelExpAbsorption(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelExpAbsorption(void) {}
	virtual TFunction *create_copy(void) { return(new TModelExpAbsorption(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(3); }
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_ABSORPTION); }
};
*/
prec TModelExpAbsorption::evaluate(prec *values)
{
	return(terms[0]*exp(terms[1]*(values[2]-terms[2])));
}

/****************************** class TModelExpAbsorptionBandGap *****************************

class TModelExpAbsorptionBandGap: public TTermsFunction {
public:
	TModelExpAbsorptionBandGap(prec *new_terms)
		: TTermsFunction(EXP_ABSORPTION_BAND_GAP_MODEL,3,new_terms,4) {}
	TModelExpAbsorptionBandGap(const TModelExpAbsorptionBandGap& new_model)
		: TTermsFunction(new_model) {}
	TModelExpAbsorptionBandGap(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelExpAbsorptionBandGap(void) {}
	virtual TFunction *create_copy(void) { return(new TModelExpAbsorptionBandGap(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(3); }
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_ABSORPTION); }
};
*/
prec TModelExpAbsorptionBandGap::evaluate(prec *values)
{
	return(terms[0]*exp(terms[1]*(values[2]-values[3]-terms[2])));
}

/******************************* class TModelAlGaAsPermitivity *********************************
class TModelAlGaAsPermitivity: public TFunction {
private:
	static float E_values[];
	static float Gamma_values[];
	static float A_values[];
	static float Phi_values[];
public:
	TModelAlGaAsPermitivity(FunctionType new_function_type)
		: TFunction(new_function_type,5) {}
	TModelAlGaAsPermitivity(const TModelAlGaAsPermitivity& new_model)
		: TFunction(new_model) {}
	TModelAlGaAsPermitivity(FILE *file_ptr)
		: TFunction(file_ptr) {}
	virtual ~TModelAlGaAsPermitivity(void) {}
	virtual void write_state_file(FILE *file_ptr) { TFunction::write_contents(file_ptr); }
	static int get_required_terms(void) { return(0); }
protected:
	complex comp_permitivity(prec *values);
};
*/

float TModelAlGaAsPermitivity::E_values[]= {
	1.39060, 2.09260, -1.60624, 1.39457,
	1.40068, 1.99832, -1.90326, 1.53105,
	2.92232, 0.82560, -0.59311, 0.83462,
	3.14001, 0.65211, -0.41695, 0.55666,
	3.25876, 0.46874, 0.17375, -0.04572,
	3.89007, 1.34836, -0.46618, -0.47756,
	4.55820, 0.43356, -0.25474, -0.08739,
	4.83763, -0.29928, 0.40196, -0.27251,
	5.70697, 1.82111, -2.62439, 2.34028,
};

float TModelAlGaAsPermitivity::Gamma_values[]= {
	0.13786, 0.13697, 0.47712, 0.09325,
	0.11079, -0.17490, 1.26335, -1.36715,
	0.08267, 0.07536, 0.00580, 0.02843,
	0.16014, 0.11849, -0.16197, 0.13568,
	0.38313, 0.18661, -0.23121, -0.19555,
	0.79424, 0.11166, -0.20242, -0.13491,
	0.33443, -0.19031, 0.01285, -0.06472,
	0.27022, 0.19000, -0.18688, 0.03856,
	0.37446, 0.54285, -6.79344, 9.63119,
};

float TModelAlGaAsPermitivity::A_values[]= {
	0.36529, 0.08038, -2.23761, 4.07322,
	0.26138, -0.36973, 0.40233, -0.34884,
	0.70159, -0.47910, 1.68160, -0.85775,
	1.36908, -0.73488, 3.34016, -0.96149,
	2.87457, 1.49872, -3.40598, -0.21577,
	5.43232, -0.37707, 8.27471, -3.76629,
	5.21290, -5.39337, 1.05408, -1.05662,
	4.85697, -1.38754, 3.82283, 0.52175,
	3.90983, 0.67149, -10.27836, 16.38186,
};

float TModelAlGaAsPermitivity::Phi_values[]= {
	2.53491, 1.37492, 2.81830, -2.88902,
	-0.88229, 1.14488, -2.90855, 0.72062,
	0.04704, 0.42993, 1.61326, 0.42177,
	0.33154, -0.77594, 0.91058, -1.06396,
	-0.49340, 0.04123, -1.76056, 0.14619,
	0.33913, -0.31718, 1.03742, -0.19826,
	0.30805, 0.92272, -0.71698, 1.17884,
	-0.24016, -1.25824, 1.17243, -0.32264,
	-0.70131, 2.01631, -1.32069, -0.21253,
};

complex TModelAlGaAsPermitivity::comp_permitivity(prec *values)
{
	int i;
	static complex previous_result(0.0,0.0);
	static prec previous_values[]={ 0.0, 0.0, 0.0, 0.0, 0.0 };
	logical compute_new_result=FALSE;
	prec e_value, gamma_value, a_value, phi_value;
	float *e_ptr, *gamma_ptr, *a_ptr, *phi_ptr;
	prec alloy_conc=values[0];
	prec photon_energy=values[2];
	prec evaluation_energy;

	for (i=0;i<5;i++) compute_new_result|=(previous_values[i]!=values[i]);
	if (!compute_new_result) return(previous_result);

	for (i=0;i<5;i++) previous_values[i]=values[i];

	previous_result=complex(0.0,0.0);

	evaluation_energy=photon_energy+(values[4]-values[3]);

	e_ptr=E_values;
	gamma_ptr=Gamma_values;
	a_ptr=A_values;
	phi_ptr=Phi_values;

	for (i=0;i<9;i++) {
		e_value=(*(e_ptr))+
				(*(e_ptr+1))*alloy_conc+
				(*(e_ptr+2))*sq(alloy_conc)+
				(*(e_ptr+3))*sq(alloy_conc)*alloy_conc;
		gamma_value=(*(gamma_ptr))+
					(*(gamma_ptr+1))*alloy_conc+
					(*(gamma_ptr+2))*sq(alloy_conc)+
					(*(gamma_ptr+3))*sq(alloy_conc)*alloy_conc;
		a_value=(*(a_ptr))+
				(*(a_ptr+1))*alloy_conc+
				(*(a_ptr+2))*sq(alloy_conc)+
				(*(a_ptr+3))*sq(alloy_conc)*alloy_conc;
		phi_value=(*(phi_ptr))+
				  (*(phi_ptr+1))*alloy_conc+
				  (*(phi_ptr+2))*sq(alloy_conc)+
				  (*(phi_ptr+3))*sq(alloy_conc)*alloy_conc;

		previous_result+=a_value*exp(complex(0,phi_value))*((1.0/complex(evaluation_energy+e_value,gamma_value))-
															(1.0/complex(evaluation_energy-e_value,gamma_value)));

		e_ptr+=4;
		gamma_ptr+=4;
		a_ptr+=4;
		phi_ptr+=4;
	}
	previous_result+=0.99;

	return(previous_result);
}

/******************************* class TModelAlGaAsRefractiveIndex *****************************
class TModelAlGaAsRefractiveIndex: public TModelAlGaAsPermitivity {
public:
	TModelAlGaAsRefractiveIndex(void)
		: TModelAlGaAsPermitivity(ALGAAS_REFRACTIVE_INDEX_MODEL) {}
	TModelAlGaAsRefractiveIndex(const TModelAlGaAsRefractiveIndex& new_model)
		: TModelAlGaAsPermitivity(new_model) {}
	TModelAlGaAsRefractiveIndex(FILE *file_ptr)
		: TModelAlGaAsPermitivity(file_ptr) {}
	virtual ~TModelAlGaAsRefractiveIndex(void) {}
	virtual TFunction *create_copy(void)
		{ return(new TModelAlGaAsRefractiveIndex(*this)); }
	virtual prec evaluate(prec *values)
		{ return(real(sqrt(comp_permitivity(values)))); }
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_REFRACTIVE_INDEX); }
};
*/

/******************************* class TModelAlGaAsAbsorption **********************************
class TModelAlGaAsAbsorption: public TModelAlGaAsPermitivity {
public:
	TModelAlGaAsAbsorption(void)
		: TModelAlGaAsPermitivity(ALGAAS_ABSORPTION_MODEL) {}
	TModelAlGaAsAbsorption(const TModelAlGaAsAbsorption& new_model)
		: TModelAlGaAsPermitivity(new_model) {}
	TModelAlGaAsAbsorption(FILE *file_ptr)
		: TModelAlGaAsPermitivity(file_ptr) {}
	virtual ~TModelAlGaAsAbsorption(void) {}
	virtual TModelAlGaAsAbsorption *create_copy(void)
		{ return(new TModelAlGaAsAbsorption(*this)); }
	virtual prec evaluate(prec *values);
	static logical valid_model(MaterialParam param)
		{ return(param==MAT_ABSORPTION); }
};
*/

prec TModelAlGaAsAbsorption::evaluate(prec *values)
{
	prec result=4.0*SIM_pi*values[2]*1e4*imag(sqrt(comp_permitivity(values)))/1.24;
	if (result<=0.0) result=0.0;
	return(result);
}

//********************************** class TModelMobility **************************************
/*
class TModelMobility: public TTermsFunction {
public:
	TModelMobility(prec *new_terms) : TTermsFunction(MOBILITY_MODEL,11,new_terms,5) {}
	TModelMobility(const TModelMobility& new_model) : TTermsFunction(new_model) {}
	TModelMobility(FILE *file_ptr) : TTermsFunction(file_ptr) {}
	virtual ~TModelMobility(void) {}
	virtual TFunction *create_copy(void) { return(new TModelMobility(*this)); }
	virtual prec evaluate(prec *values);
	static int get_required_terms(void) { return(11); }
	static logical valid_model(MaterialParam param)
		{ return((param==MAT_ELECTRON_MOBILITY) || (param==MAT_HOLE_MOBILITY)); }
};
*/
prec TModelMobility::evaluate(prec *values)
{
	return((terms[0]+terms[1]*values[0]+terms[2]*sq(values[0]))*pow(values[2]/terms[3],terms[4])+
		   (terms[5]+terms[6]*values[0]+terms[7]*sq(values[0]))*pow(values[2],terms[8])/
											(1.0+(values[3]+values[4])/terms[9]*pow(values[2]/terms[3],terms[10])));
}

//********************************** class TPieceWiseFunction **********************************
/*
class TPieceWiseFunction {
private:
	int number_variables;
	TFunction *function;
	TFunction **lower_limit;
	TFunction **upper_limit;
public:
	TPieceWiseFunction(TFunction *new_function,
					   TFunction **new_lower_limit, TFunction **new_upper_limit);
	TPieceWiseFunction(const TPieceWiseFunction& new_function);
	TPieceWiseFunction& operator=(const TPieceWiseFunction& new_function);
	TPieceWiseFunction(FILE *file_ptr) { read_state_file(file_ptr); }
	~TPieceWiseFunction(void) { clear_contents(); }
	prec evaluate(prec *values)	{ return(function->evaluate(values)); }
	void set_limits(TFunction **new_lower_limit, TFunction **new_upper_limit);
	logical is_valid(prec *values);
	int get_number_variables(void) { return(number_variables); }
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
private:
	void clear_contents(void);
};
*/

TPieceWiseFunction::TPieceWiseFunction(TFunction *new_function,
									   TFunction **new_lower_limit, TFunction **new_upper_limit)
{
	assert(new_function!=(TFunction *)NULL);
	function=new_function;
	number_variables=new_function->get_number_variables();

	lower_limit=new TFunction*[number_variables];
	upper_limit=new TFunction*[number_variables];

	set_limits(new_lower_limit,new_upper_limit);
}

TPieceWiseFunction::TPieceWiseFunction(const TPieceWiseFunction& new_function)
{
	int i;
	function=new_function.function->create_copy();
	number_variables=new_function.number_variables;

	lower_limit=new TFunction*[number_variables];
	upper_limit=new TFunction*[number_variables];

	for (i=0;i<number_variables;i++) {
		lower_limit[i]=new_function.lower_limit[i]->create_copy();
		upper_limit[i]=new_function.upper_limit[i]->create_copy();
	}
}

TPieceWiseFunction& TPieceWiseFunction::operator=(const TPieceWiseFunction& new_function)
{
	int i;

	clear_contents();

	function=new_function.function->create_copy();
	number_variables=new_function.number_variables;

	lower_limit=new TFunction*[number_variables];
	upper_limit=new TFunction*[number_variables];

	for (i=0;i<number_variables;i++) {
		lower_limit[i]=new_function.lower_limit[i]->create_copy();
		upper_limit[i]=new_function.upper_limit[i]->create_copy();
	}
	return(*this);
}

void TPieceWiseFunction::set_limits(TFunction **new_lower_limit, TFunction **new_upper_limit)
{
	int i;

	if ((new_lower_limit==(TFunction **)NULL) || (new_upper_limit==(TFunction **)NULL)){
		for (i=0;i<number_variables;i++) {
			lower_limit[i]=new TConstant(0.0,0);
			upper_limit[i]=new TConstant(0.0,0);
		}
	}
	else {
		for (i=0;i<number_variables;i++) {
			if ((new_lower_limit[i]==(TFunction *)NULL) ||
				(new_upper_limit[i]==(TFunction *)NULL)) {
				lower_limit[i]=new TConstant(0.0,0);
				upper_limit[i]=new TConstant(0.0,0);
			}
			else {
				lower_limit[i]=new_lower_limit[i];
				upper_limit[i]=new_upper_limit[i];
			}
		}
	}
}

logical TPieceWiseFunction::is_valid(prec *values)
{
	int i;
	logical result=TRUE;
	prec lower_limit_value, upper_limit_value;

	for (i=0;i<number_variables;i++) {
		lower_limit_value=lower_limit[i]->evaluate(values);
		upper_limit_value=upper_limit[i]->evaluate(values);
		if (lower_limit_value==upper_limit_value) continue;
		if ((values[i]<lower_limit_value) || (values[i]>upper_limit_value)) result&=FALSE;
	}
	return(result);
}

void TPieceWiseFunction::read_state_file(FILE *file_ptr)
{
	int i;

	function=TFunction::create_copy(file_ptr);
	number_variables=function->get_number_variables();

	lower_limit=new TFunction*[number_variables];
	upper_limit=new TFunction*[number_variables];

	for (i=0;i<number_variables;i++) {
		lower_limit[i]=TFunction::create_copy(file_ptr);
		upper_limit[i]=TFunction::create_copy(file_ptr);
	}
}

void TPieceWiseFunction::write_state_file(FILE *file_ptr)
{
	int i;

	function->write_state_file(file_ptr);
	for (i=0;i<number_variables;i++) {
		lower_limit[i]->write_state_file(file_ptr);
		upper_limit[i]->write_state_file(file_ptr);
	}
}

void TPieceWiseFunction::clear_contents(void)
{
	int i;

	for (i=0;i<number_variables;i++) {
		delete lower_limit[i];
		delete upper_limit[i];
	}

	delete function;
	delete[] lower_limit;
	delete[] upper_limit;
}

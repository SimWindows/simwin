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



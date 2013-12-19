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

//*********************************** Global Functions *****************************************

/***********************************************************************************************
void _matherr(exception *new_error)
	This function catches domain and range errors for math functions. It should not be
	called directly.
*/

int _matherr(exception *new_error)
{
	error_handler.set_error(ERROR_SIMULATION,0,"","");
	new_error->retval=0.0;
#ifdef NDEBUG
	return(1);
#else
	return(0);
#endif
}

/***********************************************************************************************
void convert_mantissa_exp(float& mantissa, int& exponent)
	Takes a number of the form mantissa*2^exponent and converts it to the form
	mantissa*10^exponent.
*/

void convert_mantissa_exp(float& mantissa, int& exponent)
{
	double new_exponent, temp_exponent;

	temp_exponent=(double)(exponent)*log10(2.0);
	new_exponent=floor(temp_exponent);

	mantissa*=(float)pow(10.0,temp_exponent-new_exponent);
	exponent=(int) new_exponent;
}

/***********************************************************************************************
prec bernoulli(prec x)
	Computes the Bernoulli function. When x<=1e-10, the bernoulli function is computed via
	an expansion.
*/

prec bernoulli(prec x)
{
	if (fabs(x)<=1e-10) return((prec) 1.0/(1.0+x/2.0+sq(x)/6.0));
	else return(x/(exp(x)-(prec) 1.0));
}

/***********************************************************************************************
prec deriv_bernoulli(prec x)
	Computes the derivative of the Bernoulli function. When x<=1e-10, the derivative is
	computed via an expansion.
*/

prec deriv_bernoulli(prec x)
{
	if (fabs(x)<=1e-10) return ((prec) (-0.5-x/3.0-sq(x)/8.0)/(1+x+(7.0/12.0)*sq(x)));
	else return((exp(x)-1.0-x*exp(x))/(sq(exp(x)-1.0)));
}

/***********************************************************************************************
prec fermi(prec x)
	Computes the fermi-dirac distribution.
*/
prec fermi(prec x, prec degeneracy)
{
	return(1.0/(1.0+degeneracy*exp(x)));
}

/***********************************************************************************************
prec deriv_fermi(prec x)
	Computes the derivative of the fermi-dirac distribution.
*/
prec deriv_fermi(prec x, prec degeneracy)
{
	return(-degeneracy*exp(x)/sq(1.0+degeneracy*exp(x)));
}

/***********************************************************************************************
	The following functions compute fermi integrals to various orders
*/

prec fermi_integral_minus_2_half(prec x)
{
	return(1.0/(1.0+exp(-x)));
}

prec fermi_integral_minus_1_half(prec x)
{
	return(1.0/
		   (1.253314137/sqrt(1.495+x+pow(pow(fabs(x-1.495),2.828427125)+4.466276461,0.353553391))+exp(-x))
		  );
}

prec fermi_integral_0_half(prec x)
{
	return(log_1_x(exp(x)));
//	return(1.0/
//		   (2.0/(1.8+x+pow(pow(fabs(x-1.8),2.585786438)+7.548142204,0.38672954))+exp(-x))
//		  );
}

prec fermi_integral_1_half(prec x)
{
	return(1.0/
		   (3.759942412/pow(2.105+x+pow(pow(fabs(x-2.105),2.414213562)+9.901280188,0.414213562),1.5)+exp(-x))
		  );
}

prec fermi_integral_2_half(prec x)
{
	return(1.0/
		   (8.0/pow(2.41+x+pow(pow(fabs(x-2.41),2.292893219)+11.78562398,0.43613021),2.0)+exp(-x))
		  );
}

prec fermi_integral_3_half(prec x)
{
	return(1.0/
		   (18.79971206/pow(2.715+x+pow(pow(fabs(x-2.715),2.207106781)+13.4388215,0.453081839),2.5)+exp(-x))
		  );
}

prec fermi_integral_4_half(prec x)
{
	return(1.0/
		   (48.0/pow(3.02+x+pow(pow(fabs(x-3.02),2.146446609)+15.00708235,0.465886268),3.0)+exp(-x))
		  );
}

prec fermi_integral_5_half(prec x)
{
	return(1.0/
		   (131.5979844/pow(3.325+x+pow(pow(fabs(x-3.325),2.103553391)+16.57024301,0.47538608),3.5)+exp(-x))
		  );
}

prec fermi_integral_6_half(prec x)
{
	return(1.0/
		   (384.0/pow(3.63+x+pow(pow(fabs(x-3.63),2.073223305)+18.16859268,0.48234071),4.0)+exp(-x))
		  );
}

prec fermi_integral_8_half(prec x)
{
	return(1.0/
		   (3840.0/pow(4.24+x+pow(pow(fabs(x-4.24),2.036611652)+21.53087754,0.491011627),5.0)+exp(-x))
		  );
}



/***********************************************************************************************
prec incomp_gamma(prec x)
	Computes the incomplete gamma function of order 3/2. This is done using	a series
	representation and the series is terminated when the next term in the series is less than
	1e-8. The series expression is achieved using eqs 6.5.4 and 6.5.29 in "Handbook of
	Mathematical Functions."
*/

prec incomp_gamma(prec x)
{
	int n;
	prec next_term, result, n_factorial;

	n_factorial=1;
	result=2.0/3.0;
	n=1;
	do {
		n_factorial*=n;
		next_term=pow(-x,(prec) n)/((1.5+(prec) n)*n_factorial);

		result+=next_term;
		n++;
	} while (fabs(next_term/result)>=1e-8);

	result*=pow(x,1.5);
	return(result);
}

/***********************************************************************************************
prec log_1_x(prec x)
	Computes the function ln(1+x). When x is small (i.e. <1e-10) then the function returns x.
*/

prec log_1_x(prec x)
{
	if ((x<1e-10) && (x>-1e-10)) return(x);
	else return(log(1.0+x));
}

prec log_1_div_1_x(prec x)
{
	if ((x<1e-10) && (x>-1e-10)) return(-x);
	else return(log(1.0/(1.0+x)));
}

/***********************************************************************************************
prec dilog(prec x)
	Computes the dilogarithm function for x<=1.
*/
prec dilog(prec x)
{
	prec n;
	prec next_term, next_numerator;
	prec result;

	assert(fabs(x)<=1.0);

	result=0.0;
	next_numerator=x;
	n=1.0;
	do {
		next_term=next_numerator/sq(n);
		result+=next_term;
		next_numerator*=x;
		n++;
	} while ((fabs(next_term/result)>=1.0e-7) && n<=2500.0);

	return(result);
}

/***********************************************************************************************
prec trilog(prec x)
	Computes the trilogarithm function for x<=1.
*/
prec trilog(prec x)
{
	prec n;
	prec next_term, next_numerator;
	prec result;

	assert(fabs(x)<=1.0);

	result=0.0;
	next_numerator=x;
	n=1.0;
	do {
		next_term=next_numerator/(sq(n)*n);
		result+=next_term;
		next_numerator*=x;
		n++;
	} while ((fabs(next_term/result)>=1.0e-7) && n<=210.0);

	return(result);
}


/***********************************************************************************************
double rnd(void)
	Compute a random number between 0 and 1. Used to override rnd() that was included in
	formulc.c
*/

double rnd(void)
{
	return(((double)rand()*2.0/(double)RAND_MAX)-1.0);
}

/***********************************************************************************************
double rnd_init(void)
	Initializes the random number generator. Used to override rnd_init() that was included in
	formulc.c
*/

void rnd_init(void)
{
	randomize();
}


void scale(float *data, int points, float& minimum, float& maximum)
{
	int i;
	float *data_ptr;

	assert(points && data);

	data_ptr=data;

	minimum=*data_ptr;
	maximum=*data_ptr;

	for (i=0;i<points;i++) {
		if (*data_ptr<minimum) minimum=*data_ptr;
		if (*data_ptr>maximum) maximum=*data_ptr;
		data_ptr++;
	}

	if (minimum==maximum) {
		minimum-=0.1*minimum;
		maximum+=0.1*maximum;
	}
}

void scale_with_skip(float *data, logical *skip, int points, float& minimum, float& maximum)
{
	int i;
	float *data_ptr;
	logical *skip_ptr;

	assert(points && data && skip);

	minimum=0.0;
	maximum=0.0;

	data_ptr=data;
	skip_ptr=skip;
	for (i=0;i<points;i++) {
		if (!*skip_ptr) {
			if (*data_ptr<minimum) minimum=*data_ptr;
			if (*data_ptr>maximum) maximum=*data_ptr;
		}
		data_ptr++;
		skip_ptr++;
	}
	if (minimum==maximum) {
		minimum-=0.1*minimum;
		maximum+=0.1*maximum;
	}
}

/***********************************************************************************************
void round_scale(float& minimum, float& maximum)
	Converts minimum and maximum to the nearest whole mantissa values. As an example if
	2.137e-8 is given as a minimum, then after round_scale is executed, minimum would
	contain 2e-8.

	*/

void round_scale(float& minimum, float& maximum)
{
	int min_exponent,max_exponent;
	float min_mantissa,max_mantissa;

	min_mantissa=frexp(minimum,&min_exponent);
	convert_mantissa_exp(min_mantissa,min_exponent);

	max_mantissa=frexp(maximum,&max_exponent);
	convert_mantissa_exp(max_mantissa,max_exponent);

	if (floor(min_mantissa)==ceil(max_mantissa)) {
		min_mantissa--;
		max_mantissa++;
	}

	minimum=floor(min_mantissa)*pow(10.0,min_exponent);
	maximum=ceil(max_mantissa)*pow(10.0,max_exponent);
}

/***********************************************************************************************
void autotic(float axis_len, float& maj_tic, float& min_tic)
	Computes the major and minor tic marks spacing for a given axis length.
*/

void autotic(float axis_len, float& maj_tic, float& min_tic)
{
	int itic;
	double mult;

	mult=1;

	while (axis_len>=10) {
		mult*=10;
		axis_len/=10;
	}

	while (axis_len<1) {
		mult/=10;
		axis_len*=10;
	}

	itic=axis_len;
	switch (itic) {
		case 9:
		case 8:
		case 7:
		case 6: maj_tic=mult;
				min_tic=maj_tic/5;
				break;
		case 5:
		case 4:
		case 3: maj_tic=mult/2;
				min_tic=maj_tic/4;
				break;
		case 2: maj_tic=mult/5;
				min_tic=maj_tic/5;
				break;
		case 1: maj_tic=mult/10;
				min_tic=maj_tic/5;
	}
}

/***********************************************************************************************
void swap(int& value_1, int& value_2)
	Swaps two integer values.
*/

void swap(int& value_1, int& value_2)
{
	int temp_value;

	temp_value=value_1;
	value_1=value_2;
	value_2=temp_value;
}

/***********************************************************************************************
void swap(int& value_1, int& value_2)
	Swaps two floating point values.
*/

void swap(float& value_1, float& value_2)
{
	float temp_value;

	temp_value=value_1;
	value_1=value_2;
	value_2=temp_value;
}

/***********************************************************************************************
int bit_position(flag flag_value)
	Computes the bit position (starting with 0) of the first set bit in flag_value.
*/

int bit_position(flag flag_value)
{
	int position=0;

	assert(flag_value);

	while (!(flag_value & 0x00000001)) {
		position++;
		flag_value>>=1;
	}
	return(position);
}

/***********************************************************************************************
int bit_position(flag flag_value)
	Computes the number of set bits in flag_value.
*/

int bit_count(flag flag_value)
{
	int i,flag_count=0;

	for (i=0;i<(int)sizeof(flag)*8;i++) {
		if (flag_value & 0x00000001) flag_count++;
		flag_value>>=1;
	}
	return(flag_count);
}

/***********************************************************************************************
prec get_normalize_value(CarrierVar quantity)
	Returns the normalization value for the particular carrier variable.
*/

prec get_normalize_value(FlagType flag_type, flag flag_value)
{
	assert(TValueFlag::valid_single_flag(flag_type,flag_value));

	switch(flag_type) {

// FREE_ELECTRON, FREE_HOLE
		case FREE_ELECTRON:
		case FREE_HOLE:
			switch (flag_value) {
				case EQUIL_DOS:
				case NON_EQUIL_DOS:
				case CONCENTRATION: return(normalization.conc);
				case MOBILITY: return(normalization.mobility);
#ifndef NDEBUG
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// BOUND_ELECTRON, BOUND_HOLE
		case BOUND_ELECTRON:
		case BOUND_HOLE:
			switch (flag_value) {
				case EQUIL_DOS:
				case NON_EQUIL_DOS:
				case CONCENTRATION: return(normalization.conc);
				case ENERGY_TOP: return(normalization.pot);
#ifndef NDEBUG
				case WAVE_FUNCTION: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// ELECTRON, HOLE
		case ELECTRON:
		case HOLE:
			switch(flag_value) {
				case TEMPERATURE: return(normalization.temp);
				case IONIZED_DOPING:
				case DOPING_CONC:
				case CONCENTRATION: return(normalization.conc);
				case BAND_EDGE: return(normalization.pot);
				case ENERGY_LIFETIME:
				case SHR_LIFETIME: return(normalization.time);
                case AUGER_COEFFICIENT: return(1.0/(sq(normalization.conc)*normalization.time));
				case CURRENT: return(normalization.current);
				case STIMULATED_FACTOR: return(normalization.energy);
                case DOPING_LEVEL:
				case EQUIL_QUASI_FERMI:
				case QUASI_FERMI: return(SIM_k_eV*normalization.temp);
				case TOTAL_HEAT:
				case SHR_HEAT:
				case B_B_HEAT:
				case STIM_HEAT:
				case RELAX_HEAT:
                case AUGER_HEAT:
				case OPTICAL_GENERATION_KIN:
				case OPTICAL_GENERATION_REF: return(normalization.recomb*normalization.energy*SIM_q);
#ifndef NDEBUG
				case COLLISION_FACTOR:
				case EQUIL_PLANCK_POT:
				case PLANCK_POT:
				case DOS_MASS:
				case DOPING_DEGENERACY:
				case COND_MASS: return(1.0);
				case MOBILITY:
				case EQUIL_DOS:
				case NON_EQUIL_DOS:
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// GRID_ELECTRICAL
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case TEMPERATURE: return(normalization.temp);
				case RADIUS:
				case POSITION: return(normalization.length/1e-4);
				case ELECTRON_AFFINITY:
				case BAND_GAP:
				case VACUUM_LEVEL:
				case POTENTIAL:	return(normalization.pot);
				case LATERAL_THERMAL_CONDUCT: return(normalization.therm_cond/sq(normalization.length));
				case THERMAL_CONDUCT: return(normalization.therm_cond);
				case FIELD:	return(normalization.pot/normalization.length);
				case B_B_RECOMB_CONSTANT: return(1.0/(normalization.conc*normalization.time));
#ifndef NDEBUG
				case NODE_NUMBER:
				case NODE_TYPE:
				case REGION_TYPE:
				case MATERIAL:
				case ALLOY_TYPE:
				case ALLOY_CONC:
				case PERMITIVITY:
				case EFFECTS: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// GRID_OPTICAL
		case GRID_OPTICAL:
			switch (flag_value) {
				case INCIDENT_ABSORPTION:
				case MODE_ABSORPTION:
				case MODE_GAIN: return(1.0/normalization.length);
				case INCIDENT_PHOTON_ENERGY:
				case MODE_PHOTON_ENERGY: return(normalization.energy);
				case INCIDENT_PHOTON_WAVELENGTH:
				case MODE_PHOTON_WAVELENGTH: return(normalization.length/1e-4);
				case INCIDENT_FORWARD_POYNTING:
				case INCIDENT_REVERSE_POYNTING:
				case INCIDENT_TOTAL_POYNTING: return(normalization.intensity*SIM_q/1e-3);
				case MODE_TOTAL_FIELD_MAG: return(sqrt(normalization.conc*1e-12));
				case MODE_GROUP_VELOCITY: return(normalization.length/normalization.time);
				case MODE_PHOTON_DENSITY: return(normalization.conc);
#ifndef NDEBUG
				case INCIDENT_OVERFLOW:
				case INCIDENT_IMPEDANCE_REAL:
				case INCIDENT_IMPEDANCE_IMAG:
				case MODE_IMPEDANCE_REAL:
				case MODE_IMPEDANCE_IMAG:
				case MODE_TOTAL_POYNTING:
				case INCIDENT_REFRACTIVE_INDEX:
				case INCIDENT_TOTAL_FIELD_MAG:
				case MODE_REFRACTIVE_INDEX:
				case INCIDENT_FORWARD_FIELD_REAL:
				case INCIDENT_FORWARD_FIELD_IMAG:
				case INCIDENT_REVERSE_FIELD_REAL:
				case INCIDENT_REVERSE_FIELD_IMAG:
				case MODE_FORWARD_FIELD_REAL:
				case MODE_FORWARD_FIELD_IMAG:
				case MODE_REVERSE_FIELD_REAL:
				case MODE_REVERSE_FIELD_IMAG:
				case MODE_TOTAL_PHOTONS: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// NODE
		case NODE:
			switch (flag_value) {
				case INTRINSIC_CONC: return(normalization.conc);
				case TOTAL_CHARGE: return(normalization.conc*normalization.charge);
				case TOTAL_RECOMB:
				case SHR_RECOMB:
				case B_B_RECOMB:
                case AUGER_RECOMB:
				case STIM_RECOMB:
				case OPTICAL_GENERATION: return(normalization.recomb);
				case TOTAL_RADIATIVE_HEAT:
				case B_B_HEAT:
				case STIM_HEAT:
				case TOTAL_HEAT:
				case SHR_HEAT:
				case OPTICAL_GENERATION_HEAT: return(normalization.recomb*normalization.energy*SIM_q);
				case TOTAL_CURRENT: return(normalization.current);
#ifndef NDEBUG
				case REDUCED_DOS_MASS: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// QW_ELECTRON
		case QW_ELECTRON:
		case QW_HOLE:
			switch (flag_value) {
				case EQUIL_DOS:
				case NON_EQUIL_DOS:
                case DOPING_CONC:
				case CONCENTRATION:	return(normalization.conc*normalization.length);
				case ENERGY_TOP:
				case ENERGY_LEVEL: return(normalization.pot);
				case STIMULATED_FACTOR: return(normalization.energy);
                case AUGER_COEFFICIENT: return(1.0/(sq(normalization.conc*normalization.length)*normalization.time));
#ifndef NDEBUG
				case EQUIL_PLANCK_POT: return(1.0);
				case WAVE_FUNCTION:
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// QUANTUM_WELL
		case QUANTUM_WELL:
			switch (flag_value) {
				case TEMPERATURE: return(normalization.temp);
				case POSITION:
				case LENGTH: return(normalization.length/1e-4);
				case BAND_GAP: return(normalization.pot);
				case INTRINSIC_CONC: return(normalization.conc*normalization.length);
				case TOTAL_RECOMB:
				case SHR_RECOMB:
				case B_B_RECOMB: return(normalization.recomb*normalization.length);
				case B_B_RECOMB_CONSTANT: return(1.0/(normalization.conc*normalization.time*normalization.length));
				case B_B_HEAT: return(normalization.recomb*normalization.energy*normalization.length*SIM_q);
#ifndef NDEBUG
				case EFFECTS:
				case NUMBER_NODES:
				case OVERLAP:
				case INCIDENT_ABSORPTION:
				case MODE_ABSORPTION:
				case MODE_GAIN: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// CONTACT
		case CONTACT:
			switch (flag_value) {
				case POSITION: return(normalization.length/1e-4);
				case ELECTRON_RECOMB_VEL:
				case HOLE_RECOMB_VEL: return(normalization.current/(normalization.charge*normalization.conc));
				case BUILT_IN_POT:
                case BARRIER_HEIGHT:
				case APPLIED_BIAS: return(normalization.pot);
				case ELECTRON_CURRENT:
				case HOLE_CURRENT:
				case TOTAL_CURRENT: return(normalization.current);
				case EQUIL_ELECTRON_CONC:
				case EQUIL_HOLE_CONC: return(normalization.conc);
				case FIELD: return(normalization.field);
#ifndef NDEBUG
				case EFFECTS:
				case NODE_NUMBER: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// SURFACE
		case SURFACE:
			switch(flag_value) {
				case ELECTRON_TEMPERATURE:
                case HOLE_TEMPERATURE:
				case TEMPERATURE: return(normalization.temp);
				case POSITION: return(normalization.length/1e-4);
				case THERMAL_CONDUCT: return(normalization.therm_cond/normalization.length);
				case INCIDENT_FORWARD_POYNTING:
				case INCIDENT_REVERSE_POYNTING:
				case INCIDENT_TOTAL_POYNTING: return(normalization.intensity*SIM_q/1e-3);
				case MODE_TOTAL_FIELD_MAG: return(sqrt(normalization.conc*1e-12));
#ifndef NDEBUG
				case EFFECTS:
				case NODE_NUMBER:
				case MODE_REVERSE_POYNTING:
				case INCIDENT_REFRACTIVE_INDEX:
				case INCIDENT_TOTAL_FIELD_MAG:
				case MODE_REFRACTIVE_INDEX:
				case INCIDENT_FORWARD_FIELD_REAL:
				case INCIDENT_FORWARD_FIELD_IMAG:
				case INCIDENT_REVERSE_FIELD_REAL:
				case INCIDENT_REVERSE_FIELD_IMAG:
				case MODE_FORWARD_FIELD_REAL:
				case MODE_FORWARD_FIELD_IMAG:
				case MODE_REVERSE_FIELD_REAL:
				case MODE_REVERSE_FIELD_IMAG:
				case MODE_FORWARD_POYNTING: return(1.0);
				case MODE_SURFACE_FIELD:
				case MODE_INTERNAL_FIELD:
				case INCIDENT_SURFACE_FIELD:
				case INCIDENT_INTERNAL_FIELD: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// MODE
		case MODE:
			switch(flag_value) {
				case PHOTON_LIFETIME: return(normalization.time);
				case MIRROR_LOSS:
				case WAVEGUIDE_LOSS:
				case MODE_GAIN: return(1.0/normalization.length);
				case MODE_GROUP_VELOCITY: return(normalization.length/normalization.time);
				case MODE_PHOTON_ENERGY: return(normalization.energy);
				case MODE_PHOTON_WAVELENGTH: return(normalization.length/1e-4);
				case TOTAL_SPONTANEOUS: return(1.0/normalization.time);
#ifndef NDEBUG
				case EFFECTS:
				case SPONT_FACTOR:
				case MODE_TOTAL_PHOTONS: return(1.0);
				case MODE_NORMALIZATION:
				case MODE_TOTAL_FIELD_MAG:
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// CAVITY
		case CAVITY:
			switch(flag_value) {
				case AREA: return(sq(normalization.length/1e-4));
				case LENGTH: return(normalization.length/1e-4);
#ifndef NDEBUG
				case TYPE: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// MIRROR
		case MIRROR:
			switch(flag_value) {
				case POSITION: return(normalization.length/1e-4);
				case POWER: return(1e3*SIM_q*normalization.energy/normalization.time);
#ifndef NDEBUG
				case NODE_NUMBER:
				case REFLECTIVITY:
				case TYPE: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// DEVICE
		case DEVICE:
#ifndef NDEBUG
			switch(flag_value) {
				case EFFECTS:
				case CURRENT_SOLUTION:
				case CURRENT_STATUS:
				case ERROR_PSI:
				case ERROR_ETA_C:
				case ERROR_ETA_V:
				case ERROR_TEMP:
				case ERROR_MODE:
				case ERROR_PHOTON:
				case INNER_ELECT_ITER:
				case INNER_THERM_ITER:
				case INNER_MODE_ITER:
				case OUTER_OPTIC_ITER:
				case OUTER_THERM_ITER: return(1.0);
				default: assert(FALSE); return(1.0);
			}
#else
			return(1.0);
#endif

// ENVIRONMENT
		case ENVIRONMENT:
			switch(flag_value) {
				case TEMPERATURE: return(normalization.temp);
				case RADIUS:
				case SPEC_START_POSITION:
				case SPEC_END_POSITION: return(normalization.length/1e-4);
#ifndef NDEBUG
				case SPECTRUM_MULTIPLIER:
				case POT_CLAMP_VALUE:
				case TEMP_CLAMP_VALUE:
                case TEMP_RELAX_VALUE:
				case EFFECTS:
				case MAX_ELECTRICAL_ERROR:
				case MAX_THERMAL_ERROR:
				case MAX_OPTIC_ERROR:
				case COARSE_MODE_ERROR:
				case FINE_MODE_ERROR:
				case MAX_INNER_ELECT_ITER:
				case MAX_INNER_THERM_ITER:
				case MAX_OUTER_OPTIC_ITER:
				case MAX_OUTER_THERM_ITER:
				case MAX_INNER_MODE_ITER: return(1.0);
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}

// SPECTRUM
		case SPECTRUM:
			switch(flag_value) {
				case INCIDENT_INPUT_INTENSITY:
				case INCIDENT_EMITTED_INTENSITY:
				case INCIDENT_REFLECT_INTENSITY: return(normalization.intensity*SIM_q/1e-3);
				case INCIDENT_PHOTON_WAVELENGTH: return(normalization.length/1e-4);
				case INCIDENT_PHOTON_ENERGY: return(normalization.energy);
#ifndef NDEBUG
				default: assert(FALSE); return(1.0);
#else
				default: return(1.0);
#endif
			}
		default: assert(FALSE); return(1.0);
	}
}

string shorten_path(string long_path)
{
	size_t start_path, end_path;

	start_path=long_path.find_first_of("\\");
	end_path=long_path.find_last_of("\\");

	if (start_path!=end_path) long_path.replace(start_path,end_path-start_path+1,"\\...\\");
	return(long_path);
}

string prec_to_string(prec value, int precision, NumberFormat format)
{
	string result;
	char number_string[25];
	char format_string[5];

	if (format==NORMAL) sprintf(format_string,"%%.%dlf",precision);
	else sprintf(format_string,"%%.%dle",precision);
	sprintf(number_string,format_string,value);
	result=number_string;
	return(result);
}

string int_to_string(int value)
{
	string result;
	char number_string[25];
	sprintf(number_string,"%d",value);
	result=number_string;
	return(result);
}

string get_short_string(FlagType flag_type, flag flag_value)
{
	extern char **short_string_table[];

	assert(TValueFlag::valid_single_flag(flag_type,flag_value));
	assert(valid_short_string(flag_type,flag_value));
	return(short_string_table[(int)(flag_type-1)][bit_position(flag_value)]);
}

string get_long_string(FlagType flag_type, flag flag_value)
{
	extern char **long_string_table[];

	assert(TValueFlag::valid_single_flag(flag_type,flag_value));
	assert(valid_long_string(flag_type,flag_value));
	return(long_string_table[(int)(flag_type-1)][bit_position(flag_value)]);
}

void long_string_to_flag(string long_string, FlagType& flag_type, flag& flag_value)
{
	int i,new_flag_type=1;
	int max_bit_position;
	flag max_flag, test_flag;
	logical match=FALSE;
	TValueFlag flag_class;
	extern char **long_string_table[];

	flag_type=(FlagType)0;
	flag_value=(flag)0;

	while (!match && new_flag_type<=NUMBER_FLAG_TYPES) {
		max_flag=flag_class.get_max((FlagType)new_flag_type);
		max_bit_position=bit_position(max_flag);
		test_flag=1;
		i=0;
		while (!match && i<=max_bit_position) {
			match=(long_string_table[(int)(new_flag_type-1)][i]==long_string);
			if (match) break;
			i++;
			test_flag<<=1;
		}
		new_flag_type++;
	}

	if (match) {
		flag_type=(FlagType)(new_flag_type-1);
		flag_value=test_flag;
	}
}

void short_string_to_flag(string short_string, FlagType& flag_type, flag& flag_value)
{
	int i,new_flag_type=1;
	int max_bit_position;
	flag max_flag, test_flag;
	logical match=FALSE;
	TValueFlag flag_class;
	extern char **short_string_table[];

	flag_type=(FlagType)0;
	flag_value=(flag)0;

	while (!match && new_flag_type<=NUMBER_FLAG_TYPES) {
		max_flag=flag_class.get_max((FlagType)new_flag_type);
		max_bit_position=bit_position(max_flag);
		test_flag=1;
		i=0;
		while (!match && i<=max_bit_position) {
			match=(short_string_table[(int)(new_flag_type-1)][i]==short_string);
			if (match) break;
			i++;
			test_flag<<=1;
		}
		new_flag_type++;
	}

	if (match) {
		flag_type=(FlagType)(new_flag_type-1);
		flag_value=test_flag;
	}
}

int material_string_to_value(string material_string)
{
	int i=0;
	logical match=FALSE;
	extern char *material_parameters_strings[];

	while (!match && i<MAT_MAX_NUMBER_PARAMETERS) {
		match=(material_parameters_strings[i]==material_string);
		i++;
	}
	if (match) return(i);
	else return(0);
}

string material_value_to_string(int material_value)
{
	extern char *material_parameters_strings[];

	assert((material_value>=1) && (material_value<=MAT_MAX_NUMBER_PARAMETERS));
	return(material_parameters_strings[material_value-1]);
}

string get_short_location_string(FlagType flag_type, int object_number)
{
	string result_string;

	switch(flag_type) {
		case FREE_ELECTRON:
		case FREE_HOLE:
		case BOUND_ELECTRON:
		case BOUND_HOLE:
		case ELECTRON:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE:
			result_string=" at Pos "+prec_to_string(environment.get_value(GRID_ELECTRICAL,POSITION,object_number),5,NORMAL);
			break;
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			result_string=" at QW "+int_to_string(object_number);
			break;
		case CONTACT:
			if (object_number==0) result_string=" at Lt Cont";
			else result_string=" at Rt Cont";
			break;
		case SURFACE:
			if (object_number==0) result_string=" at Lt Surf";
			else result_string=" at Rt Surf";
			break;
		case MIRROR:
			if (object_number==0) result_string=" at Lt Mir";
			else result_string=" at Rt Mir";
			break;
		case MODE:
			result_string=" for Mode 0";
			break;
		default: assert(FALSE);
	}
	return(result_string);
}

string get_long_location_string(FlagType flag_type,int object_number)
{
	string result_string;

	switch(flag_type) {
		case FREE_ELECTRON:
		case FREE_HOLE:
		case BOUND_ELECTRON:
		case BOUND_HOLE:
		case ELECTRON:
		case HOLE:
		case GRID_ELECTRICAL:
		case GRID_OPTICAL:
		case NODE:
			result_string=" at Position "+prec_to_string(environment.get_value(GRID_ELECTRICAL,POSITION,object_number),
														 5,NORMAL);
			break;
		case QW_ELECTRON:
		case QW_HOLE:
		case QUANTUM_WELL:
			result_string=" at QW "+int_to_string(object_number);
			break;
		case CONTACT:
			if (object_number==0) result_string=" at Left Contact";
			else result_string=" at Right Contact";
			break;
		case SURFACE:
			if (object_number==0) result_string=" at Left Surface";
			else result_string=" at Right Surface";
			break;
		case MIRROR:
			if (object_number==0) result_string=" at Left Mirror";
			else result_string=" at Right Mirror";
			break;
		case MODE:
			result_string=" for Mode 0";
			break;
		default: assert(FALSE);
	}
	return(result_string);
}


string get_region_string(RegionType region)
{
	switch(region) {
		case BULK: return("Bulk");
		case QW: return("QW");
		default: assert(FALSE); return("");
	}
}

string get_mirror_string(MirrorType mirror)
{
	switch(mirror) {
		case METAL_MIRROR: return("Metal");
		case DBR_MIRROR: return("DBR");
		default: assert(FALSE); return("");
	}
}

string get_cavity_string(CavityType cavity)
{
	switch(cavity) {
		case EDGE_CAVITY: return("Edge");
		case SURFACE_CAVITY: return("Surface");
		default: assert(FALSE); return("");
	}
}

string get_solve_string(SolveType solve)
{
	switch(solve) {
		case CHARGE_NEUTRAL: return("Charge Neutral");
		case EQUILIBRIUM: return("Equilibrium");
		case STEADY_STATE: return("Steady State");
		case TRANSIENT: return("Transient");
		default: assert(FALSE); return("");
	}
}

string get_status_string(StatusType status)
{
	switch(status) {
		case NO_DEVICE: return("No Device");
		case SIMULATE: return("Must Simulate");
		case CONVERGED: return("Converged");
		case NOT_CONVERGED: return("Not Converged");
		default: assert(FALSE); return("");
	}
}

//**************************************** Debug Functions *************************************

#ifndef NDEBUG

logical valid_short_string(FlagType flag_type, flag flag_value)
{
	extern char **short_string_table[];
	logical result=TRUE;
	int bit;

	bit=bit_position(flag_value);

	result&=(string(short_string_table[(int)(flag_type-1)][bit])=="") ? FALSE : TRUE;
	result&=(string(short_string_table[(int)(flag_type-1)][bit]).contains(",")) ? FALSE : TRUE;
	return(result);
}

logical valid_long_string(FlagType flag_type, flag flag_value)
{
	extern char **long_string_table[];
	logical result=TRUE;

	result&=(string(long_string_table[(int)(flag_type-1)][bit_position(flag_value)])=="") ? FALSE : TRUE;
	return(result);
}

void valid_string_table(void)
{
	int i,j,max_bit;
	flag test_flag;

	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++) {
		test_flag=1;
		max_bit=bit_position(TValueFlag::get_max((FlagType)i));
		for (j=0;j<=max_bit;j++) {
			if (TValueFlag::valid_write_flag((FlagType)i,test_flag)) {
				if (!valid_long_string((FlagType)i,test_flag))
					assert(FALSE);
				if (!valid_short_string((FlagType)i,test_flag))
					assert(FALSE);
			}
			test_flag<<=1;
		}
	}
}

#endif


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

//******************************** Global Function and macros **********************************

#define sq(x) ((x)*(x))
#define round(x) ((int)((((double)(x)-floor(x)) <= 0.5) ? floor(x) : ceil(x)))

int _matherr(exception *new_error);
void convert_mantissa_exp(float& mantissa, int& exponent);
prec bernoulli(prec x);
prec deriv_bernoulli(prec x);
prec fermi(prec x,prec degeneracy=1.0);
prec deriv_fermi(prec x,prec degeneracy=1.0);
prec fermi_integral_minus_2_half(prec x);
prec fermi_integral_minus_1_half(prec x);
prec fermi_integral_0_half(prec x);
prec fermi_integral_1_half(prec x);
prec fermi_integral_2_half(prec x);
prec fermi_integral_3_half(prec x);
prec fermi_integral_4_half(prec x);
prec fermi_integral_5_half(prec x);
prec fermi_integral_6_half(prec x);
prec fermi_integral_8_half(prec x);
prec incomp_gamma(prec x);
prec log_1_x(prec x);
prec log_1_div_1_x(prec x);
prec dilog(prec x);
prec trilog(prec x);
double rnd(void);
void rnd_init(void);
void scale(float *data, int points, float& minimum, float& maximum);
void scale_with_skip(float *data, logical *skip, int points, float& minimum, float& maximum);
void round_scale(float& minimum, float& maximum);
void autotic(float axis_len, float& maj_tic, float& min_tic);
void swap(int& value_1, int& value_2);
void swap(float& value_1, float& value_2);
int bit_position(flag flag_value);
int bit_count(flag flag_value);
prec get_normalize_value(FlagType flag_type, flag flag_value);
string shorten_path(string long_path);
string prec_to_string(prec value, int precision, NumberFormat format=NORMAL);
string int_to_string(int value);
string get_short_string(FlagType flag_type, flag flag_value);
string get_long_string(FlagType flag_type, flag flag_value);
void long_string_to_flag(string long_string, FlagType& flag_type, flag& flag_value);
void short_string_to_flag(string short_string, FlagType& flag_type, flag& flag_value);
int material_string_to_value(string material_string);
string material_value_to_string(int material_value);
string get_short_location_string(FlagType flag_type, int object_number);
string get_long_location_string(FlagType flag_type, int object_number);
string get_region_string(RegionType region);
string get_mirror_string(MirrorType mirror);
string get_cavity_string(CavityType cavity);
string get_solve_string(SolveType solve);
string get_status_string(StatusType status);

//*************************************** Debug Functions **************************************

#ifndef NDEBUG

logical valid_short_string(FlagType flag_type, flag flag_value);
logical valid_long_string(FlagType flag_type, flag flag_value);
void valid_string_table(void);

#endif




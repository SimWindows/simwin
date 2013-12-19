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
#include "simfrcar.h"
#include "simbdcar.h"
#include "simcar.h"
#include "sim2dcar.h"
#include "simqw.h"
#include "simgrid.h"
#include "simnode.h"

/******************************************* class TGrid **************************************

class TGrid {
protected:
	int node_number;
	prec position;
	RegionType region_type;
	NodeType node_type;
	flag effects;
	TQuantumWell *qw_ptr;
	MaterialSpecification material;
	prec radius;
	prec lattice_temp;
	prec stored_lattice_temp;
	prec electron_affinity;
	prec permitivity;
	prec thermal_conduct;
	prec deriv_therm_cond_temp;
	prec lateral_thermal_conduct;
	prec deriv_lateral_cond_temp;
	prec b_b_recomb_const;
	prec band_gap;
	prec potential;
	prec field;
	prec incident_photon_energy;
	ComplexRefractiveIndex incident_refractive_index;
	complex incident_impedance;
	OpticalField incident_field;
	prec mode_photon_energy;
	ComplexRefractiveIndex mode_refractive_index;
	complex mode_impedance;
	OpticalField mode_field;
	prec total_mode_photons;
	prec group_velocity;

public:
	TGrid(int node_num, RegionType region, TQuantumWell *qw);

	void comp_b_b_recomb_const(void);
	void comp_band_gap(void);
	void comp_deriv_thermal_conduct(void);
	void comp_deriv_lateral_conduct(void);
	void comp_electron_affinity(void);
	void comp_field(int start_node_number, prec total_charge);
	void comp_incident_optical_field(int start_node_number);
	void comp_incident_refractive_index(void);
	void comp_incident_impedance(void);
	void comp_incident_total_poynting(prec intensity_multiplier, int max_overflow_count);
	void comp_mode_optical_field(int start_node_number);
	void comp_mode_refractive_index(void);
	void comp_mode_impedance(void);
	void comp_permitivity(void);
	void comp_thermal_conductivity(void);
	void comp_lateral_thermal_conduct(void);
	void store_temperature(void) { stored_lattice_temp=lattice_temp; }
	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);
	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TGrid::TGrid(int node_num, RegionType region, TQuantumWell *qw)
{
	node_number=node_num;
	position=0.0;
	region_type=region;
	if (qw) node_type=QW;
	else node_type=BULK;
	effects=0;
	qw_ptr=qw;
	material.material_type=(MaterialType)0;
	material.alloy_type=(AlloyType)0;
	material.alloy_conc=0.0;
	radius=0.0;
	lattice_temp=0.0;
	stored_lattice_temp=0.0;
	electron_affinity=0.0;
	permitivity=0.0;
	thermal_conduct=0.0;
	deriv_therm_cond_temp=0.0;
	lateral_thermal_conduct=0.0;
	deriv_lateral_cond_temp=0.0;
	band_gap=0.0;
	potential=0.0;
	field=0.0;
	b_b_recomb_const=0.0;
	incident_photon_energy=0.0;
	incident_refractive_index.real_part=incident_refractive_index.absorption=incident_refractive_index.local_gain=0.0;
	incident_impedance=complex(0,0);
	incident_field.forward_field=incident_field.reverse_field=complex(0,0);
	incident_field.forward_poynting=incident_field.reverse_poynting=0.0;
	incident_field.total_poynting=incident_field.total_magnitude=0.0;
    incident_field.overflow_count=0;
	mode_photon_energy=0.0;
	mode_refractive_index.real_part=mode_refractive_index.absorption=mode_refractive_index.local_gain=0.0;
	mode_impedance=complex(0,0);
	mode_field.forward_field=mode_field.reverse_field=complex(0,0);
	mode_field.forward_poynting=mode_field.reverse_poynting=0.0;
	mode_field.total_poynting=mode_field.total_magnitude=0.0;
    mode_field.overflow_count=0;
	total_mode_photons=0.0;
	group_velocity=0.0;
}

void TGrid::comp_b_b_recomb_const(void)
{
	prec values[]={ material.alloy_conc,position*normalization.length/1e-4 };

	b_b_recomb_const=material_parameters.evaluate(MAT_B_B_RECOMB_CONSTANT, material.material_type,
												  material.alloy_type,values)*normalization.conc*normalization.time;
}

void TGrid::comp_band_gap(void)
{
	prec evaluation_temp;

	if (effects & GRID_BAND_NARROWING) evaluation_temp=lattice_temp*normalization.temp;
	else evaluation_temp=300.0;

	prec values[]= { material.alloy_conc, evaluation_temp, position*normalization.length/1e-4 };

	band_gap=material_parameters.evaluate(MAT_BAND_GAP,material.material_type,material.alloy_type,
										  values)/normalization.pot;
}

void TGrid::comp_deriv_thermal_conduct(void)
{
	if (effects & GRID_TEMP_THERMAL_COND) {
		prec values[]= { material.alloy_conc, lattice_temp*normalization.temp,
						 position*normalization.length/1e-4 };

		deriv_therm_cond_temp=material_parameters.evaluate(MAT_DERIV_THERMAL_CONDUCT,
														   material.material_type,
														   material.alloy_type,
														   values)*normalization.temp/normalization.therm_cond;
	}
	else deriv_therm_cond_temp=0.0;
}

void TGrid::comp_deriv_lateral_conduct(void)
{
	prec env_radius, env_temp;
	prec new_deriv_therm_cond;

	if (effects & GRID_TEMP_THERMAL_COND) {
		env_radius=environment.get_value(ENVIRONMENT,RADIUS,0,NORMALIZED);
		env_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);

		prec values[]= { material.alloy_conc,
						(lattice_temp+env_temp)*normalization.temp/2.0,
						 position*normalization.length/1e-4 };

		new_deriv_therm_cond=material_parameters.evaluate(MAT_DERIV_THERMAL_CONDUCT,
														  material.material_type,
														  material.alloy_type,
														  values)*normalization.temp/normalization.therm_cond;
		deriv_lateral_cond_temp=2.0*new_deriv_therm_cond/(sq(radius)*log(env_radius/radius));
	}
	else deriv_lateral_cond_temp=0.0;
}

void TGrid::comp_electron_affinity(void)
{
	prec evaluation_temp;

	if (effects & GRID_TEMP_ELECTRON_AFFINITY) evaluation_temp=lattice_temp*normalization.temp;
	else evaluation_temp=300.0;

	prec values[]= { material.alloy_conc, evaluation_temp, position*normalization.length/1e-4 };

	electron_affinity=material_parameters.evaluate(MAT_ELECTRON_AFFINITY,material.material_type,material.alloy_type,
												   values)/normalization.pot;
}

void TGrid::comp_field(int start_node_number, prec total_charge)
{
	static prec prev_position;
	static prec displacement,prev_displacement;
	static prec prev_charge;

	if (node_number==start_node_number) {
		prev_displacement=field*permitivity;
	}
	else {
		displacement=prev_displacement+(total_charge+prev_charge)*
									   (position-prev_position)/2.0;
		field=displacement/permitivity;
		prev_displacement=displacement;
	}
	prev_position=position;
	prev_charge=total_charge;
}

void TGrid::comp_incident_optical_field(int start_node_number)
{
	static complex i(0,1);
	static prec prev_position, wave_vector;
	static complex prev_impedance;
	static OpticalField prev_incident_field;
    static int overflow_count;
    prec real_exponent, imag_exponent;
    complex exponent;
	complex forward_propag_param, reverse_propag_param;
	complex impedance_sum, impedance_diff;
	complex temp_forward_field, temp_reverse_field;

	if (node_number==start_node_number) {
		wave_vector=2.0*SIM_pi/get_value(GRID_OPTICAL,INCIDENT_PHOTON_WAVELENGTH,NORMALIZED);
        overflow_count=0;
	}
	else {
		exponent=wave_vector*(position-prev_position)/prev_impedance;
        real_exponent=real(exponent);
        imag_exponent=exp(imag(exponent));

		forward_propag_param=exp(-i*real_exponent)*imag_exponent;
		reverse_propag_param=exp(i*real_exponent)/imag_exponent;

        if ((fabs(real(prev_incident_field.forward_field))>=1e150/imag_exponent) ||
            (fabs(imag(prev_incident_field.forward_field))>=1e150/imag_exponent)) {
        	incident_field.forward_field=prev_incident_field.forward_field/1e150;
            incident_field.reverse_field=prev_incident_field.reverse_field*1e150;
            overflow_count++;
        }
        else {
	        if ((fabs(real(prev_incident_field.reverse_field))>=1e150*imag_exponent) ||
    	        (fabs(imag(prev_incident_field.reverse_field))>=1e150*imag_exponent)) {
        	    incident_field.forward_field=prev_incident_field.forward_field*1e150;
            	incident_field.reverse_field=prev_incident_field.reverse_field/1e150;
            	overflow_count++;
	        }
    	    else {
        		incident_field.forward_field=prev_incident_field.forward_field;
            	incident_field.reverse_field=prev_incident_field.reverse_field;
            }
        }

		incident_field.forward_field*=forward_propag_param;
		incident_field.reverse_field*=reverse_propag_param;

		if (effects & GRID_INCIDENT_REFLECTION) {
			impedance_sum=prev_impedance+incident_impedance;
			impedance_diff=prev_impedance-incident_impedance;

			temp_forward_field=incident_field.forward_field;
			temp_reverse_field=incident_field.reverse_field;

			incident_field.forward_field=(temp_forward_field*impedance_sum+
										  temp_reverse_field*impedance_diff)/(2.0*prev_impedance);
			incident_field.reverse_field=(temp_forward_field*impedance_diff+
										  temp_reverse_field*impedance_sum)/(2.0*prev_impedance);
		}
	}

	incident_field.total_magnitude=sqrt(norm(incident_field.forward_field+incident_field.reverse_field));

	if (effects & GRID_INCIDENT_REFLECTION) {
		incident_field.forward_poynting=0.5*real(norm(incident_field.forward_field)/conj(incident_impedance));
		incident_field.reverse_poynting=0.5*real(norm(incident_field.reverse_field)/conj(incident_impedance));
	}
	else {
		incident_field.forward_poynting=0.5*norm(incident_field.forward_field);
		incident_field.reverse_poynting=0.5*norm(incident_field.reverse_field);
	}

    incident_field.overflow_count=overflow_count;

	prev_position=position;
	prev_impedance=incident_impedance;
	prev_incident_field.forward_field=incident_field.forward_field;
	prev_incident_field.reverse_field=incident_field.reverse_field;
}

void TGrid::comp_incident_refractive_index(void)
{
	prec equil_band_gap;
	prec band_gap_values[]= { material.alloy_conc, 300.0, position*normalization.length/1e-4 };

	equil_band_gap=material_parameters.evaluate(MAT_BAND_GAP,material.material_type,material.alloy_type,
												band_gap_values);

	prec values[]= { material.alloy_conc,
					 lattice_temp*normalization.temp,
					 incident_photon_energy*normalization.energy,
					 band_gap*normalization.pot,
					 equil_band_gap,
					 position*normalization.length/1e-4 };

//	if (qw_ptr)
//		incident_refractive_index.absorption=qw_ptr->incident_absorption/qw_ptr->qw_length;
//	else
		incident_refractive_index.absorption=material_parameters.evaluate(MAT_ABSORPTION,material.material_type,
																		  material.alloy_type,values)*normalization.length;

	incident_refractive_index.real_part=material_parameters.evaluate(MAT_REFRACTIVE_INDEX,material.material_type,
																	 material.alloy_type,values);
}

void TGrid::comp_incident_impedance(void)
{
	prec wave_vector;

	wave_vector=2.0*SIM_pi/get_value(GRID_OPTICAL,INCIDENT_PHOTON_WAVELENGTH,NORMALIZED);
	complex complex_refractive_index(incident_refractive_index.real_part,
									 -0.5*incident_refractive_index.absorption/wave_vector);
	incident_impedance=1.0/complex_refractive_index;
}

void TGrid::comp_incident_total_poynting(prec intensity_multiplier, int max_overflow_count)
{
	static complex i(0,1.0);

    if (max_overflow_count==incident_field.overflow_count) {
		incident_field.forward_poynting*=intensity_multiplier;
		incident_field.reverse_poynting*=intensity_multiplier;
    }
    else {
    	incident_field.forward_poynting=0.0;
        incident_field.reverse_poynting=0.0;
    }

	if (effects & GRID_INCIDENT_REFLECTION) {
		if (norm(incident_field.forward_field)!=0.0)
			incident_field.forward_field*=sqrt(2.0*incident_field.forward_poynting/
											   (real(1.0/incident_impedance)*norm(incident_field.forward_field)));

		if (norm(incident_field.reverse_field)!=0.0)
			incident_field.reverse_field*=sqrt(2.0*incident_field.reverse_poynting/
											   (real(1.0/incident_impedance)*norm(incident_field.reverse_field)));

		incident_field.total_poynting=incident_field.forward_poynting-incident_field.reverse_poynting+
									  real(i*imag(incident_field.reverse_field*conj(incident_field.forward_field))/
										   conj(incident_impedance));
	}
	else {
		if (norm(incident_field.forward_field)!=0.0)
			incident_field.forward_field*=sqrt(2.0*incident_field.forward_poynting/
											   norm(incident_field.forward_field));

		if (norm(incident_field.reverse_field)!=0.0)
			incident_field.reverse_field*=sqrt(2.0*incident_field.reverse_poynting/
											   norm(incident_field.reverse_field));

		incident_field.total_poynting=incident_field.forward_poynting-incident_field.reverse_poynting+
									  real(i*imag(incident_field.reverse_field*conj(incident_field.forward_field)));
	}
	incident_field.total_magnitude=sqrt(norm(incident_field.forward_field+incident_field.reverse_field));
}

void TGrid::comp_mode_optical_field(int start_node_number)
{
	complex i(0,1);
	static prec prev_position, wave_vector;
	static complex prev_impedance;
	static OpticalField prev_mode_field;
	complex forward_propag_param, reverse_propag_param;
	complex impedance_sum, impedance_diff;
	complex temp_forward_field, temp_reverse_field;

	if (node_number==start_node_number) {
		wave_vector=2.0*SIM_pi/get_value(GRID_OPTICAL,MODE_PHOTON_WAVELENGTH,NORMALIZED);
	}
	else {
		forward_propag_param=exp(-i*wave_vector*(position-prev_position)/prev_impedance);
		reverse_propag_param=exp(i*wave_vector*(position-prev_position)/prev_impedance);

		temp_forward_field=prev_mode_field.forward_field*forward_propag_param;
		temp_reverse_field=prev_mode_field.reverse_field*reverse_propag_param;

		impedance_sum=prev_impedance+mode_impedance;
		impedance_diff=prev_impedance-mode_impedance;

		mode_field.forward_field=(temp_forward_field*impedance_sum+
								  temp_reverse_field*impedance_diff)/(2.0*prev_impedance);
		mode_field.reverse_field=(temp_forward_field*impedance_diff+
								  temp_reverse_field*impedance_sum)/(2.0*prev_impedance);
	}

	mode_field.total_magnitude=sqrt(norm(mode_field.forward_field+mode_field.reverse_field));

	prev_position=position;
	prev_impedance=mode_impedance;
	prev_mode_field.forward_field=mode_field.forward_field;
	prev_mode_field.reverse_field=mode_field.reverse_field;
}

void TGrid::comp_mode_refractive_index(void)
{
	prec equil_band_gap;
	prec band_gap_values[]= { material.alloy_conc, 300.0, position*normalization.length/1e-4 };

	equil_band_gap=material_parameters.evaluate(MAT_BAND_GAP,material.material_type,material.alloy_type,
										        band_gap_values);

	prec values[]= { material.alloy_conc,
					 lattice_temp*normalization.temp,
					 mode_photon_energy*normalization.energy,
					 band_gap*normalization.pot,
					 equil_band_gap,
					 position*normalization.length/1e-4 };

	if (qw_ptr)
		mode_refractive_index.absorption=qw_ptr->mode_absorption/qw_ptr->qw_length;
	else
		mode_refractive_index.absorption=material_parameters.evaluate(MAT_ABSORPTION,material.material_type,
																	 material.alloy_type,values)*normalization.length;

	mode_refractive_index.real_part=material_parameters.evaluate(MAT_REFRACTIVE_INDEX,material.material_type,
															material.alloy_type,values);
}

void TGrid::comp_mode_impedance(void)
{
	prec wave_vector;

	wave_vector=2.0*SIM_pi/get_value(GRID_OPTICAL,MODE_PHOTON_WAVELENGTH,NORMALIZED);
	complex complex_refractive_index(mode_refractive_index.real_part,
									 +0.5*mode_refractive_index.local_gain/wave_vector);
	mode_impedance=1.0/complex_refractive_index;
}

void TGrid::comp_permitivity(void)
{
	prec values[]= { material.alloy_conc, position*normalization.length/1e-4 };

	permitivity=material_parameters.evaluate(MAT_PERMITIVITY,material.material_type,material.alloy_type,values);
}

void TGrid::comp_thermal_conductivity(void)
{
	prec evaluation_temp;

	if (effects & GRID_TEMP_THERMAL_COND) evaluation_temp=lattice_temp*normalization.temp;
	else evaluation_temp=300.0;

	prec values[]= { material.alloy_conc, evaluation_temp, position*normalization.length/1e-4 };

	thermal_conduct=material_parameters.evaluate(MAT_THERMAL_CONDUCTIVITY,material.material_type,material.alloy_type,
												 values)/normalization.therm_cond;
}

void TGrid::comp_lateral_thermal_conduct(void)
{
	prec env_radius, env_temp,new_conduct;
	prec evaluation_temp;

	env_radius=environment.get_value(ENVIRONMENT,RADIUS,0,NORMALIZED);

	if (effects & GRID_TEMP_THERMAL_COND) {
		env_temp=environment.get_value(ENVIRONMENT,TEMPERATURE,0,NORMALIZED);
		evaluation_temp=(lattice_temp+env_temp)*normalization.temp/2.0;
	}
	else evaluation_temp=300.0;

	prec values[]= { material.alloy_conc, evaluation_temp, position*normalization.length/1e-4 };

	new_conduct=material_parameters.evaluate(MAT_THERMAL_CONDUCTIVITY,material.material_type,material.alloy_type,
											 values)/normalization.therm_cond;

	lateral_thermal_conduct=2.0*new_conduct/(sq(radius)*log(env_radius/radius));
}

prec TGrid::get_value(FlagType flag_type, flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case RADIUS: return_value=radius; break;
				case TEMPERATURE: return_value=lattice_temp; break;
				case POSITION: return_value=position; break;
				case REGION_TYPE: return_value=(prec) region_type; break;
				case NODE_TYPE: return_value=(prec) node_type; break;
				case MATERIAL: return_value=(prec) material.material_type; break;
				case ELECTRON_AFFINITY: return_value=electron_affinity; break;
				case PERMITIVITY: return_value=(prec)permitivity; break;
				case THERMAL_CONDUCT: return_value=thermal_conduct; break;
				case LATERAL_THERMAL_CONDUCT: return_value=lateral_thermal_conduct; break;
				case BAND_GAP: return_value=band_gap; break;
				case POTENTIAL: return_value=potential; break;
				case FIELD: return_value=(prec)field; break;
				case B_B_RECOMB_CONSTANT: return_value=b_b_recomb_const; break;
				case ALLOY_TYPE: return_value=(prec)material.alloy_type; break;
				case ALLOY_CONC: return_value=(prec)material.alloy_conc; break;
				case NODE_NUMBER: return_value=(prec) node_number; break;
				case VACUUM_LEVEL: return_value=-potential; break;
				default: assert(FALSE); break;
			}
			break;
		case GRID_OPTICAL:
			switch(flag_value) {
            	case INCIDENT_OVERFLOW: return_value=(prec)incident_field.overflow_count; break;
				case INCIDENT_IMPEDANCE_REAL: return_value=real(incident_impedance); break;
				case INCIDENT_IMPEDANCE_IMAG: return_value=imag(incident_impedance); break;
				case MODE_IMPEDANCE_REAL: return_value=real(mode_impedance); break;
				case MODE_IMPEDANCE_IMAG: return_value=imag(mode_impedance); break;
				case INCIDENT_ABSORPTION: return_value=incident_refractive_index.absorption; break;
				case MODE_ABSORPTION: return_value=mode_refractive_index.absorption; break;
				case MODE_GAIN: return_value=mode_refractive_index.local_gain; break;
				case INCIDENT_PHOTON_ENERGY: return_value=incident_photon_energy; break;
				case INCIDENT_PHOTON_WAVELENGTH: return_value=(1.242/(incident_photon_energy*normalization.energy))*
															  1e-4/normalization.length; break;
				case INCIDENT_TOTAL_POYNTING: return_value=incident_field.total_poynting; break;
				case MODE_TOTAL_POYNTING: return_value=mode_field.total_poynting; break;
				case MODE_TOTAL_PHOTONS: return_value=total_mode_photons; break;
				case MODE_PHOTON_ENERGY: return_value=mode_photon_energy; break;
				case MODE_PHOTON_WAVELENGTH: return_value=(1.242/(mode_photon_energy*normalization.energy))*
														  1e-4/normalization.length; break;
				case MODE_GROUP_VELOCITY: return_value=group_velocity; break;
				case MODE_PHOTON_DENSITY: return_value=total_mode_photons*sq(mode_field.total_magnitude); break;
				case INCIDENT_REFRACTIVE_INDEX: return_value=incident_refractive_index.real_part; break;
				case MODE_REFRACTIVE_INDEX: return_value=mode_refractive_index.real_part; break;
				case MODE_FORWARD_FIELD_REAL: return_value=real(mode_field.forward_field); break;
				case MODE_FORWARD_FIELD_IMAG: return_value=imag(mode_field.forward_field); break;
				case MODE_REVERSE_FIELD_REAL: return_value=real(mode_field.reverse_field); break;
				case MODE_REVERSE_FIELD_IMAG: return_value=imag(mode_field.reverse_field); break;
				case MODE_TOTAL_FIELD_MAG: return_value=mode_field.total_magnitude; break;
				case INCIDENT_FORWARD_FIELD_REAL: return_value=real(incident_field.forward_field); break;
				case INCIDENT_FORWARD_FIELD_IMAG: return_value=imag(incident_field.forward_field); break;
				case INCIDENT_FORWARD_POYNTING: return_value=incident_field.forward_poynting; break;
				case INCIDENT_REVERSE_FIELD_REAL: return_value=real(incident_field.reverse_field); break;
				case INCIDENT_REVERSE_FIELD_IMAG: return_value=imag(incident_field.reverse_field); break;
				case INCIDENT_REVERSE_POYNTING: return_value=incident_field.reverse_poynting; break;
				case INCIDENT_TOTAL_FIELD_MAG: return_value=incident_field.total_magnitude; break;
				default: assert(FALSE); break;
			}
			break;
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(flag_type, flag_value);
	return(return_value);
}

void TGrid::put_value(FlagType flag_type, flag flag_value, prec value,
					  ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(flag_type, flag_value);

	switch(flag_type) {
		case GRID_ELECTRICAL:
			switch(flag_value) {
				case RADIUS: radius=value; return;
				case TEMPERATURE: lattice_temp=value; return;
				case POSITION: position=value; return;
				case REGION_TYPE: region_type=(RegionType) value; return;
				case NODE_TYPE: node_type=(NodeType) value; return;
				case MATERIAL: material.material_type=(MaterialType) value; return;
				case ELECTRON_AFFINITY: electron_affinity=value; return;
				case PERMITIVITY: permitivity=value; return;
				case THERMAL_CONDUCT: thermal_conduct=value; return;
				case LATERAL_THERMAL_CONDUCT: lateral_thermal_conduct=value; return;
				case BAND_GAP: band_gap=value; return;
				case POTENTIAL: potential=value; return;
				case FIELD: field=value; return;
				case B_B_RECOMB_CONSTANT: b_b_recomb_const=value; return;
				case ALLOY_TYPE: material.alloy_type=(AlloyType) value; return;
				case ALLOY_CONC: material.alloy_conc=value; return;
				case NODE_NUMBER: node_number=(int) value; return;
				case VACUUM_LEVEL: potential=-value; return;
				default: assert(FALSE); return;
			}
		case GRID_OPTICAL:
			switch(flag_value) {
            	case INCIDENT_OVERFLOW: incident_field.overflow_count=(int)value; return;
				case INCIDENT_IMPEDANCE_REAL:
					incident_impedance=complex(value,imag(incident_impedance));
					return;
				case INCIDENT_IMPEDANCE_IMAG:
					incident_impedance=complex(real(incident_impedance),value);
					return;
				case MODE_IMPEDANCE_REAL:
					mode_impedance=complex(value,imag(mode_impedance));
					return;
				case MODE_IMPEDANCE_IMAG:
					mode_impedance=complex(real(mode_impedance),value);
					return;
				case INCIDENT_ABSORPTION: incident_refractive_index.absorption=value; return;
				case MODE_ABSORPTION: mode_refractive_index.absorption=value; return;
				case MODE_GAIN: mode_refractive_index.local_gain=value; return;
				case INCIDENT_PHOTON_ENERGY: incident_photon_energy=value; return;
				case INCIDENT_PHOTON_WAVELENGTH: incident_photon_energy=(1.242/(value*normalization.length*1e4))/
																		normalization.energy; return;
				case INCIDENT_TOTAL_POYNTING: incident_field.total_poynting=value; return;
				case MODE_TOTAL_POYNTING: mode_field.total_poynting=value; return;
				case MODE_TOTAL_PHOTONS: total_mode_photons=value; return;
				case MODE_PHOTON_ENERGY: mode_photon_energy=value; return;
				case MODE_PHOTON_WAVELENGTH: mode_photon_energy=(1.242/(value*normalization.length*1e4))/
																normalization.energy; return;
				case MODE_GROUP_VELOCITY: group_velocity=value; return;
				case INCIDENT_REFRACTIVE_INDEX: incident_refractive_index.real_part=value; return;
				case MODE_REFRACTIVE_INDEX: mode_refractive_index.real_part=value; return;
				case MODE_FORWARD_FIELD_REAL: mode_field.forward_field=complex(value,imag(mode_field.forward_field)); return;
				case MODE_FORWARD_FIELD_IMAG: mode_field.forward_field=complex(real(mode_field.forward_field),value); return;
				case MODE_REVERSE_FIELD_REAL: mode_field.reverse_field=complex(value,imag(mode_field.reverse_field)); return;
				case MODE_REVERSE_FIELD_IMAG: mode_field.reverse_field=complex(real(mode_field.reverse_field),value); return;
				case MODE_TOTAL_FIELD_MAG: mode_field.total_magnitude=value; return;
				case INCIDENT_FORWARD_FIELD_REAL:
					incident_field.forward_field=complex(value,imag(incident_field.forward_field));
					return;
				case INCIDENT_FORWARD_FIELD_IMAG:
					incident_field.forward_field=complex(real(incident_field.forward_field),value);
					return;
				case INCIDENT_FORWARD_POYNTING: incident_field.forward_poynting=value; return;
				case INCIDENT_REVERSE_FIELD_REAL:
					incident_field.reverse_field=complex(value,imag(incident_field.reverse_field));
					return;
				case INCIDENT_REVERSE_FIELD_IMAG:
					incident_field.reverse_field=complex(real(incident_field.reverse_field),value);
					return;
				case INCIDENT_REVERSE_POYNTING: incident_field.reverse_poynting=value; return;
				case INCIDENT_TOTAL_FIELD_MAG: incident_field.total_magnitude=value; return;
				default: assert(FALSE); return;
			}
	}
}

void TGrid::read_state_file(FILE *file_ptr)
{
	fread(&position,sizeof(position),1,file_ptr);
	fread(&radius,sizeof(radius),1,file_ptr);
	fread(&effects,sizeof(effects),1,file_ptr);
	fread(&material,sizeof(material),1,file_ptr);
	fread(&lattice_temp,sizeof(lattice_temp),1,file_ptr);
	fread(&electron_affinity,sizeof(electron_affinity),1,file_ptr);
	fread(&permitivity,sizeof(permitivity),1,file_ptr);
	fread(&thermal_conduct,sizeof(thermal_conduct),1,file_ptr);
	fread(&lateral_thermal_conduct,sizeof(lateral_thermal_conduct),1,file_ptr);
	fread(&band_gap,sizeof(band_gap),1,file_ptr);
	fread(&potential,sizeof(potential),1,file_ptr);
	fread(&field,sizeof(field),1,file_ptr);
	fread(&b_b_recomb_const,sizeof(b_b_recomb_const),1,file_ptr);

	fread(&incident_photon_energy,sizeof(incident_photon_energy),1,file_ptr);
	fread(&incident_refractive_index,sizeof(incident_refractive_index),1,file_ptr);
	fread(&incident_impedance,sizeof(incident_impedance),1,file_ptr);
	fread(&incident_field,sizeof(incident_field),1,file_ptr);

	fread(&mode_photon_energy,sizeof(mode_photon_energy),1,file_ptr);
	fread(&mode_refractive_index,sizeof(mode_refractive_index),1,file_ptr);
	fread(&mode_impedance,sizeof(mode_impedance),1,file_ptr);
	fread(&mode_field,sizeof(mode_field),1,file_ptr);
	fread(&total_mode_photons,sizeof(total_mode_photons),1,file_ptr);
	fread(&group_velocity,sizeof(group_velocity),1,file_ptr);
}

void TGrid::write_state_file(FILE *file_ptr)
{
	fwrite(&position,sizeof(position),1,file_ptr);
	fwrite(&radius,sizeof(radius),1,file_ptr);
	fwrite(&effects,sizeof(effects),1,file_ptr);
	fwrite(&material,sizeof(material),1,file_ptr);
	fwrite(&lattice_temp,sizeof(lattice_temp),1,file_ptr);
	fwrite(&electron_affinity,sizeof(electron_affinity),1,file_ptr);
	fwrite(&permitivity,sizeof(permitivity),1,file_ptr);
	fwrite(&thermal_conduct,sizeof(thermal_conduct),1,file_ptr);
	fwrite(&lateral_thermal_conduct,sizeof(lateral_thermal_conduct),1,file_ptr);
	fwrite(&band_gap,sizeof(band_gap),1,file_ptr);
	fwrite(&potential,sizeof(potential),1,file_ptr);
	fwrite(&field,sizeof(field),1,file_ptr);
	fwrite(&b_b_recomb_const,sizeof(b_b_recomb_const),1,file_ptr);

	fwrite(&incident_photon_energy,sizeof(incident_photon_energy),1,file_ptr);
	fwrite(&incident_refractive_index,sizeof(incident_refractive_index),1,file_ptr);
	fwrite(&incident_impedance,sizeof(incident_impedance),1,file_ptr);
	fwrite(&incident_field,sizeof(incident_field),1,file_ptr);

	fwrite(&mode_photon_energy,sizeof(mode_photon_energy),1,file_ptr);
	fwrite(&mode_refractive_index,sizeof(mode_refractive_index),1,file_ptr);
	fwrite(&mode_impedance,sizeof(mode_impedance),1,file_ptr);
	fwrite(&mode_field,sizeof(mode_field),1,file_ptr);
	fwrite(&total_mode_photons,sizeof(total_mode_photons),1,file_ptr);
	fwrite(&group_velocity,sizeof(group_velocity),1,file_ptr);
}

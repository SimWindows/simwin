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
#include "simgrid.h"
#include "simnode.h"
#include "sim2dcar.h"

/*********************************T2DElectron Class********************************************

class T2DElectron {
	friend TBoundElectron;
protected:
	TNode** grid_ptr;
	QuantumWellNodes nodes;
	flag qw_effects;
	prec qw_length;
	prec energy_level;
	prec equil_dos;
	prec non_equil_dos;
	prec equil_planck_potential;
	prec conc;
	prec deriv_conc_eta_c;
	prec doping_conc;
	prec qw_energy_top;
	prec stimulated_factor;
    prec auger_coefficient;

public:
	T2DElectron(TNode** grid);
private:
	prec comp_dos(prec dos_mass, prec temp);
public:
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);
	void comp_equil_planck_potential(prec hole_equil_dos, prec hole_energy_level,
									 prec acceptor_conc);
	void comp_equil_planck_potential(prec hole_equil_planck_pot);
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_doping_conc(void);
	void comp_eigenvalues(void);
	void comp_wavefunctions(prec start_position);
	void comp_qw_top(void);
	void comp_stimulated_factor(prec band_gap);
    void comp_auger_coefficient(void);
    void comp_auger_hotcarriers(void);
    void comp_b_b_hotcarriers(void);
    void comp_shr_hotcarriers(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

T2DElectron::T2DElectron(TNode** grid)
{
	grid_ptr=grid;
	qw_effects=0;
	nodes.prev_node_ptr=(TNode*)0;
	nodes.curr_node_ptr=(TNode*)0;
	nodes.next_node_ptr=(TNode*)0;
	nodes.prev_node=0.0;
	nodes.curr_node=0.0;
	nodes.next_node=0.0;
	qw_length=0.0;
	energy_level=0.0;
	equil_dos=0.0;
	non_equil_dos=0.0;
	equil_planck_potential=0.0;
	conc=0.0;
	deriv_conc_eta_c=0.0;
	doping_conc=0.0;
	qw_energy_top=0.0;
	stimulated_factor=0.0;
    auger_coefficient=0.0;
}

prec T2DElectron::comp_dos(prec dos_mass, prec temp)
{
	return((dos_mass*SIM_mo*SIM_k*temp*normalization.temp/(SIM_pi*sq(SIM_hb)))/
		   (1e4*normalization.conc*normalization.length));
}

void T2DElectron::comp_equil_dos(void)
{
	prec dos_mass,temp;

	dos_mass=nodes.curr_node_ptr->get_value(ELECTRON,DOS_MASS,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);

	equil_dos=comp_dos(dos_mass,temp);
}

void T2DElectron::comp_non_equil_dos(void)
{
	prec dos_mass,temp;

	dos_mass=nodes.curr_node_ptr->get_value(ELECTRON,DOS_MASS,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(ELECTRON,TEMPERATURE,NORMALIZED);

	non_equil_dos=comp_dos(dos_mass,temp);
}

void T2DElectron::comp_equil_planck_potential(prec hole_equil_dos, prec hole_energy_level,
											  prec acceptor_conc)
{
	int i=0;
	prec iteration_error;
	prec new_planck_potential,update_planck_potential;
	prec conc_difference, deriv_conc_difference;
	prec doping_difference, deriv_doping_difference;
	prec conc_function, deriv_conc_function;
	long acceptor_degeneracy,donor_degeneracy;
	prec acceptor_level,donor_level;
	prec bulk_band_gap;
	prec lattice_temp;
	flag effects;
	static prec old_donor_conc=0.0, old_acceptor_conc=0.0;
	static prec old_electron_equil_dos=0.0, old_hole_equil_dos=0.0;
	static long old_donor_degeneracy=0, old_acceptor_degeneracy=0;
	static prec old_donor_level=0.0, old_acceptor_level=0.0;
	static prec old_electron_energy_level=0.0, old_hole_energy_level=0.0;
	static prec old_bulk_band_gap=0.0, old_lattice_temp=0.0;
	static logical old_fermi_method=FALSE, old_ionized_doping_method=FALSE;
	static prec old_result=0.0;
	logical same_values=FALSE;

	acceptor_degeneracy=nodes.curr_node_ptr->get_value(HOLE,DOPING_DEGENERACY,NORMALIZED);
	donor_degeneracy=nodes.curr_node_ptr->get_value(ELECTRON,DOPING_DEGENERACY,NORMALIZED);
	acceptor_level=nodes.curr_node_ptr->get_value(HOLE,DOPING_LEVEL,NORMALIZED);
	donor_level=nodes.curr_node_ptr->get_value(ELECTRON,DOPING_LEVEL,NORMALIZED);
	bulk_band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);
	lattice_temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);
	effects=(flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);

	if ((doping_conc==old_donor_conc) && (acceptor_conc==old_acceptor_conc) &&
		(equil_dos==old_electron_equil_dos) && (hole_equil_dos==old_hole_equil_dos) &&
		(donor_degeneracy==old_donor_degeneracy) && (acceptor_degeneracy==old_acceptor_degeneracy) &&
		(donor_level==old_donor_level) && (acceptor_level==old_acceptor_level) &&
		(energy_level==old_electron_energy_level) && (hole_energy_level==old_hole_energy_level) &&
		(bulk_band_gap==old_bulk_band_gap) && (lattice_temp==old_lattice_temp)) same_values=TRUE;

	if ( same_values &&
		 (((effects & GRID_FERMI_DIRAC)!=0)==old_fermi_method) &&
		 (((effects & GRID_INCOMPLETE_IONIZATION)!=0)==old_ionized_doping_method) )
		 equil_planck_potential=old_result;
	else {
		new_planck_potential=equil_planck_potential;

		do {
			conc_difference=equil_dos*fermi_integral_0_half(new_planck_potential-energy_level/lattice_temp)-
							hole_equil_dos*fermi_integral_0_half(-new_planck_potential-
																  bulk_band_gap/lattice_temp-
																  hole_energy_level/lattice_temp);

			deriv_conc_difference=equil_dos*fermi_integral_minus_2_half(new_planck_potential-energy_level/lattice_temp)+
								  hole_equil_dos*fermi_integral_minus_2_half(-new_planck_potential-
																			  bulk_band_gap/lattice_temp-
																			  hole_energy_level/lattice_temp);

			if (effects & GRID_INCOMPLETE_IONIZATION) {
				doping_difference=
					acceptor_conc*fermi(acceptor_level/lattice_temp-new_planck_potential-bulk_band_gap/lattice_temp,
								  (prec)acceptor_degeneracy)-
					doping_conc*fermi(donor_level/lattice_temp+new_planck_potential,(prec)donor_degeneracy);

				deriv_doping_difference=
					-acceptor_conc*deriv_fermi(acceptor_level/lattice_temp-new_planck_potential-bulk_band_gap/lattice_temp,
								  (prec)acceptor_degeneracy)-
					doping_conc*deriv_fermi(donor_level/lattice_temp+new_planck_potential,(prec)donor_degeneracy);
			}
			else {
				doping_difference=acceptor_conc-doping_conc;
				deriv_doping_difference=0.0;
			}

			conc_function=conc_difference+doping_difference;
			deriv_conc_function=deriv_conc_difference+deriv_doping_difference;

			update_planck_potential=conc_function/deriv_conc_function;
			if (new_planck_potential!=0.0) iteration_error=fabs(update_planck_potential/new_planck_potential);
			else iteration_error=1.0;
			new_planck_potential-=update_planck_potential;
			i++;
		}
		while ((i<51) && (iteration_error>=1e-8));

		equil_planck_potential=new_planck_potential;

		old_donor_conc=doping_conc;
		old_acceptor_conc=acceptor_conc;
		old_electron_equil_dos=equil_dos;
		old_hole_equil_dos=hole_equil_dos;
		old_donor_degeneracy=donor_degeneracy;
		old_acceptor_degeneracy=acceptor_degeneracy;
		old_donor_level=donor_level;
		old_acceptor_level=acceptor_level;
		old_electron_energy_level=energy_level;
		old_hole_energy_level=hole_energy_level;
		old_bulk_band_gap=bulk_band_gap;
		old_lattice_temp=lattice_temp;

		old_fermi_method=((effects & GRID_FERMI_DIRAC)!=0);
		old_ionized_doping_method=((effects & GRID_INCOMPLETE_IONIZATION)!=0);

		old_result=equil_planck_potential;
	}
}

void T2DElectron::comp_equil_planck_potential(prec hole_equil_planck_pot)
{
	prec band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);
	prec lattice_temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);

	equil_planck_potential=-hole_equil_planck_pot-band_gap/lattice_temp;
}

void T2DElectron::comp_conc(void)
{
	prec planck_pot,temp;

	planck_pot=nodes.curr_node_ptr->get_value(ELECTRON,PLANCK_POT,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(ELECTRON,TEMPERATURE,NORMALIZED);

	conc=non_equil_dos*fermi_integral_0_half(planck_pot-energy_level/temp);
}

void T2DElectron::comp_deriv_conc(void)
{
	prec planck_pot,temp;

	planck_pot=nodes.curr_node_ptr->get_value(ELECTRON,PLANCK_POT,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(ELECTRON,TEMPERATURE,NORMALIZED);

	deriv_conc_eta_c=non_equil_dos*fermi_integral_minus_2_half(planck_pot-energy_level/temp);
}

void T2DElectron::comp_doping_conc(void)
{
	TNode **temp_ptr;
	prec prev_position, prev_doping;
	prec new_position, new_doping;

	temp_ptr=grid_ptr+nodes.prev_node+1;

	prev_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
	prev_doping=(*temp_ptr)->get_value(ELECTRON,DOPING_CONC,NORMALIZED);

	doping_conc=(prev_position-(*(temp_ptr-1))->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED))*
				 prev_doping/2.0;

	for (temp_ptr=grid_ptr+nodes.prev_node+2;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		 new_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		 new_doping=(*temp_ptr)->get_value(ELECTRON,DOPING_CONC,NORMALIZED);

		 doping_conc+=(new_position-prev_position)*(new_doping+prev_doping)/2.0;

		 prev_position=new_position;
		 prev_doping=new_doping;
	}

	new_position=nodes.next_node_ptr->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	doping_conc+=(new_position-prev_position)*prev_doping/2.0;
}

void T2DElectron::comp_eigenvalues(void)
{
	prec qw_dos_mass, bulk_dos_mass;
	prec curr_trans_result, qw_depth;
	logical energy_level_found;

	qw_dos_mass=nodes.curr_node_ptr->get_value(ELECTRON,DOS_MASS);

	if (qw_effects & QW_INFINITE_SQRWELL)
		energy_level=(sq(SIM_pi*SIM_hb)/
					 (2.0*qw_dos_mass*SIM_mo*sq(qw_length*normalization.length*1e-2)))/
					 (SIM_q*normalization.pot);

	if (qw_effects & QW_FINITE_SQRWELL) {
		qw_depth=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)
				-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY);
		bulk_dos_mass=nodes.prev_node_ptr->get_value(ELECTRON,DOS_MASS);
		energy_level_found=FALSE;
		energy_level=0.0;
		while (!energy_level_found) {
			curr_trans_result=sqrt(qw_dos_mass*SIM_q*energy_level)*
							  tan(sqrt(qw_dos_mass*SIM_mo*SIM_q*energy_level/(2.0*sq(SIM_hb)))*
								  qw_length*normalization.length*1e-2)
							 -sqrt(bulk_dos_mass*SIM_q*(qw_depth-energy_level));
			if (curr_trans_result<0.0)
				energy_level+=qw_depth/50.0;
			else energy_level_found=TRUE;
		}
		energy_level/=normalization.pot;
	}
}

void T2DElectron::comp_wavefunctions(prec start_position)
{
	prec wave_function;
	TNode** temp_ptr;

	for (temp_ptr=grid_ptr+nodes.prev_node+1;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		wave_function=sqrt(2.0/qw_length)*
					  sin(SIM_pi*((*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED)-start_position)/qw_length);
		(*temp_ptr)->TBoundElectron::wave_function=wave_function;
	}
}

void T2DElectron::comp_qw_top(void)
{
	prec energy_level_ref;
	prec top_energy_prev, top_energy_next;
	TNode** temp_ptr;

	top_energy_prev=-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
					-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED);

	top_energy_next=-nodes.next_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
					-nodes.next_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED);

	if (top_energy_prev<top_energy_next) qw_energy_top=top_energy_prev;
	else qw_energy_top=top_energy_next;

	for (temp_ptr=grid_ptr+nodes.prev_node+1;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		energy_level_ref=-(*temp_ptr)->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
						 -(*temp_ptr)->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED);
		(*temp_ptr)->TBoundElectron::qw_energy_top=qw_energy_top-energy_level_ref;
	}

	qw_energy_top-=(-nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
					-nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED));
}

void T2DElectron::comp_stimulated_factor(prec band_gap)
{
	prec reduced_mass, dos_mass, mode_energy;

	reduced_mass=nodes.curr_node_ptr->get_value(NODE,REDUCED_DOS_MASS,NORMALIZED);
	dos_mass=nodes.curr_node_ptr->get_value(ELECTRON,DOS_MASS,NORMALIZED);
	mode_energy=nodes.curr_node_ptr->get_value(GRID_OPTICAL,MODE_PHOTON_ENERGY,NORMALIZED);

	stimulated_factor=(reduced_mass/dos_mass)*(mode_energy-band_gap)+energy_level;
}

void T2DElectron::comp_auger_coefficient(void)
{
	prec values[]={ nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_CONC),
					nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED) };

	auger_coefficient=material_parameters.evaluate(MAT_QW_ELECTRON_AUGER_COEFFICIENT,
												   (MaterialType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,MATERIAL),
												   (AlloyType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_TYPE),
												    values)*sq(normalization.length*normalization.conc)*normalization.time;

}

prec T2DElectron::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case EQUIL_PLANCK_POT: return_value=equil_planck_potential; break;
		case CONCENTRATION: return_value=conc; break;
		case DOPING_CONC: return_value=doping_conc; break;
		case ENERGY_TOP: return_value=qw_energy_top; break;
		case ENERGY_LEVEL: return_value=energy_level; break;
		case STIMULATED_FACTOR: return_value=stimulated_factor; break;
        case AUGER_COEFFICIENT: return_value=auger_coefficient; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(QW_ELECTRON,flag_value);
	return(return_value);
}

void T2DElectron::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(QW_ELECTRON,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case EQUIL_PLANCK_POT: equil_planck_potential=value; return;
		case CONCENTRATION: conc=value; return;
		case DOPING_CONC: doping_conc=value; return;
		case ENERGY_TOP: qw_energy_top=value; return;
		case ENERGY_LEVEL: energy_level=value; return;
		case STIMULATED_FACTOR: stimulated_factor=value; return;
        case AUGER_COEFFICIENT: auger_coefficient=value; return;
		default: assert(FALSE); return;
	}
}

void T2DElectron::read_state_file(FILE *file_ptr)
{
	fread(&qw_effects,sizeof(qw_effects),1,file_ptr);
	fread(&qw_length,sizeof(qw_length),1,file_ptr);
	fread(&energy_level,sizeof(energy_level),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fread(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fread(&conc,sizeof(conc),1,file_ptr);
	fread(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fread(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fread(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
    fread(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
}

void T2DElectron::write_state_file(FILE *file_ptr)
{
	fwrite(&qw_effects,sizeof(qw_effects),1,file_ptr);
	fwrite(&qw_length,sizeof(qw_length),1,file_ptr);
	fwrite(&energy_level,sizeof(energy_level),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fwrite(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fwrite(&conc,sizeof(conc),1,file_ptr);
	fwrite(&doping_conc,sizeof(doping_conc),1,file_ptr);
	fwrite(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fwrite(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
    fwrite(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
}

/**********************************T2DHole Class************************************************

class T2DHole {
	friend TBoundHole;
protected:
	TNode** grid_ptr;
	QuantumWellNodes nodes;
	flag qw_effects;
	prec qw_length;
	prec energy_level;
	prec equil_dos;
	prec non_equil_dos;
	prec equil_planck_potential;
	prec conc;
	prec deriv_conc_eta_v;
	prec doping_conc;
	prec qw_energy_top;
	prec stimulated_factor;
    prec auger_coefficient;

public:
	T2DHole(TNode** grid);
private:
	prec comp_dos(prec dos_mass, prec temp);
public:
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);
	void comp_equil_planck_potential(prec electron_equil_dos, prec electron_energy_level,
									 prec donor_conc);
	void comp_equil_planck_potential(prec electron_equil_planck_pot);
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_doping_conc(void);
	void comp_eigenvalues(void);
	void comp_wavefunctions(prec start_position);
	void comp_qw_top(void);
	void comp_stimulated_factor(prec band_gap);
    void comp_auger_coefficient(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

T2DHole::T2DHole(TNode** grid)
{
	grid_ptr=grid;
	qw_effects=0;
	nodes.prev_node_ptr=(TNode*)0;
	nodes.curr_node_ptr=(TNode*)0;
	nodes.next_node_ptr=(TNode*)0;
	nodes.prev_node=0.0;
	nodes.curr_node=0.0;
	nodes.next_node=0.0;
	qw_length=0.0;
	energy_level=0.0;
	equil_dos=0.0;
	non_equil_dos=0.0;
	equil_planck_potential=0.0;
	conc=0.0;
	deriv_conc_eta_v=0.0;
	doping_conc=0.0;
	qw_energy_top=0.0;
	stimulated_factor=0.0;
    auger_coefficient=0.0;
}

prec T2DHole::comp_dos(prec dos_mass, prec temp)
{
	return((dos_mass*SIM_mo*SIM_k*temp*normalization.temp/(SIM_pi*sq(SIM_hb)))/
		   (1e4*normalization.conc*normalization.length));
}

void T2DHole::comp_equil_dos(void)
{
	prec dos_mass,temp;

	temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);
	dos_mass=nodes.curr_node_ptr->get_value(HOLE,DOS_MASS,NORMALIZED);

	equil_dos=comp_dos(dos_mass,temp);
}

void T2DHole::comp_non_equil_dos(void)
{
	prec dos_mass,temp;

	temp=nodes.curr_node_ptr->get_value(HOLE,TEMPERATURE,NORMALIZED);
	dos_mass=nodes.curr_node_ptr->get_value(HOLE,DOS_MASS,NORMALIZED);

	non_equil_dos=comp_dos(dos_mass,temp);
}

void T2DHole::comp_equil_planck_potential(prec electron_equil_dos, prec electron_energy_level,
										  prec donor_conc)
{
	int i=0;
	prec iteration_error;
	prec new_planck_potential,update_planck_potential;
	prec conc_difference, deriv_conc_difference;
	prec doping_difference, deriv_doping_difference;
	prec conc_function, deriv_conc_function;
	long acceptor_degeneracy,donor_degeneracy;
	prec acceptor_level,donor_level;
	prec bulk_band_gap;
	prec lattice_temp;
	flag effects;
	static prec old_donor_conc=0.0, old_acceptor_conc=0.0;
	static prec old_electron_equil_dos=0.0, old_hole_equil_dos=0.0;
	static long old_donor_degeneracy=0, old_acceptor_degeneracy=0;
	static prec old_donor_level=0.0, old_acceptor_level=0.0;
	static prec old_electron_energy_level=0.0, old_hole_energy_level=0.0;
	static prec old_bulk_band_gap=0.0, old_lattice_temp=0.0;
	static logical old_fermi_method=FALSE, old_ionized_doping_method=FALSE;
	static prec old_result=0.0;
	logical same_values=FALSE;

	acceptor_degeneracy=nodes.curr_node_ptr->get_value(HOLE,DOPING_DEGENERACY,NORMALIZED);
	donor_degeneracy=nodes.curr_node_ptr->get_value(ELECTRON,DOPING_DEGENERACY,NORMALIZED);
	acceptor_level=nodes.curr_node_ptr->get_value(HOLE,DOPING_LEVEL,NORMALIZED);
	donor_level=nodes.curr_node_ptr->get_value(ELECTRON,DOPING_LEVEL,NORMALIZED);
	bulk_band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);
	lattice_temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);
	effects=(flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS,NORMALIZED);

	if ((donor_conc==old_donor_conc) && (doping_conc==old_acceptor_conc) &&
		(electron_equil_dos==old_electron_equil_dos) && (equil_dos==old_hole_equil_dos) &&
		(donor_degeneracy==old_donor_degeneracy) && (acceptor_degeneracy==old_acceptor_degeneracy) &&
		(donor_level==old_donor_level) && (acceptor_level==old_acceptor_level) &&
		(electron_energy_level==old_electron_energy_level) && (energy_level==old_hole_energy_level) &&
		(bulk_band_gap==old_bulk_band_gap) && (lattice_temp==old_lattice_temp)) same_values=TRUE;

	if ( same_values &&
		 (((effects & GRID_FERMI_DIRAC)!=0)==old_fermi_method) &&
		 (((effects & GRID_INCOMPLETE_IONIZATION)!=0)==old_ionized_doping_method) )
		 equil_planck_potential=old_result;
	else {
		new_planck_potential=equil_planck_potential;

		do {
			conc_difference=electron_equil_dos*fermi_integral_0_half(-new_planck_potential-
																	  bulk_band_gap/lattice_temp-
																	  electron_energy_level/lattice_temp)-
							equil_dos*fermi_integral_0_half(new_planck_potential-energy_level/lattice_temp);

			deriv_conc_difference=-electron_equil_dos*fermi_integral_minus_2_half(-new_planck_potential-
																				   bulk_band_gap/lattice_temp-
																				   electron_energy_level/lattice_temp)-
								  equil_dos*fermi_integral_minus_2_half(new_planck_potential-energy_level/lattice_temp);

			if (effects & GRID_INCOMPLETE_IONIZATION) {
				doping_difference=
					doping_conc*fermi(acceptor_level/lattice_temp-new_planck_potential-bulk_band_gap/lattice_temp,
								  (prec)acceptor_degeneracy)-
					donor_conc*fermi(donor_level/lattice_temp+new_planck_potential,(prec)donor_degeneracy);

				deriv_doping_difference=
					-doping_conc*deriv_fermi(acceptor_level/lattice_temp-new_planck_potential-bulk_band_gap/lattice_temp,
								  (prec)acceptor_degeneracy)-
					donor_conc*deriv_fermi(donor_level/lattice_temp+new_planck_potential,(prec)donor_degeneracy);
			}
			else {
				doping_difference=doping_conc-donor_conc;
				deriv_doping_difference=0.0;
			}

			conc_function=conc_difference+doping_difference;
			deriv_conc_function=deriv_conc_difference+deriv_doping_difference;

			update_planck_potential=conc_function/deriv_conc_function;
			if (new_planck_potential!=0.0) iteration_error=fabs(update_planck_potential/new_planck_potential);
			else iteration_error=1.0;
			new_planck_potential-=update_planck_potential;
			i++;
		}
		while ((i<51) && (iteration_error>=1e-8));

		equil_planck_potential=new_planck_potential;

		old_donor_conc=donor_conc;
		old_acceptor_conc=doping_conc;
		old_electron_equil_dos=electron_equil_dos;
		old_hole_equil_dos=equil_dos;
		old_donor_degeneracy=donor_degeneracy;
		old_acceptor_degeneracy=acceptor_degeneracy;
		old_donor_level=donor_level;
		old_acceptor_level=acceptor_level;
		old_electron_energy_level=electron_energy_level;
		old_hole_energy_level=energy_level;
		old_bulk_band_gap=bulk_band_gap;
		old_lattice_temp=lattice_temp;

		old_fermi_method=((effects & GRID_FERMI_DIRAC)!=0);
		old_ionized_doping_method=((effects & GRID_INCOMPLETE_IONIZATION)!=0);

		old_result=equil_planck_potential;
	}
}

void T2DHole::comp_equil_planck_potential(prec electron_equil_planck_pot)
{
	prec band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);
	prec lattice_temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);

	equil_planck_potential=-electron_equil_planck_pot-band_gap/lattice_temp;
}

void T2DHole::comp_conc(void)
{
	prec planck_pot,temp;

	planck_pot=nodes.curr_node_ptr->get_value(HOLE,PLANCK_POT,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(HOLE,TEMPERATURE,NORMALIZED);

	conc=non_equil_dos*fermi_integral_0_half(planck_pot-energy_level/temp);
}

void T2DHole::comp_deriv_conc(void)
{
	prec planck_pot,temp;

	planck_pot=nodes.curr_node_ptr->get_value(HOLE,PLANCK_POT,NORMALIZED);
	temp=nodes.curr_node_ptr->get_value(HOLE,TEMPERATURE,NORMALIZED);

	deriv_conc_eta_v=non_equil_dos*fermi_integral_minus_2_half(planck_pot-energy_level/temp);
}

void T2DHole::comp_doping_conc(void)
{
	TNode **temp_ptr;
	prec prev_position, prev_doping;
	prec new_position, new_doping;

	temp_ptr=grid_ptr+nodes.prev_node+1;

	prev_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
	prev_doping=(*temp_ptr)->get_value(HOLE,DOPING_CONC,NORMALIZED);

	doping_conc=(prev_position-(*(temp_ptr-1))->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED))*
				 prev_doping/2.0;

	for (temp_ptr=grid_ptr+nodes.prev_node+2;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		 new_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);
		 new_doping=(*temp_ptr)->get_value(HOLE,DOPING_CONC,NORMALIZED);

		 doping_conc+=(new_position-prev_position)*(new_doping+prev_doping)/2.0;

		 prev_position=new_position;
		 prev_doping=new_doping;
	}

	new_position=nodes.next_node_ptr->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	doping_conc+=(new_position-prev_position)*prev_doping/2.0;
}

void T2DHole::comp_qw_top(void)
{
	prec energy_level_ref;
	prec top_energy_prev, top_energy_next;
	TNode** temp_ptr;

	top_energy_prev=-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
					-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED)
					-nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);

	top_energy_next=-nodes.next_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
					-nodes.next_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED)
					-nodes.next_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);

	if (top_energy_prev>top_energy_next) qw_energy_top=top_energy_prev;
	else qw_energy_top=top_energy_next;

	for (temp_ptr=grid_ptr+nodes.prev_node+1;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		energy_level_ref=-(*temp_ptr)->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
						 -(*temp_ptr)->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED)
						 -(*temp_ptr)->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);

		(*temp_ptr)->TBoundHole::qw_energy_top=qw_energy_top-energy_level_ref;
	}

	qw_energy_top=(-nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY,NORMALIZED)
				   -nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,POTENTIAL,NORMALIZED)
				   -nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED))-qw_energy_top;
}

void T2DHole::comp_eigenvalues(void)
{
	prec qw_dos_mass, bulk_dos_mass;
	prec qw_depth, curr_trans_result;
	logical energy_level_found;

	qw_dos_mass=nodes.curr_node_ptr->get_value(HOLE,DOS_MASS);

	if (qw_effects & QW_INFINITE_SQRWELL)
		energy_level=(sq(SIM_pi*SIM_hb)/
					 (2.0*qw_dos_mass*SIM_mo*sq(qw_length*normalization.length*1e-2)))/
					 (SIM_q*normalization.pot);

	if (qw_effects & QW_FINITE_SQRWELL) {
		qw_depth=nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)+
				 nodes.prev_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP)-
				 nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ELECTRON_AFFINITY)-
				 nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP);
		bulk_dos_mass=nodes.prev_node_ptr->get_value(HOLE,DOS_MASS);
		energy_level_found=FALSE;
		energy_level=0.0;
		while (!energy_level_found) {
			curr_trans_result=sqrt(qw_dos_mass*SIM_q*energy_level)*
							  tan(sqrt(qw_dos_mass*SIM_mo*SIM_q*energy_level/(2.0*sq(SIM_hb)))*
								  qw_length*normalization.length*1e-2)
							 -sqrt(bulk_dos_mass*SIM_q*(qw_depth-energy_level));
			if (curr_trans_result<0.0)
				energy_level+=qw_depth/50.0;
			else energy_level_found=TRUE;
		}
		energy_level/=normalization.pot;
	}
}

void T2DHole::comp_wavefunctions(prec start_position)
{
	prec wave_function;
	TNode** temp_ptr;

	for (temp_ptr=grid_ptr+nodes.prev_node+1;
		 temp_ptr<=grid_ptr+nodes.next_node-1;
		 temp_ptr++) {
		wave_function=sqrt(2.0/qw_length)*
					  sin(SIM_pi*((*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED)-start_position)/qw_length);
		(*temp_ptr)->TBoundHole::wave_function=wave_function;
	}
}

void T2DHole::comp_stimulated_factor(prec band_gap)
{
	prec reduced_mass, dos_mass, mode_energy;

	reduced_mass=nodes.curr_node_ptr->get_value(NODE,REDUCED_DOS_MASS,NORMALIZED);
	dos_mass=nodes.curr_node_ptr->get_value(HOLE,DOS_MASS,NORMALIZED);
	mode_energy=nodes.curr_node_ptr->get_value(GRID_OPTICAL,MODE_PHOTON_ENERGY,NORMALIZED);

	stimulated_factor=(reduced_mass/dos_mass)*(mode_energy-band_gap)+energy_level;
}

void T2DHole::comp_auger_coefficient(void)
{
	prec values[]={ nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_CONC),
					nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED) };

	auger_coefficient=material_parameters.evaluate(MAT_QW_HOLE_AUGER_COEFFICIENT,
												   (MaterialType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,MATERIAL),
												   (AlloyType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_TYPE),
												    values)*sq(normalization.length*normalization.conc)*normalization.time;

}

prec T2DHole::get_value(flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_value) {
		case EQUIL_DOS: return_value=equil_dos; break;
		case NON_EQUIL_DOS: return_value=non_equil_dos; break;
		case EQUIL_PLANCK_POT: return_value=equil_planck_potential; break;
		case CONCENTRATION: return_value=conc; break;
		case DOPING_CONC: return_value=doping_conc; break;
		case ENERGY_TOP: return_value=qw_energy_top; break;
		case ENERGY_LEVEL: return_value=energy_level; break;
		case STIMULATED_FACTOR: return_value=stimulated_factor; break;
        case AUGER_COEFFICIENT: return_value=auger_coefficient; break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(QW_HOLE,flag_value);
	return(return_value);
}

void T2DHole::put_value(flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(QW_HOLE,flag_value);

	switch(flag_value) {
		case EQUIL_DOS: equil_dos=value; return;
		case NON_EQUIL_DOS: non_equil_dos=value; return;
		case EQUIL_PLANCK_POT: equil_planck_potential=value; return;
		case CONCENTRATION: conc=value; return;
		case DOPING_CONC: doping_conc=value; return;
		case ENERGY_TOP: qw_energy_top=value; return;
		case ENERGY_LEVEL: energy_level=value; return;
		case STIMULATED_FACTOR: stimulated_factor=value; return;
        case AUGER_COEFFICIENT: auger_coefficient=value; return;
		default: assert(FALSE); return;
	}
}

void T2DHole::read_state_file(FILE *file_ptr)
{
	fread(&qw_effects,sizeof(qw_effects),1,file_ptr);
	fread(&qw_length,sizeof(qw_length),1,file_ptr);
	fread(&energy_level,sizeof(energy_level),1,file_ptr);
	fread(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fread(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fread(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fread(&conc,sizeof(conc),1,file_ptr);
	fread(&doping_conc,sizeof(conc),1,file_ptr);
	fread(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fread(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
    fread(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
}

void T2DHole::write_state_file(FILE *file_ptr)
{
	fwrite(&qw_effects,sizeof(qw_effects),1,file_ptr);
	fwrite(&qw_length,sizeof(qw_length),1,file_ptr);
	fwrite(&energy_level,sizeof(energy_level),1,file_ptr);
	fwrite(&equil_dos,sizeof(equil_dos),1,file_ptr);
	fwrite(&non_equil_dos,sizeof(non_equil_dos),1,file_ptr);
	fwrite(&equil_planck_potential,sizeof(equil_planck_potential),1,file_ptr);
	fwrite(&conc,sizeof(conc),1,file_ptr);
	fwrite(&doping_conc,sizeof(conc),1,file_ptr);
	fwrite(&qw_energy_top,sizeof(qw_energy_top),1,file_ptr);
	fwrite(&stimulated_factor,sizeof(stimulated_factor),1,file_ptr);
    fwrite(&auger_coefficient,sizeof(auger_coefficient),1,file_ptr);
}

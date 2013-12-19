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
#include "simdev.h"
#include "simqw.h"

/************************************** class TQuantumWell ************************************

class TQuantumWell: protected T2DElectron, protected T2DHole {
	friend TNode;
	friend TGrid;
private:
	TDevice *device_ptr;
	TNode** grid_ptr;
	QuantumWellNodes nodes;
	prec start_position;
	prec qw_length;
	prec intrinsic_conc_2D;
	prec b_b_recomb_const_2D;
	prec band_gap;
	prec overlap;
	Recombination recombination_2D;
	prec spontaneous_heat_2D;
	RecombDerivParam deriv_qw_gain;
	RecombDerivParam deriv_recomb_2D;
	prec incident_absorption;
	prec mode_absorption;
	prec mode_gain;

private:
	prec comp_absorption(prec energy);

public:
	TQuantumWell(TDevice *device, TNode** grid);
	void comp_auger_recombination(void);
	void comp_band_gap(void);
	void comp_b_b_recomb_const_2D(void);
	void comp_intrinsic_conc(void);
	void comp_b_b_recombination(void);
	void comp_shr_recombination(void);
	void comp_spontaneous_heat(void);
	void comp_qw_gain(void);
	void comp_overlap(void);
	void comp_deriv_conc(void)
		{ T2DElectron::comp_deriv_conc(); T2DHole::comp_deriv_conc(); }
	void comp_deriv_recomb(void);
	void comp_deriv_qw_gain(void);
	void comp_mode_absorption(void);
	void comp_incident_absorption(void);
	void comp_value(FlagType flag_type, flag flag_value);

	prec get_value(FlagType flag_type, flag flag_value,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, prec value,
				   ScaleType scale=UNNORMALIZED);

	int get_node(NodeSide node);
	void put_node(NodeSide node, int node_number);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};
*/

TQuantumWell::TQuantumWell(TDevice *device, TNode** grid)
	: T2DElectron(grid), T2DHole(grid)
{
	grid_ptr=grid;
	device_ptr=device;
	nodes.prev_node_ptr=(TNode*)0;
	nodes.curr_node_ptr=(TNode*)0;
	nodes.next_node_ptr=(TNode*)0;
	nodes.prev_node=0;
	nodes.curr_node=0;
	nodes.next_node=0;
	qw_length=0.0;
	start_position=0.0;
	intrinsic_conc_2D=0.0;
	b_b_recomb_const_2D=0.0;
    recombination_2D.auger=0.0;
	recombination_2D.shr=0.0;
	recombination_2D.b_b=0.0;
	recombination_2D.stim=0.0;
	recombination_2D.total=0.0;
	deriv_recomb_2D.eta_c=0.0;
	deriv_recomb_2D.eta_v=0.0;
	deriv_qw_gain.eta_c=0.0;
	deriv_qw_gain.eta_v=0.0;
	incident_absorption=mode_absorption=mode_gain=0.0;
	spontaneous_heat_2D=0.0;
}

prec TQuantumWell::comp_absorption(prec energy)
{
	prec bulk_band_gap;

	bulk_band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED);

	if (energy<band_gap)
		return(0.0);
	else
		return(11700.0*sqrt((band_gap-bulk_band_gap)*normalization.energy)*normalization.length*qw_length);
}

void TQuantumWell::comp_auger_recombination(void)
{
	prec p,n;

	if ((flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS) & GRID_RECOMB_AUGER) {
		p=T2DHole::conc;
		n=T2DElectron::conc;

	    recombination_2D.auger=overlap*(T2DElectron::auger_coefficient*n+T2DHole::auger_coefficient*p)*
		        					   (n*p-sq(intrinsic_conc_2D));
	}
	else recombination_2D.auger=0.0;
}

void TQuantumWell::comp_band_gap(void)
{
	band_gap=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,BAND_GAP,NORMALIZED)+
			 T2DElectron::energy_level+T2DHole::energy_level;
}

void TQuantumWell::comp_b_b_recomb_const_2D(void)
{
	prec values[]={ nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_CONC),
					nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED) };

	b_b_recomb_const_2D=material_parameters.evaluate(MAT_QW_B_B_RECOMB_CONSTANT,
													 (MaterialType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,MATERIAL),
													 (AlloyType)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,ALLOY_TYPE),
													  values)*normalization.length*normalization.conc*normalization.time;

}

void TQuantumWell::comp_intrinsic_conc(void)
{
	prec qw_temp;
	prec qw_electron_planck_value, qw_hole_planck_value;

	qw_temp=nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,TEMPERATURE,NORMALIZED);

	qw_electron_planck_value=T2DElectron::equil_planck_potential-T2DElectron::energy_level/qw_temp;
	qw_hole_planck_value=T2DHole::equil_planck_potential-T2DHole::energy_level/qw_temp;

	intrinsic_conc_2D=sqrt(T2DElectron::equil_dos*T2DHole::equil_dos)*
					  exp(-band_gap/(2.0*qw_temp));

	intrinsic_conc_2D*=sqrt((fermi_integral_0_half(qw_electron_planck_value)/exp(qw_electron_planck_value))*
							(fermi_integral_0_half(qw_hole_planck_value)/exp(qw_hole_planck_value)));

}

void TQuantumWell::comp_b_b_recombination(void)
{
	if ((flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS) & GRID_RECOMB_B_B) {
		recombination_2D.b_b=overlap*b_b_recomb_const_2D*(T2DElectron::conc*T2DHole::conc-sq(intrinsic_conc_2D));
	}
	else recombination_2D.b_b=0.0;
}

void TQuantumWell::comp_shr_recombination(void)
{
	prec p,n;
	prec electron_shr_lifetime, hole_shr_lifetime;

	if ((flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS) & GRID_RECOMB_SHR) {
		p=T2DHole::conc;
		n=T2DElectron::conc;
		electron_shr_lifetime=nodes.curr_node_ptr->get_value(ELECTRON,SHR_LIFETIME,NORMALIZED);
		hole_shr_lifetime=nodes.curr_node_ptr->get_value(HOLE,SHR_LIFETIME,NORMALIZED);

		recombination_2D.shr=overlap*(p*n-sq(intrinsic_conc_2D))/
									 (electron_shr_lifetime*(p+intrinsic_conc_2D)+
									  hole_shr_lifetime*(n+intrinsic_conc_2D));
	}
	else recombination_2D.shr=0.0;
}

void TQuantumWell::comp_spontaneous_heat(void)
{
	spontaneous_heat_2D=band_gap*recombination_2D.b_b;
}

void TQuantumWell::comp_qw_gain(void)
{
	prec n_planck, p_planck;
	prec n_temp, p_temp;

	n_temp=nodes.curr_node_ptr->get_value(ELECTRON,TEMPERATURE,NORMALIZED);
	p_temp=nodes.curr_node_ptr->get_value(HOLE,TEMPERATURE,NORMALIZED);
	n_planck=nodes.curr_node_ptr->get_value(ELECTRON,PLANCK_POT,NORMALIZED);
	p_planck=nodes.curr_node_ptr->get_value(HOLE,PLANCK_POT,NORMALIZED);

	mode_gain=mode_absorption*(fermi(T2DElectron::stimulated_factor/n_temp-n_planck)-
							   fermi(-T2DHole::stimulated_factor/p_temp+p_planck));
	mode_gain*=overlap;
}

void TQuantumWell::comp_overlap(void)
{
	TNode** temp_ptr;
	prec prev_value, next_value;
	prec prev_position, next_position;
	prec integral;

	temp_ptr=grid_ptr+nodes.prev_node+1;

	prev_value=(*temp_ptr)->get_value(BOUND_ELECTRON,WAVE_FUNCTION,NORMALIZED)*
			   (*temp_ptr)->get_value(BOUND_HOLE,WAVE_FUNCTION,NORMALIZED);

	prev_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

	integral=0.0;

	do {
		temp_ptr++;
		next_value=(*temp_ptr)->get_value(BOUND_ELECTRON,WAVE_FUNCTION,NORMALIZED)*
				   (*temp_ptr)->get_value(BOUND_HOLE,WAVE_FUNCTION,NORMALIZED);
		next_position=(*temp_ptr)->get_value(GRID_ELECTRICAL,POSITION,NORMALIZED);

		integral+=(next_value+prev_value)*(next_position-prev_position)/2.0;

		prev_value=next_value;
		prev_position=next_position;
	}
	while (temp_ptr<grid_ptr+nodes.next_node-1);

	overlap=sq(integral);
}

void TQuantumWell::comp_deriv_recomb(void)
{
	prec electron_shr_lifetime, hole_shr_lifetime;
    prec electron_auger_coef, hole_auger_coef;
	prec n,p,ni;
	prec deriv_n_eta_c, deriv_p_eta_v;
	flag qw_effects;

	deriv_recomb_2D.eta_c=0;
	deriv_recomb_2D.eta_v=0;

	deriv_n_eta_c=T2DElectron::deriv_conc_eta_c;
	deriv_p_eta_v=T2DHole::deriv_conc_eta_v;

	n=T2DElectron::conc;
	p=T2DHole::conc;
    ni=intrinsic_conc_2D;

	qw_effects=(flag)nodes.curr_node_ptr->get_value(GRID_ELECTRICAL,EFFECTS);

	if ((qw_effects & GRID_RECOMB_B_B) && (recombination_2D.b_b>0.0)){
		deriv_recomb_2D.eta_c=overlap*b_b_recomb_const_2D*p*deriv_n_eta_c;
		deriv_recomb_2D.eta_v=overlap*b_b_recomb_const_2D*n*deriv_p_eta_v;
	}

	if (qw_effects & GRID_RECOMB_SHR) {

		electron_shr_lifetime=nodes.curr_node_ptr->get_value(ELECTRON,SHR_LIFETIME,NORMALIZED);
		hole_shr_lifetime=nodes.curr_node_ptr->get_value(HOLE,SHR_LIFETIME,NORMALIZED);

		deriv_recomb_2D.eta_c+=overlap*(deriv_n_eta_c*(electron_shr_lifetime*sq(p)+
													  (electron_shr_lifetime+hole_shr_lifetime)*p*ni+
													   sq(ni)*hole_shr_lifetime))/
													  (sq(electron_shr_lifetime*(p+ni)+hole_shr_lifetime*(n+ni)));

		deriv_recomb_2D.eta_v+=overlap*(deriv_p_eta_v*(hole_shr_lifetime*sq(n)+
													  (electron_shr_lifetime+hole_shr_lifetime)*n*ni+
													   sq(ni)*electron_shr_lifetime))/
													  (sq(electron_shr_lifetime*(p+ni)+hole_shr_lifetime*(n+ni)));
	}

	if (qw_effects & GRID_RECOMB_AUGER) {
        electron_auger_coef=T2DElectron::auger_coefficient;
        hole_auger_coef=T2DHole::auger_coefficient;

        deriv_recomb_2D.eta_c+=overlap*(deriv_n_eta_c*(p*(electron_auger_coef*n+hole_auger_coef*p)+
            								   			  electron_auger_coef*(n*p-sq(ni))));
        deriv_recomb_2D.eta_v+=overlap*(deriv_p_eta_v*(n*(electron_auger_coef*n+hole_auger_coef*p)+
            								   			  hole_auger_coef*(n*p-sq(ni))));
	}

	if (qw_effects & GRID_RECOMB_STIM) {
		comp_deriv_qw_gain();
		deriv_qw_gain.eta_c*=overlap;
		deriv_qw_gain.eta_v*=overlap;
	}
}

void TQuantumWell::comp_deriv_qw_gain(void)
{
	prec n_planck, p_planck;
	prec n_temp, p_temp;

	n_temp=nodes.curr_node_ptr->get_value(ELECTRON,TEMPERATURE,NORMALIZED);
	p_temp=nodes.curr_node_ptr->get_value(HOLE,TEMPERATURE,NORMALIZED);
	n_planck=nodes.curr_node_ptr->get_value(ELECTRON,PLANCK_POT,NORMALIZED);
	p_planck=nodes.curr_node_ptr->get_value(HOLE,PLANCK_POT,NORMALIZED);

	deriv_qw_gain.eta_c=-mode_absorption*deriv_fermi(T2DElectron::stimulated_factor/n_temp-n_planck);
	deriv_qw_gain.eta_v=-mode_absorption*deriv_fermi(-T2DHole::stimulated_factor/p_temp+p_planck);
}

void TQuantumWell::comp_mode_absorption(void)
{
	prec mode_energy;

	mode_energy=nodes.curr_node_ptr->get_value(GRID_OPTICAL,MODE_PHOTON_ENERGY,NORMALIZED);
	mode_absorption=comp_absorption(mode_energy);
}

void TQuantumWell::comp_incident_absorption(void)
{
	prec incident_energy;

	incident_energy=nodes.curr_node_ptr->get_value(GRID_OPTICAL,INCIDENT_PHOTON_ENERGY,NORMALIZED);
	incident_absorption=comp_absorption(incident_energy);
}

void TQuantumWell::comp_value(FlagType flag_type, flag flag_value)
{
	switch(flag_type) {
		case QW_ELECTRON:
			switch(flag_value) {
				case WAVE_FUNCTION:
					T2DElectron::comp_wavefunctions(start_position); return;
				case ENERGY_LEVEL:
					T2DElectron::comp_eigenvalues(); return;
				case ENERGY_TOP:
					T2DElectron::comp_qw_top(); return;
				case CONCENTRATION:
					T2DElectron::comp_conc(); return;
				case EQUIL_DOS:
					T2DElectron::comp_equil_dos(); return;
				case NON_EQUIL_DOS:
					T2DElectron::comp_non_equil_dos(); return;
				case EQUIL_PLANCK_POT:
					if (T2DElectron::doping_conc>=T2DHole::doping_conc) {
						T2DElectron::comp_equil_planck_potential(T2DHole::equil_dos,T2DHole::energy_level,
																 T2DHole::doping_conc);
						T2DHole::comp_equil_planck_potential(T2DElectron::equil_planck_potential);
					}
					else {
						T2DHole::comp_equil_planck_potential(T2DElectron::equil_dos,T2DElectron::energy_level,
															 T2DElectron::doping_conc);
						T2DElectron::comp_equil_planck_potential(T2DHole::equil_planck_potential);
					}
					return;
				case STIMULATED_FACTOR:
					T2DElectron::comp_stimulated_factor(band_gap); return;
                case AUGER_COEFFICIENT:
                	T2DElectron::comp_auger_coefficient(); return;
				case DOPING_CONC:
					T2DElectron::comp_doping_conc(); return;
				default: assert(FALSE); return;
			}
		case QW_HOLE:
			switch (flag_value) {
				case WAVE_FUNCTION:
					T2DHole::comp_wavefunctions(start_position); return;
				case ENERGY_LEVEL:
					T2DHole::comp_eigenvalues(); return;
				case ENERGY_TOP:
					T2DHole::comp_qw_top(); return;
				case CONCENTRATION:
					T2DHole::comp_conc(); return;
				case EQUIL_DOS:
					T2DHole::comp_equil_dos(); return;
				case NON_EQUIL_DOS:
					T2DHole::comp_non_equil_dos(); return;
				case EQUIL_PLANCK_POT:
					comp_value(QW_ELECTRON,EQUIL_PLANCK_POT); return;
				case STIMULATED_FACTOR:
					T2DHole::comp_stimulated_factor(band_gap); return;
                case AUGER_COEFFICIENT:
                	T2DHole::comp_auger_coefficient(); return;
				case DOPING_CONC:
					T2DHole::comp_doping_conc(); return;
				default: assert(FALSE); return;
			}
		case QUANTUM_WELL:
			switch(flag_value) {
				case POSITION:
					start_position=(device_ptr->get_value(GRID_ELECTRICAL,POSITION,nodes.prev_node+1,NORMALIZED)+
									device_ptr->get_value(GRID_ELECTRICAL,POSITION,nodes.prev_node,NORMALIZED))/2.0;
					return;
				case MIDPOINT:put_node(CURRENT_NODE,device_ptr->get_node(start_position+qw_length/2.0,-1,-1,NORMALIZED));
					return;
				case BAND_GAP: comp_band_gap(); return;
				case INTRINSIC_CONC: comp_intrinsic_conc(); return;
				case B_B_RECOMB: comp_b_b_recombination(); return;
				case SHR_RECOMB: comp_shr_recombination(); return;
                case AUGER_RECOMB: comp_auger_recombination(); return;
				case OVERLAP: comp_overlap(); return;
				case MODE_GAIN: comp_qw_gain(); return;
				case MODE_ABSORPTION: comp_mode_absorption(); return;
				case INCIDENT_ABSORPTION: comp_incident_absorption(); return;
				case B_B_HEAT: comp_spontaneous_heat(); return;
				case B_B_RECOMB_CONSTANT: comp_b_b_recomb_const_2D(); return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

prec TQuantumWell::get_value(FlagType flag_type, flag flag_value, ScaleType scale)
{
	prec return_value;

	switch(flag_type) {
		case QW_ELECTRON: return(T2DElectron::get_value(flag_value,scale));
		case QW_HOLE: return(T2DHole::get_value(flag_value,scale));
		case QUANTUM_WELL:
			switch(flag_value) {
				case LENGTH: return_value=qw_length; break;
				case EFFECTS: return_value=T2DElectron::qw_effects; break;
				case NUMBER_NODES: return_value=(prec) (nodes.next_node-nodes.prev_node-1); break;
				case POSITION: return_value=start_position; break;
				case OVERLAP: return_value=overlap; break;
				case MODE_ABSORPTION: return_value=mode_absorption; break;
				case INCIDENT_ABSORPTION: return_value=incident_absorption; break;
				case BAND_GAP: return_value=band_gap; break;
				case MODE_GAIN: return_value=mode_gain; break;
				case INTRINSIC_CONC: return_value=intrinsic_conc_2D; break;
				case SHR_RECOMB: return_value=recombination_2D.shr; break;
				case B_B_RECOMB: return_value=recombination_2D.b_b; break;
				case B_B_RECOMB_CONSTANT: return_value=b_b_recomb_const_2D; break;
				default: assert(FALSE); return(0.0);
			}
			break;
		default: assert(FALSE); return(0.0);
	}

	if (scale==UNNORMALIZED) return_value*=get_normalize_value(flag_type,flag_value);
	return(return_value);
}

void TQuantumWell::put_value(FlagType flag_type, flag flag_value, prec value, ScaleType scale)
{
	if (scale==UNNORMALIZED) value/=get_normalize_value(flag_type,flag_value);

	switch(flag_type) {
		case QW_ELECTRON: T2DElectron::put_value(flag_value,value,NORMALIZED); return;
		case QW_HOLE: T2DHole::put_value(flag_value,value,NORMALIZED); return;
		case QUANTUM_WELL:
			switch(flag_value) {
				case LENGTH:
					qw_length=value;
					T2DElectron::qw_length=value;
					T2DHole::qw_length=value;
					return;
				case EFFECTS:
					T2DElectron::qw_effects=(flag) value;
					T2DHole::qw_effects=(flag) value;
					return;
				default: assert(FALSE); return;
			}
		default: assert(FALSE); return;
	}
}

int TQuantumWell::get_node(NodeSide node)
{
	switch(node) {
		case PREVIOUS_NODE: return(nodes.prev_node);
		case CURRENT_NODE: return(nodes.curr_node);
		case NEXT_NODE: return(nodes.next_node);
		default: assert(FALSE); return(0);
	}
}

void TQuantumWell::put_node(NodeSide node, int node_number)
{
	switch(node) {
		case PREVIOUS_NODE:
			nodes.prev_node_ptr=*(grid_ptr+node_number);
			nodes.prev_node=node_number;
			T2DElectron::nodes.prev_node_ptr=nodes.prev_node_ptr;
			T2DElectron::nodes.prev_node=nodes.prev_node;
			T2DHole::nodes.prev_node_ptr=nodes.prev_node_ptr;
			T2DHole::nodes.prev_node=nodes.prev_node;
			return;
		case CURRENT_NODE:
			nodes.curr_node_ptr=*(grid_ptr+node_number);
			nodes.curr_node=node_number;
			T2DElectron::nodes.curr_node_ptr=nodes.curr_node_ptr;
			T2DElectron::nodes.curr_node=nodes.curr_node;
			T2DHole::nodes.curr_node_ptr=nodes.curr_node_ptr;
			T2DHole::nodes.curr_node=nodes.curr_node;
			return;
		case NEXT_NODE:
			nodes.next_node_ptr=*(grid_ptr+node_number);
			nodes.next_node=node_number;
			T2DElectron::nodes.next_node_ptr=nodes.next_node_ptr;
			T2DElectron::nodes.next_node=nodes.next_node;
			T2DHole::nodes.next_node_ptr=nodes.next_node_ptr;
			T2DHole::nodes.next_node=nodes.next_node;
			return;
		default: assert(FALSE); return;
	}
}

void TQuantumWell::read_state_file(FILE *file_ptr)
{
	fread(&start_position,sizeof(start_position),1,file_ptr);
	fread(&qw_length,sizeof(qw_length),1,file_ptr);
	fread(&intrinsic_conc_2D,sizeof(intrinsic_conc_2D),1,file_ptr);
	fread(&b_b_recomb_const_2D,sizeof(b_b_recomb_const_2D),1,file_ptr);
	fread(&overlap,sizeof(overlap),1,file_ptr);
	fread(&band_gap,sizeof(band_gap),1,file_ptr);

	fread(&recombination_2D,sizeof(recombination_2D),1,file_ptr);
	fread(&incident_absorption,sizeof(incident_absorption),1,file_ptr);
	fread(&mode_absorption,sizeof(mode_absorption),1,file_ptr);
	fread(&mode_gain,sizeof(mode_gain),1,file_ptr);
	fread(&spontaneous_heat_2D,sizeof(spontaneous_heat_2D),1,file_ptr);

	T2DElectron::read_state_file(file_ptr);
	T2DHole::read_state_file(file_ptr);
}

void TQuantumWell::write_state_file(FILE *file_ptr)
{
	fwrite(&start_position,sizeof(start_position),1,file_ptr);
	fwrite(&qw_length,sizeof(qw_length),1,file_ptr);
	fwrite(&intrinsic_conc_2D,sizeof(intrinsic_conc_2D),1,file_ptr);
	fwrite(&b_b_recomb_const_2D,sizeof(b_b_recomb_const_2D),1,file_ptr);
	fwrite(&overlap,sizeof(overlap),1,file_ptr);
	fwrite(&band_gap,sizeof(band_gap),1,file_ptr);

	fwrite(&recombination_2D,sizeof(recombination_2D),1,file_ptr);
	fwrite(&incident_absorption,sizeof(incident_absorption),1,file_ptr);
	fwrite(&mode_absorption,sizeof(mode_absorption),1,file_ptr);
	fwrite(&mode_gain,sizeof(mode_gain),1,file_ptr);
	fwrite(&spontaneous_heat_2D,sizeof(spontaneous_heat_2D),1,file_ptr);

	T2DElectron::write_state_file(file_ptr);
	T2DHole::write_state_file(file_ptr);
}

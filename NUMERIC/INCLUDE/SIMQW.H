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


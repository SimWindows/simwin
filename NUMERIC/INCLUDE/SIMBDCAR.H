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

class TBoundElectron {
	friend T2DElectron;
protected:
	TQuantumWell *qw_ptr;
	prec equil_dos;
	prec non_equil_dos;
	prec wave_function;
	prec concentration;
	prec deriv_conc_eta_c;
	prec qw_energy_top;

public:
	TBoundElectron(TQuantumWell *qw);

// Comp functions.
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};

class TBoundHole {
	friend T2DHole;
protected:
	TQuantumWell *qw_ptr;
	prec equil_dos;
	prec non_equil_dos;
	prec wave_function;
	prec concentration;
	prec deriv_conc_eta_v;
	prec qw_energy_top;

public:
	TBoundHole(TQuantumWell *qw);

// Comp functions
	void comp_conc(void);
	void comp_deriv_conc(void);
	void comp_equil_dos(void);
	void comp_non_equil_dos(void);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



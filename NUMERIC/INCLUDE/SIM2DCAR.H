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

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};

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



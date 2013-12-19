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

class TFreeElectron {
protected:
	flag effects;
	prec equil_dos;
	prec non_equil_dos;
	prec mobility;
	prec concentration;
	prec deriv_conc_eta_c;

public:
	TFreeElectron(void);

// Comp functions
private:
	prec comp_dos(prec dos_mass, prec temp);

public:
	void comp_equil_dos(prec dos_mass, prec temp) { equil_dos=comp_dos(dos_mass, temp); }
	void comp_non_equil_dos(prec dos_mass, prec temp) { non_equil_dos=comp_dos(dos_mass,temp); }
	void comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
					   prec car_temp, prec donors, prec acceptors);
	void comp_conc(prec planck_potential, prec temp=0.0,
				   prec qw_energy_top=0.0, prec bound_dos=0.0);
	void comp_deriv_conc(prec planck_potential, prec qw_energy_top=0.0);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};

class TFreeHole {
protected:
	flag effects;
	prec equil_dos;
	prec non_equil_dos;
	prec mobility;
	prec concentration;
	prec deriv_conc_eta_v;

public:
	TFreeHole(void);

// Comp functions
private:
	prec comp_dos(prec dos_mass, prec temp);
public:
	void comp_equil_dos(prec dos_mass, prec temp) { equil_dos=comp_dos(dos_mass, temp); }
	void comp_non_equil_dos(prec dos_mass, prec temp) { non_equil_dos=comp_dos(dos_mass,temp); }
	void comp_mobility(MaterialSpecification material, prec position, prec lat_temp,
					   prec car_temp, prec donors, prec acceptors);
	void comp_conc(prec planck_potential, prec temp=0.0,
				   prec qw_energy_top=0.0, prec bound_dos=0.0);
	void comp_deriv_conc(prec planck_potential,prec qw_energy_top=0.0);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};


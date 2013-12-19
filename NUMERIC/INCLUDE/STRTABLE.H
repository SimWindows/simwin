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

#define ENCODE_KEY	5

#ifdef CUSTOM
char custom_string[]=
"This is an unofficial release\n\
of SimWindows version 1.5.0.\n\
Distribution without the\n\
permission of the CU modeling\n\
group is prohibited.";

char about_string[]=
"Tkp[nofr{x!/#Ysphimhjco$[ftvmto\n\
Fsuztlkmu\"¬$6:;706:;<$gz\"Ge{jf#[3!Ylrxuqq\n\
Fmn#vnhjww%sgviwwgg\n\n\
Xge$Ubih>%xyz1tdu1gtmqueip0hhz0Ulq\\jpgs|t1vmrxkq2muoo\n\
Jnclp%cwjw%uq=$ibxlhdxkqwyppCfnhhrsy/erq";

char state_version_string[]="v.1.5.0";

#else
char about_string[]=
"Tkp[nofr{x!/#Zjsulss!31931\n\
Fsuztlkmu\"¬$6:;706:;<$gz\"Ge{jf#[3!Ylrxuqq\n\
Fmn#vnhjww%sgviwwgg\n\n\
Xge$Ubih>%xyz1tdu1gtmqueip0hhz0Ulq\\jpgs|t1vmrxkq2muoo\n\
Jnclp%cwjw%uq=$ibxlhdxkqwyppCfnhhrsy/erq";

char state_version_string[]="v.1.5.0";
#endif

char update_string[]="Last Updated:\n"__DATE__" at "__TIME__;

char state_string[]="State_File";

char undo_filename[]="simundo.tmp";
char ini_filename[]="simwin.ini";

int state_string_size=sizeof(state_string);
int state_version_string_size=sizeof(state_version_string);

#ifdef _SIMWINDOWS_32
char application_string[]="SimWindows32";
char executable_string[]="simwin32.exe";
char cap_executable_string[]="SIMWIN32.EXE";
#endif

#ifdef _SIMWINDOWS_DB
char application_string[]="SimWindows32 Debug";
char executable_string[]="simwindb.exe";
char cap_executable_string[]="SIMWINDB.EXE";
#endif

char executable_path[MAXPATH];

#ifndef NDEBUG
char debug_string[]="This is a debug version\n\
of SimWindows\n\n\
It contains code which will\n\
affect performance";
#endif

/********************************* Material Parameters Strings *********************************
	All material parameters strings must be capatilized and in the same order as the
	MAT_xxx define constants
*/

char *material_parameters_strings[]={
	"BAND_GAP",
	"ELECTRON_AFFINITY",
	"STATIC_PERMITIVITY",
	"REFRACTIVE_INDEX",
	"ABSORPTION",
	"THERMAL_CONDUCTIVITY",
	"DERIV_THERMAL_CONDUCT",
	"ELECTRON_MOBILITY",
	"HOLE_MOBILITY",
	"ELECTRON_DOS_MASS",
	"HOLE_DOS_MASS",
	"ELECTRON_COND_MASS",
	"HOLE_COND_MASS",
	"ELECTRON_SHR_LIFETIME",
	"HOLE_SHR_LIFETIME",
	"ELECTRON_ENERGY_LIFETIME",
	"HOLE_ENERGY_LIFETIME",
    "ELECTRON_AUGER_COEFFICIENT",
    "QW_ELECTRON_AUGER_COEFFICIENT",
    "HOLE_AUGER_COEFFICIENT",
    "QW_HOLE_AUGER_COEFFICIENT",
	"RAD_RECOMB_CONST",
	"QW_RAD_RECOMB_CONST",
	"ELECTRON_COLLISION_FACTOR",
	"HOLE_COLLISION_FACTOR"
};

char *material_parameters_variables[]={
	"XT",
	"XT",
	"X",
	"XTEGH",
	"XTEGH",
	"XT",
	"XT",
	"XTCNA",
	"XTCNA",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
	"X",
    "X",
    "X",
    "X",
    "X",
};


//***************************** Free Electron Strings ******************************************

char *free_electron_long_table[]={
	"Electron Density of States, Non-Equilibrium (cm-3)",
	"Electron Density of States, Equilibrium (cm-3)",
	"Electron Mobility (cm2/Vs)",
	"Electron Concentration, Free (cm-3)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *free_electron_short_table[]={
	"Non-Equil n DOS (cm-3)",
	"Equil n DOS (cm-3)",
	"n Mobil (cm2/Vs)",
	"Free n Conc (cm-3)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//********************************* Bound Electron Strings ************************************

char *bound_electron_long_table[]= {
	"Electron 3D QW Density of States, Non-Equilibrium (cm-3)",
	"Electron 3D QW Density of States, Equilibrium (cm-3)",
	"Electron Wave Function",
	"Electron Concentration, Bound (cm-3)",
	"Electron Energy Top (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *bound_electron_short_table[]= {
	"Non-Equil 3D n QW DOS (cm-3)",
	"Equil 3D n QW DOS (cm-3)",
	"n Wave Func",
	"Bound n Conc (cm-3)",
	"n Engy Top (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//********************************** Free Hole Strings ****************************************

char *free_hole_long_table[]={
	"Hole Density of States, Non-Equilibrium (cm-3)",
	"Hole Density of States, Equilibrium (cm-3)",
	"Hole Mobility (cm2/Vs)",
	"Hole Concentration, Free (cm-3)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *free_hole_short_table[]={
	"Non Equil p DOS (cm-3)",
	"Equil p DOS (cm-3)",
	"p Mobil (cm2/Vs)",
	"Free p Conc (cm-3)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//******************************** Bound Hole Strings *****************************************

char *bound_hole_long_table[]={
	"Hole 3D QW Density of States, Non-Equilibrium (cm-3)",
	"Hole 3D QW Density of States, Equilibrium (cm-3)",
	"Hole Wave Function",
	"Hole Concentration, Bound (cm-3)",
	"Hole Energy Top (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *bound_hole_short_table[]={
	"Non Equil 3D p QW DOS (cm-3)",
	"Equil 3D p QW DOS (cm-3)",
	"p Wave Func",
	"Bound p Conc (cm-3)",
	"p Engy Top (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//********************************** Electron Strings *****************************************

char *electron_long_table[]= {
	"",
	"",
	"",
	"Electron Concentration, Total (cm-3)",
	"Conduction Band (eV)",
	"Electron Temperature (K)",
	"Electron Stimulated Emission Factor (eV)",
	"Electron mass, DOS",
	"Electron mass, Conductivity",
	"",
	"Electron Quasi-fermi level (eV)",
	"Electron SHR Lifetime (s)",
	"Electron Energy Lifetime (s)",
	"",
	"Electron Current (A/cm2)",
	"",
	"Electron Collision Factor",
	"Donor Concentration (cm-3)",
	"Donor Degeneracy Factor",
	"Donor Level (ev)",
	"Ionized Donor Concentration (cm-3)",
	"Electron Auger Coefficient (cm6 s-1)",
	"",
	"Electron Opt. Gen. Potential Energy (J/cm3s)",
	"Electron Opt. Gen. Kinetic Energy (J/cm3s)",
	"Electron Bimolecular Energy (J/cm3s)",
	"",
	"Electron Stimulated Energy (J/cm3s)",
	"Electron Relaxation Energy (J/cm3s)",
	"Electron SHR Energy (J/cm3s)",
	"Electron Total Energy (J/cm3s)",
	"Electron Auger Energy (J/cm3s)",
#ifndef NDEBUG
#endif
};

char *electron_short_table[]= {
	"",
	"",
	"",
	"Total n (cm-3)",
	"Ec (eV)",
	"n Temp (K)",
	"n Stim Fact (eV)",
	"DOS Mn",
	"Cond Mn",
	"",
	"Efn (eV)",
	"Tau n (s)",
	"Tau wn (s)",
	"",
	"Jn (A/cm2)",
	"",
	"n Coll Fact",
	"Nd (cm-3)",
	"Nd Deg Fact",
	"Nd Level (eV)",
	"Nd+ (cm-3)",
	"n Aug Coeff (cm6 s-1)",
	"",
	"Pot Opt Gen wn (J/cm3s)",
	"Kin Opt Gen wn (J/cm3s)",
	"B-B wn (J/cm3s)",
	"",
	"Stim wn (J/cm3s)",
	"Relax wn (J/cm3s)",
	"SHR wn (J/cm3s)",
	"Total wn (J/cm3s)",
	"Auger wn (J/cm3s)",
#ifndef NDEBUG
#endif
};

//************************************* Hole Strings *****************************************

char *hole_long_table[]= {
	"",
	"",
	"",
	"Hole Concentration, Total (cm-3)",
	"Valence Band (eV)",
	"Hole Temperature (K)",
	"Hole Stimulated Emission Factor (eV)",
	"Hole mass, DOS",
	"Hole mass, Conductivity",
	"",
	"Hole Quasi-fermi level (eV)",
	"Hole SHR Lifetime (s)",
	"Hole Energy Lifetime (s)",
	"",
	"Hole Current (A/cm2)",
	"",
	"Hole Collision Factor",
	"Acceptor Concentration (cm-3)",
	"Acceptor Degeneracy Factor",
	"Acceptor Level (eV)",
	"Ionized Acceptor Concentration (cm-3)",
	"Hole Auger Coefficient (cm6 s-1)",
	"",
	"Hole Opt. Gen. Potential Energy (J/cm3s)",
	"Hole Opt. Gen. Kinetic Energy (J/cm3s)",
	"Hole Bimolecular Energy (J/cm3s)",
	"",
	"Hole Stimulated Energy (J/cm3s)",
	"Hole Relaxation Energy (J/cm3s)",
	"Hole SHR Energy (J/cm3s)",
	"Hole Total Energy (J/cm3s)",
	"Hole Auger Energy (J/cm3s)",
#ifndef NDEBUG
#endif
};

char *hole_short_table[]= {
	"",
	"",
	"",
	"Total p (cm-3)",
	"Ev (eV)",
	"p Temp (K)",
	"p Stim Fact (eV)",
	"DOS Mp",
	"Cond Mp",
	"",
	"Efp (eV)",
	"Tau p (s)",
	"Tau wp (s)",
	"",
	"Jp (A/cm2)",
	"",
	"p Coll Fact",
	"Na (cm-3)",
	"Na Deg Fact",
	"Na Level (eV)",
	"Na- (cm-3)",
	"p Aug Coeff (cm6 s-1)",
	"",
	"Pot Opt Gen wp (J/cm3s)",
	"Kin Opt Gen wp (J/cm3s)",
	"B_B wp (J/cm3s)",
	"",
	"Stim wp (J/cm3s)",
	"Relax wp (J/cm3s)",
	"SHR wp (J/cm3s)",
	"Total wp (J/cm3s)",
	"Auger wp (J/cm3s)",
#ifndef NDEBUG
#endif
};

//***************************** Grid Electrical Strings ***************************************

char *grid_electrical_long_table[]= {
	"Grid Position (microns)",
	"",
	"",
	"",
	"Region Type",
	"Lattice Temperature (K)",
	"Bulk Band Gap (eV)",
	"Material",
	"Alloy Type",
	"Alloy %",
	"Field (V/cm)",
	"Device Radius (microns)",
	"Bimolecular Recombination Constant (cm3/s)",
	"",
	"",
	"Thermal Conductivity (W/cmK)",
	"Lateral Thermal Conductivity (W/cm3K)",
	"",
	"",
	"",
	"",
	"Potential (V)",
	"Vacuum Level (eV)",
	"Electron Affinity (eV)",
	"Static Permitivity",
#ifndef NDEBUG
	"","","","","","","",
#endif
};

char *grid_electrical_short_table[]= {
	"Grid Pos (microns)",
	"",
	"",
	"",
	"Region Type",
	"Lat T (K)",
	"Bulk Eg (eV)",
	"Material",
	"Alloy Type",
	"Alloy %",
	"Field (V/cm)",
	"Dev Radius (microns)",
	"B-B Const (cm3/s)",
	"",
	"",
	"Therm Cond (W/cmK)",
	"Lat Therm Cond (W/cm3K)",
	"",
	"",
	"",
	"",
	"Pot (V)",
	"Evac (eV)",
	"Elec Aff (eV)",
	"Stat Perm",
#ifndef NDEBUG
	"","","","","","","",
#endif
};

//***************************** Grid Optical Strings *****************************************

char *grid_optical_long_table[]= {
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Incident Total Poynting (mW/cm2)",
	"",
	"",
	"",
	"",
	"",
	"Photon Density (cm-3)",
	"Incident Refractive Index",
	"Incident Absorption (cm-1)",
	"Incident Total Field Magnitude (a.u.)",
	"Mode Refractive Index",
	"Mode Absorption (cm-1)",
	"Local Gain (cm-1)",
	"Mode Total Field Magnitude (micron-3/2)",
	"Incident Forward Field, Real (a.u.)",
	"Incident Forward Field, Imaginary (a.u.)",
	"Incident Forward Poynting (mW/cm2)",
	"Incident Reverse Field, Real (a.u.)",
	"Incident Reverse Field, Imaginary (a.u.)",
	"Incident Reverse Poynting (mW/cm2)",
	"Mode Forward Field, Real (a.u.)",
	"Mode Forward Field, Imaginary (a.u.)",
	"Mode Reverse Field, Real (a.u.)",
	"Mode Reverse Field, Imaginary (a.u.)",
#ifndef NDEBUG
	"",
#endif
};

char *grid_optical_short_table[]= {
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Inc Tot Poynt (mW/cm2)",
	"",
	"",
	"",
	"",
	"",
	"Phot Den (cm-3)",
	"Inc Ref Index",
	"Inc Abs (cm-1)",
	"Inc Tot Field Mag (a.u.)",
	"Mod Ref Index",
	"Mode Abs (cm-1)",
	"Gain (cm-1)",
	"Mode Tot Field Mag (micr-3/2)",
	"Inc For Field Real (a.u.)",
	"Inc For Field Imag (a.u.)",
	"Inc For Poynt (mW/cm2)",
	"Inc Rev Field Real (a.u.)",
	"Inc Rev Field Imag (a.u.)",
	"Inc Rev Poynt (mW/cm2)",
	"Mode For Field Real (a.u.)",
	"Mode For Field Imag (a.u.)",
	"Mode Rev Field Real (a.u.)",
	"Mode Rev Field Imag (a.u.)",
#ifndef NDEBUG
	"",
#endif
};

//************************************** Node Strings *****************************************

char *node_long_table[]= {
	"Reduced DOS Mass",
	"Total Charge (C/cm3)",
	"",
	"Total Current (A/cm2)",
	"",
	"",
	"",
	"Bulk Intrinsic concentration (cm-3)",
	"",
	"Recombination, Total (1/cm3s)",
	"Recombination, SHR (1/cm3s)",
	"Recombination, Bimolecular (1/cm3s)",
	"",
	"Recombination, Auger (1/cm3s)",
	"Recombination, Opt. Gen. (1/cm3s)",
	"",
	"Recombination, Stimulated (1/cm3s)",
	"",
	"Total Radiative Heat (J/cm3s)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Total Bimolecular Heat(J/cm3s)",
	"Total Optical Generation Heat(J/cm3s)",
	"Total Stimulated Heat(J/cm3s)",
	"",
	"",
	"Total Lattice Energy (J/cm3s)",
#ifndef NDEBUG
	"",
#endif
};

char *node_short_table[]= {
	"DOS Mr",
	"Charge (C/cm3)",
	"",
	"J Total (A/cm2)",
	"",
	"",
	"",
	"Bulk ni (cm-3)",
	"",
	"U Total (1/cm3s)",
	"U SHR (1/cm3s)",
	"U B-B (1/cm3s)",
	"",
	"U Aug (1/cm3s)",
	"G Opt (1/cm3s)",
	"",
	"U Stim (1/cm3s)",
	"",
	"Total Rad Heat (J/cm3s)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Total B-B Heat(J/cm3s)",
	"Total Opt Gen Heat(J/cm3s)",
	"Total Stim Heat (J/cm3s)",
	"",
	"",
	"Total Lat Energy(J/cm3s)"
#ifndef NDEBUG
	"",
#endif
};

//***************************** QW Electron Strings *******************************************

char *qw_electron_long_table[]= {
	"Electron 2D QW DOS, Non-Equilibrium (cm-2)",
	"Electron 2D QW DOS, Equilibrium (cm-2)",
	"",
	"Electron 2D Concentration (cm-2)",
	"Electron QW Energy Top (eV)",
	"Electron QW Energy Level (eV)",
	"Electron QW Stimulated Emission Factor (eV)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Electron QW Auger Coefficient (cm4 s-1)",
#ifndef NDEBUG
	"","","","","","","","","","",
#endif
};

char *qw_electron_short_table[]= {
	"Non-Equil 2D n QW DOS (cm-2)",
	"Equil 2D n QW DOS (cm-2)",
	"",
	"2D n QW (cm-2)",
	"QW n Energy Top (eV)",
	"QW n Energy Level (eV)",
	"QW n Stim Fact (eV)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "QW n Aug Coef (cm4 s-1)",
#ifndef NDEBUG
	"","","","","","","","","","",
#endif
};

//*********************************** QW Hole Strings *****************************************

char *qw_hole_long_table[]= {
	"Hole 2D QW DOS, Non-Equilibrium (cm-2)",
	"Hole 2D QW DOS, Equilibrium (cm-2)",
	"",
	"Hole 2D Concentration (cm-2)",
	"Hole QW Energy Top (eV)",
	"Hole QW Energy Level (eV)",
	"Hole QW Stimulated Emission Factor (eV)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hole QW Auger Coefficient (cm4 s-1)",
#ifndef NDEBUG
	"","","","","","","","","","",
#endif
};

char *qw_hole_short_table[]= {
	"Non-Equil 2D p QW DOS (cm-2)",
	"Equil 2D p QW DOS (cm-2)",
	"",
	"2D p QW (cm-2)",
	"QW p Energy Top (eV)",
	"QW p Energy Level (eV)",
	"QW p Stim Fact (eV)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "QW p Aug Coef (cm4 s-1)",
#ifndef NDEBUG
	"","","","","","","","","","",
#endif
};

//******************************* QuantumWell Strings *****************************************

char *quantum_well_long_table[]= {
	"QW Position (microns)",
	"",
	"",
	"Number Nodes",
	"QW Length (microns)",
	"",
	"QW Band Gap (eV)",
	"QW Intrinsic Concentration (cm-2)",
	"Overlap Integral",
	"",
	"QW SHR Recombination (1/cm2s)",
	"QW Bimolecular Recombination (1/cm2s)",
	"QW Bimolecular Recombination Constant (cm2/s)",
	"QW Auger Recombination (1/cm2s)",
	"",
	"QW Incident Absorption",
	"",
	"",
	"QW Mode Absorption",
	"QW Gain",
#ifndef NDEBUG
	"","","","","","","","","","","","",
#endif
};

char *quantum_well_short_table[]= {
	"QW Pos (microns)",
	"",
	"",
	"# Nodes",
	"QW Length (microns)",
	"",
	"QW Eg (eV)",
	"QW ni (cm-2)",
	"Overlap Int",
	"",
	"QW U SHR (1/cm2s)",
	"QW U B-B (1/cm2s)",
	"QW B-B Constant (cm2/s)",
	"QW U Aug (1/cm2s)",
	"",
	"QW Inc Abs",
	"",
	"",
	"QW Mode Abs",
	"QW Gain",
#ifndef NDEBUG
	"","","","","","","","","","","","",
#endif
};


//************************************** Mode Strings *****************************************

char *mode_long_table[]= {
	"Mode Photon Lifetime (s)",
	"",
	"Mirror Loss (cm-1)",
	"Waveguide Loss (cm-1)",
	"Total Spontaneous Emission (s-1)",
	"",
	"Spontaneous Factor",
	"",
	"Mode Photon Energy (eV)",
	"Mode Photon Wavelength (microns)",
	"",
	"Mode Total Photons",
	"Mode Group Velocity (cm/s)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Mode Gain (cm-1)",
#ifndef NDEBUG
	"","","","","","","","","","","","",
#endif
};

char *mode_short_table[]= {
	"Tau Phot (s)",
	"",
	"Mir Loss (cm-1)",
	"Wavegde Loss (cm-1)",
	"Total Spont Emis (s-1)",
	"",
	"Spont Fact",
	"",
	"Phot Engy (eV)",
	"Phot Wavelgth (microns)",
	"",
	"Total Phot",
	"Grp Vel (cm/s)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Mode Gain (cm-1)",
#ifndef NDEBUG
	"","","","","","","","","","","","",
#endif
};

//************************************* Mirror Strings *****************************************

char *mirror_long_table[]= {
	"Mirror Position (microns)",
	"Mirror Type",
	"",
	"Mirror Reflectivity",
	"Optical Power (mW)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *mirror_short_table[]= {
	"Mir Pos (microns)",
	"Mir Type",
	"",
	"Mir R",
	"Opt Pwr (mW)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//************************************* Cavity Strings *****************************************

char *cavity_long_table[]= {
	"Cavity Area (microns2)",
	"Cavity Type",
	"",
	"",
	"Cavity Length (mircons)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *cavity_short_table[]= {
	"Cav A (microns2)",
	"Cav Type",
	"",
	"",
	"Cav L (mircons)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//*********************************** Contact Strings *****************************************

char *contact_long_table[]= {
	"Contact Position (microns)",
	"",
	"",
	"Contact Current, Total (A/cm2)",
	"Contact Electron Current (A/cm2)",
	"Contact Electron Recombination Velocity (cm/s)",
	"Contact Equil Electron Conc. (cm-3)",
	"",
	"Contact Potential (V)",
	"Contact Built in Potential (V)",
	"Contact Field (V/cm)",
	"",
	"Contact Hole Current (A/cm2)",
	"Contact Hole Recombination Velocity (cm/s)",
	"Contact Equil Hole Conc (cm-3)",
    "Contact Barrier Height (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","",
#endif
};

char *contact_short_table[]= {
	"Cont Pos (microns)",
	"",
	"",
	"Cont J Total (A/cm2)",
	"Cont Jn (A/cm2)",
	"Cont n Recomb Vel (cm/s)",
	"Cont Equil n (cm-3)",
	"",
	"Cont Pot (V)",
	"Cont Built Pot (V)",
	"Cont Field (V/cm)",
	"",
	"Cont Jp (A/cm2)",
	"Cont p Recomb Vel (cm/s)",
	"Cont Equil p (cm-3)",
    "Cont Bar Height (eV)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","",
#endif
};

//************************************ Surface Strings *****************************************

char *surface_long_table[]= {
	"Surface Position (microns)",
	"",
	"",
	"",
	"Surface Electron Temperature (K)",
	"Surface Lattice Temperature (K)",
	"Surface Hole Temperature (K)",
	"Surface Incident Total Poynting (mW/cm2)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Surface Incident Permitivity, Real",
	"Surface Thermal Conductivity (W/cm2K)",
	"Surface Incident Total Field Magnitude (a.u.)",
	"Surface Mode Permitivity, Real",
	"",
	"",
	"Surface Mode Total Field Magnitude (micron-3/2)",
	"Surface Incident Forward Field, Real (a.u.)",
	"Surface Incident Forward Field, Imaginary (a.u.)",
	"Surface Incident Forward Poynting (mW/cm2)",
	"Surface Incident Reverse Field, Real (a.u.)",
	"Surface Incident Reverse Field, Imaginary (a.u.)",
	"Surface Incident Reverse Poynting (mW/cm2)",
	"Surface Mode Forward Field, Real (a.u.)",
	"Surface Mode Forward Field, Imaginary (a.u.)",
	"Surface Mode Reverse Field, Real (a.u.)",
	"Surface Mode Reverse Field, Imaginary (a.u.)",
#ifndef NDEBUG
	"",
#endif
};

char *surface_short_table[]= {
	"Sur Pos (microns)",
	"",
	"",
	"",
	"Sur Elec Temp (K)",
	"Sur Lat Temp (K)",
	"Sur Hole Temp (K)",
	"Sur Inc Tot Poynt (mW/cm2)",
	"",
	"",
	"",
	"",
	"",
	"",
	"Sur Inc Perm Real",
	"Sur Therm Cond (W/cm2K)",
	"Sur Inc Tot Field Mag (a.u.)",
	"Sur Mode Perm Real (a.u.)",
	"",
	"",
	"Sur Mode Tot Field Mag (micr-3/2)",
	"Sur Inc For Field Real (a.u.)",
	"Sur Inc For Field Imag (a.u.)",
	"Sur Inc For Poynt (mW/cm2)",
	"Sur Inc Rev Field Real (a.u.)",
	"Sur Inc Rev Field Imag (a.u.)",
	"Sur Inc Rev Poynt (mW/cm2)",
	"Sur Mode For Field Real (a.u.)",
	"Sur Mode For Field Imag (a.u.)",
	"Sur Mode Rev Field Real (a.u.)",
	"Sur Mode Rev Field Imag (a.u.)",
#ifndef NDEBUG
	"",
#endif
};

//************************************ Device Strings *****************************************

char *device_long_table[]= {
	"",
	"",
	"Present Solution",
	"Present Status",
	"Error Psi",
	"Error eta c",
	"Error eta v",
	"Error T",
	"Error Lambda",
	"Error S",
	"Inner Electrical Iteration",
	"Inner Thermal Iteration",
	"Inner Mode Iteration",
	"Outer Optical Iteration",
	"Outer Thermal Iteration",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","",
#endif
};

char *device_short_table[]= {
	"",
	"",
	"Pres Sol",
	"Pres Stat",
	"Err Psi",
	"Err eta c",
	"Err eta v",
	"Err T",
	"Err Lamda",
	"Err S",
	"In Elect Iter",
	"In Therm Iter",
	"In Mode Iter",
	"Out Optic Iter",
	"Out Therm Iter",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","",
#endif
};

//******************************* Environment Strings *****************************************

char *environment_long_table[]= {
	"Potential Clamp Value",
	"",
	"Spectrum Start Position (microns)",
	"Spectrum End Position (microns)",
	"Spectrum Multiplier",
	"Environment Temperature (K)",
	"Max Electrical Error",
	"Max Thermal Error",
	"Max Optic Error",
	"Coarse Laser Mode Error (microns)",
	"Fine Laser Mode Error (microns)",
	"Environment Radius (microns)",
	"Max Inner Electrical Iteration",
	"Max Inner Thermal Iteration",
	"Max Outer Optical Iteration",
	"Max Outer Thermal Iteration",
	"Max Mode Iteration",
	"Temperature Clamp Value",
	"Temperature Relaxation Value",
#ifndef NDEBUG
	"","","","","","","","","","","","","",
#endif
};

char *environment_short_table[]= {
	"Pot Clamp Value",
	"",
	"Spec Start Pos (microns)",
	"Spec End Pos (microns)",
	"Spec Mult",
	"Env Temp (K)",
	"Max Elec Error",
	"Max Therm Error",
	"Max Optic Error",
	"Coarse Mode Error (microns)",
	"Fine Mode Error (microns)",
	"Env Radius (microns)",
	"Max In Elect Iter",
	"Max In Therm Iter",
	"Max Out Optic Iter",
	"Max Out Therm Iter",
	"Max Mode Iter",
	"Temp Clamp Value",
	"Temp Relax Value",
#ifndef NDEBUG
	"","","","","","","","","","","","","",
#endif
};

//*********************************** Spectrum Strings *****************************************

char *spectrum_long_table[]= {
	"",
	"",
	"",
	"",
	"",
	"Spectrum Photon Energy (eV)",
	"Spectrum Wavelength (microns)",
	"Spectrum Incident Intensity (mW/cm2)",
	"Spectrum Emitted Intensity (mW/cm2)",
	"Spectrum Reflected Intensity (mW/cm2)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","",
#endif
};

char *spectrum_short_table[]= {
	"",
	"",
	"",
	"",
	"",
	"Spec Phot Engy (eV)",
	"Spec Wavelgth (microns)",
	"Spec Inc I (mW/cm2)",
	"Spec Emit I (mW/cm2)",
	"Spec Ref I (mW/cm2)",
#ifndef NDEBUG
	"","","","","","","","","","","","","","","","","","","","","","",
#endif
};

//*********************************** String Tables *******************************************

char **long_string_table[]={free_electron_long_table,
							bound_electron_long_table,
							free_hole_long_table,
							bound_hole_long_table,
							electron_long_table,
							hole_long_table,
							grid_electrical_long_table,
							grid_optical_long_table,
							node_long_table,
							qw_electron_long_table,
							qw_hole_long_table,
							quantum_well_long_table,
							mode_long_table,
							mirror_long_table,
							cavity_long_table,
							contact_long_table,
							surface_long_table,
							device_long_table,
							environment_long_table,
							spectrum_long_table };

char **short_string_table[]={free_electron_short_table,
							bound_electron_short_table,
							free_hole_short_table,
							bound_hole_short_table,
							electron_short_table,
							hole_short_table,
							grid_electrical_short_table,
							grid_optical_short_table,
							node_short_table,
							qw_electron_short_table,
							qw_hole_short_table,
							quantum_well_short_table,
							mode_short_table,
							mirror_short_table,
							cavity_short_table,
							contact_short_table,
							surface_short_table,
							device_short_table,
							environment_short_table,
							spectrum_short_table };


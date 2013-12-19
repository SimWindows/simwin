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

//********************************* General Class Declarations *********************************
class TFreeElectron;
class TBoundElectron;
class TElectron;
class TFreeHole;
class TBoundHole;
class THole;
class TGrid;
class TNode;
class T2DElectron;
class T2DHole;
class TQuantumWell;
class TMode;
class TMirror;
class TCavity;
class TContact;
class TSurface;
class TDevice;
class TElectricalServices;
class TElement;
class TElectricalElement;
class TBulkElectricalElement;
class TQWElectricalElement;
class TOhmicBoundaryElement;
class TThermalElement;
class TBulkThermalElement;
class TQWThermalElement;
class TBoundaryThermalElement;
class TSolution;

class TEnvironment;
extern TEnvironment environment;

class TFlag;
class TValueFlag;
class TValueFlagWithObject;
class TEffectFlag;
class TDeviceFileInput;
class TParse;
class TParseMaterialParam;
class TParseDevice;
class TParseMaterial;
class TFunction;
class TConstant;
class TUserFunction;
class TTermsFunction;
class TPolynomial;
class TModelBandGap;
class TModelThermalConductivity;
class TModelPowerAbsorption;
class TModelPowerAbsorptionBandGap;
class TModelExpAbsorption;
class TModelExpAbsorptionBandGap;
class TModelAlGaAsPermitivity;
class TModelAlGaAsRefractiveIndex;
class TModelAlGaAsAbsorption;
class TPieceWiseFunction;
class TMaterialParamModel;
class TAlloy;
class TMaterial;

class TMaterialStorage;
extern TMaterialStorage material_parameters;

class TErrorHandler;
extern TErrorHandler error_handler;

class TPreferences;
extern TPreferences preferences;

class TMacro;
class TVoltageMacro;
class TMacroStorage;
extern TMacroStorage macro_storage;

//********************************* Device input structures ************************************

struct GridInput {
	prec length;
	short number_points;
};

struct DopingInput {
	prec length;
	TFunction *acceptor_function;
	TFunction *donor_function;
	long acceptor_degeneracy;
	prec acceptor_level;
	long donor_degeneracy;
    prec donor_level;
};

struct StructureInput {
	MaterialType material_type;
	prec length;
	AlloyType alloy_type;
	TFunction *alloy_function;
};

struct RegionInput {
	RegionType type;
	prec length;
};

struct CavityInput {
	CavityType type;
	prec length;
	prec area;
};

struct MirrorInput {
	MirrorType type;
	prec position;
	prec reflectivity;
};

struct MaterialParamInput {
	prec length;
	TMaterialParamModel *material_model;
};

//************************************** Physics Structures ************************************

struct MaterialSpecification {
	MaterialType material_type;
	AlloyType alloy_type;
	prec alloy_conc;
};

struct Recombination {
	prec total;
	prec b_b;
	prec shr;
    prec auger;
	prec stim;
	prec opt_gen;
};

struct RadiativeHeat {
	prec total;
	prec stim;
	prec opt_gen;
	prec b_b;
};

struct Hotcarriers {
	prec total;
	prec stim;
	prec b_b;
	prec shr;
    prec auger;
	prec relax;
	prec opt_kin;
	prec opt_ref;
};

struct NormalizeConstants {
	prec charge;
	prec temp;
	prec pot;
	prec energy;
	prec conc;
	prec length;
	prec mobility;
	prec time;
	prec recomb;
	prec therm_cond;
	prec field;
	prec current;
	prec intensity;
};
extern NormalizeConstants normalization;

struct FundamentalParam {
	prec psi;
	prec eta_c;
	prec eta_v;
};

struct RecombDerivParam {
	prec eta_c;
	prec eta_v;
};

struct ElecDriftDiffParam {
	prec fermi_ratio_1_half_minus_1_half;
	prec fermi_ratio_3_half_1_half_col;
	prec bernoulli_param;
	prec bernoulli_grad_temp_next;
	prec bernoulli_grad_temp_prev;
	prec deriv_bern_grad_temp_next;
	prec deriv_bern_grad_temp_prev;
};

struct ThermDriftDiffParam {
	prec fermi_ratio_5_half_3_half_col;
	prec bernoulli_param;
	prec bernoulli_grad_temp_next;
	prec bernoulli_grad_temp_prev;
};

struct ThermEmisParam {
	prec min_transmit_energy;
	prec band_discont;
	prec barrier_next;
	prec barrier_prev;
	prec richardson_const;
};

struct OpticalComponent {
	prec energy;
	prec input_intensity;
	prec output_intensity;
	prec reflected_intensity;
	OpticalComponent *next_wavelength;
};

struct Spectrum {
	OpticalComponent *first_wavelength;
	OpticalComponent *last_wavelength;
};

struct OpticalParam {
	short number_wavelengths;
	prec start_pos;
	prec end_pos;
};

struct ComplexRefractiveIndex {
	prec real_part;
	prec absorption;
	prec local_gain;
};

struct OpticalField {
	complex forward_field;
	prec forward_poynting;
	complex reverse_field;
	prec reverse_poynting;
	prec total_poynting;
    prec total_magnitude;
    int overflow_count;
};

struct QuantumWellNodes {
	TNode *prev_node_ptr;
	TNode *curr_node_ptr;
	TNode *next_node_ptr;
	int prev_node;
	int curr_node;
	int next_node;
};

//************************************ Plotting Structures ************************************

struct Axis {
	float maximum;
	float minimum;
	float maj_tic;
	float min_tic;
	float norm;
	logical auto_scale;
	logical auto_tic;
	AxisType scale_type;
	OperationType operation_type;
};
//*************************************** Misc. Structures ***********************************

struct MacroEntry {
	TMacro *macro;
	MacroEntry *next_entry;
};

struct ObjectEntry {
	int object;
	ObjectEntry *next_entry;
};



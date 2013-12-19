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

class TDialogOpticalInput : public TDialog {
protected:
	TCheckBox *IdcOn;
	TCheckBox *IdcOff;
	TCheckBox *IdcEntireDevice;
	TCheckBox *IdcLeftIncident;
	TCheckBox *IdcRightIncident;
	TStatic *IdcTextStartPosition;
	TEdit *IdcStartPosition;
	TStatic *IdcTextEndPosition;
	TEdit *IdcEndPosition;
	TCheckBox *IdcInterfaceReflection;
	TCheckBox *IdcClearInput;
	TEdit *IdcEditEnergy;
	TCheckBox *IdcCheckEnergy;
	TCheckBox *IdcWavelength;
	TEdit *IdcIntensity;
	TStatic *IdcTextFilename;
	TEdit *IdcSpectrumMultiplier;

	flag environment_effects;
	int number_components;

	logical calc_wavelength;
	logical light_entered;
	logical load_spectrum;
	string new_spectrum_filename;
	static TOpenSaveDialog::TData FileData;
	static string spectrum_filename;
	static logical energy_selected;
public:
	TDialogOpticalInput(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogOpticalInput);
	void CmOk(void);
	void CmButtonFilename(void);
	void EvClearInput(void);
	void EvEntireDevice(void);
	void EvEnergy(void);
	void EvWavelength(void);
	void EvSingleComponent(void);
};

class TDialogDeviceContacts : public TDialog {
protected:
	TStatic *IdcLPosition;
	TEdit *IdcLBias;
	TCheckBox *IdcLOhmic;
	TCheckBox *IdcLFiniteRecomb;
	TEdit *IdcLElectronVelocity;
	TEdit *IdcLHoleVelocity;
    TCheckBox *IdcLSchottky;
    TEdit *IdcLBarrierHeight;

	TStatic *IdcRPosition;
	TEdit *IdcRBias;
	TCheckBox *IdcROhmic;
	TCheckBox *IdcRFiniteRecomb;
	TEdit *IdcRElectronVelocity;
	TEdit *IdcRHoleVelocity;
    TCheckBox *IdcRSchottky;
    TEdit *IdcRBarrierHeight;

	flag left_contact_effects;
	flag right_contact_effects;

public:
	TDialogDeviceContacts(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogDeviceContacts);
	void CmOk(void);
};

class TDialogDeviceSurfaces : public TDialog {
protected:
	TStatic *IdcLSurfacePosition;
	TEdit *IdcLOpticalPermitivity;
	TEdit *IdcLTemp;
	TEdit *IdcLElectronTemp;
	TCheckBox *IdcLHeatSink;
	TCheckBox *IdcLFiniteCond;
	TEdit *IdcLThermCond;

	TStatic *IdcRSurfacePosition;
	TEdit *IdcROpticalPermitivity;
	TEdit *IdcRTemp;
	TEdit *IdcRElectronTemp;
	TCheckBox *IdcRHeatSink;
	TCheckBox *IdcRFiniteCond;
	TEdit *IdcRThermCond;

	flag left_surface_effects;
	flag right_surface_effects;

public:
	TDialogDeviceSurfaces(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogDeviceSurfaces);
	void CmOk(void);
};

class TDialogElectricalModels : public TDialog {
protected:
	TCheckBox *IdcBoltzmann;
	TCheckBox *IdcFermiDirac;
	TCheckBox *IdcIncompleteIonization;
	TCheckBox *IdcQWFermiDirac;
	TCheckBox *IdcBound;
	TCheckBox *IdcFree;
	TCheckBox *IdcQWInfinite;
	TCheckBox *IdcQWFinite;
	TCheckBox *IdcSHR;
	TCheckBox *IdcBimolecular;
    TCheckBox *IdcAuger;
	TCheckBox *IdcStimulated;
	TCheckBox *IdcOpticalGeneration;
	TCheckBox *IdcDriftDiffusion;
	TCheckBox *IdcThermionic;
	TCheckBox *IdcTunneling;
	TCheckBox *IdcInternalCriteria;
	TCheckBox *IdcAbruptMaterials;
	TCheckBox *IdcDopingDepMobility;

	flag grid_effects;
	flag device_effects;
	flag environment_effects;
	int number_components;
	logical quantum_wells;
	flag qw_effects;
public:
	TDialogElectricalModels(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogElectricalModels);
	void CmOk(void);
};

class TDialogThermalModels : public TDialog {
protected:
	TCheckBox *IdcIsothermal;
	TCheckBox *IdcSingleTemp;
	TCheckBox *IdcSeparateTemp;
	TCheckBox *IdcElectronTemp;
	TCheckBox *IdcJoule;
	TCheckBox *IdcThermoelectric;
	TCheckBox *IdcLateral;
	TCheckBox *IdcCarrierRelax;
	TCheckBox *IdcBandGap;
	TCheckBox *IdcTempDepMobility;
	TCheckBox *IdcTempDepElectronAffinity;
	TCheckBox *IdcThermalConduct;
	TCheckBox *IdcIncPerm;
	TCheckBox *IdcModePerm;

	flag grid_effects;
	flag device_effects;
public:
	TDialogThermalModels(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogThermalModels);
	void CmOk(void);
	void EvIsothermal(void);
	void EvSingleTemp(void);
	void EvSeparateTemp(void);
};

class TDialogLaser : public TDialog {
protected:
	TEdit *IdcCavityLength;
	TEdit *IdcCavityArea;
	TEdit *IdcRef1;
	TStatic *IdcPower1;
	TEdit *IdcRef2;
	TStatic *IdcPower2;
	TEdit *IdcSpontFactor;
	TEdit *IdcWaveguideLoss;
	TEdit *IdcWavelength;
	TCheckBox *IdcCompModeField;
	TCheckBox *IdcSearchWavelength;
	TStatic *IdcTextFilename;
	TStatic *IdcModeGain;
	TStatic *IdcMirrorLoss;
	TStatic *IdcGroupVelocity;
	TStatic *IdcPhotonLifetime;
	TStatic *IdcPhotonNumber;

	flag mode_effects;
	logical load_mode;
	static TOpenSaveDialog::TData FileData;
	static string mode_filename;

public:
	TDialogLaser(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogLaser);
	void CmOk(void);
	void CmButtonFilename(void);
};

class TDialogWriteBand : public TDialog {
protected:
	TCheckBox *IdcEVac;
	TCheckBox *IdcEc;
	TCheckBox *IdcEv;
	TCheckBox *IdcEfn;
	TCheckBox *IdcEfp;
	FlagType& reference_flag_type;
	flag& reference_flag;
public:
	TDialogWriteBand(TWindow *parent, TResId resId, FlagType& new_flag_type, flag& new_reference_flag,
					 TModule *module=0);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogWriteBand);
	void CmOk(void);
};

class TDialogDataWriteAll : public TDialog {
protected:
	TCheckBox *IdcPositionDep;
	TCheckBox *IdcQWDep;
	TCheckBox *IdcModeDep;
	TCheckBox *IdcMirrorDep;
	TCheckBox *IdcCavityDep;
	TCheckBox *IdcContactDep;
	TCheckBox *IdcSurfaceDep;
	TCheckBox *IdcDeviceDep;
	TCheckBox *IdcSpectrumDep;
	TCheckBox *IdcEnvDep;
	static TOpenSaveDialog::TData FileData;

public:
	TDialogDataWriteAll(TWindow *parent, TResId resId, TModule *module=0);

protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogDataWriteAll);
	void CmOk(void);
};



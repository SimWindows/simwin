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

class TDialogSelectOneParameter : public TDialog {
protected:
	TListBox *IdcUnselected;
	TValueFlag& total_flags;
	FlagType &selected_flag_type;
	flag &selected_flag;
public:
	TDialogSelectOneParameter(TWindow *parent, TResId resId, TValueFlag& new_total,
							  FlagType &new_selected_type, flag &new_selected_flag,
							  TModule *module=0);
protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogSelectOneParameter);
	void CmOk(void);
};

class TDialogSelectParameters : public TDialog {
protected:
	TListBox *IdcUnselected;
	TListBox *IdcSelected;

	TValueFlag& total_flags;
	TValueFlag& selected_flags;

	int max_strings;

public:
	TDialogSelectParameters(TWindow *parent, TResId resId, TValueFlag& new_total,
							TValueFlag& new_selected, TModule *module=0);
protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogSelectParameters);
	void CmOk(void);
	void CmAdd(void);
	void CmRemove(void);
};

class TDialogEnvironmentPreferences: public TDialog {
protected:
	TCheckBox *IdcToolBar;
	TCheckBox *IdcStatusBar;
	TCheckBox *IdcSimulationUndo;
	TEdit *IdcMaterialFile;
	TEdit *IdcWriteMultiplier;
    TCheckBox *IdcSingleThread;
    TCheckBox *IdcMultiThread;
    THSlider *IdcPriority;

	flag environment_effects;

public:
	TDialogEnvironmentPreferences(TWindow *parent, TResId resId, TModule *module=0);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogEnvironmentPreferences);
	void CmOk(void);
    void EvSingleThread(void) { IdcPriority->EnableWindow(FALSE); }
    void EvMultiThread(void) { IdcPriority->EnableWindow(TRUE); }
};

class TDialogSimulationPreferences: public TDialog {
protected:
	TCheckBox *IdcClampPot;
	TEdit *IdcClampValue;
	TCheckBox *IdcClampTemp;
	TEdit *IdcTempClampValue;
	TEdit *IdcTempRelaxValue;
	TEdit *IdcMaxElectError;
	TEdit *IdcMaxOpticError;
	TEdit *IdcMaxThermError;
	TEdit *IdcCoarseModeError;
	TEdit *IdcFineModeError;
	TEdit *IdcInnerElectrical;
	TEdit *IdcInnerThermal;
	TEdit *IdcInnerMode;
	TEdit *IdcOuterOptical;
	TEdit *IdcOuterThermal;

	flag environment_effects;

public:
	TDialogSimulationPreferences(TWindow *parent, TResId resId, TModule *module=0);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogSimulationPreferences);
	void CmOk(void);
};


class TDialogSelectMacro : public TDialog {
protected:
	TListBox *IdcSelectMacro;
	const char *caption;
	int &macro_number;
	logical with_data;
public:
	TDialogSelectMacro(TWindow *parent, TResId resId, int &new_number, const char *new_caption,
					   logical new_with_data=FALSE, TModule *module=0);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogSelectMacro);
	void CmOk(void);
};


class TDialogVoltageMacro : public TDialog {
protected:
	TEdit *IdcMacroName;
	TEdit *IdcStartValue;
	TEdit *IdcEndValue;
	TEdit *IdcIncrement;
	TCheckBox *IdcLeftContact;
	TCheckBox *IdcRightContact;
	TCheckBox *IdcInitialState;
	TCheckBox *IdcPresentState;

	TVoltageMacro *macro;
	TValueFlagWithObject record_flags;
public:
	TDialogVoltageMacro(TWindow *parent, TResId resId, TVoltageMacro *new_macro,
						TModule *module=0);
protected:
	void SetupWindow(void);

	DECLARE_RESPONSE_TABLE(TDialogVoltageMacro);
	void CmOk(void);
	void CmButtonSimple(void);
};

class TDialogDeviceInfo : public TDialog {
protected:
	TStatic *IdcGridPoints;
	TStatic *IdcCurrentSolution;
	TStatic *IdcCurrentStatus;
	TStatic *IdcBuiltPotential;
	TStatic *IdcAppliedBias;
	TStatic *IdcTotalCurrent;
public:
	TDialogDeviceInfo(TWindow *parent, TResId resId, TModule *module=0);
protected:
	void SetupWindow(void);
};

class TDialogScale : public TDialog {
protected:
	TEdit *IdcXMin;
	TEdit *IdcXMax;
	TCheckBox *IdcXAuto;
	TCheckBox *IdcXLinear;
	TEdit *IdcYMin;
	TEdit *IdcYMax;
	TCheckBox *IdcYAuto;
	TCheckBox *IdcYLinear;
	TCheckBox *IdcYLog;
	TCheckBox *IdcYNormal;
	TCheckBox *IdcYNegative;
	TCheckBox *IdcYAbsolute;
	Axis& x_axis;
	Axis& y_axis;
public:
	TDialogScale(TWindow *parent, TResId resId, Axis& x, Axis& y, TModule *module=0);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogScale);
	void CmOk(void);
	void EvXScale(void)	{ IdcXAuto->Uncheck(); }
	void EvYScale(void)	{ IdcYAuto->Uncheck(); }
};

class TDialogTrace : public TDialog {
protected:
	TStatic *IdcString[MAX_NUMBER_PLOTS+1];
	TStatic *IdcNumber[MAX_NUMBER_PLOTS+1];

	int number_labels;
	string labels[MAX_NUMBER_PLOTS+1];
	int number_values;
	float values[MAX_NUMBER_PLOTS+1];

public:
	TDialogTrace(TWindow *parent, TResId resId, TModule *module=0);
	void update_labels(string *label_string, int number);
	void update_values(float *value_float, int number);
	void redraw_labels(void);
	void redraw_values(void);

protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TDialogTrace);
	void EvClose(void) { Show(SW_HIDE); }
	void CmIdCancel(void) {}
};

class TDialogAbout : public TDialog {
protected:
	TStatic *IdcAboutText;

public:
	TDialogAbout(TWindow *parent, TResId resId, TModule *module=0)
		: TDialog(parent,resId,module)
	{ IdcAboutText=new TStatic(this,IDC_ABOUTTEXT); }

protected:
	void SetupWindow(void);
};


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

// Standard Include files for all Windows functions
#include "comincl.h"
#include <owl\owlpch.h>
#include <owl\inputdia.h>
#include <owl\checkbox.h>
#include <owl\listbox.h>
#include <owl\validate.h>
#include <owl\editfile.h>
#include <owl\buttonga.h>
#include <owl\slider.h>
#include <owl\controlb.h>
#include <owl\statusba.h>
#include <owl\printer.h>
#include "winfunc.h"
#include "simwin.rh"
#include "simmac.h"
#include "simcdial.h"
#include "simdial.h"
#include "simvalid.h"
#include "simapp.h"
#include "simclien.h"
#include "simedit.h"
#include "simplot.h"
// End of standard include files. Precompiled headers stop here

//****************************** class TDialogSelectOneParameter *******************************
/*
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
*/

TDialogSelectOneParameter::TDialogSelectOneParameter(TWindow *parent, TResId resId,
													 TValueFlag& new_total,
													 FlagType &new_selected_type,
													 flag &new_selected_flag,
													 TModule *module)
	: TDialog(parent,resId,module), total_flags(new_total),
	  selected_flag_type(new_selected_type), selected_flag(new_selected_flag)
{
	IdcUnselected=new TListBox(this, IDC_UNSELECTED);
}

void TDialogSelectOneParameter::SetupWindow(void)
{
	int j,flag_type;
	flag test_flag, valid_flags;

	TDialog::SetupWindow();

	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (total_flags.any_set((FlagType)flag_type)) {
			test_flag=1;
			valid_flags=total_flags.get_valid((FlagType)flag_type);
			for (j=0;j<sizeof(flag)*8;j++) {
				if ((test_flag & valid_flags) && total_flags.is_set((FlagType)flag_type,test_flag)) {
					IdcUnselected->AddString(get_long_string((FlagType)flag_type,test_flag).c_str());
				}
				test_flag<<=1;
			}
		}
	}
}

DEFINE_RESPONSE_TABLE1(TDialogSelectOneParameter,TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogSelectOneParameter::CmOk(void)
{
	char selected_string[100];
	int number_selected;

	number_selected=IdcUnselected->GetSelIndex();

	selected_flag_type=(FlagType)0;
	selected_flag=(flag)0;

	if (number_selected>=0) {
		IdcUnselected->GetSelString(selected_string,sizeof(selected_string));
		long_string_to_flag(selected_string,selected_flag_type,selected_flag);
	}

	TDialog::CmOk();
}

//******************************* class TDialogSelectParameters *******************************
/*
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
*/

TDialogSelectParameters::TDialogSelectParameters(TWindow *parent, TResId resId,
												 TValueFlag& new_total, TValueFlag& new_selected,
												 TModule *module)
	: TDialog(parent,resId,module), total_flags(new_total), selected_flags(new_selected)
{
	max_strings=0;
	IdcUnselected=new TListBox(this, IDC_UNSELECTED);
	IdcSelected=new TListBox(this, IDC_SELECTED);
}

void TDialogSelectParameters::SetupWindow(void)
{
	int j,flag_type;
	flag test_flag, valid_flags;
	TValueFlag unselected_flags(total_flags), clear_flags(selected_flags);

	TDialog::SetupWindow();

	unselected_flags&=(~clear_flags);

	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (unselected_flags.any_set((FlagType)flag_type)) {
			test_flag=1;
			valid_flags=unselected_flags.get_valid((FlagType)flag_type);
			for (j=0;j<sizeof(flag)*8;j++) {
				if ((valid_flags & test_flag) && unselected_flags.is_set((FlagType)flag_type,test_flag)) {
					IdcUnselected->AddString(get_long_string((FlagType)flag_type,test_flag).c_str());
					max_strings++;
				}
				test_flag<<=1;
			}
		}
	}

	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (selected_flags.any_set((FlagType)flag_type)) {
			test_flag=1;
			valid_flags=selected_flags.get_valid((FlagType)flag_type);
			for (j=0;j<sizeof(flag)*8;j++) {
				if ((valid_flags & test_flag) && selected_flags.is_set((FlagType)flag_type,test_flag)) {
					IdcSelected->AddString(get_long_string((FlagType)flag_type,test_flag).c_str());
					max_strings++;
				}
				test_flag<<=1;
			}
		}
	}
}

DEFINE_RESPONSE_TABLE1(TDialogSelectParameters,TDialog)
	EV_COMMAND(IDC_ADD, CmAdd),
	EV_COMMAND(IDC_REMOVE, CmRemove),
	EV_COMMAND(IDOK, CmOk),
	EV_CHILD_NOTIFY(IDC_UNSELECTED,LBN_DBLCLK,CmAdd),
	EV_CHILD_NOTIFY(IDC_SELECTED,LBN_DBLCLK,CmRemove),
END_RESPONSE_TABLE;

void TDialogSelectParameters::CmOk(void)
{
	char selected_string[100];
	FlagType flag_type=(FlagType)0;
	flag flag_value=0;
	int i,number_selected;

	selected_flags.clear_all();
	number_selected=IdcSelected->GetCount();

	::SetCursor(TCursor(NULL,IDC_WAIT));
	for (i=0;i<number_selected;i++) {
		IdcSelected->GetString(selected_string,i);
		long_string_to_flag(selected_string,flag_type,flag_value);
		selected_flags.set(flag_type,flag_value);
	}
	::SetCursor(TCursor(NULL,IDC_ARROW));

	TDialog::CmOk();
}

void TDialogSelectParameters::CmAdd(void)
{
	char selected_string[100];
	int i;
	int *selected_indexes;
	int number_selected;

	number_selected=IdcUnselected->GetSelCount();

	if (number_selected>=1) {
		selected_indexes=new int[max_strings];
		IdcUnselected->GetSelIndexes(selected_indexes,max_strings);
		for (i=0;i<number_selected;i++) {
			IdcUnselected->GetString(selected_string,selected_indexes[i]);
			IdcSelected->AddString(selected_string);
		}
		for (i=0;i<number_selected;i++) IdcUnselected->DeleteString(selected_indexes[i]-i);
		delete[] selected_indexes;
	}
}

void TDialogSelectParameters::CmRemove(void)
{
	char selected_string[100];
	int i;
	int *selected_indexes;
	int number_selected;

	number_selected=IdcSelected->GetSelCount();

	if (number_selected>=1) {
		selected_indexes=new int[max_strings];
		IdcSelected->GetSelIndexes(selected_indexes,max_strings);
		for (i=0;i<number_selected;i++) {
			IdcSelected->GetString(selected_string,selected_indexes[i]);
			IdcUnselected->AddString(selected_string);
		}
		for (i=0;i<number_selected;i++) IdcSelected->DeleteString(selected_indexes[i]-i);
		delete[] selected_indexes;
	}
}

//*********************** class TDialogEnvironmentPreferences ***********************************
/*
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
    void EvSingleThread(void) { IdcThreadPriority->EnableWindow(FALSE); }
    void EvMultiThread(void) { IdcThreadPriority->EnableWindow(TRUE); }
};
*/

TDialogEnvironmentPreferences::TDialogEnvironmentPreferences(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcToolBar=new TCheckBox(this, IDC_TOOLBAR);
	IdcStatusBar=new TCheckBox(this, IDC_STATUSBAR);
	IdcSimulationUndo=new TCheckBox(this, IDC_SIMULATIONUNDO);
	IdcMaterialFile=new TEdit(this, IDC_MATERIALFILE);
	IdcWriteMultiplier=new TEdit(this,IDC_WRITEMULTIPLIER);
	IdcWriteMultiplier->SetValidator(new TRangeValidator(1,100));
    IdcSingleThread=new TCheckBox(this, IDC_SINGLETHREAD);
    IdcMultiThread=new TCheckBox(this, IDC_MULTITHREAD);
    IdcPriority=new THSlider(this, IDC_PRIORITY);

	environment_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);
}

void TDialogEnvironmentPreferences::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	if (preferences.is_toolbar()) IdcToolBar->SetCheck(BF_CHECKED);
	if (preferences.is_statusbar()) IdcStatusBar->SetCheck(BF_CHECKED);

	if (environment_effects & ENV_UNDO_SIMULATION) IdcSimulationUndo->SetCheck(BF_CHECKED);

	IdcMaterialFile->SetText(preferences.get_material_parameters_file().c_str());

	sprintf(number_string,"%d",preferences.get_write_grid_multiplier());
	IdcWriteMultiplier->SetText(number_string);

    if (preferences.is_multi_threaded()) {
    	IdcMultiThread->SetCheck(BF_CHECKED);
        IdcPriority->EnableWindow(TRUE);
    }
    else IdcSingleThread->SetCheck(BF_CHECKED);

    IdcPriority->SetRange(1,5);
    IdcPriority->SetRuler(1,TRUE);
	IdcPriority->SetPosition(preferences.get_thread_priority());
}

DEFINE_RESPONSE_TABLE1(TDialogEnvironmentPreferences, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_CHILD_NOTIFY(IDC_SINGLETHREAD, BN_CLICKED, EvSingleThread),
	EV_CHILD_NOTIFY(IDC_MULTITHREAD, BN_CLICKED, EvMultiThread),
END_RESPONSE_TABLE;

void TDialogEnvironmentPreferences::CmOk(void)
{
	char number_string[20];
	char file_string[20];

	if (IdcWriteMultiplier->IsValid()) {

		if (IdcToolBar->GetCheck()==BF_CHECKED) preferences.enable_toolbar(TRUE);
		else preferences.enable_toolbar(FALSE);

		if (IdcStatusBar->GetCheck()==BF_CHECKED) preferences.enable_statusbar(TRUE);
		else preferences.enable_statusbar(FALSE);

		if (IdcSimulationUndo->GetCheck()==BF_CHECKED) environment_effects|=ENV_UNDO_SIMULATION;
		else environment_effects&=(~ENV_UNDO_SIMULATION);

		IdcMaterialFile->GetText(file_string,sizeof(file_string));
		preferences.put_material_parameters_file(file_string);

		IdcWriteMultiplier->GetText(number_string,sizeof(number_string));
		preferences.put_write_grid_multiplier(atoi(number_string));

        if (IdcMultiThread->GetCheck()==BF_CHECKED) preferences.enable_multi_threaded(TRUE);
        else preferences.enable_multi_threaded(FALSE);

        preferences.put_thread_priority(IdcPriority->GetPosition());

		environment.put_value(ENVIRONMENT,EFFECTS,(prec)environment_effects);
	}

	TDialog::CmOk();
}

//************************** class TDialogSimulationPreferences ***********************************
/*
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
*/

TDialogSimulationPreferences::TDialogSimulationPreferences(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcClampPot=new TCheckBox(this,IDC_CLAMPPOT);
	IdcClampValue=new TEdit(this,IDC_CLAMPVALUE);
	IdcClampValue->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcClampTemp=new TCheckBox(this, IDC_CLAMPTEMP);
	IdcTempClampValue=new TEdit(this, IDC_TEMPCLAMPVALUE);
	IdcTempClampValue->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcTempRelaxValue=new TEdit(this, IDC_TEMPRELAXVALUE);
	IdcTempRelaxValue->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcMaxElectError=new TEdit(this,IDC_MAXELECTERROR);
	IdcMaxElectError->SetValidator(new TScientificLowerValidator(1e-13,INCLUSIVE));
	IdcMaxOpticError=new TEdit(this,IDC_MAXOPTICERROR);
	IdcMaxOpticError->SetValidator(new TScientificLowerValidator(1e-13,INCLUSIVE));
	IdcMaxThermError=new TEdit(this,IDC_MAXTHERMERROR);
	IdcMaxThermError->SetValidator(new TScientificLowerValidator(1e-13,INCLUSIVE));
	IdcCoarseModeError=new TEdit(this,IDC_COARSEMODEERROR);
	IdcCoarseModeError->SetValidator(new TScientificLowerValidator(1e-13,INCLUSIVE));
	IdcFineModeError=new TEdit(this,IDC_FINEMODEERROR);
	IdcFineModeError->SetValidator(new TScientificLowerValidator(1e-13,INCLUSIVE));
	IdcInnerElectrical=new TEdit(this,IDC_INNERELECTRICAL);
	IdcInnerElectrical->SetValidator(new TRangeValidator(1,100));
	IdcInnerThermal=new TEdit(this,IDC_INNERTHERMAL);
	IdcInnerThermal->SetValidator(new TRangeValidator(1,100));
	IdcInnerMode=new TEdit(this,IDC_INNERMODE);
	IdcInnerMode->SetValidator(new TRangeValidator(1,100));
	IdcOuterOptical=new TEdit(this,IDC_OUTEROPTICAL);
	IdcOuterOptical->SetValidator(new TRangeValidator(1,100));
	IdcOuterThermal=new TEdit(this,IDC_OUTERTHERMAL);
	IdcOuterThermal->SetValidator(new TRangeValidator(1,100));

	environment_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);
}

void TDialogSimulationPreferences::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	if (environment_effects & ENV_CLAMP_POTENTIAL) IdcClampPot->Check();
	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,POT_CLAMP_VALUE));
	IdcClampValue->SetText(number_string);

	if (environment_effects & ENV_CLAMP_TEMPERATURE) IdcClampTemp->Check();
	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,TEMP_CLAMP_VALUE));
	IdcTempClampValue->SetText(number_string);

	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,TEMP_RELAX_VALUE));
	IdcTempRelaxValue->SetText(number_string);

	sprintf(number_string,"%.3e",(float)environment.get_value(ENVIRONMENT,MAX_ELECTRICAL_ERROR));
	IdcMaxElectError->SetText(number_string);
	sprintf(number_string,"%.3e",(float)environment.get_value(ENVIRONMENT,MAX_THERMAL_ERROR));
	IdcMaxThermError->SetText(number_string);
	sprintf(number_string,"%.3e",(float)environment.get_value(ENVIRONMENT,MAX_OPTIC_ERROR));
	IdcMaxOpticError->SetText(number_string);
	sprintf(number_string,"%.3e",(float)environment.get_value(ENVIRONMENT,COARSE_MODE_ERROR));
	IdcCoarseModeError->SetText(number_string);
	sprintf(number_string,"%.3e",(float)environment.get_value(ENVIRONMENT,FINE_MODE_ERROR));
	IdcFineModeError->SetText(number_string);
	sprintf(number_string,"%d",(int)environment.get_value(ENVIRONMENT,MAX_INNER_ELECT_ITER));
	IdcInnerElectrical->SetText(number_string);
	sprintf(number_string,"%d",(int)environment.get_value(ENVIRONMENT,MAX_INNER_THERM_ITER));
	IdcInnerThermal->SetText(number_string);
	sprintf(number_string,"%d",(int)environment.get_value(ENVIRONMENT,MAX_INNER_MODE_ITER));
	IdcInnerMode->SetText(number_string);
	sprintf(number_string,"%d",(int)environment.get_value(ENVIRONMENT,MAX_OUTER_OPTIC_ITER));
	IdcOuterOptical->SetText(number_string);
	sprintf(number_string,"%d",(int)environment.get_value(ENVIRONMENT,MAX_OUTER_THERM_ITER));
	IdcOuterThermal->SetText(number_string);
}

DEFINE_RESPONSE_TABLE1(TDialogSimulationPreferences, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogSimulationPreferences::CmOk(void)
{
	char number_string[20];

	if (IdcClampValue->IsValid() && IdcMaxElectError->IsValid() &&
		IdcMaxThermError->IsValid() && IdcMaxOpticError->IsValid() && IdcCoarseModeError->IsValid() &&
		IdcFineModeError->IsValid() && IdcInnerElectrical->IsValid() && IdcInnerThermal->IsValid() &&
		IdcInnerMode->IsValid() && IdcOuterOptical->IsValid() && IdcOuterThermal->IsValid() &&
		IdcTempClampValue->IsValid() && IdcTempRelaxValue->IsValid()) {

		if (IdcClampPot->GetCheck()==BF_CHECKED) environment_effects|=ENV_CLAMP_POTENTIAL;
		else environment_effects&=(~ENV_CLAMP_POTENTIAL);
		IdcClampValue->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,POT_CLAMP_VALUE,atof(number_string));

		if (IdcClampTemp->GetCheck()==BF_CHECKED) environment_effects|=ENV_CLAMP_TEMPERATURE;
		else environment_effects&=(~ENV_CLAMP_TEMPERATURE);
		IdcTempClampValue->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,TEMP_CLAMP_VALUE,atof(number_string));

		IdcTempRelaxValue->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,TEMP_RELAX_VALUE,atof(number_string));

		IdcMaxElectError->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_ELECTRICAL_ERROR,atof(number_string));
		IdcMaxThermError->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_THERMAL_ERROR,atof(number_string));
		IdcMaxOpticError->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_OPTIC_ERROR,atof(number_string));
		IdcCoarseModeError->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,COARSE_MODE_ERROR,atof(number_string));
		IdcFineModeError->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,FINE_MODE_ERROR,atof(number_string));

		IdcInnerElectrical->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_INNER_ELECT_ITER,atof(number_string));
		IdcInnerThermal->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_INNER_THERM_ITER,atof(number_string));
		IdcInnerMode->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_INNER_MODE_ITER,atof(number_string));
		IdcOuterOptical->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_OUTER_OPTIC_ITER,atof(number_string));
		IdcOuterThermal->GetText(number_string,sizeof(number_string));
		environment.put_value(ENVIRONMENT,MAX_OUTER_THERM_ITER,atof(number_string));

		environment.put_value(ENVIRONMENT,EFFECTS,(prec)environment_effects);

	}

	TDialog::CmOk();
}

//********************************* class TDialogSelectMacro **********************************
/*
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
*/

TDialogSelectMacro::TDialogSelectMacro(TWindow *parent, TResId resId, int &new_number,
									   const char *new_caption, logical new_with_data,
									   TModule *module)
	: TDialog(parent,resId,module), macro_number(new_number)
{
	with_data=new_with_data;
	caption=new_caption;
	IdcSelectMacro=new TListBox(this, IDC_SELECTMACRO);
}


void TDialogSelectMacro::SetupWindow(void)
{
	int i,number_macros;

	TDialog::SetupWindow();
	SetCaption(caption);
	number_macros=macro_storage.get_number_macros();
	if (with_data) {
		for (i=0;i<number_macros;i++) {
			if (macro_storage.get_macro(i)->has_data())
				IdcSelectMacro->AddString(macro_storage.get_macro(i)->get_macro_name().c_str());
		}
	}
	else {
		for (i=0;i<number_macros;i++)
			IdcSelectMacro->AddString(macro_storage.get_macro(i)->get_macro_name().c_str());
	}
}

DEFINE_RESPONSE_TABLE1(TDialogSelectMacro, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogSelectMacro::CmOk(void)
{
	int i,number_macros;
	int select_index;
	char selected_string[50];

	select_index=IdcSelectMacro->GetSelIndex();

	if (select_index<0) return;
	else {
		number_macros=macro_storage.get_number_macros();
		IdcSelectMacro->GetString(selected_string,IdcSelectMacro->GetSelIndex());
		for (i=0;i<number_macros;i++) {
			if (string(selected_string)==macro_storage.get_macro(i)->get_macro_name()) {
				macro_number=i;
				break;
			}
		}
	}
	TDialog::CmOk();
}

//***************************** class TDialogVoltageMacro **************************************
/*
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
*/

TDialogVoltageMacro::TDialogVoltageMacro(TWindow *parent, TResId resId,
										 TVoltageMacro *new_macro, TModule *module)
	: TDialog(parent,resId,module)
{
	macro=new_macro;

	IdcMacroName=new TEdit(this,IDC_MACRONAME);
	IdcStartValue=new TEdit(this, IDC_STARTVALUE);
	IdcEndValue=new TEdit(this, IDC_ENDVALUE);
	IdcIncrement=new TEdit(this, IDC_INCREMENT);
	IdcLeftContact=new TCheckBox(this, IDC_LEFTCONTACT);
	IdcRightContact=new TCheckBox(this, IDC_RIGHTCONTACT);
	IdcInitialState=new TCheckBox(this, IDC_INITIALSTATE);
	IdcPresentState=new TCheckBox(this, IDC_PRESENTSTATE);
}

void TDialogVoltageMacro::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	if (macro->get_macro_name()!="") {
		SetCaption("Edit Voltage Macro");
		IdcMacroName->SetText(macro->get_macro_name().c_str());

		sprintf(number_string,"%.3f",macro->get_start_value());
		IdcStartValue->SetText(number_string);
		sprintf(number_string,"%.3f",macro->get_end_value());
		IdcEndValue->SetText(number_string);
		sprintf(number_string,"%.3f",macro->get_increment_value());
		IdcIncrement->SetText(number_string);

		if (macro->get_increment_object_number()==0) IdcLeftContact->SetCheck(BF_CHECKED);
		else IdcRightContact->SetCheck(BF_CHECKED);

		if (macro->get_reset_device()) IdcInitialState->SetCheck(BF_CHECKED);
		else IdcPresentState->SetCheck(BF_CHECKED);

		record_flags=macro->get_record_flags();
	}
	else {
		SetCaption("Create Voltage Macro");
		IdcLeftContact->SetCheck(BF_CHECKED);
		IdcInitialState->SetCheck(BF_CHECKED);
	}
}

DEFINE_RESPONSE_TABLE1(TDialogVoltageMacro, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_COMMAND(IDC_BUTTONSIMPLE, CmButtonSimple),
END_RESPONSE_TABLE;

void TDialogVoltageMacro::CmOk(void)
{
	float start_value, end_value;
	char number_string[20];
	char name_string[50];

	if (!record_flags.any_set()) {
		if (MessageBox("You have not selected any parameters to record during your macro. Is This correct?","Warning",
					   MB_ICONQUESTION | MB_YESNO)==IDNO) return;
	}

	IdcMacroName->SetValidator(new TMustEnterValidator());

	if (IdcMacroName->IsValid()) {

		IdcStartValue->GetText(number_string,sizeof(number_string));
		start_value=atof(number_string);

		IdcEndValue->GetText(number_string,sizeof(number_string));
		end_value=atof(number_string);

		if (end_value!=start_value) {
			if (end_value<start_value) IdcIncrement->SetValidator(new TScientificRangeValidator(end_value-start_value,
																								0,EXCLUSIVE));
			else IdcIncrement->SetValidator(new TScientificRangeValidator(0,end_value-start_value,EXCLUSIVE));
		}

		if (IdcIncrement->IsValid()) {
			IdcMacroName->GetText(name_string,sizeof(name_string));
			macro->put_macro_name(name_string);

			IdcIncrement->GetText(number_string,sizeof(number_string));
			macro->put_increment_value(atof(number_string));

			if (IdcLeftContact->GetCheck()==BF_CHECKED) macro->put_increment_object_number(0);
			else macro->put_increment_object_number(1);

			if (IdcInitialState->GetCheck()==BF_CHECKED) macro->put_reset_device(TRUE);
			else macro->put_reset_device(FALSE);

			macro->put_start_value(start_value);
			macro->put_end_value(end_value);
			macro->put_record_flags(record_flags);
		}
	}

	TDialog::CmOk();

	IdcIncrement->SetValidator(NULL);
	IdcMacroName->SetValidator(NULL);
}

void TDialogVoltageMacro::CmButtonSimple(void)
{
	TValueFlag total_flags;

	total_flags.set(SURFACE,SURFACE_MACRO);
	total_flags.set(CONTACT,CONTACT_MACRO);
	total_flags.set(MIRROR,MIRROR_MACRO);
	total_flags.set(MODE,MODE_MACRO);
	if (TDialogSelectParameters(this,DG_SELECTPARAMETERS,total_flags, record_flags).Execute()==IDOK) {
		record_flags.clear_all_objects();
		if (record_flags.any_set(CONTACT)) {
			record_flags.add_object(CONTACT,0);
			record_flags.add_object(CONTACT,1);
		}
		if (record_flags.any_set(SURFACE)) {
			record_flags.add_object(SURFACE,0);
			record_flags.add_object(SURFACE,1);
		}
		if (record_flags.any_set(MIRROR)) {
			record_flags.add_object(MIRROR,0);
			record_flags.add_object(MIRROR,1);
		}
		if (record_flags.any_set(MODE)) {
			record_flags.add_object(MODE,0);
		}
	}
}

//******************************** class TDialogDeviceInfo ************************************
/*
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
*/

TDialogDeviceInfo::TDialogDeviceInfo(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcGridPoints=new TStatic(this,IDC_GRIDPOINTS);
	IdcCurrentSolution=new TStatic(this,IDC_CURRENTSOLUTION);
	IdcCurrentStatus=new TStatic(this,IDC_CURRENTSTATUS);
	IdcBuiltPotential=new TStatic(this,IDC_BUILTPOTENTIAL);
	IdcAppliedBias=new TStatic(this,IDC_APPLIEDBIAS);
	IdcTotalCurrent=new TStatic(this,IDC_TOTALCURRENT);
}

void TDialogDeviceInfo::SetupWindow(void)
{
	char number_string[20];
	string message_string;

	TDialog::SetupWindow();
	sprintf(number_string,"%d",environment.get_number_objects(NODE));
	IdcGridPoints->SetText(number_string);
	message_string=get_solve_string((SolveType)environment.get_value(DEVICE,CURRENT_SOLUTION));
	IdcCurrentSolution->SetText(message_string.c_str());
	message_string=get_status_string((StatusType)environment.get_value(DEVICE,CURRENT_STATUS));
	IdcCurrentStatus->SetText(message_string.c_str());
	sprintf(number_string,"%.5f",environment.get_value(CONTACT,BUILT_IN_POT,1)-
    							 environment.get_value(CONTACT,BUILT_IN_POT,0));
	IdcBuiltPotential->SetText(number_string);
	sprintf(number_string,"%.5f",environment.get_value(CONTACT,APPLIED_BIAS,0)-
								 environment.get_value(CONTACT,APPLIED_BIAS,1));
	IdcAppliedBias->SetText(number_string);
	sprintf(number_string,"%.5e",environment.get_value(CONTACT,TOTAL_CURRENT,0));
	IdcTotalCurrent->SetText(number_string);
}

//*********************************** class TDialogScale **************************************
/*
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
*/

TDialogScale::TDialogScale(TWindow *parent, TResId resId, Axis& x, Axis& y, TModule *module)
	: TDialog(parent,resId,module), x_axis(x), y_axis(y)
{
	IdcXMin=new TEdit(this,IDC_XMIN);
	IdcXMax=new TEdit(this,IDC_XMAX);
	IdcXAuto=new TCheckBox(this,IDC_XAUTO);
	IdcXLinear=new TCheckBox(this,IDC_XLINEAR);
	IdcYMin=new TEdit(this,IDC_YMIN);
	IdcYMax=new TEdit(this,IDC_YMAX);
	IdcYAuto=new TCheckBox(this,IDC_YAUTO);
	IdcYLinear=new TCheckBox(this,IDC_YLINEAR);
	IdcYLog=new TCheckBox(this,IDC_YLOG);
	IdcYNormal=new TCheckBox(this, IDC_YNORMAL);
	IdcYNegative=new TCheckBox(this, IDC_YNEGATIVE);
	IdcYAbsolute=new TCheckBox(this, IDC_YABSOLUTE);
}

void TDialogScale::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	sprintf(number_string,"%.3f",x_axis.minimum);
	IdcXMin->SetText(number_string);
	sprintf(number_string,"%.3f",x_axis.maximum);
	IdcXMax->SetText(number_string);
	if (x_axis.auto_scale) IdcXAuto->Check();

	if ((fabs(y_axis.minimum)>100) ||
		(fabs(y_axis.minimum)<0.01))
		sprintf(number_string,"%.3e",y_axis.minimum);
	else
		sprintf(number_string,"%.3f",y_axis.minimum);
	IdcYMin->SetText(number_string);

	if ((fabs(y_axis.maximum)>100) ||
		(fabs(y_axis.maximum)<0.01))
		sprintf(number_string,"%.3e",y_axis.maximum);
	else
		sprintf(number_string,"%.3f",y_axis.maximum);
	IdcYMax->SetText(number_string);
	if (y_axis.auto_scale) IdcYAuto->Check();

	IdcXLinear->Check();

	if (y_axis.scale_type==LIN) IdcYLinear->Check();
	else IdcYLog->Check();

	 switch (y_axis.operation_type) {
		case EXACT:
			IdcYNormal->Check();
			break;
		case NEGATIVE:
			IdcYNegative->Check();
			break;
		case ABS:
			IdcYAbsolute->Check();
			break;
	 }
}

DEFINE_RESPONSE_TABLE1(TDialogScale, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_CHILD_NOTIFY(IDC_XMIN, EN_UPDATE, EvXScale),
	EV_CHILD_NOTIFY(IDC_XMAX, EN_UPDATE, EvXScale),
	EV_CHILD_NOTIFY(IDC_YMIN, EN_UPDATE, EvYScale),
	EV_CHILD_NOTIFY(IDC_YMAX, EN_UPDATE, EvYScale),
END_RESPONSE_TABLE;

void TDialogScale::CmOk(void)
{
	char number_string[20];
	logical y_min_valid, y_max_valid;
	logical x_min_valid, x_max_valid;
	float x_min, x_max, y_min, y_max;
	logical x_auto, y_auto;

	if (IdcXAuto->GetCheck()!=BF_CHECKED) {
		IdcXMin->SetValidator(new TScientificRangeValidator(-1e30,1e30,INCLUSIVE));
		IdcXMax->SetValidator(new TScientificRangeValidator(-1e30,1e30,INCLUSIVE));
		x_min_valid=IdcXMin->IsValid();
		x_max_valid=IdcXMax->IsValid();
	}
	else {
		x_min_valid=TRUE;
		x_max_valid=TRUE;
	}

	if (IdcYAuto->GetCheck()!=BF_CHECKED) {
		if (IdcYLog->GetCheck()==BF_CHECKED) {
			IdcYMin->SetValidator(new TScientificRangeValidator(1e-30,1e30,INCLUSIVE));
			IdcYMax->SetValidator(new TScientificRangeValidator(1e-30,1e30,INCLUSIVE));
		}
		else {
			IdcYMin->SetValidator(new TScientificRangeValidator(-1e30,1e30,INCLUSIVE));
			IdcYMax->SetValidator(new TScientificRangeValidator(-1e30,1e30,INCLUSIVE));
		}
		y_min_valid=IdcYMin->IsValid();
		y_max_valid=IdcYMax->IsValid();
	}
	else {
		y_min_valid=TRUE;
		y_max_valid=TRUE;
	}

	if (x_min_valid && x_max_valid && y_min_valid && y_max_valid) {
		if (IdcXAuto->GetCheck()==BF_CHECKED) x_auto=TRUE;
		else {
			x_auto=FALSE;
			IdcXMin->GetText(number_string,sizeof(number_string));
			x_min=atof(number_string);
			IdcXMax->GetText(number_string,sizeof(number_string));
			x_max=atof(number_string);
			if (x_max==x_min) error_handler.set_error(WARNING_PLOT_SAME_SCALE,0,"","");
		}

		if (!error_handler.fail()) {
			if (IdcYAuto->GetCheck()==BF_CHECKED) y_auto=TRUE;
			else {
				y_auto=FALSE;
				IdcYMin->GetText(number_string,sizeof(number_string));
				y_min=atof(number_string);
				IdcYMax->GetText(number_string,sizeof(number_string));
				y_max=atof(number_string);
				if (y_max==y_min) error_handler.set_error(WARNING_PLOT_SAME_SCALE,0,"","");
			}
		}

		if (!error_handler.fail()) {
			x_axis.auto_scale=x_auto;
			if (x_min>x_max) swap(x_min,x_max);
			x_axis.minimum=x_min;
			x_axis.maximum=x_max;

			y_axis.auto_scale=y_auto;
			if (y_min>y_max) swap(y_min,y_max);
			y_axis.minimum=y_min;
			y_axis.maximum=y_max;

			if (IdcYLinear->GetCheck()==BF_CHECKED) y_axis.scale_type=LIN;
			else y_axis.scale_type=LOG;

			if (IdcYNormal->GetCheck()==BF_CHECKED) y_axis.operation_type=EXACT;
			if (IdcYNegative->GetCheck()==BF_CHECKED) y_axis.operation_type=NEGATIVE;
			if (IdcYAbsolute->GetCheck()==BF_CHECKED) y_axis.operation_type=ABS;
		}
	}

	if (error_handler.fail()) out_error_message(TRUE);
	else TDialog::CmOk();

	IdcXMin->SetValidator(NULL);
	IdcXMax->SetValidator(NULL);
	IdcYMin->SetValidator(NULL);
	IdcYMax->SetValidator(NULL);
}

//*********************************** class TDialogTrace ***************************************
/*
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
*/

TDialogTrace::TDialogTrace(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	number_labels=0;
	number_values=0;

	IdcString[0]=new TStatic(this,IDC_STRING1);
	IdcNumber[0]=new TStatic(this,IDC_NUMBER1);
	IdcString[1]=new TStatic(this,IDC_STRING2);
	IdcNumber[1]=new TStatic(this,IDC_NUMBER2);
	IdcString[2]=new TStatic(this,IDC_STRING3);
	IdcNumber[2]=new TStatic(this,IDC_NUMBER3);
	IdcString[3]=new TStatic(this,IDC_STRING4);
	IdcNumber[3]=new TStatic(this,IDC_NUMBER4);
	IdcString[4]=new TStatic(this,IDC_STRING5);
	IdcNumber[4]=new TStatic(this,IDC_NUMBER5);
	IdcString[5]=new TStatic(this,IDC_STRING6);
	IdcNumber[5]=new TStatic(this,IDC_NUMBER6);
	IdcString[6]=new TStatic(this,IDC_STRING7);
	IdcNumber[6]=new TStatic(this,IDC_NUMBER7);
}

void TDialogTrace::update_labels(string *label_string, int number)
{
	int i;

	if (number>MAX_NUMBER_PLOTS+1) number=MAX_NUMBER_PLOTS+1;
	for (i=0;i<number;i++) labels[i]=label_string[i];
	number_labels=number;
}

void TDialogTrace::update_values(float *value_float, int number)
{
	int i;

	if (number>MAX_NUMBER_PLOTS+1) number=MAX_NUMBER_PLOTS+1;
	for (i=0;i<number;i++) values[i]=value_float[i];
	number_values=number;
}

void TDialogTrace::redraw_labels(void)
{
	int i;

	for (i=0;i<number_labels;i++) IdcString[i]->SetText(labels[i].c_str());
}

void TDialogTrace::redraw_values(void)
{
	int i;

	for (i=0;i<number_values;i++) IdcNumber[i]->SetText(prec_to_string(values[i],5,SCIENTIFIC).c_str());
}

void TDialogTrace::SetupWindow(void)
{
	TDialog::SetupWindow();
	redraw_labels();
	redraw_values();
}

DEFINE_RESPONSE_TABLE1(TDialogTrace, TDialog)
	EV_WM_CLOSE,
	EV_COMMAND(IDCANCEL,CmIdCancel),
END_RESPONSE_TABLE;

//*********************************** class TDialogAbout **************************************
/*
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
*/

void TDialogAbout::SetupWindow(void)
{
	extern char about_string[];
	TDialog::SetupWindow();
	IdcAboutText->SetText(about_string);
}


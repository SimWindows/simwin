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

//******************************* class TDialogOpticalInput ***********************************
/*
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
*/
TOpenSaveDialog::TData TDialogOpticalInput::FileData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST,
													 "Spectrum Files (*.spc)|*.spc|All Files (*.*)|*.*|",
													 "", "", "spc");
string TDialogOpticalInput::spectrum_filename;
logical TDialogOpticalInput::energy_selected=TRUE;

TDialogOpticalInput::TDialogOpticalInput(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcOn=new TCheckBox(this,IDC_ON);
	IdcOff=new TCheckBox(this, IDC_OFF);
	IdcEntireDevice=new TCheckBox(this, IDC_ENTIREDEVICE);
	IdcLeftIncident=new TCheckBox(this, IDC_LEFTINCIDENT);
	IdcRightIncident=new TCheckBox(this, IDC_RIGHTINCIDENT);
	IdcTextStartPosition=new TStatic(this, IDC_TEXTSTARTPOSITION);
	IdcStartPosition=new TEdit(this, IDC_STARTPOSITION);
	IdcTextEndPosition=new TStatic(this, IDC_TEXTENDPOSITION);
	IdcEndPosition=new TEdit(this, IDC_ENDPOSITION);
	IdcInterfaceReflection=new TCheckBox(this, IDC_INTERFACEREFLECTION);
	IdcClearInput=new TCheckBox(this, IDC_CLEARINPUT);
	IdcEditEnergy=new TEdit(this, IDC_EDITENERGY);
	IdcCheckEnergy=new TCheckBox(this,IDC_CHECKENERGY);
	IdcWavelength=new TCheckBox(this, IDC_WAVELENGTH);
	IdcIntensity=new TEdit(this, IDC_INTENSITY);
	IdcTextFilename=new TStatic(this, IDC_TEXTFILENAME);
	IdcSpectrumMultiplier=new TEdit(this, IDC_SPECTRUMMULTIPLIER);
	IdcSpectrumMultiplier->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));

	environment_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);
	number_components=environment.get_number_objects(SPECTRUM);

	calc_wavelength=TRUE;
	light_entered=FALSE;
	load_spectrum=FALSE;

	if (number_components<=1) spectrum_filename="";
	new_spectrum_filename=spectrum_filename;
}

void TDialogOpticalInput::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	if (environment_effects & ENV_OPTICAL_GEN) IdcOn->Check();
	else IdcOff->Check();

	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,SPEC_START_POSITION));
	IdcStartPosition->SetText(number_string);
	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,SPEC_END_POSITION));
	IdcEndPosition->SetText(number_string);

	if (environment_effects & ENV_SPEC_ENTIRE_DEVICE) {
		IdcEntireDevice->Check();
		if (environment_effects & ENV_SPEC_LEFT_INCIDENT)
			IdcLeftIncident->Check();
		else
			IdcRightIncident->Check();
		IdcStartPosition->EnableWindow(FALSE);
		IdcEndPosition->EnableWindow(FALSE);
	}
	else {
		IdcLeftIncident->EnableWindow(FALSE);
		IdcRightIncident->EnableWindow(FALSE);
	}

	if (environment_effects & ENV_INCIDENT_REFLECTION) IdcInterfaceReflection->Check();

	if (energy_selected) IdcCheckEnergy->Check();
	else IdcWavelength->Check();

	switch(number_components) {
		case 0:
			IdcClearInput->SetCheck(BF_CHECKED);
			break;
		case 1:
			IdcClearInput->EnableWindow(TRUE);
			if (energy_selected) {
				sprintf(number_string,"%.4f",(float)environment.get_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,0));
				IdcEditEnergy->SetText(number_string);
			}
			else {
				sprintf(number_string,"%.4f",(float)environment.get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,0));
				IdcEditEnergy->SetText(number_string);
			}
			sprintf(number_string,"%.4f",(float)environment.get_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,0));
			IdcIntensity->SetText(number_string);
			break;
		default:
			IdcClearInput->EnableWindow(TRUE);
			IdcTextFilename->SetText(shorten_path(spectrum_filename).c_str());
			break;
	}

	sprintf(number_string,"%.3f",(float)environment.get_value(ENVIRONMENT,SPECTRUM_MULTIPLIER));
	IdcSpectrumMultiplier->SetText(number_string);
}

DEFINE_RESPONSE_TABLE1(TDialogOpticalInput, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_COMMAND(IDC_BUTTONFILENAME, CmButtonFilename),
	EV_CHILD_NOTIFY(IDC_CLEARINPUT, BN_CLICKED, EvClearInput),
	EV_CHILD_NOTIFY(IDC_ENTIREDEVICE, BN_CLICKED, EvEntireDevice),
	EV_CHILD_NOTIFY(IDC_CHECKENERGY, BN_CLICKED, EvEnergy),
	EV_CHILD_NOTIFY(IDC_WAVELENGTH, BN_CLICKED, EvWavelength),
	EV_CHILD_NOTIFY(IDC_EDITENERGY, EN_UPDATE, EvSingleComponent),
	EV_CHILD_NOTIFY(IDC_INTENSITY, EN_UPDATE, EvSingleComponent),
END_RESPONSE_TABLE;

void TDialogOpticalInput::CmOk(void)
{
	char number_string[20];
	float light_energy, light_intensity;
    bool accept_values;

	IdcStartPosition->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcEndPosition->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	if (light_entered) {
		IdcEditEnergy->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
		IdcIntensity->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	}

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcStartPosition->IsValid() && IdcEndPosition->IsValid() &&
			IdcEditEnergy->IsValid() && IdcIntensity->IsValid() &&
			IdcSpectrumMultiplier->IsValid()) {

			if (IdcEntireDevice->GetCheck()==BF_CHECKED) {
				environment_effects|=ENV_SPEC_ENTIRE_DEVICE;
				if (IdcLeftIncident->GetCheck()==BF_CHECKED)
					environment_effects|=ENV_SPEC_LEFT_INCIDENT;
				else
					environment_effects&=(~ENV_SPEC_LEFT_INCIDENT);
			}
			else {
				environment_effects&=(~ENV_SPEC_ENTIRE_DEVICE);
				environment_effects|=ENV_SPEC_LEFT_INCIDENT;
				IdcStartPosition->GetText(number_string,sizeof(number_string));
				environment.put_value(ENVIRONMENT,SPEC_START_POSITION,atof(number_string));
				IdcEndPosition->GetText(number_string,sizeof(number_string));
				environment.put_value(ENVIRONMENT,SPEC_END_POSITION,atof(number_string));
			}

			spectrum_filename=new_spectrum_filename;

			if (IdcClearInput->GetCheck()==BF_CHECKED) environment.delete_spectrum();
			else {
				if (load_spectrum) {
					::SetCursor(TCursor(NULL,IDC_WAIT));
					environment.load_spectrum(spectrum_filename.c_str());
					::SetCursor(TCursor(NULL,IDC_ARROW));
				}

				if (light_entered) {
					IdcEditEnergy->GetText(number_string,sizeof(number_string));
					light_energy=atof(number_string);

					switch(number_components) {
						case 0: environment.add_spectral_comp(); break;
						case 1: break;
						default: environment.delete_spectrum(); environment.add_spectral_comp(); break;
					}

					if (!error_handler.fail()) {
						if (IdcCheckEnergy->GetCheck()==BF_CHECKED) {
							energy_selected=TRUE;
							environment.put_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,light_energy,0);
						}
						else {
							energy_selected=FALSE;
							environment.put_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,light_energy,0);
						}

						IdcIntensity->GetText(number_string,sizeof(number_string));
						light_intensity=atof(number_string);

						environment.put_value(SPECTRUM,INCIDENT_INPUT_INTENSITY,light_intensity,0);
						environment.put_value(SPECTRUM,INCIDENT_EMITTED_INTENSITY,light_intensity,0);
					}
				}
			}

			IdcSpectrumMultiplier->GetText(number_string,sizeof(number_string));
			environment.put_value(ENVIRONMENT,SPECTRUM_MULTIPLIER,atof(number_string));

			if ((environment.get_number_objects(SPECTRUM)>0) && (IdcOn->GetCheck()==BF_CHECKED))
				environment_effects|=ENV_OPTICAL_GEN;
			else
				environment_effects&=(~ENV_OPTICAL_GEN);

			if (IdcInterfaceReflection->GetCheck()==BF_CHECKED)
				environment_effects|=ENV_INCIDENT_REFLECTION;
			else
				environment_effects&=(~ENV_INCIDENT_REFLECTION);

			environment.put_value(ENVIRONMENT,EFFECTS,(prec)environment_effects);
		}

		if (!error_handler.fail()) TDialog::CmOk();
		else out_error_message(TRUE);
    }

	IdcStartPosition->SetValidator(NULL);
	IdcEndPosition->SetValidator(NULL);
	IdcEditEnergy->SetValidator(NULL);
	IdcIntensity->SetValidator(NULL);
}

void TDialogOpticalInput::CmButtonFilename(void)
{
	if (TFileOpenDialog(this, FileData).Execute()==IDOK) {
		IdcClearInput->EnableWindow(TRUE);
		IdcClearInput->SetCheck(BF_UNCHECKED);
		new_spectrum_filename=FileData.FileName;
		calc_wavelength=FALSE;
		light_entered=FALSE;
		load_spectrum=TRUE;
		IdcTextFilename->SetText(shorten_path(new_spectrum_filename).c_str());
		IdcEditEnergy->Clear();
		IdcEditEnergy->ClearModify();
		IdcIntensity->Clear();
		IdcIntensity->ClearModify();
	}
}

void TDialogOpticalInput::EvClearInput(void)
{
	if (IdcClearInput->GetCheck()==BF_CHECKED) {
		IdcEditEnergy->Clear();
		IdcEditEnergy->ClearModify();
		IdcIntensity->Clear();
		IdcIntensity->ClearModify();
		IdcTextFilename->Clear();
		new_spectrum_filename="";
		calc_wavelength=FALSE;
		light_entered=FALSE;
		load_spectrum=FALSE;
		IdcClearInput->EnableWindow(FALSE);
	}
}

void TDialogOpticalInput::EvEntireDevice(void)
{
	if (IdcEntireDevice->GetCheck()==BF_CHECKED) {
		IdcLeftIncident->EnableWindow(TRUE);
		IdcRightIncident->EnableWindow(TRUE);
		if (environment_effects & ENV_SPEC_LEFT_INCIDENT) IdcLeftIncident->Check();
		else IdcRightIncident->Check();
		IdcStartPosition->EnableWindow(FALSE);
		IdcEndPosition->EnableWindow(FALSE);
	}
	else {
		IdcLeftIncident->Uncheck();
		IdcRightIncident->Uncheck();
		IdcLeftIncident->EnableWindow(FALSE);
		IdcRightIncident->EnableWindow(FALSE);
		IdcStartPosition->EnableWindow(TRUE);
		IdcEndPosition->EnableWindow(TRUE);
	}
}

void TDialogOpticalInput::EvEnergy(void)
{
	char number_string[20];

	if ((number_components==1) && (calc_wavelength)) {
		sprintf(number_string,"%.3f",(float)environment.get_value(SPECTRUM,INCIDENT_PHOTON_ENERGY,0));
		IdcEditEnergy->SetText(number_string);
	}
}

void TDialogOpticalInput::EvWavelength(void)
{
	char number_string[20];

	if ((number_components==1) && (calc_wavelength)) {
		sprintf(number_string,"%.3f",(float)environment.get_value(SPECTRUM,INCIDENT_PHOTON_WAVELENGTH,0));
		IdcEditEnergy->SetText(number_string);
	}
}

void TDialogOpticalInput::EvSingleComponent(void)
{
	if ((IdcEditEnergy->HWindow==GetFocus()) ||
		(IdcIntensity->HWindow==GetFocus())) {
		IdcClearInput->EnableWindow(TRUE);
		IdcClearInput->SetCheck(BF_UNCHECKED);
		IdcTextFilename->Clear();
		new_spectrum_filename="";
		calc_wavelength=FALSE;
		light_entered=TRUE;
		load_spectrum=FALSE;
	}
}

//*********************************** class TDialogDeviceContacts ******************************
/*
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
*/

TDialogDeviceContacts::TDialogDeviceContacts(TWindow *parent, TResId resId,TModule *module)
	: TDialog(parent,resId,module)
{
	left_contact_effects=(flag)environment.get_value(CONTACT,EFFECTS,0);
	right_contact_effects=(flag)environment.get_value(CONTACT,EFFECTS,1);

	IdcLPosition=new TStatic(this, IDC_LPOSITION);
	IdcLBias=new TEdit(this, IDC_LBIAS);
	IdcLBias->SetValidator(new TScientificRangeValidator(-100,100,INCLUSIVE));
	IdcLOhmic=new TCheckBox(this,IDC_LOHMIC);
	IdcLFiniteRecomb=new TCheckBox(this,IDC_LFINITERECOMB);
	IdcLElectronVelocity=new TEdit(this,IDC_LELECTRONVELOCITY);
	IdcLElectronVelocity->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcLHoleVelocity=new TEdit(this,IDC_LHOLEVELOCITY);
	IdcLHoleVelocity->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcLSchottky=new TCheckBox(this,IDC_LSCHOTTKY);
    IdcLBarrierHeight=new TEdit(this,IDC_LBARRIERHEIGHT);
    IdcLBarrierHeight->SetValidator(new TScientificRangeValidator(-100,100,INCLUSIVE));

	IdcRPosition=new TStatic(this, IDC_RPOSITION);
	IdcRBias=new TEdit(this, IDC_RBIAS);
	IdcRBias->SetValidator(new TScientificRangeValidator(-100,100,INCLUSIVE));
	IdcROhmic=new TCheckBox(this,IDC_ROHMIC);
	IdcRFiniteRecomb=new TCheckBox(this,IDC_RFINITERECOMB);
	IdcRElectronVelocity=new TEdit(this,IDC_RELECTRONVELOCITY);
	IdcRElectronVelocity->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcRHoleVelocity=new TEdit(this,IDC_RHOLEVELOCITY);
	IdcRHoleVelocity->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcRSchottky=new TCheckBox(this,IDC_RSCHOTTKY);
    IdcRBarrierHeight=new TEdit(this,IDC_RBARRIERHEIGHT);
    IdcRBarrierHeight->SetValidator(new TScientificRangeValidator(-100,100,INCLUSIVE));
}

void TDialogDeviceContacts::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,POSITION,0));
	IdcLPosition->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,APPLIED_BIAS,0));
	IdcLBias->SetText(number_string);
	if (left_contact_effects & CONTACT_IDEALOHMIC) IdcLOhmic->Check();
	if (left_contact_effects & CONTACT_FINITERECOMB) IdcLFiniteRecomb->Check();
	if (left_contact_effects & CONTACT_SCHOTTKY) IdcLSchottky->Check();
	sprintf(number_string,"%.4e",(float)environment.get_value(CONTACT,ELECTRON_RECOMB_VEL,0));
	IdcLElectronVelocity->SetText(number_string);
	sprintf(number_string,"%.4e",(float)environment.get_value(CONTACT,HOLE_RECOMB_VEL,0));
	IdcLHoleVelocity->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,BARRIER_HEIGHT,0));
	IdcLBarrierHeight->SetText(number_string);

	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,POSITION,1));
	IdcRPosition->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,APPLIED_BIAS,1));
	IdcRBias->SetText(number_string);
	if (right_contact_effects & CONTACT_IDEALOHMIC) IdcROhmic->Check();
	if (right_contact_effects & CONTACT_FINITERECOMB) IdcRFiniteRecomb->Check();
	if (right_contact_effects & CONTACT_SCHOTTKY) IdcRSchottky->Check();
	sprintf(number_string,"%.4e",(float)environment.get_value(CONTACT,ELECTRON_RECOMB_VEL,1));
	IdcRElectronVelocity->SetText(number_string);
	sprintf(number_string,"%.4e",(float)environment.get_value(CONTACT,HOLE_RECOMB_VEL,1));
	IdcRHoleVelocity->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(CONTACT,BARRIER_HEIGHT,1));
	IdcRBarrierHeight->SetText(number_string);
}

DEFINE_RESPONSE_TABLE1(TDialogDeviceContacts, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogDeviceContacts::CmOk(void)
{
	char number_string[20];
    bool accept_values;

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
	       	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcLBias->IsValid() && IdcLElectronVelocity->IsValid() &&
        	IdcLHoleVelocity->IsValid() && IdcLBarrierHeight->IsValid() &&
			IdcRBias->IsValid() && IdcRElectronVelocity->IsValid() &&
            IdcRHoleVelocity->IsValid() && IdcRBarrierHeight->IsValid()) {

			IdcLBias->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,APPLIED_BIAS,atof(number_string),0);

			if (IdcLOhmic->GetCheck()==BF_CHECKED) left_contact_effects|=CONTACT_IDEALOHMIC;
			else left_contact_effects&=(~CONTACT_IDEALOHMIC);
			if (IdcLFiniteRecomb->GetCheck()==BF_CHECKED) left_contact_effects|=CONTACT_FINITERECOMB;
			else left_contact_effects&=(~CONTACT_FINITERECOMB);
			if (IdcLSchottky->GetCheck()==BF_CHECKED) left_contact_effects|=CONTACT_SCHOTTKY;
			else left_contact_effects&=(~CONTACT_SCHOTTKY);
			IdcLElectronVelocity->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,ELECTRON_RECOMB_VEL,atof(number_string),0);
			IdcLHoleVelocity->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,HOLE_RECOMB_VEL,atof(number_string),0);
			IdcLBarrierHeight->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,BARRIER_HEIGHT,atof(number_string),0);

			IdcRBias->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,APPLIED_BIAS,atof(number_string),1);

			if (IdcROhmic->GetCheck()==BF_CHECKED) right_contact_effects|=CONTACT_IDEALOHMIC;
			else right_contact_effects&=(~CONTACT_IDEALOHMIC);
			if (IdcRFiniteRecomb->GetCheck()==BF_CHECKED) right_contact_effects|=CONTACT_FINITERECOMB;
			else right_contact_effects&=(~CONTACT_FINITERECOMB);
			if (IdcRSchottky->GetCheck()==BF_CHECKED) right_contact_effects|=CONTACT_SCHOTTKY;
			else right_contact_effects&=(~CONTACT_SCHOTTKY);
			IdcRElectronVelocity->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,ELECTRON_RECOMB_VEL,atof(number_string),1);
			IdcRHoleVelocity->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,HOLE_RECOMB_VEL,atof(number_string),1);
			IdcRBarrierHeight->GetText(number_string,sizeof(number_string));
			environment.put_value(CONTACT,BARRIER_HEIGHT,atof(number_string),1);

			environment.put_value(CONTACT,EFFECTS,(prec)left_contact_effects,0);
			environment.put_value(CONTACT,EFFECTS,(prec)right_contact_effects,1);
		}
		TDialog::CmOk();
    }
}

//*********************************** class TDialogDeviceSurface ******************************
/*
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
*/

TDialogDeviceSurfaces::TDialogDeviceSurfaces(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	left_surface_effects=(flag)environment.get_value(SURFACE,EFFECTS,0);
	right_surface_effects=(flag)environment.get_value(SURFACE,EFFECTS,1);

	IdcLSurfacePosition=new TStatic(this, IDC_LSURFACEPOSITION);
	IdcLTemp=new TEdit(this, IDC_LTEMPERATURE);
	IdcLTemp->SetValidator(new TScientificRangeValidator(50,5000,INCLUSIVE));
	IdcLElectronTemp=new TEdit(this, IDC_LELECTRONTEMP);
	IdcLElectronTemp->SetValidator(new TScientificRangeValidator(50,5000,INCLUSIVE));
	IdcLOpticalPermitivity=new TEdit(this,IDC_LOPTICALPERMITIVITY);
	IdcLOpticalPermitivity->SetValidator(new TScientificLowerValidator(1,INCLUSIVE));
	IdcLHeatSink=new TCheckBox(this,IDC_LHEATSINK);
	IdcLFiniteCond=new TCheckBox(this,IDC_LFINITECOND);
	IdcLThermCond=new TEdit(this,IDC_LTHERMCOND);
	IdcLThermCond->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));

	IdcRSurfacePosition=new TStatic(this, IDC_RSURFACEPOSITION);
	IdcRTemp=new TEdit(this, IDC_RTEMPERATURE);
	IdcRTemp->SetValidator(new TScientificRangeValidator(50,5000,INCLUSIVE));
	IdcRElectronTemp=new TEdit(this, IDC_RELECTRONTEMP);
	IdcRElectronTemp->SetValidator(new TScientificRangeValidator(50,5000,INCLUSIVE));
	IdcROpticalPermitivity=new TEdit(this,IDC_ROPTICALPERMITIVITY);
	IdcROpticalPermitivity->SetValidator(new TScientificLowerValidator(1,INCLUSIVE));
	IdcRHeatSink=new TCheckBox(this,IDC_RHEATSINK);
	IdcRFiniteCond=new TCheckBox(this,IDC_RFINITECOND);
	IdcRThermCond=new TEdit(this,IDC_RTHERMCOND);
	IdcRThermCond->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
}

void TDialogDeviceSurfaces::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,POSITION,0));
	IdcLSurfacePosition->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,TEMPERATURE,0));
	IdcLTemp->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,ELECTRON_TEMPERATURE,0));
	IdcLElectronTemp->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,INCIDENT_REFRACTIVE_INDEX,0));
	IdcLOpticalPermitivity->SetText(number_string);

	if (left_surface_effects & SURFACE_HEAT_SINK) IdcLHeatSink->Check();
	else IdcLFiniteCond->Check();
	sprintf(number_string,"%.4e",(float)environment.get_value(SURFACE,THERMAL_CONDUCT,0));
	IdcLThermCond->SetText(number_string);

	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,POSITION,1));
	IdcRSurfacePosition->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,TEMPERATURE,1));
	IdcRTemp->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,ELECTRON_TEMPERATURE,1));
	IdcRElectronTemp->SetText(number_string);
	sprintf(number_string,"%.4f",(float)environment.get_value(SURFACE,INCIDENT_REFRACTIVE_INDEX,1));
	IdcROpticalPermitivity->SetText(number_string);

	if (right_surface_effects & SURFACE_HEAT_SINK) IdcRHeatSink->Check();
	else IdcRFiniteCond->Check();
	sprintf(number_string,"%.4e",(float)environment.get_value(SURFACE,THERMAL_CONDUCT,1));
	IdcRThermCond->SetText(number_string);
}

DEFINE_RESPONSE_TABLE1(TDialogDeviceSurfaces, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogDeviceSurfaces::CmOk(void)
{
	char number_string[20];
    bool accept_values;

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcLTemp->IsValid() && IdcLElectronTemp->IsValid() && IdcLOpticalPermitivity->IsValid() &&
			IdcLThermCond->IsValid() &&
			IdcRTemp->IsValid() && IdcRElectronTemp->IsValid() && IdcROpticalPermitivity->IsValid() &&
			IdcRThermCond->IsValid()) {
			IdcLTemp->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,TEMPERATURE,atof(number_string),0);
			IdcLElectronTemp->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,ELECTRON_TEMPERATURE,atof(number_string),0);
			IdcLOpticalPermitivity->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,INCIDENT_REFRACTIVE_INDEX,atof(number_string),0);

			if (IdcLHeatSink->GetCheck()==BF_CHECKED) left_surface_effects|=SURFACE_HEAT_SINK;
			else left_surface_effects&=(~SURFACE_HEAT_SINK);
			IdcLThermCond->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,THERMAL_CONDUCT,atof(number_string),0);

			IdcRTemp->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,TEMPERATURE,atof(number_string),1);
			IdcRElectronTemp->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,ELECTRON_TEMPERATURE,atof(number_string),1);
			IdcROpticalPermitivity->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,INCIDENT_REFRACTIVE_INDEX,atof(number_string),1);

			if (IdcRHeatSink->GetCheck()==BF_CHECKED) right_surface_effects|=SURFACE_HEAT_SINK;
			else right_surface_effects&=(~SURFACE_HEAT_SINK);
			IdcRThermCond->GetText(number_string,sizeof(number_string));
			environment.put_value(SURFACE,THERMAL_CONDUCT,atof(number_string),1);

			environment.put_value(SURFACE,EFFECTS,(prec)left_surface_effects,0);
			environment.put_value(SURFACE,EFFECTS,(prec)right_surface_effects,1);
		}
		TDialog::CmOk();
    }
}

//**************************** class TDialogElectricalModels ***********************************
/*
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
*/

TDialogElectricalModels::TDialogElectricalModels(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	grid_effects=(flag)environment.get_value(GRID_ELECTRICAL,EFFECTS);
	device_effects=(flag)environment.get_value(DEVICE,EFFECTS);
	environment_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);
	number_components=environment.get_number_objects(SPECTRUM);
	quantum_wells=environment.get_number_objects(QUANTUM_WELL)!=0;
	if (quantum_wells) qw_effects=(flag)environment.get_value(QUANTUM_WELL,EFFECTS);
	else qw_effects=(flag)0;

	IdcBoltzmann=new TCheckBox(this, IDC_BOLTZMANN);
	IdcFermiDirac=new TCheckBox(this, IDC_FERMIDIRAC);
	IdcIncompleteIonization=new TCheckBox(this, IDC_INCOMPLETEIONIZATION);
	IdcQWFermiDirac=new TCheckBox(this, IDC_QWFERMIDIRAC);
	IdcBound=new TCheckBox(this, IDC_BOUND);
	IdcFree=new TCheckBox(this, IDC_FREE);
	IdcQWInfinite=new TCheckBox(this, IDC_QWINFINITE);
	IdcQWFinite=new TCheckBox(this, IDC_QWFINITE);
	IdcSHR=new TCheckBox(this, IDC_SHR);
	IdcBimolecular=new TCheckBox(this, IDC_BIMOLECULAR);
    IdcAuger=new TCheckBox(this, IDC_AUGER);
	IdcStimulated=new TCheckBox(this, IDC_STIMULATED);
	IdcOpticalGeneration=new TCheckBox(this, IDC_OPTICALGENERATION);
	IdcDriftDiffusion=new TCheckBox(this, IDC_DRIFTDIFFUSION);
	IdcThermionic=new TCheckBox(this, IDC_THERMIONIC);
	IdcTunneling=new TCheckBox(this, IDC_TUNNELING);
	IdcInternalCriteria=new TCheckBox(this, IDC_INTERNALCRITERIA);
	IdcAbruptMaterials=new TCheckBox(this, IDC_ABRUPTMATERIALS);
	IdcDopingDepMobility=new TCheckBox(this,IDC_DOPINGDEPMOBILITY);
}

void TDialogElectricalModels::SetupWindow(void)
{
	TDialog::SetupWindow();

	if (grid_effects & GRID_FERMI_DIRAC) IdcFermiDirac->SetCheck(BF_CHECKED);
	else IdcBoltzmann->SetCheck(BF_CHECKED);

	if (grid_effects & GRID_INCOMPLETE_IONIZATION)
		IdcIncompleteIonization->SetCheck(BF_CHECKED);

	if (quantum_wells) {
		IdcQWFermiDirac->SetCheck(BF_CHECKED);
		IdcBound->SetCheck(BF_CHECKED);
		IdcQWFinite->EnableWindow(TRUE);
		IdcFree->EnableWindow(TRUE);
		if (grid_effects & GRID_QW_FREE_CARR) IdcFree->SetCheck(BF_CHECKED);
		if (qw_effects & QW_INFINITE_SQRWELL) IdcQWInfinite->SetCheck(BF_CHECKED);
		if (qw_effects & QW_FINITE_SQRWELL) IdcQWFinite->SetCheck(BF_CHECKED);
	}


	if (grid_effects & GRID_RECOMB_SHR) IdcSHR->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_RECOMB_B_B) IdcBimolecular->SetCheck(BF_CHECKED);
    if (grid_effects & GRID_RECOMB_AUGER) IdcAuger->SetCheck(BF_CHECKED);
	if (device_effects & DEVICE_LASER) {
		IdcStimulated->EnableWindow(TRUE);
		if (grid_effects & GRID_RECOMB_STIM) IdcStimulated->SetCheck(BF_CHECKED);
	}
	if (number_components>0) {
		IdcOpticalGeneration->EnableWindow(TRUE);
		if (environment_effects & ENV_OPTICAL_GEN)
			IdcOpticalGeneration->SetCheck(BF_CHECKED);
	}

	IdcDriftDiffusion->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_THERMIONIC) IdcThermionic->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_TUNNELING) IdcTunneling->SetCheck(BF_CHECKED);

	if (grid_effects & GRID_ABRUPT_MATERIALS) IdcAbruptMaterials->SetCheck(BF_CHECKED);
	else IdcInternalCriteria->SetCheck(BF_CHECKED);

	if (grid_effects & GRID_DOPING_MOBILITY)
		IdcDopingDepMobility->SetCheck(BF_CHECKED);
}

DEFINE_RESPONSE_TABLE1(TDialogElectricalModels, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogElectricalModels::CmOk(void)
{
    bool accept_values;

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcFermiDirac->GetCheck()==BF_CHECKED) grid_effects|=GRID_FERMI_DIRAC;
		else grid_effects&=(~GRID_FERMI_DIRAC);
		if (IdcIncompleteIonization->GetCheck()==BF_CHECKED) grid_effects|=GRID_INCOMPLETE_IONIZATION;
		else grid_effects&=(~GRID_INCOMPLETE_IONIZATION);

		if (quantum_wells) {
			if (IdcFree->GetCheck()==BF_CHECKED) grid_effects|=GRID_QW_FREE_CARR;
			else grid_effects&=(~GRID_QW_FREE_CARR);

			qw_effects&=(~(QW_INFINITE_SQRWELL | QW_FINITE_SQRWELL | QW_EXACT));

			if (IdcQWInfinite->GetCheck()==BF_CHECKED) qw_effects|=QW_INFINITE_SQRWELL;
			else {
				if (IdcQWFinite->GetCheck()==BF_CHECKED) qw_effects|=QW_FINITE_SQRWELL;
				else qw_effects|=QW_EXACT;
			}
		}

		if (IdcSHR->GetCheck()==BF_CHECKED) grid_effects|=GRID_RECOMB_SHR;
		else grid_effects&=(~GRID_RECOMB_SHR);
		if (IdcBimolecular->GetCheck()==BF_CHECKED) grid_effects|=GRID_RECOMB_B_B;
		else grid_effects&=(~GRID_RECOMB_B_B);
    	if (IdcAuger->GetCheck()==BF_CHECKED) grid_effects|=GRID_RECOMB_AUGER;
	    else grid_effects&=(~GRID_RECOMB_AUGER);
		if (IdcStimulated->GetCheck()==BF_CHECKED) grid_effects|=GRID_RECOMB_STIM;
		else grid_effects&=(~GRID_RECOMB_STIM);
		if (IdcOpticalGeneration->GetCheck()==BF_CHECKED) environment_effects|=ENV_OPTICAL_GEN;
		else environment_effects&=(~ENV_OPTICAL_GEN);

		if (IdcThermionic->GetCheck()==BF_CHECKED) grid_effects|=GRID_THERMIONIC;
		else grid_effects&=(~GRID_THERMIONIC);
		if (IdcTunneling->GetCheck()==BF_CHECKED) grid_effects|=GRID_TUNNELING;
		else grid_effects&=(~GRID_TUNNELING);

		if (IdcAbruptMaterials->GetCheck()==BF_CHECKED) grid_effects|=GRID_ABRUPT_MATERIALS;
		else grid_effects&=(~GRID_ABRUPT_MATERIALS);

		if (IdcDopingDepMobility->GetCheck()==BF_CHECKED) grid_effects|=GRID_DOPING_MOBILITY;
		else grid_effects&=(~GRID_DOPING_MOBILITY);

		environment.put_value(GRID_ELECTRICAL,EFFECTS,grid_effects);
		if (quantum_wells) environment.put_value(QUANTUM_WELL,EFFECTS,qw_effects);
		environment.put_value(DEVICE,EFFECTS,device_effects);
		environment.put_value(ENVIRONMENT,EFFECTS,environment_effects);

		TDialog::CmOk();
    }
}

//**************************** class TDialogThermalModels ***********************************
/*
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
*/

TDialogThermalModels::TDialogThermalModels(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	grid_effects=(flag)environment.get_value(GRID_ELECTRICAL,EFFECTS);
	device_effects=(flag)environment.get_value(DEVICE,EFFECTS);

	IdcIsothermal=new TCheckBox(this, IDC_ISOTHERMAL);
	IdcSingleTemp=new TCheckBox(this, IDC_SINGLETEMP);
	IdcSeparateTemp=new TCheckBox(this, IDC_SEPARATETEMP);
	IdcElectronTemp=new TCheckBox(this, IDC_ELECTRONTEMP);
	IdcJoule=new TCheckBox(this, IDC_JOULE);
	IdcThermoelectric=new TCheckBox(this, IDC_THERMOELECTRIC);
	IdcCarrierRelax=new TCheckBox(this, IDC_CARRIERRELAX);
	IdcLateral=new TCheckBox(this, IDC_LATERAL);
	IdcBandGap=new TCheckBox(this, IDC_BANDGAP);
	IdcTempDepMobility=new TCheckBox(this, IDC_TEMPDEPMOBILITY);
	IdcTempDepElectronAffinity=new TCheckBox(this, IDC_TEMPDEPELECTRONAFFINITY);
	IdcThermalConduct=new TCheckBox(this, IDC_THERMALCONDUCT);
	IdcIncPerm=new TCheckBox(this, IDC_INCPERM);
	IdcModePerm=new TCheckBox(this, IDC_MODEPERM);
}

void TDialogThermalModels::SetupWindow(void)
{
	TDialog::SetupWindow();

	if (device_effects & DEVICE_NON_ISOTHERMAL) {
		if (device_effects & DEVICE_SINGLE_TEMP) {
			IdcJoule->EnableWindow(TRUE);
			IdcThermoelectric->EnableWindow(TRUE);
			IdcLateral->EnableWindow(TRUE);
			IdcSingleTemp->SetCheck(BF_CHECKED);
		}
		else {
			IdcCarrierRelax->EnableWindow(TRUE);
			IdcElectronTemp->EnableWindow(TRUE);
			IdcSeparateTemp->SetCheck(BF_CHECKED);
		}
	}
	else IdcIsothermal->SetCheck(BF_CHECKED);

	IdcElectronTemp->SetCheck(BF_CHECKED);

	if (grid_effects & GRID_JOULE_HEAT) IdcJoule->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_THERMOELECTRIC_HEAT) IdcThermoelectric->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_LATERAL_HEAT) IdcLateral->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_RELAX) IdcCarrierRelax->SetCheck(BF_CHECKED);

	if (grid_effects & GRID_BAND_NARROWING) IdcBandGap->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_TEMP_MOBILITY) IdcTempDepMobility->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_TEMP_ELECTRON_AFFINITY) IdcTempDepElectronAffinity->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_TEMP_THERMAL_COND) IdcThermalConduct->SetCheck(BF_CHECKED);
	if (grid_effects & GRID_TEMP_INC_PERM) IdcIncPerm->SetCheck(BF_CHECKED);

	if (device_effects & DEVICE_LASER) {
		IdcModePerm->EnableWindow(TRUE);
		if (grid_effects & GRID_TEMP_MODE_PERM) IdcModePerm->SetCheck(BF_CHECKED);
	}
}

DEFINE_RESPONSE_TABLE1(TDialogThermalModels, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_CHILD_NOTIFY(IDC_ISOTHERMAL, BN_CLICKED, EvIsothermal),
	EV_CHILD_NOTIFY(IDC_SINGLETEMP, BN_CLICKED, EvSingleTemp),
	EV_CHILD_NOTIFY(IDC_SEPARATETEMP, BN_CLICKED, EvSeparateTemp),
END_RESPONSE_TABLE;

void TDialogThermalModels::CmOk(void)
{
    bool accept_values;

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcIsothermal->GetCheck()==BF_CHECKED) device_effects&=(~DEVICE_NON_ISOTHERMAL);
		else device_effects|=DEVICE_NON_ISOTHERMAL;
		if (IdcSingleTemp->GetCheck()==BF_CHECKED) device_effects|=DEVICE_SINGLE_TEMP;
		else device_effects&=(~DEVICE_SINGLE_TEMP);
		if (IdcElectronTemp->GetCheck()==BF_CHECKED) device_effects|=DEVICE_VARY_ELECTRON_TEMP;
		else device_effects&=(~DEVICE_VARY_ELECTRON_TEMP);

		if (IdcJoule->GetCheck()==BF_CHECKED) grid_effects|=GRID_JOULE_HEAT;
		else grid_effects&=(~GRID_JOULE_HEAT);
		if (IdcThermoelectric->GetCheck()==BF_CHECKED) grid_effects|=GRID_THERMOELECTRIC_HEAT;
		else grid_effects&=(~GRID_THERMOELECTRIC_HEAT);
		if (IdcLateral->GetCheck()==BF_CHECKED) grid_effects|=GRID_LATERAL_HEAT;
		else grid_effects&=(~GRID_LATERAL_HEAT);
		if (IdcCarrierRelax->GetCheck()==BF_CHECKED) grid_effects|=GRID_RELAX;
		else grid_effects&=(~GRID_RELAX);

		if (IdcBandGap->GetCheck()==BF_CHECKED) grid_effects|=GRID_BAND_NARROWING;
		else grid_effects&=(~GRID_BAND_NARROWING);

		if (IdcTempDepMobility->GetCheck()==BF_CHECKED) grid_effects|=GRID_TEMP_MOBILITY;
		else grid_effects&=(~GRID_TEMP_MOBILITY);

		if (IdcTempDepElectronAffinity->GetCheck()==BF_CHECKED) grid_effects|=GRID_TEMP_ELECTRON_AFFINITY;
		else grid_effects&=(~GRID_TEMP_ELECTRON_AFFINITY);

		if (IdcThermalConduct->GetCheck()==BF_CHECKED) grid_effects|=GRID_TEMP_THERMAL_COND;
		else grid_effects&=(~GRID_TEMP_THERMAL_COND);

		if (IdcIncPerm->GetCheck()==BF_CHECKED) grid_effects|=GRID_TEMP_INC_PERM;
		else grid_effects&=(~GRID_TEMP_INC_PERM);

		if (device_effects & DEVICE_LASER) {
			if (IdcModePerm->GetCheck()==BF_CHECKED) grid_effects|=GRID_TEMP_MODE_PERM;
			else grid_effects&=(~GRID_TEMP_MODE_PERM);
		}

		environment.put_value(GRID_ELECTRICAL,EFFECTS,grid_effects);
		environment.put_value(DEVICE,EFFECTS,device_effects);

		TDialog::CmOk();
    }
}

void TDialogThermalModels::EvIsothermal(void)
{
	IdcElectronTemp->EnableWindow(FALSE);
	IdcJoule->EnableWindow(FALSE);
	IdcThermoelectric->EnableWindow(FALSE);
	IdcLateral->EnableWindow(FALSE);
	IdcCarrierRelax->EnableWindow(FALSE);
}

void TDialogThermalModels::EvSingleTemp(void)
{
	IdcElectronTemp->EnableWindow(FALSE);
	IdcJoule->EnableWindow(TRUE);
	IdcThermoelectric->EnableWindow(TRUE);
	IdcLateral->EnableWindow(TRUE);
	IdcCarrierRelax->EnableWindow(FALSE);
}

void TDialogThermalModels::EvSeparateTemp(void)
{
	IdcElectronTemp->EnableWindow(TRUE);
	IdcJoule->EnableWindow(FALSE);
	IdcThermoelectric->EnableWindow(FALSE);
	IdcLateral->EnableWindow(FALSE);
	IdcCarrierRelax->EnableWindow(TRUE);
}

//*********************************** class TDialogLaser **************************************
/*
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
*/

TOpenSaveDialog::TData TDialogLaser::FileData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST,
											  "Data Files (*.dat)|*.dat|All Files (*.*)|*.*|",
											  "", "", "dat");
string TDialogLaser::mode_filename;

TDialogLaser::TDialogLaser(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcCavityLength=new TEdit(this, IDC_CAVITYLENGTH);
	IdcCavityLength->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcCavityArea=new TEdit(this, IDC_CAVITYAREA);
	IdcCavityArea->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcRef1=new TEdit(this,IDC_REF1);
	IdcRef1->SetValidator(new TScientificRangeValidator(0,1,EXCLUSIVE));
	IdcPower1=new TStatic(this, IDC_POWER1);
	IdcRef2=new TEdit(this, IDC_REF2);
	IdcRef2->SetValidator(new TScientificRangeValidator(0,1,EXCLUSIVE));
	IdcPower2=new TStatic(this, IDC_POWER2);
	IdcSpontFactor=new TEdit(this, IDC_SPONTFACTOR);
	IdcSpontFactor->SetValidator(new TScientificRangeValidator(0,1,INCLUSIVE));
	IdcWaveguideLoss=new TEdit(this, IDC_WAVEGUIDELOSS);
	IdcWaveguideLoss->SetValidator(new TScientificLowerValidator(0,INCLUSIVE));
	IdcWavelength=new TEdit(this, IDC_WAVELENGTH);
	IdcWavelength->SetValidator(new TScientificLowerValidator(0,EXCLUSIVE));
	IdcCompModeField=new TCheckBox(this, IDC_COMPMODEFIELD);
	IdcSearchWavelength=new TCheckBox(this, IDC_SEARCHWAVELENGTH);
	IdcTextFilename=new TStatic(this, IDC_TEXTFILENAME);
	IdcModeGain=new TStatic(this, IDC_MODEGAIN);
	IdcMirrorLoss=new TStatic(this, IDC_MIRRORLOSS);
	IdcGroupVelocity=new TStatic(this, IDC_GROUPVELOCITY);
	IdcPhotonLifetime=new TStatic(this, IDC_PHOTONLIFETIME);
	IdcPhotonNumber=new TStatic(this, IDC_PHOTONNUMBER);

	mode_effects=environment.get_value(MODE,EFFECTS);
	load_mode=FALSE;
}

void TDialogLaser::SetupWindow(void)
{
	char number_string[20];

	TDialog::SetupWindow();

	sprintf(number_string,"%.3f",environment.get_value(CAVITY,LENGTH));
	IdcCavityLength->SetText(number_string);
	sprintf(number_string,"%.3f",environment.get_value(CAVITY,AREA));
	IdcCavityArea->SetText(number_string);

	sprintf(number_string,"%.5f",environment.get_value(MIRROR,REFLECTIVITY,0));
	IdcRef1->SetText(number_string);
	sprintf(number_string,"%.5f",environment.get_value(MIRROR,REFLECTIVITY,1));
	IdcRef2->SetText(number_string);

	sprintf(number_string,"%.2e",environment.get_value(MIRROR,POWER,0));
	IdcPower1->SetText(number_string);
	sprintf(number_string,"%.2e",environment.get_value(MIRROR,POWER,1));
	IdcPower2->SetText(number_string);

	sprintf(number_string,"%.5e",environment.get_value(MODE,SPONT_FACTOR));
	IdcSpontFactor->SetText(number_string);
	sprintf(number_string,"%.3e",environment.get_value(MODE,WAVEGUIDE_LOSS));
	IdcWaveguideLoss->SetText(number_string);
	sprintf(number_string,"%.6f",environment.get_value(MODE,MODE_PHOTON_WAVELENGTH));
	IdcWavelength->SetText(number_string);

	if (mode_effects & MODE_COMPUTE) IdcCompModeField->SetCheck(BF_CHECKED);
	if (mode_effects & MODE_SEARCH_WAVELENGTH) IdcSearchWavelength->SetCheck(BF_CHECKED);

	IdcTextFilename->SetText(shorten_path(mode_filename).c_str());

	sprintf(number_string,"%.3e",environment.get_value(MODE,MODE_GAIN));
	IdcModeGain->SetText(number_string);
	sprintf(number_string,"%.3e",environment.get_value(MODE,MIRROR_LOSS));
	IdcMirrorLoss->SetText(number_string);
	sprintf(number_string,"%.5e",environment.get_value(MODE,MODE_GROUP_VELOCITY));
	IdcGroupVelocity->SetText(number_string);

	sprintf(number_string,"%.3e",environment.get_value(MODE,PHOTON_LIFETIME));
	IdcPhotonLifetime->SetText(number_string);
	sprintf(number_string,"%.3e",environment.get_value(MODE,MODE_TOTAL_PHOTONS));
	IdcPhotonNumber->SetText(number_string);
}

DEFINE_RESPONSE_TABLE1(TDialogLaser, TDialog)
	EV_COMMAND(IDOK, CmOk),
	EV_COMMAND(IDC_BUTTONFILENAME, CmButtonFilename),
END_RESPONSE_TABLE;

void TDialogLaser::CmOk(void)
{
	char number_string[20];
    bool accept_values;

    TMacro *solving_macro=macro_storage.get_solving_macro();
    if (environment.is_solving() || (solving_macro!=NULL)) {
    	int result = MessageBox("The Simulation is currently running. Do you want to stop the simulation and accept new values?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
            accept_values=TRUE;
        }
        else accept_values=FALSE;
    }
    else accept_values=TRUE;

    if (accept_values) {
		if (IdcCavityLength->IsValid() && IdcCavityArea->IsValid() && IdcRef1->IsValid() &&
			IdcRef2->IsValid() && IdcSpontFactor->IsValid() && IdcWavelength->IsValid() &&
			IdcWaveguideLoss->IsValid()) {

			IdcCavityLength->GetText(number_string,sizeof(number_string));
			environment.put_value(CAVITY,LENGTH,atof(number_string));
			IdcCavityArea->GetText(number_string,sizeof(number_string));
			environment.put_value(CAVITY,AREA,atof(number_string));

			IdcRef1->GetText(number_string,sizeof(number_string));
			environment.put_value(MIRROR,REFLECTIVITY,atof(number_string),0);
			IdcRef2->GetText(number_string,sizeof(number_string));
			environment.put_value(MIRROR,REFLECTIVITY,atof(number_string),1);

			IdcSpontFactor->GetText(number_string,sizeof(number_string));
			environment.put_value(MODE,SPONT_FACTOR,atof(number_string));
			IdcWaveguideLoss->GetText(number_string,sizeof(number_string));
			environment.put_value(MODE,WAVEGUIDE_LOSS,atof(number_string));

			IdcWavelength->GetText(number_string,sizeof(number_string));
			environment.put_value(MODE,MODE_PHOTON_WAVELENGTH,atof(number_string));

			if (IdcCompModeField->GetCheck()==BF_CHECKED) mode_effects|=MODE_COMPUTE;
			else mode_effects&=(~MODE_COMPUTE);

			if (IdcSearchWavelength->GetCheck()==BF_CHECKED) mode_effects|=MODE_SEARCH_WAVELENGTH;
			else mode_effects&=(~MODE_SEARCH_WAVELENGTH);

			if (load_mode) {
				::SetCursor(TCursor(NULL,IDC_WAIT));
				environment.read_data_file(mode_filename.c_str());
				::SetCursor(TCursor(NULL,IDC_ARROW));
			}

			environment.put_value(MODE,EFFECTS,mode_effects);

			if (!error_handler.fail()) TDialog::CmOk();
			else out_error_message(TRUE);
		}
		else TDialog::CmOk();
    }
}

void TDialogLaser::CmButtonFilename(void)
{
	string new_filename;

	if (TFileOpenDialog(this, FileData).Execute()==IDOK) {
		new_filename=FileData.FileName;
		if (mode_filename!=new_filename) {
			load_mode=TRUE;
			mode_filename=new_filename;
			IdcTextFilename->SetText(shorten_path(mode_filename).c_str());
		}
	}
}

//*********************************** class TDialogWriteBand ***********************************
/*
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
*/


TDialogWriteBand::TDialogWriteBand(TWindow *parent, TResId resId,
								   FlagType& new_flag_type, flag& new_reference_flag,
								   TModule *module)
	: TDialog(parent,resId,module),
	  reference_flag_type(new_flag_type),
	  reference_flag(new_reference_flag)
{
	IdcEVac=new TCheckBox(this,IDC_EVAC);
	IdcEc=new TCheckBox(this, IDC_EC);
	IdcEv=new TCheckBox(this, IDC_EV);
	IdcEfn=new TCheckBox(this, IDC_EFN);
	IdcEfp=new TCheckBox(this, IDC_EFP);
}

void TDialogWriteBand::SetupWindow(void)
{
	TDialog::SetupWindow();

	if (reference_flag_type==ELECTRON) {
		if (reference_flag==BAND_EDGE) IdcEc->Check();
		else IdcEfn->Check();
	}
	else {
		if (reference_flag_type==HOLE) {
			if (reference_flag==BAND_EDGE) IdcEv->Check();
			else IdcEfp->Check();
		}
		else IdcEVac->Check();
	}
}

DEFINE_RESPONSE_TABLE1(TDialogWriteBand, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogWriteBand::CmOk(void)
{
	if (IdcEVac->GetCheck()==BF_CHECKED) {
		reference_flag_type=GRID_ELECTRICAL;
		reference_flag=VACUUM_LEVEL;
	}
	if (IdcEc->GetCheck()==BF_CHECKED) {
		reference_flag_type=ELECTRON;
		reference_flag=BAND_EDGE;
	}
	if (IdcEv->GetCheck()==BF_CHECKED) {
		reference_flag_type=HOLE;
		reference_flag=BAND_EDGE;
	}
	if (IdcEfn->GetCheck()==BF_CHECKED) {
		reference_flag_type=ELECTRON;
		reference_flag=QUASI_FERMI;
	}
	if (IdcEfp->GetCheck()==BF_CHECKED) {
		reference_flag_type=HOLE;
		reference_flag=QUASI_FERMI;
	}

	TDialog::CmOk();
}

//*********************************** class TDialogDataWriteAll ********************************
/*
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
*/
TOpenSaveDialog::TData TDialogDataWriteAll::FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
													 "Data Files (*.dat)|*.dat|",
									   				 "", "", "dat");

TDialogDataWriteAll::TDialogDataWriteAll(TWindow *parent, TResId resId, TModule *module)
	: TDialog(parent,resId,module)
{
	IdcPositionDep=new TCheckBox(this,IDC_POSITIONDEP);
	IdcQWDep=new TCheckBox(this,IDC_QWDEP);
	IdcModeDep=new TCheckBox(this,IDC_MODEDEP);
	IdcMirrorDep=new TCheckBox(this,IDC_MIRRORDEP);
	IdcCavityDep=new TCheckBox(this,IDC_CAVITYDEP);
	IdcContactDep=new TCheckBox(this,IDC_CONTACTDEP);
	IdcSurfaceDep=new TCheckBox(this,IDC_SURFACEDEP);
	IdcDeviceDep=new TCheckBox(this,IDC_DEVICEDEP);
	IdcSpectrumDep=new TCheckBox(this,IDC_SPECTRUMDEP);
	IdcEnvDep=new TCheckBox(this,IDC_ENVDEP);
}

void TDialogDataWriteAll::SetupWindow(void)
{
	TDialog::SetupWindow();

	if (environment.device()) {
		IdcPositionDep->EnableWindow(TRUE);

		if (environment.get_number_objects(QUANTUM_WELL)) IdcQWDep->EnableWindow(TRUE);

		if ((flag)environment.get_value(DEVICE,EFFECTS) & DEVICE_LASER) {
			IdcModeDep->EnableWindow(TRUE);
			IdcMirrorDep->EnableWindow(TRUE);
			IdcCavityDep->EnableWindow(TRUE);
		}

		IdcContactDep->EnableWindow(TRUE);
		IdcSurfaceDep->EnableWindow(TRUE);
		IdcDeviceDep->EnableWindow(TRUE);
	}

	if (environment.get_number_objects(SPECTRUM)) IdcSpectrumDep->EnableWindow(TRUE);
}

DEFINE_RESPONSE_TABLE1(TDialogDataWriteAll, TDialog)
	EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

void TDialogDataWriteAll::CmOk(void)
{
	TValueFlag write_flags;

	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		if (IdcPositionDep->GetCheck()==BF_CHECKED) {
			write_flags.set(FREE_ELECTRON,FREE_ELECTRON_WRITE);
			write_flags.set(FREE_HOLE,FREE_HOLE_WRITE);

			if (environment.get_number_objects(QUANTUM_WELL)) {
				write_flags.set(BOUND_ELECTRON,BOUND_ELECTRON_WRITE);
				write_flags.set(BOUND_HOLE,BOUND_HOLE_WRITE);
			}

			write_flags.set(ELECTRON,ELECTRON_WRITE);
			write_flags.set(HOLE,HOLE_WRITE);
			write_flags.set(GRID_ELECTRICAL,GRID_ELECTRICAL_WRITE);
			write_flags.set(GRID_OPTICAL,GRID_OPTICAL_WRITE);
			write_flags.set(NODE,NODE_WRITE);
		}
		if (IdcQWDep->GetCheck()==BF_CHECKED) {
			write_flags.set(QW_ELECTRON,QW_ELECTRON_WRITE);
			write_flags.set(QW_HOLE,QW_HOLE_WRITE);
			write_flags.set(QUANTUM_WELL,QUANTUM_WELL_WRITE);
		}
		if (IdcModeDep->GetCheck()==BF_CHECKED) write_flags.set(MODE,MODE_WRITE);

		if (IdcMirrorDep->GetCheck()==BF_CHECKED) write_flags.set(MIRROR,MIRROR_WRITE);

		if (IdcCavityDep->GetCheck()==BF_CHECKED) write_flags.set(CAVITY,CAVITY_WRITE);

		if (IdcContactDep->GetCheck()==BF_CHECKED) write_flags.set(CONTACT,CONTACT_WRITE);

		if (IdcSurfaceDep->GetCheck()==BF_CHECKED) write_flags.set(SURFACE, SURFACE_WRITE);

		if (IdcDeviceDep->GetCheck()==BF_CHECKED) write_flags.set(DEVICE,DEVICE_WRITE);

		if (IdcSpectrumDep->GetCheck()==BF_CHECKED) write_flags.set(SPECTRUM,SPECTRUM_WRITE);

		if (IdcEnvDep->GetCheck()==BF_CHECKED) write_flags.set(ENVIRONMENT,ENVIRONMENT_WRITE);

		::SetCursor(TCursor(NULL,IDC_WAIT));
		environment.write_data_file(FileData.FileName,write_flags);
		::SetCursor(TCursor(NULL,IDC_ARROW));
		if (error_handler.fail()) out_error_message(TRUE);

		TDialog::CmOk();
	}
}



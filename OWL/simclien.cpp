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

#include "simparse.h"

extern TWindow* error_window;
extern TSimWindowsDeviceStatus* status_window;
extern TPrinter* printer;
extern TSimulateThread simulate_thread;
extern TMacroThread macro_thread;

#ifndef NDEBUG
extern logical enable_flag_check;
#endif


/*************************** class TSimWindowsMDIClient ***************************************

class TSimWindowsMDIClient: public TMDIClient {
private:
	TOpenSaveDialog::TData DeviceFileData;
	TOpenSaveDialog::TData MaterialFileData;
	int number_params;
	char** command_ptr;
	flag device_effects;

public:
	TSimWindowsMDIClient(int argc, char** argv);
	~TSimWindowsMDIClient(void);

#ifndef NDEBUG
public:
	virtual bool IdleAction(long idleCount);
#endif
protected:
	void SetupWindow(void);

private:
	void OpenFile(const char *filename);
	void CreateMultipleEnvironPlot(TValueFlag plot_flags);
	void CreateSingleEnvironPlot(TValueFlag plot_flags, string y_label, string title,
								 logical multi_colored=TRUE);
protected:
	DECLARE_RESPONSE_TABLE(TSimWindowsMDIClient);
	void EvMDIDestroy(HWND hWnd);

// Command Enablers
	void CmEditUndoEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.SetText("&Undo\tCtrl+Z"); commandHandler.Enable(FALSE); }
    void CmEnvironmentResetEnabler(TCommandEnabler& commandHandler)
    	{ commandHandler.Enable(!environment.is_solving() &&
        						(macro_storage.get_solving_macro()==NULL)); }
	void CmEnvironmentLoadMaterialEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(!environment.device()); }
	void CmMacroMenuEnabler(TCommandEnabler& commandHandler);
	void CmCreateMacroPopupEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(TRUE); }
	void CmDeviceMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device()); }
    void CmDeviceSolvingMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() && !environment.is_solving() &&
        						(macro_storage.get_solving_macro()==NULL)); }
	void CmQWMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() &&
								environment.get_number_objects(QUANTUM_WELL)); }
	void CmLaserMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() &&
							   ((flag)environment.get_value(DEVICE,EFFECTS) & DEVICE_LASER)); }
	void CmDeviceStartEnabler(TCommandEnabler& commandHandler);
    void CmDeviceStopEnabler(TCommandEnabler& commandHandler);
	void CmDeviceExecuteMacroEnabler(TCommandEnabler& commandHandler);
	void CmExternalSpectraEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.get_number_objects(SPECTRUM)); }
	void CmPlotFreezeEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.SetText("Freeze &Plot\tCtrl+F"); commandHandler.Enable(FALSE); }
	void CmPlotSelectedEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.get_number_objects(SPECTRUM) ||
								environment.device()); }
	void CmPlotMacroEnabler(TCommandEnabler& commandHandler);

// Command responses
	void CmFileNew(void);
	void CmFileOpen(void)
		{ if (TFileOpenDialog(this, DeviceFileData).Execute()==IDOK) OpenFile(DeviceFileData.FileName); }
	void CmFilePrinterSetup(void);
	void CmEnvironmentReset(void);
	void CmEnvironmentTemperature(void);
	void CmEnvironmentRadius(void);
	void CmEnvironmentOptical(void);
	void CmEnvironmentLoadMaterial(void);
	void CmEnvironmentVoltageMacro(void);
	void CmEnvironmentEditMacro(void);
	void CmEnvironmentDeleteMacro(void);
	void CmDeviceReset(void);
	void CmDeviceInformation(void) { TDialogDeviceInfo(this,DG_DEVICEINFO).Execute(); }
	void CmDeviceContacts(void);
	void CmDeviceSurfaces(void);
	void CmDeviceElectricalModels(void);
	void CmDeviceThermalModels(void);
	void CmDeviceLaserParameters(void);
	void CmDeviceStart(void);
    void CmDeviceStop(void);
	void CmDeviceExecuteMacro(void);
	void CmPlotExternalSpectra(void);
	void CmPlotLatticeTemp(void);
	void CmPlotElectronTemp(void);
	void CmPlotHoleTemp(void);
	void CmPlotDoping(void);
	void CmPlotBoundCarrierConc(void);
	void CmPlotFreeCarrierConc(void);
	void CmPlotTotalCarrierConc(void);
	void CmPlotCharge(void);
	void CmPlotField(void);
	void CmPlotPotential(void);
	void CmPlotBandDiagram(void);
	void CmPlotSHR(void);
	void CmPlotBimolecular(void);
    void CmPlotAuger(void);
	void CmPlotStimulated(void);
	void CmPlotOpticalGeneration(void);
	void CmPlotTotalRecomb(void);
	void CmPlotAllRecomb(void);
	void CmPlotCurrents(void);
	void CmPlotMode(void);
	void CmPlotGain(void);
	void CmPlotPhotonDensity(void);
	void CmPlotSelected(void);
	void CmPlotMacro(void);
	void CmDataRead(void);
	void CmDataWriteDevice(void);
	void CmDataWriteMaterial(void);
	void CmDataWriteSelected(void);
	void CmDataWriteAll(void) { TDialogDataWriteAll(this,DG_WRITEALLPARAMETERS).Execute(); }
	void CmHelpAbout(void) { TDialogAbout(this,DG_ABOUT).Execute(); }
	void CmHelpCredits(void) { TDialog(this,DG_CREDITS).Execute(); }
	void CmLastUpdated(void)
		{ extern char update_string[];
		  MessageBox(update_string,"Last Updated", MB_OK | MB_ICONINFORMATION); }
};

*/

TSimWindowsMDIClient::TSimWindowsMDIClient(int argc, char** argv)
	: TMDIClient(),
	  DeviceFileData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST,
					 "Device Files (*.dev)|*.dev|State Files (*.sta)|*.sta|Data Files (*.dat)|*.dat|Material Files (*.prm)|*.prm|All Files (*.*)|*.*|", "", "", "dev"),

	  MaterialFileData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST,
					   "Material Files (*.prm)|*.prm|All Files (*.*)|*.*|","", "", "prm")
{
	printer=new TPrinter;
	number_params=argc;
	command_ptr=argv;
}

TSimWindowsMDIClient::~TSimWindowsMDIClient(void)
{
	delete printer;
	printer=(TPrinter *)0;
}

#ifndef NDEBUG
bool TSimWindowsMDIClient::IdleAction(long /*idleCount*/)
{
	if (enable_flag_check) {
    	assert(!environment.effects_change_flags_any_set());
        assert(!environment.update_flags_any_set());
        assert(!environment.recompute_flags_any_set());
    }
    assert(error_window);
    assert(error_window->IsWindow());
	return(FALSE);
}
#endif

void TSimWindowsMDIClient::SetupWindow(void)
{
	extern char executable_path[MAXPATH];
	string material_string;
	TParseMaterial *material_parser;

	::SetCursor(TCursor(NULL,IDC_WAIT));
	TMDIClient::SetupWindow();
	material_string=string(executable_path)+preferences.get_material_parameters_file();
	strcpy(MaterialFileData.FileName,material_string.c_str());
	material_parser=new TParseMaterial(MaterialFileData.FileName);
	material_parser->parse_material();
	delete material_parser;

	if (error_handler.fail()) out_error_message(TRUE);
	else {
		if (number_params>=2) {
			strcpy(DeviceFileData.FileName,command_ptr[1]);
			OpenFile(DeviceFileData.FileName);
		}
	}
	::SetCursor(TCursor(NULL,IDC_ARROW));
}

void TSimWindowsMDIClient::OpenFile(const char *filename)
{
	TSimWindowsEditFile *editor_file;
	TSimWindowsDeviceStatus *new_status_window;
	TMDIChild *child_window;
	FILE *file_ptr;
	extern char state_string[];
	extern int state_string_size;
	char new_state_string[40];

	file_ptr=fopen(filename,"rb");
	if (!file_ptr) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		out_error_message(TRUE);
		return;
	}

	fread(new_state_string,state_string_size,1,file_ptr);
	fclose(file_ptr);
	if (!strcmp(new_state_string,state_string)) {
		if (status_window) {
			if(status_window->CanClear()) {
				delete status_window;
				status_window=(TSimWindowsDeviceStatus *)0;
			}
			else return;
		}
		::SetCursor(TCursor(NULL,IDC_WAIT));
		new_status_window=new TSimWindowsDeviceStatus();
		if(!new_status_window->Read(filename)) {
			out_error_message(TRUE);
			delete new_status_window;
			return;
		}
		else {
			child_window=new TMDIChild(*this,0,new_status_window);
			child_window->SetIcon(GetApplication(),IDI_STATE);
			child_window->Create();
			new_status_window->SetFileName(filename);
			new_status_window->Insert("Device successfully created\r\n\r\n");
			status_window=new_status_window;
		}
		ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
	else {
		editor_file=new TSimWindowsEditFile();
		editor_file->SetFileName(filename);
		child_window=new TMDIChild(*this,0,editor_file);
		child_window->SetIcon(GetApplication(),IDI_DEVICE);
		child_window->Create();
	}
}

void TSimWindowsMDIClient::CreateMultipleEnvironPlot(TValueFlag plot_flags)
{
	int i,j,max_bit;
	flag test_flag, valid_flag;

	for (i=START_FLAG_TYPE;i<=END_FLAG_TYPE;i++) {
		if (!plot_flags.any_set((FlagType)i)) continue;
		test_flag=1;
		max_bit=bit_position(TValueFlag::get_max((FlagType)i));
		valid_flag=TValueFlag::get_valid((FlagType)i);
		for (j=0;j<=max_bit;j++) {
			if ((test_flag & valid_flag) && plot_flags.is_set((FlagType)i,test_flag))
				CreateSingleEnvironPlot(TValueFlag((FlagType)i,test_flag),
										get_short_string((FlagType)i,test_flag),
										get_long_string((FlagType)i,test_flag),FALSE);
			test_flag<<=1;
		}
	}
}

void TSimWindowsMDIClient::CreateSingleEnvironPlot(TValueFlag plot_flags, string y_label, string title,
												   logical multi_colored)
{
	TMDIChild *plot;
	FlagType x_flag_type;
	flag x_flag;

#ifndef NDEBUG
	int i;

	for (i=1;i<=NUMBER_FLAG_TYPES;i++) {
		if (plot_flags.any_set((FlagType)i))
			assert(TValueFlag::valid_plot_flag((FlagType)i,plot_flags.get_flag((FlagType)i)));
	}
#endif

	::SetCursor(TCursor(NULL,IDC_WAIT));

	if (plot_flags.any_set(SPECTRUM)) {
		x_flag_type=SPECTRUM;
		x_flag=INCIDENT_PHOTON_WAVELENGTH;
	}
	else {
		x_flag_type=GRID_ELECTRICAL;
		x_flag=POSITION;
	}

	plot=new TMDIChild(*this,title.c_str(),
					   new TSimWindowsEnvironPlot(0,x_flag_type, x_flag, plot_flags,y_label.c_str(),multi_colored));
	if (!error_handler.fail()) {
		plot->SetIcon(GetApplication(),IDI_PLOT);
		plot->Create();
	}
	else out_error_message(TRUE);
	::SetCursor(TCursor(NULL,IDC_ARROW));
}

DEFINE_RESPONSE_TABLE1(TSimWindowsMDIClient, TMDIClient)
	EV_WM_MDIDESTROY,

// Command Enablers
	EV_COMMAND_ENABLE(CM_EDITUNDO, CmEditUndoEnabler),
    EV_COMMAND_ENABLE(CM_ENVIRONMENTRESET, CmEnvironmentResetEnabler),
	EV_COMMAND_ENABLE(CM_ENVIRONMENTLOADMATERIAL, CmEnvironmentLoadMaterialEnabler),
	EV_COMMAND_ENABLE(CM_ENVIRONMENTVOLTAGEMACRO-1,CmCreateMacroPopupEnabler),
	EV_COMMAND_ENABLE(CM_ENVIRONMENTEDITMACRO, CmMacroMenuEnabler),
	EV_COMMAND_ENABLE(CM_ENVIRONMENTDELETEMACRO, CmMacroMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICERESET, CmDeviceSolvingMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICEINFORMATION, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICECONTACTS, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICESURFACES, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICEELECTRICALMODELS, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICETHERMALMODELS, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICELASERPARAMETERS, CmLaserMenuEnabler),
	EV_COMMAND_ENABLE(CM_DEVICESTART, CmDeviceStartEnabler),
    EV_COMMAND_ENABLE(CM_DEVICESTOP, CmDeviceStopEnabler),
	EV_COMMAND_ENABLE(CM_DEVICEEXECUTEMACRO, CmDeviceExecuteMacroEnabler),
	EV_COMMAND_ENABLE(CM_PLOTFREEZE, CmPlotFreezeEnabler),
	EV_COMMAND_ENABLE(CM_PLOTEXTERNALSPECTRA, CmExternalSpectraEnabler),
	EV_COMMAND_ENABLE(CM_PLOTLATTEMP-1, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTLATTEMP, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTELECTEMP, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTHOLETEMP, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTDOPING, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTBOUNDCARRIERCONC-1, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTBOUNDCARRIERCONC, CmQWMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTFREECARRIERCONC, CmQWMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTCHARGE-1, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTBANDDIAGRAM, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTSHR-1, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTCURRENTS, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTMODE-1, CmLaserMenuEnabler),
	EV_COMMAND_ENABLE(CM_PLOTSELECTED, CmPlotSelectedEnabler),
	EV_COMMAND_ENABLE(CM_PLOTMACRO, CmPlotMacroEnabler),
	EV_COMMAND_ENABLE(CM_DATAREAD, CmDeviceSolvingMenuEnabler),
	EV_COMMAND_ENABLE(CM_DATAWRITEDEVICE, CmDeviceMenuEnabler),
	EV_COMMAND_ENABLE(CM_DATAWRITEMATERIAL, CmDeviceMenuEnabler),

// Command Responses
	EV_COMMAND(CM_FILENEW, CmFileNew),
	EV_COMMAND(CM_FILEOPEN, CmFileOpen),
	EV_COMMAND(CM_FILEPRINTERSETUP, CmFilePrinterSetup),

	EV_COMMAND(CM_ENVIRONMENTRESET, CmEnvironmentReset),
	EV_COMMAND(CM_ENVIRONMENTTEMPERATURE, CmEnvironmentTemperature),
	EV_COMMAND(CM_ENVIRONMENTRADIUS, CmEnvironmentRadius),
	EV_COMMAND(CM_ENVIRONMENTOPTICAL, CmEnvironmentOptical),
	EV_COMMAND(CM_ENVIRONMENTLOADMATERIAL, CmEnvironmentLoadMaterial),
	EV_COMMAND(CM_ENVIRONMENTVOLTAGEMACRO, CmEnvironmentVoltageMacro),
	EV_COMMAND(CM_ENVIRONMENTEDITMACRO, CmEnvironmentEditMacro),
	EV_COMMAND(CM_ENVIRONMENTDELETEMACRO, CmEnvironmentDeleteMacro),

	EV_COMMAND(CM_DEVICERESET, CmDeviceReset),
	EV_COMMAND(CM_DEVICEINFORMATION, CmDeviceInformation),
	EV_COMMAND(CM_DEVICECONTACTS, CmDeviceContacts),
	EV_COMMAND(CM_DEVICESURFACES, CmDeviceSurfaces),
	EV_COMMAND(CM_DEVICEELECTRICALMODELS, CmDeviceElectricalModels),
	EV_COMMAND(CM_DEVICETHERMALMODELS, CmDeviceThermalModels),
	EV_COMMAND(CM_DEVICELASERPARAMETERS, CmDeviceLaserParameters),
	EV_COMMAND(CM_DEVICESTART, CmDeviceStart),
    EV_COMMAND(CM_DEVICESTOP, CmDeviceStop),
	EV_COMMAND(CM_DEVICEEXECUTEMACRO, CmDeviceExecuteMacro),

	EV_COMMAND(CM_PLOTEXTERNALSPECTRA, CmPlotExternalSpectra),
	EV_COMMAND(CM_PLOTLATTEMP, CmPlotLatticeTemp),
	EV_COMMAND(CM_PLOTELECTEMP, CmPlotElectronTemp),
	EV_COMMAND(CM_PLOTHOLETEMP, CmPlotHoleTemp),
	EV_COMMAND(CM_PLOTDOPING, CmPlotDoping),
	EV_COMMAND(CM_PLOTBOUNDCARRIERCONC, CmPlotBoundCarrierConc),
	EV_COMMAND(CM_PLOTFREECARRIERCONC, CmPlotFreeCarrierConc),
	EV_COMMAND(CM_PLOTTOTALCARRIERCONC, CmPlotTotalCarrierConc),
	EV_COMMAND(CM_PLOTCHARGE, CmPlotCharge),
	EV_COMMAND(CM_PLOTFIELD, CmPlotField),
	EV_COMMAND(CM_PLOTPOTENTIAL, CmPlotPotential),
	EV_COMMAND(CM_PLOTBANDDIAGRAM,CmPlotBandDiagram),
	EV_COMMAND(CM_PLOTSHR, CmPlotSHR),
	EV_COMMAND(CM_PLOTBIMOLECULAR, CmPlotBimolecular),
    EV_COMMAND(CM_PLOTAUGER, CmPlotAuger),
	EV_COMMAND(CM_PLOTSTIMULATED, CmPlotStimulated),
	EV_COMMAND(CM_PLOTOPTICALGENERATION, CmPlotOpticalGeneration),
	EV_COMMAND(CM_PLOTTOTALRECOMB, CmPlotTotalRecomb),
	EV_COMMAND(CM_PLOTALLRECOMB, CmPlotAllRecomb),
	EV_COMMAND(CM_PLOTCURRENTS,CmPlotCurrents),
	EV_COMMAND(CM_PLOTMODE,CmPlotMode),
	EV_COMMAND(CM_PLOTGAIN,CmPlotGain),
	EV_COMMAND(CM_PLOTPHOTONDENSITY,CmPlotPhotonDensity),
	EV_COMMAND(CM_PLOTSELECTED, CmPlotSelected),
	EV_COMMAND(CM_PLOTMACRO, CmPlotMacro),

	EV_COMMAND(CM_DATAREAD, CmDataRead),
	EV_COMMAND(CM_DATAWRITEDEVICE, CmDataWriteDevice),
	EV_COMMAND(CM_DATAWRITEMATERIAL, CmDataWriteMaterial),
	EV_COMMAND(CM_DATAWRITESELECTED, CmDataWriteSelected),
	EV_COMMAND(CM_DATAWRITEALL, CmDataWriteAll),

	EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
    EV_COMMAND(CM_HELPCREDITS, CmHelpCredits),
	EV_COMMAND(CM_HELPLASTUPDATED, CmLastUpdated),
END_RESPONSE_TABLE;

void TSimWindowsMDIClient::EvMDIDestroy(HWND hWnd)
{
	if (status_window) {
		if (hWnd==status_window->Parent->HWindow) {
			environment.delete_device();
			ForEach(UpdateValidEnvironPlot);
			status_window=(TSimWindowsDeviceStatus *)0;
		}
	}
	TMDIClient::EvMDIDestroy(hWnd);
}

void TSimWindowsMDIClient::CmMacroMenuEnabler(TCommandEnabler& commandHandler)
{
	commandHandler.Enable(macro_storage.get_number_macros() &&
    					  (macro_storage.get_solving_macro()==NULL));
}

void TSimWindowsMDIClient::CmDeviceStartEnabler(TCommandEnabler& commandHandler)
{
	commandHandler.Enable(environment.device() &&
    					  simulate_thread.GetStatus()!=TThread::Running &&
                          macro_thread.GetStatus()!=TThread::Running);
}

void TSimWindowsMDIClient::CmDeviceStopEnabler(TCommandEnabler& commandHandler)
{
	commandHandler.Enable(simulate_thread.GetStatus()==TThread::Running ||
    					  macro_thread.GetStatus()==TThread::Running);
}

void TSimWindowsMDIClient::CmDeviceExecuteMacroEnabler(TCommandEnabler& commandHandler)
{
	commandHandler.Enable(environment.device() && macro_storage.get_number_macros() &&
    					  simulate_thread.GetStatus()!=TThread::Running &&
                          macro_thread.GetStatus()!=TThread::Running);
}

void TSimWindowsMDIClient::CmPlotMacroEnabler(TCommandEnabler& commandHandler)
{
	int i,number_macros;
	bool enable=FALSE;

	number_macros=macro_storage.get_number_macros();

	i=0;
	while (!enable && i<number_macros) {
		enable=(macro_storage.get_macro(i++)->has_data());
	}
	commandHandler.Enable(enable);
}

void TSimWindowsMDIClient::CmFileNew(void)
{
	TMDIChild *new_file;

	new_file=new TMDIChild(*this,0,new TSimWindowsEditFile());
	new_file->SetIcon(GetApplication(),IDI_DEVICE);
	new_file->Create();
}

void TSimWindowsMDIClient::CmFilePrinterSetup(void)
{
	if (printer) printer->Setup(this);
}

void TSimWindowsMDIClient::CmEnvironmentReset(void)
{

	if (MessageBox("Reset Environment?","Environment", MB_ICONQUESTION | MB_OKCANCEL)==IDOK) {
		environment.init_environment();
		ForEach(UpdateValidEnvironPlot);
	}

}

void TSimWindowsMDIClient::CmEnvironmentTemperature(void)
{
	char string[20];
	float temperature;
    bool accept_values;

	sprintf(string,"%.3f",(float)environment.get_value(ENVIRONMENT,TEMPERATURE));

	if(TInputDialog(this,
					"Environment","Input Environment Temperature (K)",
					string,sizeof(string),
					0, new TScientificRangeValidator(50.0,5000.0,INCLUSIVE)).Execute()==IDOK) {

		TMacro *solving_macro=macro_storage.get_solving_macro();
	    if (environment.is_solving() && (solving_macro!=NULL)) {
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
			::SetCursor(TCursor(NULL,IDC_WAIT));
			temperature=atof(string);
			environment.put_value(ENVIRONMENT,TEMPERATURE,temperature);
			if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
			::SetCursor(TCursor(NULL,IDC_ARROW));
        }
	}
}

void TSimWindowsMDIClient::CmEnvironmentRadius(void)
{
	char string[20];
	float device_radius,radius;
    bool accept_values;

	sprintf(string,"%.3f",(float)environment.get_value(ENVIRONMENT,RADIUS));

	if (environment.device()) device_radius=environment.get_value(GRID_ELECTRICAL,RADIUS,0);
	else device_radius=0.0;

	if(TInputDialog(this,
					"Environment","Input Environment Radius (microns)",
					string,sizeof(string),
					0, new TScientificRangeValidator(device_radius,1000,EXCLUSIVE)).Execute()==IDOK) {

		TMacro *solving_macro=macro_storage.get_solving_macro();
	    if (environment.is_solving() && (solving_macro!=NULL)) {
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
			::SetCursor(TCursor(NULL,IDC_WAIT));
			radius=atof(string);
			environment.put_value(ENVIRONMENT,RADIUS,radius);
			if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
			::SetCursor(TCursor(NULL,IDC_ARROW));
        }
	}
}

void TSimWindowsMDIClient::CmEnvironmentOptical(void)
{
	if (TDialogOpticalInput(this,DG_OPTICALINPUT).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmEnvironmentLoadMaterial(void)
{
	TParseMaterial *material_parser;

	if (TFileOpenDialog(this, MaterialFileData).Execute()==IDOK) {
		material_parser=new TParseMaterial(MaterialFileData.FileName);
		if (!error_handler.fail()) {
			material_parameters.clear();
			material_parser->parse_material();
		}
		if (error_handler.fail()) {
			material_parameters.clear();
			out_error_message(TRUE);
		}
		delete material_parser;
	}
}

void TSimWindowsMDIClient::CmEnvironmentVoltageMacro(void)
{
	TVoltageMacro *new_macro=new TVoltageMacro;

	if (TDialogVoltageMacro(this,DG_VOLTAGEMACRO,new_macro).Execute()==IDOK)
		macro_storage.add_macro(new_macro);
	else
		delete new_macro;
}

void TSimWindowsMDIClient::CmEnvironmentEditMacro(void)
{
	TVoltageMacro *new_macro;
	int macro_number;

	if (macro_storage.get_number_macros()==1) macro_number=0;
	else {
		if (TDialogSelectMacro(this,DG_SELECTMACRO,macro_number,"Select Macro to Edit").Execute()!=IDOK)
			return;
	}

	new_macro=(TVoltageMacro *)macro_storage.get_macro(macro_number);
	if (TDialogVoltageMacro(this,DG_VOLTAGEMACRO,new_macro).Execute()==IDOK)
		ForEach(UpdateValidMacroPlot);
}

void TSimWindowsMDIClient::CmEnvironmentDeleteMacro(void)
{
	int macro_number;
	string message_string;

	if (macro_storage.get_number_macros()==1) {
		macro_number=0;
		message_string="Are you sure you want to delete the macro \""+macro_storage.get_macro(0)->get_macro_name()+"\"";
		if (MessageBox(message_string.c_str(),"Delete Macro",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
	}
	else {
		if (TDialogSelectMacro(this,DG_SELECTMACRO,macro_number,"Select Macro to Edit").Execute()!=IDOK)
			return;
	}

	macro_storage.delete_macro(macro_number);
	ForEach(UpdateValidMacroPlot);
}

void TSimWindowsMDIClient::CmDeviceReset(void)
{
	if (MessageBox("Reset Device?","Device", MB_ICONQUESTION | MB_OKCANCEL)==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		environment.init_device();
		ForEach(UpdateValidEnvironPlot);
		status_window->Clear();
		status_window->Insert("Device Reset\r\n\r\n");
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceContacts(void)
{
	if (TDialogDeviceContacts(this,DG_CONTACT).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceSurfaces(void)
{
	if (TDialogDeviceSurfaces(this,DG_SURFACE).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceElectricalModels(void)
{
	if (TDialogElectricalModels(this,DG_ELECTRICALMODELS).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceThermalModels(void)
{
	if (TDialogThermalModels(this,DG_THERMALMODELS).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceLaserParameters(void)
{
	if (TDialogLaser(this,DG_LASER).Execute()==IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDeviceStart(void)
{
	if (preferences.is_multi_threaded()) {
    	switch(preferences.get_thread_priority()) {
        	case 1:
				simulate_thread.SetPriority(THREAD_PRIORITY_LOWEST);
                break;
            case 2:
				simulate_thread.SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
                break;
            case 3:
				simulate_thread.SetPriority(THREAD_PRIORITY_NORMAL);
                break;
            case 4:
				simulate_thread.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
                break;
            case 5:
				simulate_thread.SetPriority(THREAD_PRIORITY_HIGHEST);
                break;
            default: assert(FALSE); break;
        }
		simulate_thread.Start();
    }
    else {
		simulate_thread.Solve();
    }
}

void TSimWindowsMDIClient::CmDeviceStop(void)
{
	TMacro *solving_macro;

    solving_macro=macro_storage.get_solving_macro();
    if (solving_macro) solving_macro->set_stop_solution(TRUE);
	environment.set_stop_solution(TRUE);
}

void TSimWindowsMDIClient::CmDeviceExecuteMacro(void)
{
	int macro_number;
	static TOpenSaveDialog::TData FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
										   "Data Files (*.dat)|*.dat|",
										   "", "", "dat");

	if (macro_storage.get_number_macros()==1) macro_number=0;
	else {
		if (TDialogSelectMacro(this,DG_SELECTMACRO,macro_number,"Select Macro to Edit").Execute()!=IDOK)
			return;
	}

	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
    	macro_thread.SetMacroNumber(macro_number);
        macro_thread.SetFileName(FileData.FileName);
		if (preferences.is_multi_threaded()) {
    		switch(preferences.get_thread_priority()) {
        		case 1:
					macro_thread.SetPriority(THREAD_PRIORITY_LOWEST);
                	break;
	            case 2:
					macro_thread.SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
        	        break;
            	case 3:
					macro_thread.SetPriority(THREAD_PRIORITY_NORMAL);
                	break;
    	        case 4:
					macro_thread.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
            	    break;
	            case 5:
					macro_thread.SetPriority(THREAD_PRIORITY_HIGHEST);
        	        break;
            	default: assert(FALSE); break;
	        }
			macro_thread.Start();
	    }
    	else {
			macro_thread.Execute();
	    }
	}
}

void TSimWindowsMDIClient::CmPlotExternalSpectra(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_INCIDENT_SPECTRUM);
	CreateSingleEnvironPlot(plot_flags,"Intensity (mW cm-2)","External Optical Spectra");
}

void TSimWindowsMDIClient::CmPlotLatticeTemp(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_ELECTRICAL, TEMPERATURE);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_ELECTRICAL,TEMPERATURE),
								 get_long_string(GRID_ELECTRICAL, TEMPERATURE));
}

void TSimWindowsMDIClient::CmPlotElectronTemp(void)
{
	TValueFlag plot_flags;

	plot_flags.set(ELECTRON, TEMPERATURE);
	CreateSingleEnvironPlot(plot_flags,get_short_string(ELECTRON,TEMPERATURE),
								 get_long_string(ELECTRON, TEMPERATURE));
}

void TSimWindowsMDIClient::CmPlotHoleTemp(void)
{
	TValueFlag plot_flags;

	plot_flags.set(HOLE, TEMPERATURE);
	CreateSingleEnvironPlot(plot_flags,get_short_string(HOLE,TEMPERATURE),
								 get_long_string(HOLE,TEMPERATURE));
}

void TSimWindowsMDIClient::CmPlotDoping(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_DOPING);
	CreateSingleEnvironPlot(plot_flags,"Conc (cm-3)","Doping Conc (cm-3)");
}

void TSimWindowsMDIClient::CmPlotBoundCarrierConc(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_BOUND_CONC);
	CreateSingleEnvironPlot(plot_flags,"Conc (cm-3)","Bound Carrier Conc (cm-3)");
}

void TSimWindowsMDIClient::CmPlotFreeCarrierConc(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_FREE_CONC);
	CreateSingleEnvironPlot(plot_flags,"Conc (cm-3)","Free Carrier Conc (cm-3)");
}

void TSimWindowsMDIClient::CmPlotTotalCarrierConc(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_TOTAL_CONC);
	CreateSingleEnvironPlot(plot_flags,"Conc (cm-3)","Total Carrier Conc (cm-3)");
}

void TSimWindowsMDIClient::CmPlotCharge(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE, TOTAL_CHARGE);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE, TOTAL_CHARGE),
								 get_long_string(NODE,TOTAL_CHARGE));
}

void TSimWindowsMDIClient::CmPlotField(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_ELECTRICAL, FIELD);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_ELECTRICAL,FIELD),
								 get_long_string(GRID_ELECTRICAL,FIELD));
}

void TSimWindowsMDIClient::CmPlotPotential(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_ELECTRICAL, POTENTIAL);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_ELECTRICAL,POTENTIAL),
								 get_long_string(GRID_ELECTRICAL,POTENTIAL));
}

void TSimWindowsMDIClient::CmPlotBandDiagram(void)
{
	TMDIChild *plot;
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_BAND);
	::SetCursor(TCursor(NULL,IDC_WAIT));
	plot=new TMDIChild(*this,"Band Diagram",
					   new TSimWindowsBandDiagram(0,plot_flags,"Energy (eV)",TRUE));
	if (!error_handler.fail()) {
		plot->SetIcon(GetApplication(),IDI_PLOT);
		plot->Create();
	}
	else out_error_message(TRUE);
	::SetCursor(TCursor(NULL,IDC_ARROW));
}

void TSimWindowsMDIClient::CmPlotSHR(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE,SHR_RECOMB);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,SHR_RECOMB),
								 get_long_string(NODE,SHR_RECOMB));
}

void TSimWindowsMDIClient::CmPlotBimolecular(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE,B_B_RECOMB);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,B_B_RECOMB),
								 get_long_string(NODE,B_B_RECOMB));
}

void TSimWindowsMDIClient::CmPlotAuger(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE,AUGER_RECOMB);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,AUGER_RECOMB),
								 get_long_string(NODE,AUGER_RECOMB));
}

void TSimWindowsMDIClient::CmPlotStimulated(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE, STIM_RECOMB);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,STIM_RECOMB),
								 get_long_string(NODE,STIM_RECOMB));
}

void TSimWindowsMDIClient::CmPlotOpticalGeneration(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE,OPTICAL_GENERATION);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,OPTICAL_GENERATION),
								 get_long_string(NODE,OPTICAL_GENERATION));
}

void TSimWindowsMDIClient::CmPlotTotalRecomb(void)
{
	TValueFlag plot_flags;

	plot_flags.set(NODE,TOTAL_RECOMB);
	CreateSingleEnvironPlot(plot_flags,get_short_string(NODE,TOTAL_RECOMB),
								 get_long_string(NODE,TOTAL_RECOMB));
}

void TSimWindowsMDIClient::CmPlotAllRecomb(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_RECOMB);
	CreateSingleEnvironPlot(plot_flags,"Rate (1/cm3s)","All Recombination");
}

void TSimWindowsMDIClient::CmPlotCurrents(void)
{
	TValueFlag plot_flags;

	plot_flags.set_plot_combo(COMBO_CURRENT);
	CreateSingleEnvironPlot(plot_flags,"Current (A/cm2)","Carrier Currents");
}

void TSimWindowsMDIClient::CmPlotMode(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG),
								 get_long_string(GRID_OPTICAL,MODE_TOTAL_FIELD_MAG));
}

void TSimWindowsMDIClient::CmPlotGain(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_OPTICAL,MODE_GAIN);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_OPTICAL,MODE_GAIN),
								 get_long_string(GRID_OPTICAL,MODE_GAIN));
}

void TSimWindowsMDIClient::CmPlotPhotonDensity(void)
{
	TValueFlag plot_flags;

	plot_flags.set(GRID_OPTICAL,MODE_PHOTON_DENSITY);
	CreateSingleEnvironPlot(plot_flags,get_short_string(GRID_OPTICAL,MODE_PHOTON_DENSITY),
								 get_long_string(GRID_OPTICAL,MODE_PHOTON_DENSITY));
}

void TSimWindowsMDIClient::CmPlotSelected(void)
{
	TValueFlag plot_flags;
	TValueFlag selected_flags;

	if (environment.device()) {
		plot_flags.set(FREE_ELECTRON, FREE_ELECTRON_PLOT);
		plot_flags.set(FREE_HOLE, FREE_HOLE_PLOT);
		plot_flags.set(ELECTRON, ELECTRON_PLOT);
		plot_flags.set(HOLE, HOLE_PLOT);
		plot_flags.set(GRID_ELECTRICAL, GRID_ELECTRICAL_PLOT);
		plot_flags.set(GRID_OPTICAL, GRID_OPTICAL_PLOT);
		plot_flags.set(NODE, NODE_PLOT);

		if (environment.get_number_objects(QUANTUM_WELL)) {
			plot_flags.set(BOUND_ELECTRON, BOUND_ELECTRON_PLOT);
			plot_flags.set(BOUND_HOLE, BOUND_HOLE_PLOT);
		}
		else {
			selected_flags.clear(BOUND_ELECTRON, BOUND_ELECTRON_PLOT);
			selected_flags.clear(BOUND_HOLE, BOUND_HOLE_PLOT);
		}
	}
	else {
		selected_flags.clear(FREE_ELECTRON, FREE_ELECTRON_PLOT);
		selected_flags.clear(FREE_HOLE, FREE_HOLE_PLOT);
		selected_flags.clear(ELECTRON, ELECTRON_PLOT);
		selected_flags.clear(HOLE, HOLE_PLOT);
		selected_flags.clear(GRID_ELECTRICAL, GRID_ELECTRICAL_PLOT);
		selected_flags.clear(GRID_OPTICAL, GRID_OPTICAL_PLOT);
		selected_flags.clear(NODE, NODE_PLOT);
	}

	if (environment.get_number_objects(SPECTRUM)) plot_flags.set(SPECTRUM, SPECTRUM_PLOT);
	else selected_flags.clear(SPECTRUM, SPECTRUM_PLOT);

	if (TDialogSelectParameters(this,DG_SELECTPARAMETERS,plot_flags,selected_flags).Execute()==IDOK) {
		if (selected_flags.any_set()) {
			::SetCursor(TCursor(NULL,IDC_WAIT));
			CreateMultipleEnvironPlot(selected_flags);
			::SetCursor(TCursor(NULL,IDC_ARROW));
			if (error_handler.fail()) out_error_message(TRUE);
		}
	}
}

void TSimWindowsMDIClient::CmPlotMacro(void)
{

	int macro_number;
	FlagType y_flag_type, x_flag_type;
	flag y_flag_value, x_flag_value;
	int x_object_number;
	TMDIChild *plot;
	string y_label, title;

	if (macro_storage.get_number_macros()==1) macro_number=0;
	else {
		if (TDialogSelectMacro(this,DG_SELECTMACRO,macro_number,"Select Macro to Plot",TRUE).Execute()!=IDOK)
			return;
	}

    TValueFlagWithObject record_flags=macro_storage.get_macro(macro_number)->get_record_flags();
	if (TDialogSelectOneParameter(this,DG_SELECTONEPARAMETER,
								  record_flags, y_flag_type,y_flag_value).Execute()==IDOK) {
		if (y_flag_value) {
			::SetCursor(TCursor(NULL,IDC_WAIT));

			macro_storage.get_macro(macro_number)->get_increment_object(x_flag_type,x_flag_value,x_object_number);

			y_label=get_short_string(y_flag_type,y_flag_value);
			title=get_long_string(y_flag_type,y_flag_value);

			plot=new TMDIChild(*this,title.c_str(),
							   new TSimWindowsMacroPlot(0,macro_number,
														x_flag_type, x_flag_value, x_object_number,
														y_flag_type,y_flag_value,y_label.c_str(),TRUE));
			::SetCursor(TCursor(NULL,IDC_ARROW));
			if (!error_handler.fail()) {
				plot->SetIcon(GetApplication(),IDI_PLOT);
				plot->Create();
			}
			else out_error_message(TRUE);
		}
	}

}

void TSimWindowsMDIClient::CmDataRead(void)
{
	static TOpenSaveDialog::TData FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST,
										   "Data Files (*.dat)|*.dat|All Files (*.*)|*.*|",
										   "", "", "dat");

	if ((new TFileOpenDialog(this, FileData))->Execute() == IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		environment.read_data_file(FileData.FileName);
		if (environment.process_recompute_flags()) ForEach(UpdateValidEnvironPlot);
		if (error_handler.fail()) out_error_message(TRUE);
		::SetCursor(TCursor(NULL,IDC_ARROW));
	}
}

void TSimWindowsMDIClient::CmDataWriteDevice(void)
{
	TValueFlag write_flags;
	static TOpenSaveDialog::TData FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
										   "Data Files (*.dat)|*.dat|",
										   "", "", "dat");

	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		write_flags.set_write_combo(COMBO_STRUCTURE);
		::SetCursor(TCursor(NULL,IDC_WAIT));
		environment.write_data_file(FileData.FileName,write_flags);
		::SetCursor(TCursor(NULL,IDC_ARROW));
		if (error_handler.fail()) out_error_message(TRUE);
	}
}

void TSimWindowsMDIClient::CmDataWriteMaterial(void)
{
	TValueFlag write_flags;
	static TOpenSaveDialog::TData FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
										   "Data Files (*.dat)|*.dat|",
										   "", "", "dat");

	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		write_flags.set_write_combo(COMBO_MATERIAL);
		::SetCursor(TCursor(NULL,IDC_WAIT));
		environment.write_data_file(FileData.FileName,write_flags);
		::SetCursor(TCursor(NULL,IDC_ARROW));
		if (error_handler.fail()) out_error_message(TRUE);
	}
}

void TSimWindowsMDIClient::CmDataWriteSelected(void)
{
	TValueFlag write_flags;
	static TValueFlag selected_flags;
	static TOpenSaveDialog::TData FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
										   "Data Files (*.dat)|*.dat|",
										   "", "", "dat");

	if (environment.device()) {
		write_flags.set(FREE_ELECTRON, FREE_ELECTRON_WRITE);
		write_flags.set(FREE_HOLE, FREE_HOLE_WRITE);
		write_flags.set(ELECTRON, ELECTRON_WRITE);
		write_flags.set(HOLE, HOLE_WRITE);
		write_flags.set(GRID_ELECTRICAL, GRID_ELECTRICAL_WRITE);
		write_flags.set(GRID_OPTICAL, GRID_OPTICAL_WRITE);
		write_flags.set(NODE, NODE_WRITE);

		if (environment.get_number_objects(QUANTUM_WELL)) {
			write_flags.set(BOUND_ELECTRON, BOUND_ELECTRON_WRITE);
			write_flags.set(BOUND_HOLE, BOUND_HOLE_WRITE);
			write_flags.set(QW_ELECTRON, QW_ELECTRON_WRITE);
			write_flags.set(QW_HOLE, QW_HOLE_WRITE);
			write_flags.set(QUANTUM_WELL, QUANTUM_WELL_WRITE);
		}
		else {
			selected_flags.clear(BOUND_ELECTRON, BOUND_ELECTRON_WRITE);
			selected_flags.clear(BOUND_HOLE, BOUND_HOLE_WRITE);
			selected_flags.clear(QW_ELECTRON, QW_ELECTRON_WRITE);
			selected_flags.clear(QW_HOLE, QW_HOLE_WRITE);
			selected_flags.clear(QUANTUM_WELL, QUANTUM_WELL_WRITE);
		}

		if ((flag)environment.get_value(DEVICE,EFFECTS) & DEVICE_LASER) {
			write_flags.set(MIRROR, MIRROR_WRITE);
			write_flags.set(MODE, MODE_WRITE);
			write_flags.set(CAVITY, CAVITY_WRITE);
		}
		else {
			selected_flags.clear(MIRROR, MIRROR_WRITE);
			selected_flags.clear(MODE, MODE_WRITE);
			selected_flags.clear(CAVITY, CAVITY_WRITE);
		}

		write_flags.set(CONTACT, CONTACT_WRITE);
		write_flags.set(SURFACE, SURFACE_WRITE);
		write_flags.set(DEVICE, DEVICE_WRITE);
	}
	else {
		selected_flags.clear(FREE_ELECTRON, FREE_ELECTRON_WRITE);
		selected_flags.clear(FREE_HOLE, FREE_HOLE_WRITE);
		selected_flags.clear(ELECTRON, ELECTRON_WRITE);
		selected_flags.clear(HOLE, HOLE_WRITE);
		selected_flags.clear(GRID_ELECTRICAL, GRID_ELECTRICAL_WRITE);
		selected_flags.clear(GRID_OPTICAL, GRID_OPTICAL_WRITE);
		selected_flags.clear(NODE, NODE_WRITE);

		selected_flags.clear(BOUND_ELECTRON, BOUND_ELECTRON_WRITE);
		selected_flags.clear(BOUND_HOLE, BOUND_HOLE_WRITE);
		selected_flags.clear(QW_ELECTRON, QW_ELECTRON_WRITE);
		selected_flags.clear(QW_HOLE, QW_HOLE_WRITE);
		selected_flags.clear(QUANTUM_WELL, QUANTUM_WELL_WRITE);

		selected_flags.clear(MIRROR, MIRROR_WRITE);
		selected_flags.clear(MODE, MODE_WRITE);
		selected_flags.clear(CAVITY, CAVITY_WRITE);

		selected_flags.clear(CONTACT, CONTACT_WRITE);
		selected_flags.clear(SURFACE, SURFACE_WRITE);
		selected_flags.clear(DEVICE, DEVICE_WRITE);
	}


	if (environment.get_number_objects(SPECTRUM)) write_flags.set(SPECTRUM, SPECTRUM_WRITE);
	else selected_flags.clear(SPECTRUM, SPECTRUM_WRITE);

	write_flags.set(ENVIRONMENT, ENVIRONMENT_WRITE);

	if (TDialogSelectParameters(this,DG_SELECTPARAMETERS,write_flags,selected_flags).Execute()==IDOK) {
		if (selected_flags.any_set()) {
			if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
				::SetCursor(TCursor(NULL,IDC_WAIT));
				environment.write_data_file(FileData.FileName,selected_flags);
				::SetCursor(TCursor(NULL,IDC_ARROW));
				if (error_handler.fail()) out_error_message(TRUE);
			}
		}
	}
}



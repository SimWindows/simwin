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

//********************************* Global Variables *******************************************

#include "strtable.h"

TEnvironment environment;
TMaterialStorage material_parameters;
TErrorHandler error_handler;
TMacroStorage macro_storage;
TPreferences preferences;
NormalizeConstants normalization;
TSimulateThread simulate_thread;
TMacroThread macro_thread;

TWindow *error_window=(TWindow *)0;
TSimWindowsDeviceStatus *status_window=(TSimWindowsDeviceStatus *)0;
TPrinter *printer=(TPrinter *)0;
TTextGadget *caret_position=(TTextGadget *)0;

#ifndef NDEBUG
logical enable_flag_check=TRUE;
#endif

/********************************** class TSimWindows ****************************************

class TSimWindows: public TApplication {
private:
	int number_params;
	char** command_ptr;
	TControlBar *cb;
	TStatusBar *sb;
	TMDIClient *SimWindowsMDIClient;
	TDecoratedMDIFrame *SimWindowsFrame;
public:
	TSimWindows(int argc, char* argv[]);
	virtual void InitMainWindow(void);
protected:
	DECLARE_RESPONSE_TABLE(TSimWindows);
	void CmEnvironmentPreferences(void);
    void CmSimulationPreferences(void);
    void CmSavePreferences(void) { SavePreferences(); }
};
*/

TSimWindows::TSimWindows(int argc, char* argv[])
	: TApplication(application_string)
{
	extern char executable_path[MAXPATH];
    char full_path[MAXPATH];
	int executable_position;
	char enc_parameter;
	size_t i;

	number_params=argc;
	command_ptr=argv;

	rnd_init();

	enc_parameter=1;
	for (i=0;i<strlen(about_string);i++) {
		if (about_string[i]!='\n') about_string[i]-=enc_parameter;
		enc_parameter++;
		if (enc_parameter>ENCODE_KEY) enc_parameter-=(char)ENCODE_KEY;
	}

	cb=(TControlBar *)0;
	sb=(TStatusBar *)0;
	SimWindowsMDIClient=(TSimWindowsMDIClient *)0;
	SimWindowsFrame=(TDecoratedMDIFrame *)0;

	GetModuleFileName(full_path,sizeof(full_path));
	string path_string(full_path);
	executable_position=path_string.find(executable_string);
    if (executable_position<0) executable_position=path_string.find(cap_executable_string);
	assert(executable_position>=0);
	path_string.remove(executable_position);

	strcpy(executable_path,path_string.c_str());
}

void TSimWindows::InitMainWindow(void)
{

#ifdef NDEBUG
	clock_t sim_start_time, sim_end_time;
#endif

	LoadPreferences();

	SimWindowsMDIClient=new TSimWindowsMDIClient(number_params,command_ptr);
	error_window=SimWindowsMDIClient;
	::SetCursor(TCursor(NULL,IDC_WAIT));
	SimWindowsFrame=new TDecoratedMDIFrame(application_string, MAIN_MENU,*SimWindowsMDIClient,TRUE);

	sb=new TStatusBar(SimWindowsFrame, TGadget::Recessed, TStatusBar::CapsLock | TStatusBar::NumLock);
	cb=new TControlBar(SimWindowsFrame);
	caret_position=new TTextGadget(0,TTextGadget::Recessed,TTextGadget::Center,5);

	sb->Insert(*new TSeparatorGadget(4));
	sb->Insert(*caret_position);

	SimWindowsFrame->SetIcon(this,IDI_SIMWINDOWS);

	TDialog intro_dialog(SimWindowsFrame, DG_INTRO);

	nCmdShow = (nCmdShow != SW_SHOWMINNOACTIVE) ? SW_SHOWMAXIMIZED : nCmdShow;

	cb->Insert(*new TButtonGadget(CM_EXIT, CM_EXIT));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_FILENEW, CM_FILENEW));
	cb->Insert(*new TButtonGadget(CM_FILEOPEN, CM_FILEOPEN));
	cb->Insert(*new TButtonGadget(CM_FILESAVE, CM_FILESAVE));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_FILEPRINT, CM_FILEPRINT));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_EDITUNDO, CM_EDITUNDO));
	cb->Insert(*new TButtonGadget(CM_EDITCUT, CM_EDITCUT));
	cb->Insert(*new TButtonGadget(CM_EDITCOPY, CM_EDITCOPY));
	cb->Insert(*new TButtonGadget(CM_EDITPASTE, CM_EDITPASTE));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_DEVICEGENERATE, CM_DEVICEGENERATE));
	cb->Insert(*new TButtonGadget(CM_DEVICEINFORMATION,CM_DEVICEINFORMATION));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_DEVICESTART, CM_DEVICESTART));
	cb->Insert(*new TButtonGadget(CM_DEVICESTOP, CM_DEVICESTOP));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_DEVICECONTACTS, CM_DEVICECONTACTS));
	cb->Insert(*new TButtonGadget(CM_DEVICESURFACES, CM_DEVICESURFACES));
	cb->Insert(*new TSeparatorGadget(6));
	cb->Insert(*new TButtonGadget(CM_PLOTSHOWTRACE,CM_PLOTSHOWTRACE));
	cb->SetHintMode(TGadgetWindow::EnterHints);

	if (preferences.is_toolbar())
		SimWindowsFrame->Insert(*cb, TDecoratedFrame::Top);

	if (preferences.is_statusbar())
		SimWindowsFrame->Insert(*sb, TDecoratedFrame::Bottom);

	SimWindowsFrame->Attr.AccelTable = MAIN_MENU;
	SimWindowsFrame->AssignMenu(MAIN_MENU);

	SetMainWindow(SimWindowsFrame);

	EnableBWCC(TRUE);

#ifdef NDEBUG
	intro_dialog.Create();
	sim_start_time=clock();
	do {
		sim_end_time=clock();
	} while (((sim_end_time-sim_start_time)/CLK_TCK) <=4.0);
	intro_dialog.Destroy();
#endif

	::SetCursor(TCursor(NULL,IDC_ARROW));

#ifndef NDEBUG
	SimWindowsFrame->MessageBox(debug_string,SimWindowsFrame->GetApplication()->GetName(),
								MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
	valid_string_table();
#else

#ifdef CUSTOM
	SimWindowsFrame->MessageBox(custom_string,SimWindowsFrame->GetApplication()->GetName(),
								MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
#endif

#endif

}

DEFINE_RESPONSE_TABLE1(TSimWindows, TApplication)
	EV_COMMAND(CM_ENVIRONMENTPREFERENCES, CmEnvironmentPreferences),
    EV_COMMAND(CM_ENVIRONMENTSIMPREFERENCES, CmSimulationPreferences),
    EV_COMMAND(CM_ENVIRONMENTSAVE, CmSavePreferences),
END_RESPONSE_TABLE;

void TSimWindows::CmEnvironmentPreferences(void)
{

	if (TDialogEnvironmentPreferences(SimWindowsMDIClient,DG_ENVPREFERENCES).Execute()==IDOK) {
		if (preferences.is_toolbar()) {
            cb->Show(SW_SHOW);
        	SimWindowsFrame->Insert(*cb, TDecoratedFrame::Top);
        }
		else {
            cb->Show(SW_HIDE);
        	SimWindowsFrame->Insert(*cb, TDecoratedFrame::None);
        }

		if (preferences.is_statusbar()) {
        	sb->Show(SW_SHOW);
        	SimWindowsFrame->Insert(*sb, TDecoratedFrame::Bottom);
        }
		else {
        	sb->Show(SW_HIDE);
        	SimWindowsFrame->Insert(*sb, TDecoratedFrame::None);
        }

        SimWindowsFrame->Layout();
        environment.process_recompute_flags();
	}
}

void TSimWindows::CmSimulationPreferences(void)
{
	if (TDialogSimulationPreferences(SimWindowsMDIClient,DG_SIMPREFERENCES).Execute()==IDOK) {
		environment.process_recompute_flags();
	}
}

/*************************** class TSimulateThread **********************************

class TSimulateThread : public TThread
{
private:
	int Run(void) { Solve(); return(0); }
public:
	void Solve(void);
};
*/

void TSimulateThread::Solve(void)
{
	clock_t start_time, end_time;
	char time_string[40];

#ifndef NDEBUG
	enable_flag_check=FALSE;
#endif
	status_window->Insert("Start Simulation\r\n");
	out_operating_condition();
	start_time=clock();
	environment.solve();
	end_time=clock();
	out_simulation_result();
	if (error_handler.fail()) {
		sprintf(time_string,"Simulation Terminated - time = %.3f s\r\n\r\n",(end_time-start_time)/CLK_TCK);
		status_window->Insert(time_string);
		out_error_message(TRUE);
	}
	else {
		sprintf(time_string,"End Simulation - time = %.3f s\r\n\r\n",(end_time-start_time)/CLK_TCK);
		status_window->Insert(time_string);
	}
	error_window->ForEach(UpdateValidEnvironPlot);
#ifndef NDEBUG
	enable_flag_check=TRUE;
#endif

}

/*************************** class TSimulateThread **********************************

class TMacroThread : public TThread
{
private:
	int MacroNumber;
    const char *FileName;

	int Run(void) { Execute(); return(0); }
public:
	TMacroThread(void) : TThread(), MacroNumber(0), FileName(NULL) {}
	void Execute(void);
    void SetMacroNumber(int macroNumber) { MacroNumber=macroNumber; }
    void SetFileName(const char *fileName) { FileName=fileName; }
};
*/

void TMacroThread::Execute(void)
{
#ifndef NDEBUG
	enable_flag_check=FALSE;
#endif
	macro_storage.get_macro(MacroNumber)->execute(FileName);
    error_window->ForEach(UpdateValidMacroPlot);
    if (error_handler.fail()) out_error_message(TRUE);
#ifndef NDEBUG
	enable_flag_check=TRUE;
#endif
}


//************************************ function OwlMain ***************************************
// This is the official start and end of the program

int OwlMain(int argc, char* argv[])
{
	TSimWindows application(argc, argv);
	return application.Run();
}


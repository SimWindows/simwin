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

#include <winsys\profile.h>

/************************************* child iterator functions *******************************/

void UpdateValidEnvironPlot(TWindow *window, void*)
{
	TMDIChild *mdi_child;
	TWindow *client_window=(TWindow *)0;
	TSimWindowsEnvironPlot *plot_window=(TSimWindowsEnvironPlot *)0;

	mdi_child=dynamic_cast<TMDIChild *>(window);
	if (mdi_child) client_window=mdi_child->GetClientWindow();
	if (client_window) plot_window=dynamic_cast<TSimWindowsEnvironPlot *>(client_window);
	if (plot_window) {
		if (plot_window->should_close()) {
			mdi_child->CloseWindow();
			delete mdi_child;
		}
		else {
			if (!plot_window->is_frozen()) {
				plot_window->update_data();
				plot_window->RedrawWindow(NULL,NULL);
			}
		}
	}
}

void UpdateValidMacroPlot(TWindow *window, void*)
{
	TMDIChild *mdi_child;
	TWindow *client_window=(TWindow *)0;
	TSimWindowsMacroPlot *plot_window=(TSimWindowsMacroPlot *)0;

	mdi_child=dynamic_cast<TMDIChild *>(window);
	if (mdi_child) client_window=mdi_child->GetClientWindow();
	if (client_window) plot_window=dynamic_cast<TSimWindowsMacroPlot *>(client_window);
	if (plot_window) {
		if (plot_window->should_close()) {
			mdi_child->CloseWindow();
			delete mdi_child;
		}
		else {
			if (!plot_window->is_frozen()) {
				plot_window->update_data();
				plot_window->Invalidate(TRUE);
			}
		}
	}
}

//*********************************** Misc Windows Functions ***********************************

void LoadPreferences(void)
{
	extern char executable_path[MAXPATH], ini_filename[];
	flag env_effects;
	char file_string[20];
	char number_string[20];

	string ini_string(string(executable_path)+string(ini_filename));

	TProfile profile("Environment Defaults",ini_string.c_str());

	env_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);

	preferences.enable_toolbar(profile.GetInt("ToolBar",1)!=0);
	preferences.enable_statusbar(profile.GetInt("StatusBar",1)!=0);

	if(profile.GetInt("SimulationUndo",0)!=0) env_effects|=ENV_UNDO_SIMULATION;
	else env_effects&=(~ENV_UNDO_SIMULATION);

	profile.GetString("MaterialFile",file_string,sizeof(file_string),"material.prm");
	preferences.put_material_parameters_file(file_string);

	preferences.put_write_grid_multiplier(profile.GetInt("GridMultiplier",1));

    preferences.enable_multi_threaded(profile.GetInt("MultiThreaded",1)!=0);
    preferences.put_thread_priority(profile.GetInt("ThreadPriority",5));

	if (profile.GetInt("ClampPotential",0)!=0) env_effects|=ENV_CLAMP_POTENTIAL;
	else env_effects&=(~ENV_CLAMP_POTENTIAL);
	profile.GetString("PotentialClampValue",number_string,sizeof(number_string),"6.000");
	environment.put_value(ENVIRONMENT,POT_CLAMP_VALUE,atof(number_string));

	if (profile.GetInt("ClampTemperature",0)!=0) env_effects|=ENV_CLAMP_TEMPERATURE;
	else env_effects&=(~ENV_CLAMP_TEMPERATURE);
	profile.GetString("TemperatureClampValue",number_string,sizeof(number_string),"6.000");
	environment.put_value(ENVIRONMENT,TEMP_CLAMP_VALUE,atof(number_string));

	profile.GetString("TemperatureRelaxValue",number_string,sizeof(number_string),"1.000");
	environment.put_value(ENVIRONMENT,TEMP_RELAX_VALUE,atof(number_string));

	profile.GetString("MaxElectricalError",number_string,sizeof(number_string),"1e-8");
	environment.put_value(ENVIRONMENT,MAX_ELECTRICAL_ERROR,atof(number_string));
	profile.GetString("MaxThermalError",number_string,sizeof(number_string),"1e-8");
	environment.put_value(ENVIRONMENT,MAX_THERMAL_ERROR,atof(number_string));
	profile.GetString("MaxPhotonError",number_string,sizeof(number_string),"1e-8");
	environment.put_value(ENVIRONMENT,MAX_OPTIC_ERROR,atof(number_string));
	profile.GetString("CoarseModeError",number_string,sizeof(number_string),"1e-3");
	environment.put_value(ENVIRONMENT,COARSE_MODE_ERROR,atof(number_string));
	profile.GetString("FineModeError",number_string,sizeof(number_string),"1e-4");
	environment.put_value(ENVIRONMENT,FINE_MODE_ERROR,atof(number_string));

	environment.put_value(ENVIRONMENT,MAX_INNER_ELECT_ITER,profile.GetInt("MaxInnerElectricalIter",15));
	environment.put_value(ENVIRONMENT,MAX_INNER_THERM_ITER,profile.GetInt("MaxInnerThermalIter",15));
	environment.put_value(ENVIRONMENT,MAX_INNER_MODE_ITER,profile.GetInt("MaxInnerModeIter",15));
	environment.put_value(ENVIRONMENT,MAX_OUTER_OPTIC_ITER,profile.GetInt("MaxOuterPhotonIter",15));
	environment.put_value(ENVIRONMENT,MAX_OUTER_THERM_ITER,profile.GetInt("MaxOuterThermalIter",15));

	environment.put_value(ENVIRONMENT,EFFECTS,env_effects);
	environment.process_recompute_flags();
}

void SavePreferences(void)
{
	extern char executable_path[MAXPATH], ini_filename[];
	char number_string[20];
	flag env_effects;

	string ini_string(string(executable_path)+string(ini_filename));

	TProfile profile("Environment Defaults",ini_string.c_str());

	env_effects=(flag)environment.get_value(ENVIRONMENT,EFFECTS);

	if (preferences.is_toolbar()) profile.WriteInt("ToolBar",1);
	else profile.WriteInt("ToolBar",0);

	if (preferences.is_statusbar()) profile.WriteInt("StatusBar",1);
	else profile.WriteInt("StatusBar",0);

	if (env_effects & ENV_UNDO_SIMULATION) profile.WriteInt("SimulationUndo",1);
	else profile.WriteInt("SimulationUndo",0);

	profile.WriteString("MaterialFile",preferences.get_material_parameters_file().c_str());
	profile.WriteInt("GridMultiplier",preferences.get_write_grid_multiplier());

    if (preferences.is_multi_threaded()) profile.WriteInt("MultiThreaded",1);
    else profile.WriteInt("MultiThreaded",0);

    profile.WriteInt("ThreadPriority",preferences.get_thread_priority());

	if (env_effects & ENV_CLAMP_POTENTIAL) profile.WriteInt("ClampPotential",1);
	else profile.WriteInt("ClampPotential",0);
	sprintf(number_string,"%.3lf",environment.get_value(ENVIRONMENT,POT_CLAMP_VALUE));
	profile.WriteString("PotentialClampValue",number_string);

	if (env_effects & ENV_CLAMP_TEMPERATURE) profile.WriteInt("ClampTemperature",1);
	else profile.WriteInt("ClampTemperature",0);
	sprintf(number_string,"%.3lf",environment.get_value(ENVIRONMENT,TEMP_CLAMP_VALUE));
	profile.WriteString("TemperatureClampValue",number_string);

	sprintf(number_string,"%.3lf",environment.get_value(ENVIRONMENT,TEMP_RELAX_VALUE));
	profile.WriteString("TemperatureRelaxValue",number_string);

	sprintf(number_string,"%.3le",environment.get_value(ENVIRONMENT,MAX_ELECTRICAL_ERROR));
	profile.WriteString("MaxElectricalError",number_string);
	sprintf(number_string,"%.3le",environment.get_value(ENVIRONMENT,MAX_THERMAL_ERROR));
	profile.WriteString("MaxThermalError",number_string);
	sprintf(number_string,"%.3le",environment.get_value(ENVIRONMENT,MAX_OPTIC_ERROR));
	profile.WriteString("MaxPhotonError",number_string);
	sprintf(number_string,"%.3le",environment.get_value(ENVIRONMENT,COARSE_MODE_ERROR));
	profile.WriteString("CoarseModeError",number_string);
	sprintf(number_string,"%.3le",environment.get_value(ENVIRONMENT,FINE_MODE_ERROR));
	profile.WriteString("FineModeError",number_string);

	profile.WriteInt("MaxInnerElectricalIter",(int)environment.get_value(ENVIRONMENT,MAX_INNER_ELECT_ITER));
	profile.WriteInt("MaxInnerThermalIter",(int)environment.get_value(ENVIRONMENT,MAX_INNER_THERM_ITER));
	profile.WriteInt("MaxInnerModeIter",(int)environment.get_value(ENVIRONMENT,MAX_INNER_MODE_ITER));
	profile.WriteInt("MaxOuterPhotonIter",(int)environment.get_value(ENVIRONMENT,MAX_OUTER_OPTIC_ITER));
	profile.WriteInt("MaxOuterThermalIter",(int)environment.get_value(ENVIRONMENT,MAX_OUTER_THERM_ITER));
}




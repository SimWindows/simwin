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

extern TWindow* error_window;
extern TSimWindowsDeviceStatus* status_window;

#ifndef NDEBUG
extern logical enable_flag_check;
#endif

/********************************** device output functions ***********************************/

void out_error_message(logical clear_error)
{
	string error_string;

	assert(error_window);
	assert(error_window->IsWindow());
	error_string=error_handler.get_error_string();
	error_window->MessageBox(error_string.c_str(),error_window->GetApplication()->GetName(),
							 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
#ifndef NDEBUG
	error_string="Error code: "+int_to_string((int)error_handler.get_error_number())+
				 "\nFile: "+error_handler.get_source_file()+
                 "\nLine: "+int_to_string(error_handler.get_line_number());
	error_window->MessageBox(error_string.c_str(),error_window->GetApplication()->GetName(),
				  MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
#endif
	if (clear_error) error_handler.clear();
}

void out_elect_convergence(short iterations, FundamentalParam error)
{
	char error_string[70];

	assert(status_window);
	assert(status_window->IsWindow());

	if (iterations==1) {
		if (status_window->GetNumLines()>200) {
			status_window->Clear();
			status_window->UpdateWindow();
		}
		status_window->Insert("Electrical Convergence Values\r\n");
	}
	if (error.eta_c || error.eta_v)
		sprintf(error_string,"%d\tEta c = %.4le\t\tPsi = %.4le\t\tEta v = %.4le\r\n",
				iterations,error.eta_c,error.psi,error.eta_v);
	else
		sprintf(error_string,"%d\tPsi = %.4le\r\n",iterations,error.psi);
	status_window->Insert(error_string);
}

void out_optic_convergence(short iterations, prec error)
{
	char error_string[50];

	assert(status_window);
	assert(status_window->IsWindow());

	if (status_window->GetNumLines()>200) {
		status_window->Clear();
		status_window->UpdateWindow();
	}
	status_window->Insert("Photon Convergence Values\r\n");
	sprintf(error_string,"%d\tS = %.4le\r\n",iterations,error);
	status_window->Insert(error_string);
}

void out_therm_convergence(short iterations, prec error)
{
	char error_string[50];

	assert(status_window);
	assert(status_window->IsWindow());

	if (iterations==1) {
		if (status_window->GetNumLines()>200) {
			status_window->Clear();
			status_window->UpdateWindow();
		}
		status_window->Insert("Thermal Convergence Values\r\n");
	}
	sprintf(error_string,"%d\tT = %.4le\r\n",iterations,error);
	status_window->Insert(error_string);
}

void out_coarse_mode_convergence(short iterations, prec error)
{
	char error_string[50];

	assert(status_window);
	assert(status_window->IsWindow());

	if (iterations==1) {
		if (status_window->GetNumLines()>200) {
			status_window->Clear();
			status_window->UpdateWindow();
		}
		status_window->Insert("Coarse Laser Mode Search\r\n");
	}
	sprintf(error_string,"%d\tLambda = %.4le\r\n",iterations,error);
	status_window->Insert(error_string);
}

void out_fine_mode_convergence(short iterations, prec error)
{
	char error_string[50];

	assert(status_window);
	assert(status_window->IsWindow());

	if (iterations==1) {
		if (status_window->GetNumLines()>200) {
			status_window->Clear();
			status_window->UpdateWindow();
		}
		status_window->Insert("Fine Laser Mode Search\r\n");
	}
	sprintf(error_string,"%d\tLambda = %.4le\r\n",iterations,error);
	status_window->Insert(error_string);
}

void out_operating_condition(void)
{

	char output_string[75];

	assert(status_window);
	assert(status_window->IsWindow());

	if (status_window->GetNumLines()>200) {
		status_window->Clear();
		status_window->UpdateWindow();
	}
	sprintf(output_string,"Bias: Left Contact=%.3lf V Right Contact=%.3lf V\r\n",
						  environment.get_value(CONTACT,APPLIED_BIAS,0),
						  environment.get_value(CONTACT,APPLIED_BIAS,1));
	status_window->Insert(output_string);
	sprintf(output_string,"Temp: Left Surface=%.3lf K Right Surface=%.3lf K\r\n",
						  environment.get_value(SURFACE,TEMPERATURE,0),
						  environment.get_value(SURFACE,TEMPERATURE,1));
	status_window->Insert(output_string);

}

void out_simulation_result(void)
{

	string result_string;

	assert(status_window);
	assert(status_window->IsWindow());

	result_string="Current solution: "+get_solve_string((SolveType)environment.get_value(DEVICE,CURRENT_SOLUTION))+"\r\n";
	status_window->Insert(result_string.c_str());
	result_string="Current status: "+get_status_string((StatusType)environment.get_value(DEVICE,CURRENT_STATUS))+"\r\n";
	status_window->Insert(result_string.c_str());

}

void out_message(string message)
{
#ifndef NDEBUG
	enable_flag_check=FALSE;
#endif
	assert(error_window);
	assert(error_window->IsWindow());
	error_window->MessageBox(message.c_str(),error_window->GetApplication()->GetName(),MB_OK);
#ifndef NDEBUG
	enable_flag_check=TRUE;
#endif
}



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

/*************************************************************************

Output functions - callable by the device
These functions must be redefined for the particular user interface

**************************************************************************/

void out_error_message(logical clear_error);
void out_elect_convergence(short iterations, FundamentalParam error);
void out_optic_convergence(short iterations, prec error);
void out_therm_convergence(short iterations, prec error);
void out_coarse_mode_convergence(short iterations, prec error);
void out_fine_mode_convergence(short iterations, prec error);
void out_operating_condition(void);
void out_simulation_result(void);
void out_message(string message);


#ifdef NDEBUG

#define out_debug_message(message) ((void)0)

#else

#define out_debug_message(message) out_message(message)

#endif

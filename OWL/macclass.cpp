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

extern TSimWindowsDeviceStatus* status_window;

//************************************* class TMacro ******************************************
/*
class TMacro {
protected:
	string macro_name;
    logical solving;
    logical stop_solution;
	TValueFlagWithObject record_flags;
	FlagType increment_flag_type;
	flag increment_flag;
	int increment_object_number;
	prec start_value;
	prec end_value;
	prec increment;
	int number_parameters;
	int number_values;
	prec **record_data;
	FlagType *flag_type_array;
	flag *flag_array;
	int *object_array;
	logical data_valid;
public:
	TMacro(FlagType new_flag_type, flag new_flag);
	virtual ~TMacro(void) { initialize_data(); }
	string get_macro_name(void) { return(macro_name); }
	TValueFlagWithObject get_record_flags(void) { return(record_flags); }
	prec get_start_value(void) { return(start_value); }
	prec get_end_value(void) { return(end_value); }
	void get_increment_object(FlagType &flag_type, flag &flag_value, int &object_number)
		{ flag_type=increment_flag_type; flag_value=increment_flag; object_number=increment_object_number; }
	int get_increment_object_number(void) { return(increment_object_number); }
	prec get_increment_value(void) { return(increment); }
	int get_number_values(void) { return(number_values); }
	int get_data_point(prec value, int start_point=-1, int end_point=-1);
	prec get_value(FlagType flag_type, flag flag_value, int object_number, int data_number);
	logical has_data(void) { return(data_valid); }
    void set_stop_solution(logical stop) { stop_solution=stop; }
    logical do_stop_solution(void) { return(stop_solution); }
    logical is_solving(void) { return(solving); }
	void put_macro_name(const string& new_macro_name) { macro_name=new_macro_name; }
	void put_record_flags(const TValueFlagWithObject& new_flags) { record_flags=new_flags; }
	void put_start_value(prec new_start) { start_value=new_start; }
	void put_end_value(prec new_end) { end_value=new_end; }
	void put_increment_object_number(int new_object) { increment_object_number=new_object; }
	void put_increment_value(prec new_increment) { increment=new_increment; }
	void write_data_file(const char *filename, FlagType y_flag_type, flag y_flag);
	virtual void execute(const char *filename)=0;
protected:
	void allocate_data(void);
	void initialize_data(void);
};
*/
TMacro::TMacro(FlagType new_flag_type, flag new_flag)
	: macro_name("")
{
	increment_flag_type=new_flag_type;
	increment_flag=new_flag;
	increment_object_number=0;
	start_value=0.0;
	end_value=0.0;
	increment=0.0;
	number_parameters=0;
	number_values=0;
	record_data=(prec **)0;
	flag_type_array=(FlagType *)0;
	flag_array=(flag *)0;
	object_array=(int *)0;
	data_valid=FALSE;
    solving=FALSE;
    stop_solution=FALSE;
}

int TMacro::get_data_point(prec value, int start_point, int end_point)
{
	int test_point, return_point;
	prec test_value;
	prec start_value, end_value;

	if (number_values<=1) return(0);

	if (start_point==-1) start_point=0;
	if (end_point==-1) end_point=number_values-1;

	if ((end_point-start_point)==1) {
		start_value=record_data[0][start_point];
		end_value=record_data[0][end_point];
		if ((value-start_value)<(end_value-value)) return_point=start_point;
		else return_point=end_point;
	}
	else {
		test_point=(int)(start_point+(end_point-start_point)/2);
		test_value=record_data[0][test_point];

		if (test_value>value) return_point=get_data_point(value,start_point,test_point);
		else return_point=get_data_point(value,test_point,end_point);
	}
	return(return_point);
}

prec TMacro::get_value(FlagType flag_type, flag flag_value, int object_number, int data_number)
{
	int i;

	if (!has_data()) return(0.0);
	if (data_number>=number_values) data_number=number_values-1;

	for (i=0;i<number_parameters;i++) {
		if (flag_type_array[i]==flag_type && flag_array[i]==flag_value && object_array[i]==object_number)
			return(record_data[i][data_number]);
	}
	return(0.0);
}

void TMacro::allocate_data(void)
{
	int i,j,k,flag_array_index;
	flag test_flag, valid_flags;
	int max_bit;

	initialize_data();

	number_parameters=record_flags.count_all_with_object()+1;
	if (end_value!=start_value && increment!=0)
		number_values=round((end_value-start_value)/increment)+1;
	else number_values=1;

	flag_type_array=new FlagType[number_parameters];
	flag_array=new flag[number_parameters];
	object_array=new int[number_parameters];

	record_data=new prec*[number_parameters];

	for (i=0;i<number_parameters;i++) {
		record_data[i]=new prec[number_values];
		for (j=0;j<number_values;j++) record_data[i][j]=0.0;
	}

	flag_type_array[0]=increment_flag_type;
	flag_array[0]=increment_flag;
	object_array[0]=increment_object_number;

	flag_array_index=1;
	for (i=1;i<=NUMBER_FLAG_TYPES;i++) {
		if (record_flags.any_set((FlagType)i)) {
			test_flag=1;
			max_bit=bit_position(record_flags.get_max((FlagType)i));
			valid_flags=record_flags.get_valid((FlagType)i);
			for (j=0;j<=max_bit;j++) {
				if ((valid_flags & test_flag) && record_flags.is_set((FlagType)i,test_flag)) {
					for (k=0;k<record_flags.get_number_objects((FlagType)i);k++) {
						flag_type_array[flag_array_index]=(FlagType)i;
						flag_array[flag_array_index]=test_flag;
						object_array[flag_array_index]=record_flags.get_object((FlagType)i,k);
						flag_array_index++;
					}
				}
				test_flag<<=1;
			}
		}
	}
}

void TMacro::initialize_data(void)
{
	int i;

	if (flag_array) {
		delete[] flag_array;
		flag_array=(flag *)0;
	}
	if (flag_type_array) {
		delete[] flag_type_array;
		flag_type_array=(FlagType *)0;
	}

	if (object_array) {
		delete[] object_array;
		object_array=(int *)0;
	}

	if (record_data) {
		for (i=0;i<number_parameters;i++) delete[] record_data[i];
		delete[] record_data;
	}

	number_parameters=0;
	number_values=0;
}

void TMacro::write_data_file(const char *filename, FlagType y_flag_type, flag y_flag)
{
	int i,j,new_precision=4;
	flag present_flag;
	FlagType present_flag_type;
	ofstream output_file(filename);

	if (!output_file) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	output_file.setf(ios::scientific,ios::floatfield);
	output_file.precision(new_precision);

	output_file << get_short_string(flag_type_array[0], flag_array[0]) <<
				   get_short_location_string(flag_type_array[0],object_array[0]);

	for (i=1;i<number_parameters;i++) {
		present_flag=flag_array[i];
		present_flag_type=flag_type_array[i];
		if ((present_flag_type==y_flag_type) && (present_flag==y_flag)) {
			output_file << "," << get_short_string(present_flag_type, present_flag) <<
								  get_short_location_string(present_flag_type,object_array[i]);
		}
	}
	output_file << '\n';

	for (i=0;i<number_values;i++) {
		output_file << record_data[0][i];
		for (j=1;j<number_parameters;j++) {
			present_flag=flag_array[j];
			present_flag_type=flag_type_array[j];
			if ((present_flag_type==y_flag_type) && (present_flag==y_flag)) {
				output_file << "," << record_data[j][i];
			}
		}
		output_file << '\n';
	}
}

//****************************** class TVoltageMacro ******************************************
/*
class TVoltageMacro: public TMacro {
protected:
	logical reset_device;
public:
	TVoltageMacro(void)
		: TMacro(CONTACT,APPLIED_BIAS) {}
	virtual ~TVoltageMacro(void) {}
	logical get_reset_device(void) { return(reset_device); }
	void put_reset_device(logical reset) { reset_device=reset; }
	virtual void execute(const char *filename);
};
*/
void TVoltageMacro::execute(const char *filename)
{
	TMDIClient *client_window;
	clock_t start_time, end_time;
	int i,iteration_count, new_precision=4;
	char time_string[40];
	FlagType present_flag_type;
	flag present_flag;
	int present_object;
	ofstream output_file;

    solving=TRUE;

	start_time=clock();

	client_window=dynamic_cast<TMDIClient *>(status_window->GetApplication()->GetMainWindow()->GetClientWindow());

	allocate_data();
	output_file.open(filename,ios::out | ios::trunc);

	if (output_file.fail()) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
		return;
	}

	::SetCursor(TCursor(NULL,IDC_WAIT));

	status_window->Clear();
	status_window->UpdateWindow();

	if (reset_device) {
		environment.init_device();
		status_window->Insert("Reset Device\r\n\r\n");
	}

	status_window->Insert("Executing Macro: ");
	status_window->Insert(macro_name.c_str());
	status_window->Insert("\r\n");

	output_file.setf(ios::scientific,ios::floatfield);
	output_file.precision(new_precision);

	for (i=0;i<number_parameters;i++) {
		present_flag=flag_array[i];
		present_flag_type=flag_type_array[i];
		output_file << get_short_string(present_flag_type, present_flag) <<
					   get_short_location_string(present_flag_type,object_array[i]);
		if (i<number_parameters-1) output_file << ",";
	}
	output_file << '\n';

	output_file.close();

	for (iteration_count=0;iteration_count<number_values;iteration_count++)
		record_data[0][iteration_count]=start_value+(prec)iteration_count*increment;

	for (iteration_count=0;iteration_count<number_values;iteration_count++) {
		environment.put_value(CONTACT,APPLIED_BIAS,record_data[0][iteration_count],increment_object_number);
		status_window->Insert("\r\n");
		out_operating_condition();
		environment.solve();
		out_simulation_result();
		if (error_handler.fail()) break;
		client_window->ForEach(UpdateValidEnvironPlot);
		output_file.open(filename,ios::out | ios::app);
		if (output_file.fail()) {
			error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",filename);
			break;
		}
		output_file << record_data[0][iteration_count];
		if (number_parameters>1) output_file << ",";
		for (i=1;i<number_parameters;i++) {
			present_flag=flag_array[i];
			present_flag_type=flag_type_array[i];
			present_object=object_array[i];
			record_data[i][iteration_count]=environment.get_value(present_flag_type,present_flag,present_object);
			output_file << record_data[i][iteration_count];
			if (i<number_parameters-1) output_file << ",";
		}
		output_file << '\n';

		output_file.close();
        if (stop_solution) iteration_count=number_values;
	}
	if (record_flags.any_set()) data_valid=TRUE;
	end_time=clock();
	if (error_handler.fail())
		sprintf(time_string,"Simulation Terminated - time = %.3f s\r\n\r\n",(end_time-start_time)/CLK_TCK);
	else
		sprintf(time_string,"End Macro - time = %.3f s\r\n\r\n",(end_time-start_time)/CLK_TCK);
	status_window->Insert(time_string);
	::SetCursor(TCursor(NULL,IDC_ARROW));
    stop_solution=FALSE;
    solving=FALSE;
}

//****************************** class TMacroStorage *******************************************
/*
class TMacroStorage {
private:
	int number_entries;
	MacroEntry *first_entry;
	MacroEntry *last_entry;
public:
	TMacroStorage(void);
	~TMacroStorage(void);
	void add_macro(TMacro *new_macro);
	void delete_macro(int macro_number);
	void delete_macros(void);
	TMacro *get_macro(int macro_number);
    TMacro *get_solving_macro(void);
	int get_number_macros(void) { return(number_entries); }
};
*/
TMacroStorage::TMacroStorage(void)
{
	number_entries=0;
	first_entry=(MacroEntry *)0;
	last_entry=(MacroEntry *)0;
}

TMacroStorage::~TMacroStorage(void)
{
	delete_macros();
}

void TMacroStorage::add_macro(TMacro *new_macro)
{
	if (!number_entries) {
		first_entry=new MacroEntry;
		first_entry->macro=new_macro;
		first_entry->next_entry=(MacroEntry *)NULL;
		last_entry=first_entry;
	}
	else {
		last_entry->next_entry=new MacroEntry;
		last_entry=last_entry->next_entry;
		last_entry->macro=new_macro;
		last_entry->next_entry=(MacroEntry *)NULL;
	}
	number_entries++;
}

void TMacroStorage::delete_macro(int macro_number)
{
	int i;
	MacroEntry *curr_entry_ptr, *prev_entry_ptr;

	if (!number_entries) return;

	if (macro_number>=number_entries) macro_number=number_entries-1;

	prev_entry_ptr=(MacroEntry *)0;
	curr_entry_ptr=first_entry;
	for (i=0;i<macro_number;i++) {
		prev_entry_ptr=curr_entry_ptr;
		curr_entry_ptr=curr_entry_ptr->next_entry;
	}

	if (curr_entry_ptr==first_entry) first_entry=curr_entry_ptr->next_entry;
	if (curr_entry_ptr==last_entry) last_entry=prev_entry_ptr;
	if (prev_entry_ptr)	prev_entry_ptr->next_entry=curr_entry_ptr->next_entry;

	delete curr_entry_ptr->macro;
	delete curr_entry_ptr;
	number_entries--;
}

void TMacroStorage::delete_macros(void)
{
	int i;
	MacroEntry *curr_entry_ptr, *next_entry_ptr;

	curr_entry_ptr=first_entry;
	for (i=0;i<number_entries;i++) {
		next_entry_ptr=curr_entry_ptr->next_entry;
		delete curr_entry_ptr->macro;
		delete curr_entry_ptr;
		curr_entry_ptr=next_entry_ptr;
	}

	number_entries=0;
	first_entry=(MacroEntry *)0;
	last_entry=(MacroEntry *)0;
}

TMacro *TMacroStorage::get_macro(int macro_number)
{
	int i;
	MacroEntry *curr_entry_ptr;

	if (!number_entries) return(NULL);

	if (macro_number>=number_entries) macro_number=number_entries-1;

	curr_entry_ptr=first_entry;
	for (i=0;i<macro_number;i++) curr_entry_ptr=curr_entry_ptr->next_entry;

	return(curr_entry_ptr->macro);
}

TMacro *TMacroStorage::get_solving_macro(void)
{
	MacroEntry *curr_entry_ptr;

	if (!number_entries) return(NULL);

	curr_entry_ptr=first_entry;
	while(curr_entry_ptr) {
    	if (curr_entry_ptr->macro->is_solving()) return(curr_entry_ptr->macro);
    	curr_entry_ptr=curr_entry_ptr->next_entry;
    }

	return(NULL);
}


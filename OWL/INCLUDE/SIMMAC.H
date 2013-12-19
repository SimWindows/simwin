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


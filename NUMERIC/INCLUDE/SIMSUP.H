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

class TErrorHandler {
private:
	error error_number;
	float misc_parameter;
	string misc_string;
	string file;
#ifndef NDEBUG
	string source_file;
	int line_number;
#endif

public:
	TErrorHandler(void) { clear(); }
	logical fail(void) { return(error_number!=ERROR_NONE); }
#ifndef NDEBUG
	void clear(void)
		{ error_number=ERROR_NONE; misc_parameter=0; misc_string=""; file=""; source_file=""; line_number=0; }
	void set(error number, float new_parameter, string new_string, string new_file,
			 string new_source_file, int new_line_number);
#else
	void clear(void)
		{ error_number=ERROR_NONE; misc_parameter=0; misc_string=""; file=""; }
	void set(error number, float new_parameter, string new_string, string new_file);
#endif
	error get_error_number(void) { return(error_number); }
	float get_misc_parameter(void) { return(misc_parameter); }
	string get_misc_string(void) { return(misc_string); }
	string get_file(void) { return(file); }
	string get_error_string(void);

#ifndef NDEBUG
	string get_source_file(void) { return(source_file); }
	int get_line_number(void) { return(line_number); }
#endif
};

#ifndef NDEBUG
#define set_error(number,parameter,string,file) set(number,parameter,string,file,__FILE__,__LINE__)
#else
#define set_error(number,parameter,string,file) set(number,parameter,string,file)
#endif

class TPreferences {
private:
	logical tool_bar;
	logical status_bar;
	string material_parameters_file;
	int write_grid_multiplier;
    logical multi_threaded;
    int thread_priority;
public:
	TPreferences(void)
    	: tool_bar(TRUE), status_bar(TRUE),
          material_parameters_file("material.prm"),
          write_grid_multiplier(1),
          multi_threaded(TRUE), thread_priority(5) {}
	void enable_toolbar(logical enable) { tool_bar=enable; }
	logical is_toolbar(void) { return(tool_bar); }
	void enable_statusbar(logical enable) { status_bar=enable; }
	logical is_statusbar(void) { return(status_bar); }
	void put_material_parameters_file(const string& file)
		{ material_parameters_file=file; }
	string get_material_parameters_file(void) { return(material_parameters_file); }
	void put_write_grid_multiplier(int multiplier) { write_grid_multiplier=multiplier; }
	int get_write_grid_multiplier(void) { return(write_grid_multiplier); }
    void enable_multi_threaded(logical enable) { multi_threaded=enable; }
    logical is_multi_threaded(void) { return(multi_threaded); }
    void put_thread_priority(int priority) { thread_priority=priority; }
    int get_thread_priority(void) { return(thread_priority); }
};

class TFlag {
protected:
	FlagGroup flag_group;
	flag flag_array[NUMBER_FLAG_TYPES];
protected:
	TFlag(FlagGroup new_group) { flag_group=new_group; clear_all(); }
	TFlag(FlagGroup new_group, FlagType flag_type, flag flag_value)
		{ flag_group=new_group; clear_all(); set(flag_type,flag_value); }
	TFlag(const TFlag& new_flag);
public:
	~TFlag(void) {}
	int count(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(bit_count(flag_array[flag_type-1])); }
	int count_all(void);
	void clear(FlagType flag_type, flag flag_value) { flag_array[flag_type-1]&=(~flag_value); }
	void clear_all(void);
	flag get_flag(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value) { flag_array[flag_type-1]|=flag_value; }
	logical is_set(FlagType flag_type, flag flag_value) { return((flag_array[flag_type-1]&flag_value)!=0); }
	logical any_set(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(flag_array[flag_type-1]!=0); }
	logical any_set(void);
	TFlag& operator=(const TFlag& new_flag);
	TFlag& operator~(void);
	TFlag& operator|=(const TFlag& new_flag);
	TFlag& operator&=(const TFlag& new_flag);
#ifndef NDEBUG
	static logical valid_flag_type(FlagType flag_type);
#endif
};

class TValueFlag : public TFlag {
protected:
	static flag max_flag_array[];
	static flag valid_flag_array[];

#ifndef NDEBUG
	static flag valid_write_flag_array[];
	static flag valid_plot_flag_array[];
#endif

public:
	TValueFlag(void) : TFlag(VALUE_FLAGS) {}
	TValueFlag(FlagType flag_type, flag flag_value) : TFlag(VALUE_FLAGS,flag_type,flag_value) {}
	TValueFlag(const TValueFlag& new_flag) : TFlag(new_flag) {}
	~TValueFlag(void) {}
	void clear(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::clear(flag_type,flag_value); }
	static flag get_max(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(max_flag_array[flag_type-1]); }
	static flag get_valid(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::set(flag_type,flag_value); }
	logical is_set(FlagType flag_type, flag flag_value)
		{ assert(valid_single_flag(flag_type,flag_value)); return(TFlag::is_set(flag_type,flag_value)); }
	void set_all(void);
	void set_plot_combo(FlagCombo flag_combo);
	void set_write_combo(FlagCombo flag_combo);

#ifndef NDEBUG
	static logical valid_single_flag(FlagType flag_type, flag flag_value);
	static logical valid_multiple_flag(FlagType flag_type, flag flag_value);
	static flag get_write(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_write_flag_array[flag_type-1]); }
	static logical valid_write_flag(FlagType flag_type, flag flag_value);
	static flag get_plot(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_plot_flag_array[flag_type-1]); }
	static logical valid_plot_flag(FlagType flag_type, flag flag_value);
#endif

};

class TValueFlagWithObject : public TValueFlag {
protected:
	int number_objects[NUMBER_FLAG_TYPES];
	ObjectEntry *first_entry[NUMBER_FLAG_TYPES];
	ObjectEntry *last_entry[NUMBER_FLAG_TYPES];
public:
	TValueFlagWithObject(void);
	TValueFlagWithObject(FlagType flag_type, flag flag_value);
	TValueFlagWithObject(const TValueFlagWithObject& new_flag);
	~TValueFlagWithObject(void) { clear_all_objects(); }
	void add_object(FlagType flag_type, int object_number);
	void clear_objects(FlagType flag_type);
	void clear_all_objects(void);
	int count_with_object(FlagType flag_type) { return(count(flag_type)*number_objects[flag_type-1]); }
	int count_all_with_object(void);
	int get_number_objects(FlagType flag_type) { return(number_objects[flag_type-1]); }
	int get_object(FlagType flag_type, int object_number);
	void put_object(FlagType flag_type, int object_number);
	TValueFlagWithObject& operator=(const TValueFlagWithObject& new_flag);
};

class TEffectFlag : public TFlag {
protected:
	static flag max_flag_array[];
	static flag valid_flag_array[];
public:
	TEffectFlag(void) : TFlag(EFFECT_FLAGS) {}
	TEffectFlag(FlagType flag_type, flag flag_value) : TFlag(EFFECT_FLAGS,flag_type,flag_value) {}
	TEffectFlag(const TEffectFlag& new_flag) : TFlag(new_flag) {}
	~TEffectFlag(void) {}
	void clear(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::clear(flag_type,flag_value); }
	static flag get_max(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(max_flag_array[flag_type-1]); }
	static flag get_valid(FlagType flag_type)
		{ assert(valid_flag_type(flag_type)); return(valid_flag_array[flag_type-1]); }
	void set(FlagType flag_type, flag flag_value)
		{ assert(valid_multiple_flag(flag_type, flag_value)); TFlag::set(flag_type,flag_value); }
	logical is_set(FlagType flag_type, flag flag_value)
		{ assert(valid_single_flag(flag_type,flag_value)); return(TFlag::is_set(flag_type,flag_value)); }
	void set_all(void);

#ifndef NDEBUG
	static logical valid_single_flag(FlagType flag_type, flag flag_value);
	static logical valid_multiple_flag(FlagType flag_type, flag flag_value);
#endif

};



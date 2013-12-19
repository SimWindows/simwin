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

class TPlotPrintout: public TPrintout {
protected:
	logical show_nodes;
	int number_plots;
	int data_points;
	Axis x_axis;
	float *x_data;
	float *x_coord;
	string x_label;
	Axis y_axis;
	float **y_data;
	float **y_coord;
	logical **y_skip;
	string y_label;
	TRect plot_rect;
public:
	TPlotPrintout::TPlotPrintout(const char* title, int new_number_plots, int new_data_points,
								 Axis new_x_axis, float *new_x_data, string new_x_label,
								 Axis new_y_axis, float **new_y_data, string new_y_label,
								 logical nodes);
	~TPlotPrintout(void) { free_coordinates(); }
	void GetDialogInfo(int& minPage, int& maxPage,
					   int& selFromPage, int& selToPage);
	void PrintPage(int page, TRect& rect, unsigned flags);
	bool HasPage(int pageNumber) {return pageNumber == 1;}
protected:
	void allocate_coordinates(void);
	void free_coordinates(void);
	void data_to_coordinates(void);
	void NumberOut(int x, int y, float num);
};

class TSimWindowsPlot: public TWindow {
protected:
	TOpenSaveDialog::TData FileData;
	logical modified;
	logical frozen;
	logical multi_colored;
	logical show_nodes;
	int number_plots;
	int data_points;
	FlagType x_flag_type;
	flag x_flag;
	Axis x_axis;
	float *x_data;
	float *x_coord;
	string x_label;
	TValueFlag y_flags;
	Axis y_axis;
	float **y_data;
	float **y_coord;
	logical **y_skip;
	string y_label;
	TRect plot_rect;
	logical scale_rect_ready;
	TRect scale_rect;
	float *trace_values;
	string *trace_labels;
	TDialogTrace trace_window;
public:
	TSimWindowsPlot(TWindow* parent, FlagType new_x_flag_type, flag new_x_flag,
					const TValueFlag& new_y_flags, const string& y_name, logical colored=TRUE,
					logical nodes=FALSE, const char far* title=0, TModule* module=0);
	virtual ~TSimWindowsPlot(void) { free_coordinates(); }
	virtual void SetupWindow(void);
	logical is_frozen(void) { return(frozen); }
	logical should_close(void) { return(!is_frozen() && !can_update()); }
	virtual logical can_update(void)=0;
	virtual void update_data(void)=0;
	virtual void update_trace_labels(void)=0;
protected:
	void allocate_coordinates(int new_number_plots, int new_data_points);
	void free_coordinates(void);
	void data_to_coordinates(void);
	int get_data_point(prec value, int start_point=-1, int end_point=-1);
	void update_trace_values(float x_coord);
	void write_data_file(float ref_value=0);
	void NumberOut(int x, int y, float num, TDC& device_context);
public:
	virtual void Paint(TDC &dc, bool erase, TRect& rect);
	virtual void Save(void) { SaveAs(); }
	virtual void SaveAs(void)=0;
protected:
	DECLARE_RESPONSE_TABLE(TSimWindowsPlot);
	void EvSize(UINT sizeType, TSize& size) { Invalidate(TRUE); }
	void EvMouseMove(UINT modKeys, TPoint& point);
	void EvLButtonUp(UINT modKeys, TPoint& point);
	void EvLButtonDblClk(UINT modKeys, TPoint& point)
		{ if (modKeys==MK_LBUTTON) CmPlotScale(); }
	void CmFileSave(void) { Save(); }
	void CmFileSaveEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(modified); }
	void CmFileSaveAs(void) { SaveAs(); }
	void CmFilePrint(void);
	void CmPlotScale(void);
	void CmPlotAutoScale(void);
	void CmPlotShowTrace(void);
	void CmPlotShowNodes(void);
	void CmPlotShowNodesEnabler(TCommandEnabler& commandHandler);
	void CmPlotFreeze(void);
	void CmPlotFreezeEnabler(TCommandEnabler& commandHandler);
};

class TSimWindowsEnvironPlot: public TSimWindowsPlot {
public:
	TSimWindowsEnvironPlot(TWindow* parent, FlagType new_x_flag_type, flag new_x_flag,
						   const TValueFlag& new_y_flags, const string& y_name, logical colored=TRUE,
						   logical nodes=FALSE, const char far* title=0, TModule* module=0)
		: TSimWindowsPlot(parent,new_x_flag_type,new_x_flag,new_y_flags,y_name,colored,nodes,title,module) {}
	virtual logical can_update(void) { return(environment.get_number_objects(x_flag_type)!=0); }
	virtual void update_data(void);
	virtual void update_trace_labels(void);
	virtual void SaveAs(void);
};

class TSimWindowsBandDiagram: public TSimWindowsEnvironPlot {
private:
	FlagType ref_flag_type;
	flag ref_flag;
public:
	TSimWindowsBandDiagram(TWindow* parent, const TValueFlag& new_y_flags, const string& y_name,
						   logical colored=TRUE, logical nodes=FALSE, const char far* title=0, TModule* module=0)
		: TSimWindowsEnvironPlot(parent,GRID_ELECTRICAL,POSITION,new_y_flags,y_name,colored,nodes,title,module),
		  ref_flag_type(GRID_ELECTRICAL), ref_flag(VACUUM_LEVEL) {}
	virtual void SaveAs(void);
};



class TSimWindowsMacroPlot: public TSimWindowsPlot {
protected:
	int x_object_number;
	FlagType y_flag_type;
	flag y_flag;
	TMacro *macro;
public:
	TSimWindowsMacroPlot(TWindow* parent, int macro_number, FlagType new_x_flag_type,
						 flag new_x_flag, int x_object, FlagType new_y_flag_type,
						 flag new_y_flag, string y_name,
						 logical colored=TRUE, logical nodes=FALSE,const char far* title=0, TModule* module=0);
	virtual logical can_update(void);
	virtual void update_data(void);
	virtual void update_trace_labels(void);
	virtual void SaveAs(void);
};



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

extern TPrinter* printer;

/******************************* class TPlotPrinterout ***************************************

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

*/

TPlotPrintout::TPlotPrintout(const char* title, int new_number_plots, int new_data_points,
							 Axis new_x_axis, float *new_x_data, string new_x_label,
							 Axis new_y_axis, float **new_y_data, string new_y_label,
							 logical nodes)
  : TPrintout(title)
{
	show_nodes=nodes;
	number_plots=new_number_plots;
	data_points=new_data_points;
	x_axis=new_x_axis;
	x_data=new_x_data;
	x_label=new_x_label,
	y_axis=new_y_axis;
	y_data=new_y_data;
	y_label=new_y_label;
	allocate_coordinates();
	Banding=FALSE;
}

void TPlotPrintout::PrintPage(int, TRect& rect, unsigned)
{
	float *x_coord_ptr, *y_coord_ptr;
	logical *skip_ptr,previous_skip;
	prec next_tic,next_min_tic,next_number_tic;
	prec maj_coord_tic, min_coord_tic;
	int i,plot_count;
	int left_margin, right_margin, top_margin, bot_margin;
	int maj_tic_length, min_tic_length, tic_label_spacing;
	int log_pixel_x, log_pixel_y;
	TRect client_rect, x_label_rect, y_label_rect, title_rect;
	TEXTMETRIC text_metrics;
	TSize client_size;

	DC->SaveDC();

	log_pixel_x=DC->GetDeviceCaps(LOGPIXELSX);
	log_pixel_y=DC->GetDeviceCaps(LOGPIXELSY);
	TFont font("Arial", 3*log_pixel_y/16);
	DC->SelectObject(font);
	DC->GetTextMetrics(text_metrics);
	maj_tic_length=log_pixel_x/8;
	min_tic_length=maj_tic_length/2;
	tic_label_spacing=min_tic_length;
	left_margin=log_pixel_x/2+7*text_metrics.tmAveCharWidth+tic_label_spacing+maj_tic_length;
	right_margin=log_pixel_x/2;
	bot_margin=log_pixel_y/2+maj_tic_length+tic_label_spacing+2*text_metrics.tmHeight;
	top_margin=log_pixel_y/2+text_metrics.tmHeight;
	plot_rect.left=rect.left+left_margin;
	plot_rect.right=rect.right-right_margin;
	plot_rect.top=rect.top+top_margin;
	plot_rect.bottom=rect.bottom-bot_margin;
	DC->SetTextColor(BLACK);

	if (plot_rect.Height()<1) plot_rect.bottom=plot_rect.top+1;
	if (plot_rect.Width()<1) plot_rect.right=plot_rect.left+1;

	data_to_coordinates();

	DC->SelectStockObject(BLACK_PEN);
	DC->SelectStockObject(HOLLOW_BRUSH);

	DC->MoveTo(plot_rect.left,plot_rect.top);
	DC->LineTo(plot_rect.left,plot_rect.bottom);
	DC->LineTo(plot_rect.right,plot_rect.bottom);

	DC->SelectClipRgn(plot_rect.InflatedBy(1,1));

	DC->SelectObject(TPen(TColor::Black,2));

// plot data
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		previous_skip=TRUE;
		x_coord_ptr=x_coord;
		y_coord_ptr=y_coord[plot_count];
		skip_ptr=y_skip[plot_count];
		if (data_points==1) {
			if ((*x_coord_ptr>=plot_rect.left) && (*x_coord_ptr<=plot_rect.right))  {
				if (!*skip_ptr) {
					DC->MoveTo((int) *x_coord_ptr, (int) plot_rect.bottom);
					DC->LineTo((int) *x_coord_ptr, (int) *y_coord_ptr);
					if (show_nodes) DC->Ellipse((int)((*x_coord_ptr)-min_tic_length/2),
												(int)((*y_coord_ptr)-min_tic_length/2),
												(int)((*x_coord_ptr)+min_tic_length/2),
												(int)((*y_coord_ptr)+min_tic_length/2));
				}
			}
		}
		else {
			for (i=0;i<data_points;i++) {
				if ((*x_coord_ptr>=plot_rect.left) && (*x_coord_ptr<=plot_rect.right))  {
					if (!*skip_ptr) {
						if (previous_skip) DC->MoveTo((int) *x_coord_ptr, (int) *y_coord_ptr);
						else DC->LineTo((int) *x_coord_ptr, (int) *y_coord_ptr);
						if (show_nodes) DC->Ellipse((int)((*x_coord_ptr)-min_tic_length/2),
													(int)((*y_coord_ptr)-min_tic_length/2),
													(int)((*x_coord_ptr)+min_tic_length/2),
													(int)((*y_coord_ptr)+min_tic_length/2));
					}
					previous_skip=*skip_ptr;
				}
				x_coord_ptr++;
				y_coord_ptr++;
				skip_ptr++;
			}
		}
	}

	DC->SelectStockObject(BLACK_PEN);

	DC->SelectClipRgn(rect);

// Display title only if printing
	title_rect.top=plot_rect.top-top_margin;
	title_rect.left=plot_rect.left;
	title_rect.right=plot_rect.right;
	title_rect.bottom=plot_rect.top-text_metrics.tmHeight;
	DC->DrawText(Title,-1,title_rect, DT_BOTTOM | DT_CENTER | DT_SINGLELINE | DT_NOCLIP);

// Display x and y labels.
	if (x_label.length()) {
		x_label_rect.top=plot_rect.bottom+maj_tic_length+tic_label_spacing+3*text_metrics.tmHeight/2;
		x_label_rect.left=plot_rect.left;
		x_label_rect.right=plot_rect.right;
		x_label_rect.bottom=plot_rect.bottom+maj_tic_length+tic_label_spacing+5*text_metrics.tmHeight/2;
		DC->DrawText(x_label.c_str(),-1,x_label_rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	}

	if (y_label.length()) {
		y_label_rect.top=plot_rect.top-top_margin;
		y_label_rect.left=plot_rect.left-7*text_metrics.tmAveCharWidth;
		y_label_rect.right=plot_rect.right+right_margin;
		y_label_rect.bottom=plot_rect.top-text_metrics.tmHeight;
		DC->DrawText(y_label.c_str(),-1,y_label_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE | DT_NOCLIP);
	}

// output major tic marks
	if (x_axis.maj_tic>0) {
		next_tic=plot_rect.left;
		next_number_tic=x_axis.minimum;
		DC->SetTextAlign(TA_CENTER);
		min_coord_tic=x_axis.min_tic*x_axis.norm;
		maj_coord_tic=x_axis.maj_tic*x_axis.norm;
		while (maj_coord_tic<7*text_metrics.tmAveCharWidth) {
			maj_coord_tic*=2;
			min_coord_tic*=2;
			x_axis.maj_tic*=2;
		}
		while (next_tic<=plot_rect.right+1) {
			DC->MoveTo(next_tic,plot_rect.bottom);
			DC->LineTo(next_tic,plot_rect.bottom+maj_tic_length);
			NumberOut(next_tic,plot_rect.bottom+maj_tic_length+tic_label_spacing,next_number_tic);
			next_min_tic=next_tic+min_coord_tic;
			next_tic+=maj_coord_tic;
			while((next_min_tic<next_tic-1) && (next_min_tic<=plot_rect.right+1)){
				DC->MoveTo(next_min_tic,plot_rect.bottom);
				DC->LineTo(next_min_tic,plot_rect.bottom+min_tic_length);
				next_min_tic+=min_coord_tic;
			}
			next_number_tic+=x_axis.maj_tic;
		}
	}

	if (y_axis.maj_tic>0) {
		next_tic=plot_rect.bottom;
		next_number_tic=y_axis.minimum;
		DC->SetTextAlign(TA_RIGHT);
		if (y_axis.scale_type==LIN) {
			min_coord_tic=y_axis.min_tic*y_axis.norm;
			maj_coord_tic=y_axis.maj_tic*y_axis.norm;
			while (maj_coord_tic<text_metrics.tmHeight) {
				maj_coord_tic*=2;
				min_coord_tic*=2;
				y_axis.maj_tic*=2;
			}
			while (next_tic>=plot_rect.top-1) {
				DC->MoveTo(plot_rect.left,next_tic);
				DC->LineTo(plot_rect.left-maj_tic_length,next_tic);
				NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,next_tic-text_metrics.tmHeight/2,next_number_tic);
				next_min_tic=next_tic-min_coord_tic;
				next_tic-=maj_coord_tic;
				while((next_min_tic>next_tic+1) && (next_min_tic>=plot_rect.top-1)){
					DC->MoveTo(plot_rect.left,next_min_tic);
					DC->LineTo(plot_rect.left-min_tic_length,next_min_tic);
					next_min_tic-=min_coord_tic;
				}
				next_number_tic+=y_axis.maj_tic;
			}
		}
		else {
			maj_coord_tic=log10(y_axis.maj_tic)*y_axis.norm;
			while ((maj_coord_tic<text_metrics.tmHeight) &&
				   (y_axis.maj_tic<=1e15)) {
				maj_coord_tic*=2;
				min_coord_tic*=2;
				y_axis.maj_tic*=y_axis.maj_tic;
			}
			if (maj_coord_tic<text_metrics.tmHeight) {
				NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,next_tic-text_metrics.tmHeight/2,next_number_tic);
			}
			else {
				while (next_tic>=plot_rect.top-1) {
					DC->MoveTo(plot_rect.left,next_tic);
					DC->LineTo(plot_rect.left-maj_tic_length,next_tic);
					NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,
							  next_tic-text_metrics.tmHeight/2,next_number_tic);
					next_tic-=maj_coord_tic;
					next_number_tic*=y_axis.maj_tic;
				}
			}
		}
	}
	DC->RestoreDC();
}

void TPlotPrintout::GetDialogInfo(int& minPage, int& maxPage,
								  int& selFromPage, int& selToPage)
{
  minPage = 0;
  maxPage = 0;
  selFromPage = selToPage = 0;
}

void TPlotPrintout::allocate_coordinates(void)
{

	int i, plot_count;

	x_coord=new float[data_points];
	y_coord=new float*[number_plots];
	y_skip=new int*[number_plots];

	if (!x_coord || !y_coord || !y_skip) {
		if (x_coord) delete[] x_coord;
		if (y_coord) delete[] y_coord;
		if (y_skip) delete[] y_skip;
		error_handler.set_error(ERROR_MEM_PLOT,0,"","");
		return;
	}

	for (plot_count=0;plot_count<number_plots;plot_count++) {
		y_coord[plot_count]=new float[data_points];
		y_skip[plot_count]=new int[data_points];
		if (!y_coord[plot_count] || !y_skip[plot_count]) {
			for (i=0;i<plot_count;i++) if (!y_coord[i]) delete[] y_coord[i];
			for (i=0;i<plot_count;i++) if (!y_skip[i]) delete[] y_skip[i];
			error_handler.set_error(ERROR_MEM_PLOT,0,"","");
			return;
		}
	}
}

void TPlotPrintout::free_coordinates(void)
{
	int plot_count;

	if (x_coord) delete[] x_coord;

	if (y_coord) {
		for(plot_count=0;plot_count<number_plots;plot_count++)
			delete[] y_coord[plot_count];
		delete[] y_coord;
	}

	if (y_skip) {
		for(plot_count=0;plot_count<number_plots;plot_count++)
			delete[] y_skip[plot_count];
		delete[] y_skip;
	}
}

void TPlotPrintout::data_to_coordinates(void)
{
	int i,plot_count;
	float *data_ptr, *coord_ptr;
	logical *skip_ptr;

// Initialize y_skip array.
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		skip_ptr=y_skip[plot_count];
		for (i=0;i<data_points;i++) *(skip_ptr++)=FALSE;
	}

// Perform required operation on y data.
	switch (y_axis.operation_type) {
		case EXACT:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=*(data_ptr++);
			}
			break;
		case NEGATIVE:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=-(*(data_ptr++));
			}
			break;
		case ABS:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=fabs((*(data_ptr++)));
			}
			break;
	}

// if y-axis is log, then take the log of the values of the y
// 		coordinates. If a value is less than zero then skip the data point.
	if (y_axis.scale_type==LOG) {
		for (plot_count=0;plot_count<number_plots;plot_count++) {
			coord_ptr=y_coord[plot_count];
			skip_ptr=y_skip[plot_count];
			for (i=0;i<data_points;i++) {
				if (*coord_ptr<1e-30) *skip_ptr=TRUE;
				else {
					*skip_ptr=FALSE;
					*coord_ptr=log10(*coord_ptr);
				}
				coord_ptr++;
				skip_ptr++;
			}
		}

// if auto-scale on the y_axis is off then take log of the given scale.
		y_axis.minimum=log10(y_axis.minimum);
		y_axis.maximum=log10(y_axis.maximum);
	}

// calculate data offsets and normalization factors
	x_axis.norm=plot_rect.Width()/(x_axis.maximum-x_axis.minimum);
	y_axis.norm=plot_rect.Height()/(y_axis.maximum-y_axis.minimum);

// convert x data to x coordinates relative to plot area.
	data_ptr=x_data;
	coord_ptr=x_coord;
	for (i=0;i<data_points;i++) {
		*coord_ptr=((*data_ptr)-x_axis.minimum)*x_axis.norm+plot_rect.left;
		if (*coord_ptr>15000) *coord_ptr=15000;
		if (*coord_ptr<-15000) *coord_ptr=-15000;
		coord_ptr++;
		data_ptr++;
	}

// convert y data to y coordinates relative to plot area.
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		coord_ptr=y_coord[plot_count];
		skip_ptr=y_skip[plot_count];
		for (i=0;i<data_points;i++) {
			if (!(*skip_ptr)) {
				if (y_axis.minimum-(*coord_ptr)>((prec)(15000-plot_rect.bottom))/(prec)y_axis.norm) {
					*skip_ptr=TRUE;
					*coord_ptr=15000;
				}
				else {
					if (y_axis.minimum-(*coord_ptr)<((prec)(-15000-plot_rect.bottom))/(prec)y_axis.norm) {
						*skip_ptr=TRUE;
						*coord_ptr=-15000;
					}
					else *coord_ptr=plot_rect.bottom-((*coord_ptr)-y_axis.minimum)*y_axis.norm;
				}
			}
			else *coord_ptr=15000;
			coord_ptr++;
			skip_ptr++;
		}
	}

// Compute tic spacing
	autotic(x_axis.maximum-x_axis.minimum,x_axis.maj_tic,x_axis.min_tic);
	autotic(y_axis.maximum-y_axis.minimum,y_axis.maj_tic,y_axis.min_tic);

	if (y_axis.scale_type==LOG) {
		y_axis.minimum=pow(10.0,y_axis.minimum);
		y_axis.maximum=pow(10.0,y_axis.maximum);

		y_axis.maj_tic=pow(10.0,y_axis.maj_tic);
	}
}

void TPlotPrintout::NumberOut(int x, int y, float num)
{
	char numstr[25];

	if((fabs(num)>=1000) || ((fabs(num)<0.1) && (num != 0)))
		sprintf(numstr,"%6.1e",num);
	else
		sprintf(numstr,"%6.2f",num);
	DC->TextOut(x,y,numstr,strlen(numstr));
}

/******************************* class TSimWindowsPlot ***************************************

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

*/

TSimWindowsPlot::TSimWindowsPlot(TWindow* parent, FlagType new_x_flag_type, flag new_x_flag,
								 const TValueFlag& new_y_flags, const string& y_name,
								 logical colored, logical nodes, const char far* title,
								 TModule* module)
	: TWindow(parent,title,module),
	  FileData(OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,"Data Files (*.dat)|*.dat|","", "", "dat"),
	  trace_window(this,DG_TRACE),
	  x_flag_type(new_x_flag_type),
	  x_flag(new_x_flag),
	  y_flags(new_y_flags),
	  x_label(get_long_string(x_flag_type,x_flag)),
	  multi_colored(colored),
	  show_nodes(nodes)
{
	number_plots=0;
	data_points=0;
	x_data=(float *)0;
	x_coord=(float *)0;
	y_data=(float **)0;
	y_coord=(float **)0;
	y_skip=(logical **)0;
	y_label=y_name;
	trace_values=(float *)0;
	trace_labels=(string *)0;

	x_axis.maximum=y_axis.maximum=0;
	x_axis.minimum=y_axis.minimum=0;
	x_axis.maj_tic=y_axis.maj_tic=0;
	x_axis.min_tic=y_axis.min_tic=0;
	x_axis.norm=y_axis.norm=0;
	x_axis.auto_scale=y_axis.auto_scale=TRUE;
	x_axis.auto_tic=y_axis.auto_tic=TRUE;
	x_axis.scale_type=y_axis.scale_type=LIN;
	x_axis.operation_type=y_axis.operation_type=EXACT;

	scale_rect_ready=FALSE;
	modified=TRUE;
	frozen=FALSE;
}

void TSimWindowsPlot::SetupWindow(void)
{
	TWindow::SetupWindow();
	update_data();
}

void TSimWindowsPlot::allocate_coordinates(int new_number_plots, int new_data_points)
{

	int i, plot_count;

	if ((new_data_points!=data_points) || (new_number_plots!=number_plots)) {
		free_coordinates();
		data_points=new_data_points;
		number_plots=new_number_plots;

		x_data=new float[data_points];
		x_coord=new float[data_points];
		y_data=new float*[number_plots];
		y_coord=new float*[number_plots];
		y_skip=new logical*[number_plots];
		trace_labels=new string[number_plots+1];
		trace_values=new float[number_plots+1];

		if (!x_data || !x_coord || !y_data || !y_coord || !y_skip || !trace_labels || !trace_values) {
			if (x_data) delete[] x_data;
			if (x_coord) delete[] x_coord;
			if (y_data) delete[] y_data;
			if (y_coord) delete[] y_coord;
			if (y_skip) delete[] y_skip;
			if (trace_labels) delete[] trace_labels;
			if (trace_values) delete[] trace_values;
			error_handler.set_error(ERROR_MEM_PLOT,0,"","");
			return;
		}

		for (plot_count=0;plot_count<number_plots;plot_count++) {
			y_data[plot_count]=new float[data_points];
			y_coord[plot_count]=new float[data_points];
			y_skip[plot_count]=new logical[data_points];
			if (!y_data[plot_count] || !y_coord[plot_count] || !y_skip[plot_count]) {
				for (i=0;i<plot_count;i++) if (!y_data[i]) delete[] y_data[i];
				for (i=0;i<plot_count;i++) if (!y_coord[i]) delete[] y_coord[i];
				for (i=0;i<plot_count;i++) if (!y_skip[i]) delete[] y_skip[i];
				delete[] x_coord;
				error_handler.set_error(ERROR_MEM_PLOT,0,"","");
				return;
			}
		}
		update_trace_labels();
	}
}

void TSimWindowsPlot::free_coordinates(void)
{
	int plot_count;

	if (x_data) delete[] x_data;

	if (x_coord) delete[] x_coord;

	if (y_data) {
		for(plot_count=0;plot_count<number_plots;plot_count++)
			delete[] y_data[plot_count];
		delete[] y_data;
	}

	if (y_coord) {
		for(plot_count=0;plot_count<number_plots;plot_count++)
			delete[] y_coord[plot_count];
		delete[] y_coord;
	}

	if (y_skip) {
		for(plot_count=0;plot_count<number_plots;plot_count++)
			delete[] y_skip[plot_count];
		delete[] y_skip;
	}

	if (trace_labels) delete[] trace_labels;

	if (trace_values) delete[] trace_values;

	x_data=(float *)0;
	x_coord=(float *)0;
	y_data=(float **)0;
	y_coord=(float **)0;
	y_skip=(logical **)0;
	trace_values=(float *)0;
	data_points=0;
	number_plots=0;
}

void TSimWindowsPlot::data_to_coordinates(void)
{
	int i,plot_count;
	float *data_ptr, *coord_ptr;
	logical *skip_ptr;
	float minimum, maximum;

// Initialize y_skip array.
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		skip_ptr=y_skip[plot_count];
		for (i=0;i<data_points;i++) *(skip_ptr++)=FALSE;
	}

// Perform required operation on y data.
	switch (y_axis.operation_type) {
		case EXACT:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=*(data_ptr++);
			}
			break;
		case NEGATIVE:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=-(*(data_ptr++));
			}
			break;
		case ABS:
			for (plot_count=0;plot_count<number_plots;plot_count++) {
				data_ptr=y_data[plot_count];
				coord_ptr=y_coord[plot_count];
				for (i=0;i<data_points;i++) *(coord_ptr++)=fabs((*(data_ptr++)));
			}
			break;
	}

// if y-axis is log, then take the log of the values of the y
// 		coordinates. If a value is less than zero then skip the data point.
	if (y_axis.scale_type==LOG) {
		for (plot_count=0;plot_count<number_plots;plot_count++) {
			coord_ptr=y_coord[plot_count];
			skip_ptr=y_skip[plot_count];
			for (i=0;i<data_points;i++) {
				if (*coord_ptr<1e-30) *skip_ptr=TRUE;
				else {
					*skip_ptr=FALSE;
					*coord_ptr=log10(*coord_ptr);
				}
				coord_ptr++;
				skip_ptr++;
			}
		}

// if auto-scale on the y_axis is off then take log of the given scale.
		if (!y_axis.auto_scale) {
			y_axis.minimum=log10(y_axis.minimum);
			y_axis.maximum=log10(y_axis.maximum);
		}
	}

// Find minimum and maximum values of x coordinates.
	if (x_axis.auto_scale) {
		scale(x_data,data_points,x_axis.minimum,x_axis.maximum);
		if (x_axis.minimum==x_axis.maximum) round_scale(x_axis.minimum,x_axis.maximum);
	}

// Find minimum and maximum values of y coordinates and round the scale.
	if (y_axis.auto_scale){
		for (plot_count=0;plot_count<number_plots;plot_count++) {
			scale_with_skip(y_coord[plot_count],y_skip[plot_count],data_points,minimum,maximum);
			if (plot_count==0) {
				y_axis.minimum=minimum;
				y_axis.maximum=maximum;
			}
			else {
				if (minimum<y_axis.minimum) y_axis.minimum=minimum;
				if (maximum>y_axis.maximum) y_axis.maximum=maximum;
			}
		}
		round_scale(y_axis.minimum,y_axis.maximum);
	}

// calculate data offsets and normalization factors
	x_axis.norm=plot_rect.Width()/(x_axis.maximum-x_axis.minimum);
	y_axis.norm=plot_rect.Height()/(y_axis.maximum-y_axis.minimum);

// convert x data to x coordinates relative to plot area.
	data_ptr=x_data;
	coord_ptr=x_coord;
	for (i=0;i<data_points;i++) {
		*coord_ptr=((*data_ptr)-x_axis.minimum)*x_axis.norm+plot_rect.left;
		if (*coord_ptr>15000) *coord_ptr=15000;
		if (*coord_ptr<-15000) *coord_ptr=-15000;
		coord_ptr++;
		data_ptr++;
	}

// convert y data to y coordinates relative to plot area.
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		coord_ptr=y_coord[plot_count];
		skip_ptr=y_skip[plot_count];
		for (i=0;i<data_points;i++) {
			if (!(*skip_ptr)) {
				if (y_axis.minimum-(*coord_ptr)>((prec)(15000-plot_rect.bottom))/(prec)y_axis.norm) {
					*skip_ptr=TRUE;
					*coord_ptr=15000;
				}
				else {
					if (y_axis.minimum-(*coord_ptr)<((prec)(-15000-plot_rect.bottom))/(prec)y_axis.norm) {
						*skip_ptr=TRUE;
						*coord_ptr=-15000;
					}
					else *coord_ptr=plot_rect.bottom-((*coord_ptr)-y_axis.minimum)*y_axis.norm;
				}
			}
			else *coord_ptr=15000;
			coord_ptr++;
			skip_ptr++;
		}
	}

// Compute tic spacing
	autotic(x_axis.maximum-x_axis.minimum,x_axis.maj_tic,x_axis.min_tic);
	autotic(y_axis.maximum-y_axis.minimum,y_axis.maj_tic,y_axis.min_tic);

	if (y_axis.scale_type==LOG) {
		y_axis.minimum=pow(10.0,y_axis.minimum);
		y_axis.maximum=pow(10.0,y_axis.maximum);

		y_axis.maj_tic=pow(10.0,y_axis.maj_tic);
	}
}

int TSimWindowsPlot::get_data_point(prec value, int start_point, int end_point)
{
	int test_point, return_point;
	prec test_value;
	prec start_value, end_value;

	if (data_points<=1) return(0);

	if (start_point==-1) start_point=0;
	if (end_point==-1) end_point=data_points-1;

	if ((end_point-start_point)==1) {
		start_value=x_data[start_point];
		end_value=x_data[end_point];
		if ((value-start_value)<(end_value-value)) return_point=start_point;
		else return_point=end_point;
	}
	else {
		test_point=(int)(start_point+(end_point-start_point)/2);
		test_value=x_data[test_point];

		if (test_value>value) return_point=get_data_point(value,start_point,test_point);
		else return_point=get_data_point(value,test_point,end_point);
	}
	return(return_point);
}

void TSimWindowsPlot::update_trace_values(float x_coord)
{
	int i,data_point;

	data_point=get_data_point(x_coord);

	trace_values[0]=x_data[data_point];

	for (i=1;i<=number_plots;i++) trace_values[i]=y_data[i-1][data_point];

	trace_window.update_values(trace_values,number_plots+1);
}

void TSimWindowsPlot::write_data_file(float ref_value)
{
	int i,j;
	int data_multiplier, precision=4;

	assert(strcmp(FileData.FileName,""));

	ofstream output_file(FileData.FileName);

	if (!output_file) {
		error_handler.set_error(ERROR_FILE_NOT_OPEN,0,"",FileData.FileName);
		return;
	}

	if ((x_flag_type==GRID_ELECTRICAL) && (x_flag==POSITION)) data_multiplier=preferences.get_write_grid_multiplier();
    else data_multiplier=1;

	output_file.setf(ios::scientific,ios::floatfield);
	output_file.precision(precision);

	output_file << trace_labels[0];
	for (i=1;i<number_plots+1;i++) output_file << "," << trace_labels[i];
	output_file << '\n';

	for (i=0;i<data_points;i+=data_multiplier) {
		output_file << x_data[i];
		for (j=0;j<number_plots;j++) output_file << "," << (y_data[j][i]-ref_value);
		output_file << '\n';
	}
	output_file.close();
}

void TSimWindowsPlot::NumberOut(int x, int y, float num, TDC& device_context)
{
	char numstr[25];

	if((fabs(num)>=1000) || ((fabs(num)<0.1) && (num != 0)))
		sprintf(numstr,"%6.1e",num);
	else
		sprintf(numstr,"%6.2f",num);
	device_context.TextOut(x,y,numstr,strlen(numstr));
}

void TSimWindowsPlot::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
	float *x_coord_ptr, *y_coord_ptr;
	logical *skip_ptr,previous_skip;
	prec next_tic,next_min_tic,next_number_tic;
	prec maj_coord_tic, min_coord_tic;
	int i,plot_count,color_count;
	int left_margin, right_margin, top_margin, bot_margin;
	int maj_tic_length, min_tic_length, tic_label_spacing;
	TRect client_rect, x_label_rect, y_label_rect, title_rect;
	COLORREF color_order[5];
	COLORREF background;
	TEXTMETRIC text_metrics;
	TSize client_size;

	dc.SaveDC();

	background=::GetSysColor(COLOR_WINDOW);

	dc.SelectStockObject(SYSTEM_FONT);
	dc.SelectStockObject(HOLLOW_BRUSH);
	dc.GetTextMetrics(text_metrics);
	maj_tic_length=10;
	min_tic_length=maj_tic_length/2;
	tic_label_spacing=2;
	GetClientRect(client_rect);
	left_margin=LEFT_MARGIN;
	right_margin=RIGHT_MARGIN;
	bot_margin=BOT_MARGIN;
	top_margin=TOP_MARGIN;
	plot_rect.left=client_rect.left+left_margin;
	plot_rect.right=client_rect.right-right_margin;
	plot_rect.top=client_rect.top+top_margin;
	plot_rect.bottom=client_rect.bottom-bot_margin;
	if (background==BLUE) color_order[0]=WHITE;
	else color_order[0]=BLUE;
	if (background==RED) color_order[1]=WHITE;
	else color_order[1]=RED;
	if (background==BLACK) {
		dc.SetTextColor(WHITE);
		color_order[2]=WHITE;
	}
	else {
		color_order[2]=BLACK;
		dc.SetTextColor(BLACK);
	}
	if (background==GREEN) color_order[3]=WHITE;
	else color_order[3]=GREEN;
	if (background==YELLOW) color_order[4]=WHITE;
	else color_order[4]=YELLOW;

	if (plot_rect.Height()<1) plot_rect.bottom=plot_rect.top+1;
	if (plot_rect.Width()<1) plot_rect.right=plot_rect.left+1;

	data_to_coordinates();

	if (background==BLACK) dc.SelectStockObject(WHITE_PEN);
	else dc.SelectStockObject(BLACK_PEN);

	dc.MoveTo(plot_rect.left,plot_rect.top);
	dc.LineTo(plot_rect.left,plot_rect.bottom);
	dc.LineTo(plot_rect.right,plot_rect.bottom);

	dc.SelectClipRgn(plot_rect.InflatedBy(1,1));

// plot data
	color_count=0;
	for (plot_count=0;plot_count<number_plots;plot_count++) {
		dc.SelectObject(TPen(color_order[color_count]));

		if (multi_colored) {
			color_count++;
			if (color_count>4) color_count-=5;
		}

		previous_skip=TRUE;
		x_coord_ptr=x_coord;
		y_coord_ptr=y_coord[plot_count];
		skip_ptr=y_skip[plot_count];
		if (data_points==1) {
			if ((*x_coord_ptr>=plot_rect.left) && (*x_coord_ptr<=plot_rect.right))  {
				if (!*skip_ptr) {
					dc.MoveTo((int) *x_coord_ptr, (int) plot_rect.bottom);
					dc.LineTo((int) *x_coord_ptr, (int) *y_coord_ptr);
					if (show_nodes) dc.Ellipse((int)((*x_coord_ptr)-2.0),(int)((*y_coord_ptr)-2.0),
											   (int)((*x_coord_ptr)+3.0),(int)((*y_coord_ptr)+3.0));
				}
			}
		}
		else {
			for (i=0;i<data_points;i++) {
				if ((*x_coord_ptr>=plot_rect.left) && (*x_coord_ptr<=plot_rect.right))  {
					if (!*skip_ptr) {
						if (previous_skip) dc.MoveTo((int) *x_coord_ptr, (int) *y_coord_ptr);
						else dc.LineTo((int) *x_coord_ptr, (int) *y_coord_ptr);
						if (show_nodes) dc.Ellipse((int)((*x_coord_ptr)-2.0),(int)((*y_coord_ptr)-2.0),
												   (int)((*x_coord_ptr)+3.0),(int)((*y_coord_ptr)+3.0));
					}
					previous_skip=*skip_ptr;
				}
				x_coord_ptr++;
				y_coord_ptr++;
				skip_ptr++;
			}
		}
	}

	dc.SelectClipRgn(client_rect);
	if (background==BLACK) dc.SelectStockObject(WHITE_PEN);
	else dc.SelectStockObject(BLACK_PEN);
	dc.SetBkColor(background);

// Display x and y labels.
	if (x_label.length()) {
		x_label_rect.top=plot_rect.bottom+maj_tic_length+tic_label_spacing+3*text_metrics.tmHeight/2;
		x_label_rect.left=plot_rect.left;
		x_label_rect.right=plot_rect.right;
		x_label_rect.bottom=plot_rect.bottom+maj_tic_length+tic_label_spacing+5*text_metrics.tmHeight/2;
		dc.DrawText(x_label.c_str(),-1,x_label_rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	}

	if (y_label.length()) {
		y_label_rect.top=plot_rect.top-top_margin;
		y_label_rect.left=plot_rect.left-7*text_metrics.tmAveCharWidth;
		y_label_rect.right=plot_rect.right+right_margin;
		y_label_rect.bottom=plot_rect.top-text_metrics.tmHeight;
		dc.DrawText(y_label.c_str(),-1,y_label_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE | DT_NOCLIP);
	}

// output major tic marks
	if (x_axis.maj_tic>0) {
		next_tic=plot_rect.left;
		next_number_tic=x_axis.minimum;
		dc.SetTextAlign(TA_CENTER);
		min_coord_tic=x_axis.min_tic*x_axis.norm;
		maj_coord_tic=x_axis.maj_tic*x_axis.norm;
		while (maj_coord_tic<7*text_metrics.tmAveCharWidth) {
			maj_coord_tic*=2;
			min_coord_tic*=2;
			x_axis.maj_tic*=2;
		}
		while (next_tic<=plot_rect.right+1) {
			dc.MoveTo(next_tic,plot_rect.bottom);
			dc.LineTo(next_tic,plot_rect.bottom+maj_tic_length);
			NumberOut(next_tic,plot_rect.bottom+maj_tic_length+tic_label_spacing,next_number_tic,dc);
			next_min_tic=next_tic+min_coord_tic;
			next_tic+=maj_coord_tic;
			while((next_min_tic<next_tic-1) && (next_min_tic<=plot_rect.right+1)){
				dc.MoveTo(next_min_tic,plot_rect.bottom);
				dc.LineTo(next_min_tic,plot_rect.bottom+min_tic_length);
				next_min_tic+=min_coord_tic;
			}
			next_number_tic+=x_axis.maj_tic;
		}
	}

	if (y_axis.maj_tic>0) {
		next_tic=plot_rect.bottom;
		next_number_tic=y_axis.minimum;
		dc.SetTextAlign(TA_RIGHT);
		if (y_axis.scale_type==LIN) {
			min_coord_tic=y_axis.min_tic*y_axis.norm;
			maj_coord_tic=y_axis.maj_tic*y_axis.norm;
			while (maj_coord_tic<text_metrics.tmHeight) {
				maj_coord_tic*=2;
				min_coord_tic*=2;
				y_axis.maj_tic*=2;
			}
			while (next_tic>=plot_rect.top-1) {
				dc.MoveTo(plot_rect.left,next_tic);
				dc.LineTo(plot_rect.left-maj_tic_length,next_tic);
				NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,
						  next_tic-text_metrics.tmHeight/2,next_number_tic,dc);
				next_min_tic=next_tic-min_coord_tic;
				next_tic-=maj_coord_tic;
				while((next_min_tic>next_tic+1) && (next_min_tic>=plot_rect.top-1)){
					dc.MoveTo(plot_rect.left,next_min_tic);
					dc.LineTo(plot_rect.left-min_tic_length,next_min_tic);
					next_min_tic-=min_coord_tic;
				}
				next_number_tic+=y_axis.maj_tic;
			}
		}
		else {
			maj_coord_tic=log10(y_axis.maj_tic)*y_axis.norm;
			while ((maj_coord_tic<text_metrics.tmHeight) &&
				   (y_axis.maj_tic<=1e15)) {
				maj_coord_tic*=2;
				min_coord_tic*=2;
				y_axis.maj_tic*=y_axis.maj_tic;
			}
			if (maj_coord_tic<text_metrics.tmHeight) {
				NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,
						  next_tic-text_metrics.tmHeight/2,next_number_tic,dc);
			}
			else {
				while (next_tic>=plot_rect.top-1) {
					dc.MoveTo(plot_rect.left,next_tic);
					dc.LineTo(plot_rect.left-maj_tic_length,next_tic);
					NumberOut(plot_rect.left-maj_tic_length-tic_label_spacing,
							  next_tic-text_metrics.tmHeight/2,next_number_tic,dc);
					next_tic-=maj_coord_tic;
					next_number_tic*=y_axis.maj_tic;
				}
			}
		}
	}
	dc.RestoreDC();
}

DEFINE_RESPONSE_TABLE1(TSimWindowsPlot, TWindow)
	EV_WM_SIZE,
	EV_WM_MOUSEMOVE,
	EV_WM_LBUTTONUP,
	EV_WM_LBUTTONDBLCLK,
	EV_COMMAND(CM_FILESAVE, CmFileSave),
	EV_COMMAND_ENABLE(CM_FILESAVE, CmFileSaveEnabler),
	EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
	EV_COMMAND(CM_FILEPRINT, CmFilePrint),
	EV_COMMAND(CM_PLOTSCALE, CmPlotScale),
	EV_COMMAND(CM_PLOTAUTOSCALE, CmPlotAutoScale),
	EV_COMMAND(CM_PLOTSHOWTRACE, CmPlotShowTrace),
	EV_COMMAND(CM_PLOTSHOWNODES, CmPlotShowNodes),
	EV_COMMAND_ENABLE(CM_PLOTSHOWNODES, CmPlotShowNodesEnabler),
	EV_COMMAND(CM_PLOTFREEZE, CmPlotFreeze),
	EV_COMMAND_ENABLE(CM_PLOTFREEZE, CmPlotFreezeEnabler),
END_RESPONSE_TABLE;

void TSimWindowsPlot::EvMouseMove(UINT modKeys, TPoint& point)
{

	TPoint screen_point_1(plot_rect.left,plot_rect.top);
	TPoint screen_point_2(plot_rect.right+1,plot_rect.bottom+1);
	TPoint screen_current_position=point;
	TPoint new_point;
	COLORREF background;
	float x_coord;
	TClientDC *client_dc;

	if (TRect(screen_point_1,screen_point_2).Contains(point)) {
		background=::GetSysColor(COLOR_WINDOW);
		if (background==BLACK) SetCursor(GetApplication(),IDC_WHITECROSS);
		else SetCursor(GetApplication(), IDC_BLACKCROSS);
		if (trace_window.IsWindow() && trace_window.IsWindowVisible()) {
			x_coord=((float)(point.x-plot_rect.left)/x_axis.norm)+x_axis.minimum;
			update_trace_values(x_coord);
			trace_window.redraw_values();
		}
	}
	else SetCursor(NULL,IDC_ARROW);

	if (modKeys==MK_LBUTTON) {
		if (!scale_rect_ready) {
			ClientToScreen(screen_point_1);
			ClientToScreen(screen_point_2);
			ClientToScreen(screen_current_position);
			::ClipCursor(new TRect(screen_point_1,screen_point_2));
			::SetCursorPos(screen_current_position.x,screen_current_position.y);
			GetCursorPos(new_point);
			ScreenToClient(new_point);
			scale_rect.left=new_point.x;
			scale_rect.right=new_point.x;
			scale_rect.top=new_point.y;
			scale_rect.bottom=new_point.y;
			scale_rect_ready=TRUE;
		}
		else {
			client_dc=new TClientDC(*this);
			client_dc->SelectStockObject(BLACK_PEN);
			client_dc->SelectStockObject(NULL_BRUSH);
			client_dc->SaveDC();
			client_dc->SetROP2(R2_NOT);
			client_dc->Rectangle(scale_rect);
			scale_rect.right=point.x;
			scale_rect.bottom=point.y;
			client_dc->Rectangle(scale_rect);
			client_dc->RestoreDC(-1);
			delete client_dc;
		}
	}
}

void TSimWindowsPlot::EvLButtonUp(UINT /*modKeys*/, TPoint& /*point*/)
{
	float new_x_min, new_x_max, new_y_min, new_y_max;
	TClientDC *client_dc;

	if (scale_rect_ready) {
		client_dc=new TClientDC(*this);
		client_dc->SelectStockObject(BLACK_PEN);
		client_dc->SelectStockObject(NULL_BRUSH);
		client_dc->SaveDC();
		client_dc->SetROP2(R2_NOT);
		client_dc->Rectangle(scale_rect);
		client_dc->RestoreDC(-1);
		::ClipCursor(NULL);
		scale_rect_ready=FALSE;
		scale_rect.Normalize();
		if (scale_rect.Width()>10 && scale_rect.Height()>10) {
			x_axis.auto_scale=FALSE;
			y_axis.auto_scale=FALSE;

			new_x_min=(float)(scale_rect.left-plot_rect.left)/(float)plot_rect.Width();
			new_x_max=(float)(plot_rect.right-scale_rect.right)/(float)plot_rect.Width();
			if (x_axis.scale_type==LOG) {
				new_x_min*=(log10(x_axis.maximum)-log10(x_axis.minimum));
				new_x_max*=(log10(x_axis.maximum)-log10(x_axis.minimum));
				x_axis.minimum*=pow(10.0,new_x_min);
				x_axis.maximum/=pow(10.0,new_x_max);
			}
			else {
				new_x_min*=(x_axis.maximum-x_axis.minimum);
				new_x_max*=(x_axis.maximum-x_axis.minimum);
				x_axis.minimum+=new_x_min;
				x_axis.maximum-=new_x_max;
			}

			new_y_min=(float)(plot_rect.bottom-scale_rect.bottom)/(float)plot_rect.Height();
			new_y_max=(float)(scale_rect.top-plot_rect.top)/(float)plot_rect.Height();
			if (y_axis.scale_type==LOG) {
				new_y_min*=(log10(y_axis.maximum)-log10(y_axis.minimum));
				new_y_max*=(log10(y_axis.maximum)-log10(y_axis.minimum));
				y_axis.minimum*=pow(10.0,new_y_min);
				y_axis.maximum/=pow(10.0,new_y_max);
			}
			else {
				new_y_min*=(y_axis.maximum-y_axis.minimum);
				new_y_max*=(y_axis.maximum-y_axis.minimum);
				y_axis.minimum+=new_y_min;
				y_axis.maximum-=new_y_max;
			}
			Invalidate(TRUE);
		}
		delete client_dc;
	}
}

void TSimWindowsPlot::CmFilePrint(void)
{
	if (printer) {
		TPlotPrintout printout(Parent->Title, number_plots, data_points,
							   x_axis,x_data,x_label,y_axis,y_data,y_label,show_nodes);
		printer->Print(this,printout,TRUE);
	}
}

void TSimWindowsPlot::CmPlotScale(void)
{
	if (TDialogScale(this,DG_SCALE,x_axis,y_axis).Execute()==IDOK) {
		Invalidate(TRUE);
	}
}

void TSimWindowsPlot::CmPlotAutoScale(void)
{
	x_axis.auto_scale=TRUE;
	y_axis.auto_scale=TRUE;
	Invalidate(TRUE);
}

void TSimWindowsPlot::CmPlotShowTrace(void)
{
	if (!trace_window.IsWindow()) trace_window.Create();
	trace_window.Show(SW_NORMAL);
}

void TSimWindowsPlot::CmPlotShowNodes(void)
{
	show_nodes^=TRUE; Invalidate(TRUE);
}

void TSimWindowsPlot::CmPlotShowNodesEnabler(TCommandEnabler& commandHandler)
{
	if (show_nodes) commandHandler.SetText("Hide &Nodes\tCtrl+N");
	else commandHandler.SetText("Show &Nodes\tCtrl+N");
	commandHandler.Enable(TRUE);
}

void TSimWindowsPlot::CmPlotFreeze(void)
{
	string caption_string,title(Parent->Title),frozen_string(" (frozen)");
	frozen=~frozen;

	if (frozen) {
		caption_string=title+frozen_string;
		Parent->SetCaption(caption_string.c_str());
	}
	else {
		title.resize(title.length()-frozen_string.length());
		Parent->SetCaption(title.c_str());
		update_data();
		Invalidate(TRUE);
	}
}

void TSimWindowsPlot::CmPlotFreezeEnabler(TCommandEnabler& commandHandler)
{
	if (frozen) commandHandler.SetText("Melt &Plot\tCtrl+F");
	else commandHandler.SetText("Freeze &Plot\tCtrl+F");
	commandHandler.Enable(can_update());
}


/******************************** class TSimWindowsEnvironPlot ********************************

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
*/

void TSimWindowsEnvironPlot::update_data(void)
{

	float *x_data_ptr, *y_data_ptr;
	int i,j,flag_type,plot_count;
	int new_data_points, new_number_plots;
	int max_bit;
	flag test_flag, valid_flags;

	new_number_plots=0;
	new_data_points=(short)environment.get_number_objects(x_flag_type);
	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++)
		new_number_plots+=y_flags.count((FlagType)flag_type);

	allocate_coordinates(new_number_plots, new_data_points);
	if (error_handler.fail()) return;

	x_data_ptr=x_data;

	for (i=0;i<data_points;i++) *(x_data_ptr++)=environment.get_value(x_flag_type,x_flag,i);

	plot_count=0;
	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (y_flags.any_set((FlagType)flag_type)) {
			test_flag=1;
			max_bit=bit_position(y_flags.get_max((FlagType)flag_type));
			valid_flags=y_flags.get_valid((FlagType)flag_type);
			for (i=0;i<=max_bit;i++) {
				if ((test_flag & valid_flags) && (y_flags.is_set((FlagType)flag_type,test_flag))) {
					y_data_ptr=y_data[plot_count];
					if ((test_flag==QUASI_FERMI) && ((flag_type==ELECTRON) ||
													 (flag_type==HOLE))) {
						if (flag_type==ELECTRON) {
							for (j=0;j<data_points;j++)
								*(y_data_ptr++)=environment.get_value(ELECTRON,BAND_EDGE,j)-
												environment.get_value(ELECTRON,QUASI_FERMI,j);
						}
						else {
							for (j=0;j<data_points;j++)
								*(y_data_ptr++)=environment.get_value(HOLE,BAND_EDGE,j)+
												environment.get_value(HOLE,QUASI_FERMI,j);
						}
					}
					else {
						for (j=0;j<data_points;j++)
							*(y_data_ptr++)=environment.get_value((FlagType)flag_type,test_flag,j);
					}
					plot_count++;
				}
				test_flag<<=1;
			}
		}
	}

	modified=TRUE;
}

void TSimWindowsEnvironPlot::update_trace_labels(void)
{
	int i,j=0,flag_type;
	flag test_flag, valid_flags;
	int max_bit;

	trace_labels[j++]=get_short_string(x_flag_type,x_flag);

	for (flag_type=1;flag_type<=NUMBER_FLAG_TYPES;flag_type++) {
		if (y_flags.any_set((FlagType)flag_type)) {
			test_flag=1;
			max_bit=bit_position(y_flags.get_max((FlagType)flag_type));
			valid_flags=y_flags.get_valid((FlagType)flag_type);
			for (i=0;i<=max_bit;i++) {
				if ((test_flag & valid_flags) && y_flags.is_set((FlagType)flag_type,test_flag))
					trace_labels[j++]=get_short_string((FlagType)flag_type,test_flag);
				test_flag<<=1;
			}
		}
	}

	trace_window.update_labels(trace_labels,number_plots+1);
}

void TSimWindowsEnvironPlot::SaveAs(void)
{
	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		write_data_file();
		::SetCursor(TCursor(NULL,IDC_ARROW));
		if (error_handler.fail()) out_error_message(TRUE);
		else modified=FALSE;
	}
}

/******************************** class TSimWindowsBandDiagram ********************************

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
*/

void TSimWindowsBandDiagram::SaveAs(void)
{

	int i;
	float reference_value;

	if (TDialogWriteBand(this,DG_WRITEBAND,ref_flag_type,ref_flag).Execute()==IDOK) {
		if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
			if ((ref_flag_type==GRID_ELECTRICAL) && (ref_flag==VACUUM_LEVEL))
				reference_value=0.0;
			else {
				for (i=1;i<number_plots+1;i++) {
					if (get_short_string(ref_flag_type,ref_flag)!=trace_labels[i]) continue;
					else reference_value=y_data[i-1][0];
				}
			}
			write_data_file(reference_value);
			if (error_handler.fail()) out_error_message(TRUE);
			else modified=FALSE;
		}
	}

}

/******************************** class TSimWindowsMacroPlot ********************************

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
						 logical colored=TRUE, logical nodes=FALSE, const char far* title=0, TModule* module=0);
	virtual logical can_update(void);
	virtual void update_data(void);
	virtual void update_trace_labels(void);
	virtual void SaveAs(void);
};
*/

TSimWindowsMacroPlot::TSimWindowsMacroPlot(TWindow* parent, int macro_number,
										   FlagType new_x_flag_type, flag new_x_flag,
										   int x_object,
										   FlagType new_y_flag_type, flag new_y_flag,
										   string y_name, logical colored,logical nodes,
										   const char far* title, TModule* module)
	: TSimWindowsPlot(parent,new_x_flag_type,new_x_flag,TValueFlag(new_y_flag_type,new_y_flag),
					  y_name,colored,nodes,title,module)
{
	x_object_number=x_object;
	y_flag_type=new_y_flag_type;
	y_flag=new_y_flag;
	x_label+=get_long_location_string(x_flag_type,x_object_number);
	macro=macro_storage.get_macro(macro_number);
}

logical TSimWindowsMacroPlot::can_update(void)
{
	int i,number_macros;
	TValueFlagWithObject record_flags;
	logical return_value=FALSE;

	number_macros=macro_storage.get_number_macros();

	for (i=0;i<number_macros;i++) {
		if (macro==macro_storage.get_macro(i)) {
			return_value=TRUE;
			break;
		}
	}
	if (!return_value) return(return_value);

	if (macro->get_increment_object_number()!=x_object_number) return(FALSE);
	record_flags=macro->get_record_flags();
	return(record_flags.is_set(y_flag_type,y_flag));
}

void TSimWindowsMacroPlot::update_data(void)
{
	float *x_data_ptr, *y_data_ptr;
	int i,plot_count;
	int new_data_points, new_number_plots;
	int y_object_number;
	size_t base_string_start;
	TValueFlagWithObject record_flags;
	string title_string(Parent->Title);

	base_string_start=title_string.find_first_of(":");
	if (base_string_start!=NPOS) title_string.remove(0,base_string_start+2);
	title_string.prepend(macro->get_macro_name()+": ");
	Parent->SetCaption(title_string.c_str());

	new_data_points=macro->get_number_values();
	record_flags=macro->get_record_flags();
	new_number_plots=record_flags.get_number_objects(y_flag_type);

	allocate_coordinates(new_number_plots, new_data_points);
	if (error_handler.fail()) return;

	x_data_ptr=x_data;

	for (i=0;i<data_points;i++) *(x_data_ptr++)=macro->get_value(x_flag_type,x_flag,x_object_number,i);

	for (plot_count=0;plot_count<new_number_plots;plot_count++) {
		y_data_ptr=y_data[plot_count];
		y_object_number=record_flags.get_object(y_flag_type,plot_count);
		for (i=0;i<data_points;i++)
			*(y_data_ptr++)=macro->get_value(y_flag_type, y_flag, y_object_number,i);
	}

	if (*(x_data)>*(x_data+data_points-1)) {
    	for (i=0;i<data_points-i;i++) swap(*(x_data+i),*(x_data+data_points-i-1));
		for (plot_count=0;plot_count<new_number_plots;plot_count++) {
			for (i=0;i<data_points-i;i++) swap(*(y_data[plot_count]+i),
            								   *(y_data[plot_count]+data_points-i-1));
        }
    }

	modified=TRUE;
}

void TSimWindowsMacroPlot::update_trace_labels(void)
{
	int plot_count;
	TValueFlagWithObject record_flags;

	record_flags=macro->get_record_flags();

	trace_labels[0]=get_short_string(x_flag_type,x_flag)+get_short_location_string(x_flag_type,x_object_number);

	for (plot_count=1;plot_count<number_plots+1;plot_count++) {
		trace_labels[plot_count]=get_short_string(y_flag_type,y_flag)+
								 get_short_location_string(y_flag_type,record_flags.get_object(y_flag_type,plot_count-1));
	}

	trace_window.update_labels(trace_labels,number_plots+1);
}

void TSimWindowsMacroPlot::SaveAs(void)
{
	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		::SetCursor(TCursor(NULL,IDC_WAIT));
		write_data_file();
		::SetCursor(TCursor(NULL,IDC_ARROW));
		if (error_handler.fail()) out_error_message(TRUE);
		else modified=FALSE;
	}
}



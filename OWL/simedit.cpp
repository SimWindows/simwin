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
extern TPrinter* printer;

/********************************** TPrintEditFilePrintOut ********************************************

class TPrintEditFilePrintOut : public TPrintout {
protected:
	TWindow *window;
	bool scale;
	int map_mode;
	int prev_mode;
	TSize OldVExt;
	TSize OldWExt;
	TRect OrgR;
public:
	TPrintEditFilePrintOut(const char far *title, TWindow* new_window, bool new_scale = true)
		: TPrintout(title) { window = new_window; scale = new_scale; map_mode = MM_ANISOTROPIC; }
	void GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage);
	void BeginPrinting(void);
	void BeginPage(TRect& clientR);
	void PrintPage(int page, TRect& rect, unsigned flags);
	void EndPage (void);
	void SetBanding(bool new_banding){ Banding = new_banding; }
	bool HasPage(int pageNumber);
};
*/

void TPrintEditFilePrintOut::GetDialogInfo (int& minPage, int& maxPage,
											int& selFromPage, int& selToPage)
{
	maxPage=minPage=0;
	selFromPage=selToPage=0;
}

void TPrintEditFilePrintOut::BeginPrinting(void)
{
	TRect clientR;
	TEXTMETRIC text_metrics;
	int font_height, lines_per_page, max_page;
	MINMAXINFO minmaxinfo;

	BeginPage(clientR);

	TFont font("Arial", -14);
	DC->SelectObject(font);
	DC->GetTextMetrics(text_metrics);
	font_height=text_metrics.tmHeight + text_metrics.tmExternalLeading;

	DC->RestoreFont();

	lines_per_page = MulDiv(clientR.Height(), 1, font_height);
	TPrintDialog::TData &printerData = printer->GetSetup();

	// GetMinMaxInfo event is overrided to return the number of lines when printing.
	window->SendMessage(WM_GETMINMAXINFO, 0, (long)&minmaxinfo);
	max_page = ((minmaxinfo.ptMaxSize.y / lines_per_page) + 1.0);

	// Compute the number of pages to print.
	printerData.MinPage = 1;
	printerData.MaxPage = max_page;

	EndPage();

	TPrintout::BeginPrinting();
}


void TPrintEditFilePrintOut::BeginPage (TRect &clientR)
{
	int adjX, adjY;
	TScreenDC screenDC;
	TSize screenRes(screenDC.GetDeviceCaps(LOGPIXELSX),
					screenDC.GetDeviceCaps(LOGPIXELSY));
	TSize printRes(DC->GetDeviceCaps(LOGPIXELSX),
				   DC->GetDeviceCaps(LOGPIXELSY));

	// Temporarily change the window size (so any WM_PAINT queries on the total window size (GetClientRect) is
	// the window size for the WM_PAINT of the window and the printer page size when Paint is called from
	// PrintPage. Notice, we don't use AdjustWindowRect because its harder and not accurate.  Instead we
	// compute the difference (in pixels) between the client window and the frame window.  This difference
	// is then added to the clientRect to compute the new frame window size for SetWindowPos.
	clientR = window->GetClientRect();
	window->MapWindowPoints(HWND_DESKTOP, (TPoint*)&clientR, 2);

	// Compute extra X and Y pixels to bring a client window dimensions to equal the frame window.
	OrgR = window->GetWindowRect();
	adjX = OrgR.Width() - clientR.Width();
	adjY = OrgR.Height() - clientR.Height();

	// Conditionally scale the DC to the window so the printout will resemble the window.
	if (scale) {
		clientR = window->GetClientRect();
		prev_mode = DC->SetMapMode(map_mode);
		DC->SetViewportExt(PageSize, &OldVExt);

		// Scale window to logical page size (assumes left & top are 0)
		clientR.right = MulDiv(PageSize.cx, screenRes.cx, printRes.cx);
		clientR.bottom = MulDiv(PageSize.cy, screenRes.cy, printRes.cy);

		DC->SetWindowExt(clientR.Size(), &OldWExt);
	}

	// Compute the new size of the window based on the printer DC dimensions.
	// Resize the window, notice position, order, and redraw are not done the window size changes but the user
	// doesn't see any visible change to the window.
	window->SetRedraw(false);
	window->SetWindowPos(0, 0, 0, clientR.Width() + adjX, clientR.Height() + adjY,
						 SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER| SWP_NOACTIVATE);
}


void TPrintEditFilePrintOut::PrintPage (int page, TRect& bandRect, unsigned)
{
	TRect clientR;
	int from_page, to_page;

	BeginPage(clientR);

	if (scale) DC->DPtoLP(bandRect, 2);

	// Change the printer range to this current page.
	TPrintDialog::TData& printerData = printer->GetSetup();
	from_page = printerData.FromPage;
	to_page = printerData.ToPage;

	printerData.FromPage = page;
	printerData.ToPage = page;

	// Call the window to paint itself to the printer DC.
	window->Paint(*DC, FALSE, bandRect);

	printerData.FromPage = from_page;
	printerData.ToPage = to_page;

	if (scale) DC->LPtoDP(bandRect, 2);

	EndPage();
}


void TPrintEditFilePrintOut::EndPage ()
{
	// Resize to original window size, no one's the wiser.
	window->SetWindowPos(0, 0, 0, OrgR.Width(), OrgR.Height(),
						 SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER| SWP_NOACTIVATE);
	window->SetRedraw(true);

	// Restore changes made to the DC
	if (scale) {
		DC->SetWindowExt(OldWExt);
		DC->SetViewportExt(OldVExt);
		DC->SetMapMode(prev_mode);
	}
}

bool TPrintEditFilePrintOut::HasPage (int pageNumber)
{
	TPrintDialog::TData &printerData = printer->GetSetup();
	return (pageNumber >= printerData.MinPage) && (pageNumber <= printerData.MaxPage);
}


//************************************ TPrintEditFile *****************************************
/*
class TPrintEditFile: public TEditFile {
protected:
	logical printing;
public:
	TPrintEditFile(TWindow*        parent = 0,
				   int             id = 0,
				   const char far* text = 0,
				   int x = 0, int y = 0, int w = 0, int h = 0,
				   const char far* fileName = 0,
				   TModule*        module = 0)
	: TEditFile(parent,id,text,x,y,w,h,fileName,module) { printing=FALSE; }

protected:
	void Paint(TDC& dc, bool, TRect& rect);
	DECLARE_RESPONSE_TABLE(TPrintEditFile);
	void CmFilePrint(void);
	void EvGetMinMaxInfo (MINMAXINFO far& minmaxinfo);
};
*/

void TPrintEditFile::Paint(TDC& dc, bool, TRect& rect)
{
	TEXTMETRIC text_metrics;
	int font_height, lines_per_page, max_page;
	int from_page, to_page, current_page;
	int start_line, line_index;
	char buffer[255];

	if (printing && !rect.IsEmpty()) {
		// Use pageSize to get the size of the window to render into.  For a Window it's the client area,
		// for a printer it's the printer DC dimensions and for print preview it's the layout window.
		TSize   pageSize(rect.right - rect.left, rect.bottom - rect.top);

		TFont   font("Arial", -14);
		dc.SelectObject(font);

		dc.GetTextMetrics(text_metrics);
		font_height=text_metrics.tmHeight + text_metrics.tmExternalLeading;

		// How many lines of this font can we fit on a page.
		lines_per_page = MulDiv(pageSize.cy, 1, font_height);
		if (lines_per_page) {
			TPrintDialog::TData &printerData = printer->GetSetup();
			max_page = ((GetNumLines() / lines_per_page) + 1.0);

			// Compute the number of pages to print.
			printerData.MinPage = 1;
			printerData.MaxPage = max_page;

			// Do the text stuff:
			from_page = printerData.FromPage == -1 ? 1 : printerData.FromPage;
			to_page = printerData.ToPage == -1 ? 1 : printerData.ToPage;
			current_page = from_page;

			while (current_page <= to_page) {
				start_line = (current_page - 1) * lines_per_page;
				line_index = 0;
				while (line_index < lines_per_page) {
				// If the string is no longer valid then there's nothing more to display.
					if (!GetLine(buffer, sizeof(buffer), start_line + line_index))
						break;
					dc.TabbedTextOut(TPoint(0, line_index*font_height), buffer, strlen(buffer), 0, NULL, 0);
					line_index++;
				}
				current_page++;
			}
		}
	}
}

DEFINE_RESPONSE_TABLE1(TPrintEditFile, TEditFile)
	EV_COMMAND(CM_FILEPRINT, CmFilePrint),
	EV_WM_GETMINMAXINFO,
END_RESPONSE_TABLE;

void TPrintEditFile::CmFilePrint(void)
{
	printing=TRUE;
	TPrintEditFilePrintOut printout(Parent->Title, this);
	printout.SetBanding(true);
	printer->Print(this, printout, TRUE);
	printing=FALSE;
}

void TPrintEditFile::EvGetMinMaxInfo (MINMAXINFO far& minmaxinfo)
{
	if (printing) {
		minmaxinfo.ptMaxSize = TPoint(32000, 32000);
		minmaxinfo.ptMaxTrackSize = TPoint(32000, 32000);
		return;
	}
	TEditFile::EvGetMinMaxInfo(minmaxinfo);
}

/********************************* class TSimWindowsDeviceStatus ******************************/
/*
class TSimWindowsDeviceStatus: public TPrintEditFile {
public:
	TSimWindowsDeviceStatus(TWindow*        parent = 0,
							int             id = 0,
							const char far* text = 0,
							int x = 0, int y = 0, int w = 0, int h = 0,
							const char far* fileName = 0,
							TModule*        module = 0)
	: TPrintEditFile(parent,id,text,x,y,w,h,fileName,module) {}

	void SetupWindow(void);
	virtual bool CanClear(void);
	void Clear(void);
	bool IsModified(void) { return(environment.ismodified()); }
    bool IsSolving(void) { return(environment.is_solving()); }
	bool Save(void);
	bool SaveAs(void);
	bool Read(const char far* fileName=0);
	bool Write(const char far* fileName=0);
protected:
	DECLARE_RESPONSE_TABLE(TSimWindowsDeviceStatus);
	void EvLButtonDown(UINT modKeys, TPoint& point) {}
	void EvLButtonDblClk(UINT modKeys, TPoint& point) {}
	void EvKeyDown(UINT key, UINT repeatCount, UINT flags) {}
	void CmFileSaveEnabler(TCommandEnabler& commandHandler) { commandHandler.Enable(IsModified()); }
	void CmEditUndoEnabler(TCommandEnabler& commandHandler);
	void CmEditMenuEnabler(TCommandEnabler& commandHandler) { commandHandler.Enable(FALSE); }
	void CmFileSave(void) { Save(); }
	void CmFileSaveAs(void) { SaveAs(); }
	void CmEditUndo(void);
	void CmEditClear(void) { Clear(); }
};

*/

void TSimWindowsDeviceStatus::SetupWindow(void)
{
	Parent->SetCaption("STATE FILE");
	TPrintEditFile::SetupWindow();
	FileData.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	FileData.SetFilter(GetModule()->LoadString(IDS_FILEFILTER).c_str());

	SetFileName(FileName);
	if (FileName && !Read()) {
		out_error_message(TRUE);
		return;
	}
	FileData.SetFilter("State Files (*.sta)|*.sta|");
	SetReadOnly(TRUE);
}

bool TSimWindowsDeviceStatus::CanClear(void)
{
	TMacro* solving_macro=macro_storage.get_solving_macro();
    int result;
	string short_path;

    if (IsSolving() || (solving_macro!=NULL)) {
    	result = MessageBox("The Simulation is currently running. Do you want to stop?",
    						GetApplication()->GetName(), MB_YESNO|MB_ICONQUESTION);
		if (result==IDYES) {
        	if (solving_macro) solving_macro->set_stop_solution(TRUE);
        	environment.set_stop_solution(TRUE);
        }
        else
        	return FALSE;
    }
	if (IsModified()) {
		string msgTemplate("The state file %s has changed.\nDo you want to save changes?");
		string untitled(GetModule()->LoadString(IDS_UNTITLEDFILE));
		char*  msg = new char[MAXPATH+msgTemplate.length()];

		short_path=shorten_path(FileName);

		wsprintf(msg, msgTemplate.c_str(),
				 FileName ? (const char far*)short_path.c_str() : untitled.c_str());

		result = MessageBox(msg, GetApplication()->GetName(), MB_YESNOCANCEL|MB_ICONQUESTION);
		delete[] msg;
		return result==IDYES ? Save() : result != IDCANCEL;
	}
	return TRUE;
}

void TSimWindowsDeviceStatus::Clear(void)
{
	SetReadOnly(FALSE);
	TPrintEditFile::Clear();
	SetReadOnly(TRUE);
}

bool TSimWindowsDeviceStatus::Save()
{
	if (IsModified()) {
		if (!FileName) return SaveAs();

		if (!Write()) {
			string msgTemplate(GetModule()->LoadString(IDS_UNABLEWRITE));
			char*  msg = new char[MAXPATH + msgTemplate.length()];
			wsprintf(msg, msgTemplate.c_str(), FileName);
			MessageBox(msg, GetApplication()->GetName(), MB_ICONEXCLAMATION | MB_OK);
			delete msg;
			return FALSE;
		}
	}
	return TRUE;
}

bool TSimWindowsDeviceStatus::SaveAs()
{
	if (FileName) strcpy(FileData.FileName, FileName);
	else *FileData.FileName = 0;

	if ((new TFileSaveDialog(this, FileData))->Execute() == IDOK) {
		if (Write(FileData.FileName)) {
			SetFileName(FileData.FileName);
			return TRUE;
		}
		string msgTemplate(GetModule()->LoadString(IDS_UNABLEWRITE));
		char*  msg = new char[MAXPATH + msgTemplate.length()];
		wsprintf(msg, msgTemplate.c_str(), FileName);
		MessageBox(msg, GetApplication()->GetName(), MB_ICONEXCLAMATION | MB_OK);
		delete msg;
	}
	return FALSE;
}

bool TSimWindowsDeviceStatus::Read(const char far* fileName)
{
	if (!fileName)
		if (FileName) fileName = FileName;
		else return FALSE;

	::SetCursor(TCursor(NULL,IDC_WAIT));
	environment.load_file(fileName);
	::SetCursor(TCursor(NULL,IDC_ARROW));
	return(!error_handler.fail());
}

bool TSimWindowsDeviceStatus::Write(const char far* fileName)
{
	if (!fileName)
		if (FileName) fileName = FileName;
		else return FALSE;

	::SetCursor(TCursor(NULL,IDC_WAIT));
	environment.write_state_file(fileName);
	::SetCursor(TCursor(NULL,IDC_ARROW));
	return(!error_handler.fail());
}

DEFINE_RESPONSE_TABLE1(TSimWindowsDeviceStatus,TPrintEditFile)
	EV_WM_LBUTTONDOWN,
	EV_WM_LBUTTONDBLCLK,
	EV_WM_KEYDOWN,
	EV_COMMAND_ENABLE(CM_FILESAVE, CmFileSaveEnabler),
	EV_COMMAND_ENABLE(CM_EDITUNDO,CmEditUndoEnabler),
	EV_COMMAND_ENABLE(CM_EDITFIND,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITFINDNEXT,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITREPLACE,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITCUT,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITCOPY,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITDELETE,CmEditMenuEnabler),
	EV_COMMAND_ENABLE(CM_EDITPASTE,CmEditMenuEnabler),
	EV_COMMAND(CM_FILESAVE, CmFileSave),
	EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
	EV_COMMAND(CM_EDITUNDO, CmEditUndo),
	EV_COMMAND(CM_EDITCLEAR, CmEditClear),
END_RESPONSE_TABLE;

void TSimWindowsDeviceStatus::CmEditUndoEnabler(TCommandEnabler& commandHandler)
{
	commandHandler.SetText("&Undo Simulation\tCtrl+Z");
	commandHandler.Enable(environment.canundo());
}

void TSimWindowsDeviceStatus::CmEditUndo(void)
{
	TMDIClient *client_window;

	::SetCursor(TCursor(NULL,IDC_WAIT));
	environment.undo();
	client_window=dynamic_cast<TMDIClient *>(GetApplication()->GetMainWindow()->GetClientWindow());
	client_window->ForEach(UpdateValidEnvironPlot);
	::SetCursor(TCursor(NULL,IDC_ARROW));

	if (error_handler.fail()) out_error_message(TRUE);
	else Insert("Simulation Successfully Restored\r\n\r\n");
}

/******************************* class TSimWindowsEditFile ************************************

class TSimWindowsEditFile: public TPrintEditFile {
public:
	TSimWindowsEditFile(TWindow* parent = 0, int id = 0, const char far* text = 0,
						int x = 0, int y = 0, int w = 0, int h = 0,
						const char far* fileName = 0, TModule* module = 0)
		: TPrintEditFile(parent,id,text,x,y,w,h,fileName,module) {}

	virtual bool CanClear(void);
private:
	void UpdateLineNumber(void);
protected:
	void SetupWindow(void);
	DECLARE_RESPONSE_TABLE(TSimWindowsEditFile);
	void EvSetFocus(HWND hWndLostFocus)
		{ TPrintEditFile::EvSetFocus(hWndLostFocus); UpdateLineNumber(); }
	void EvKillFocus(HWND hWndGetFocus);
	void EvLButtonDown(UINT modKeys, TPoint& point)
		{ DefaultProcessing(); UpdateLineNumber(); }
	void EvLButtonUp(UINT modKeys, TPoint& point)
		{ DefaultProcessing(); UpdateLineNumber(); }
	void EvKeyDown(UINT key, UINT repeatCount, UINT flags)
		{ TPrintEditFile::EvKeyDown(key,repeatCount,flags); UpdateLineNumber(); }
	void EvChar(UINT key, UINT repeatCount, UINT flags)
		{ TPrintEditFile::EvChar(key,repeatCount,flags); UpdateLineNumber(); }
	void EvMouseMove(UINT modKeys, TPoint& point)
		{ DefaultProcessing(); if (modKeys==MK_LBUTTON) UpdateLineNumber(); }
	void CmEditClear(void)
		{ TPrintEditFile::CmEditClear(); UpdateLineNumber();}
	void CmEditCut(void)
		{ TPrintEditFile::CmEditCut(); UpdateLineNumber(); }
	void CmEditDelete(void)
		{ TPrintEditFile::CmEditDelete(); UpdateLineNumber(); }
	void CmEditPaste(void)
		{ TPrintEditFile::CmEditPaste(); UpdateLineNumber(); }
	void CmEditUndo(void)
		{ TPrintEditFile::CmEditUndo(); UpdateLineNumber(); }
	void CmEditUndoEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.SetText("&Undo Editing\tCtrl+Z"); TPrintEditFile::CmModEnable(commandHandler); }
	void CmDeviceGenerate(void);
	void CmDeviceGenerateEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(material_parameters.is_ready()); }
};

*/

bool TSimWindowsEditFile::CanClear(void)
{
	string short_path;

	if (IsModified()) {
		string msgTemplate("The device file %s has changed.\nDo you want to save changes?");
		string untitled(GetModule()->LoadString(IDS_UNTITLEDFILE));
		char*  msg = new char[MAXPATH+msgTemplate.length()];

		short_path=shorten_path(FileName);

		wsprintf(msg, msgTemplate.c_str(),
				 FileName ? (const char far*)short_path.c_str() : untitled.c_str());

		int result = MessageBox(msg, GetApplication()->GetName(), MB_YESNOCANCEL|MB_ICONQUESTION);
		delete msg;
		return result==IDYES ? Save() : result != IDCANCEL;
	}
	return TRUE;
}

void TSimWindowsEditFile::UpdateLineNumber(void)
{
	extern TTextGadget *caret_position;
	char number_string[5];

	sprintf(number_string,"%u",GetLineFromPos(GetLineIndex(-1))+1);
	caret_position->SetText(number_string);
}

void TSimWindowsEditFile::SetupWindow(void)
{
	Parent->SetCaption("DEVICE FILE");
	TPrintEditFile::SetupWindow();
	FileData.SetFilter("Device Files (*.dev)|*.dev|");
}

DEFINE_RESPONSE_TABLE1(TSimWindowsEditFile, TPrintEditFile)
	EV_WM_SETFOCUS,
	EV_WM_KILLFOCUS,
	EV_WM_LBUTTONDOWN,
	EV_WM_LBUTTONUP,
	EV_WM_KEYDOWN,
	EV_WM_CHAR,
	EV_WM_MOUSEMOVE,
	EV_COMMAND(CM_EDITCLEAR, CmEditClear),
	EV_COMMAND(CM_EDITCUT, CmEditCut),
	EV_COMMAND(CM_EDITDELETE, CmEditDelete),
	EV_COMMAND(CM_EDITPASTE, CmEditPaste),
	EV_COMMAND(CM_EDITUNDO, CmEditUndo),
	EV_COMMAND_ENABLE(CM_EDITUNDO, CmEditUndoEnabler),
	EV_COMMAND(CM_DEVICEGENERATE, CmDeviceGenerate),
	EV_COMMAND_ENABLE(CM_DEVICEGENERATE, CmDeviceGenerateEnabler),
END_RESPONSE_TABLE;

void TSimWindowsEditFile::EvKillFocus(HWND hWndGetFocus)
{
	extern TTextGadget *caret_position;

	caret_position->SetText("");
	TPrintEditFile::EvKillFocus(hWndGetFocus);
}

void TSimWindowsEditFile::CmDeviceGenerate(void)
{
	TSimWindowsDeviceStatus *new_status_window;
	TMDIChild *child_window;
	TMDIClient *client_window;

	Save();
	if(FileName) {
		if (status_window) {
			if(status_window->CanClear()) {
				delete status_window;
				status_window=(TSimWindowsDeviceStatus *)0;
			}
			else return;
		}
		client_window=dynamic_cast<TMDIClient *>(GetApplication()->GetMainWindow()->GetClientWindow());
		new_status_window=new TSimWindowsDeviceStatus();
		if(!new_status_window->Read(FileName)) {
			delete new_status_window;
			out_error_message(TRUE);
		}
		else {
			Parent->Show(SW_MINIMIZE);
			child_window=new TMDIChild(*client_window,0,new_status_window);
			child_window->SetIcon(GetApplication(),IDI_STATE);
			child_window->Create();
			new_status_window->Insert("Device successfully created\r\n\r\n");
			status_window=new_status_window;
		}
		client_window->ForEach(UpdateValidEnvironPlot);
	}
}



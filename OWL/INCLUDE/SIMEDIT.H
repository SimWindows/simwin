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
	void EvLButtonDown(UINT /*modKeys*/, TPoint& /*point*/) {}
	void EvLButtonDblClk(UINT /*modKeys*/, TPoint& /*point*/) {}
	void EvKeyDown(UINT /*key*/, UINT /*repeatCount*/, UINT /*flags*/) {}
	void CmFileSaveEnabler(TCommandEnabler& commandHandler) { commandHandler.Enable(IsModified()); }
	void CmEditUndoEnabler(TCommandEnabler& commandHandler);
	void CmEditMenuEnabler(TCommandEnabler& commandHandler) { commandHandler.Enable(FALSE); }
	void CmFileSave(void) { Save(); }
	void CmFileSaveAs(void) { SaveAs(); }
	void CmEditUndo(void);
	void CmEditClear(void) { Clear(); }
};

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
	void EvLButtonDown(UINT /*modKeys*/, TPoint& /*point*/)
		{ DefaultProcessing(); UpdateLineNumber(); }
	void EvLButtonUp(UINT /*modKeys*/, TPoint& /*point*/)
		{ DefaultProcessing(); UpdateLineNumber(); }
	void EvKeyDown(UINT key, UINT repeatCount, UINT flags)
		{ TPrintEditFile::EvKeyDown(key,repeatCount,flags); UpdateLineNumber(); }
	void EvChar(UINT key, UINT repeatCount, UINT flags)
		{ TPrintEditFile::EvChar(key,repeatCount,flags); UpdateLineNumber(); }
	void EvMouseMove(UINT modKeys, TPoint& /*point*/)
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




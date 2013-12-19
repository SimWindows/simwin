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

class TSimWindows: public TApplication {
private:
	int number_params;
	char** command_ptr;
	TControlBar *cb;
	TStatusBar *sb;
	TMDIClient *SimWindowsMDIClient;
	TDecoratedMDIFrame *SimWindowsFrame;
public:
	TSimWindows(int argc, char* argv[]);
	virtual void InitMainWindow(void);
protected:
	DECLARE_RESPONSE_TABLE(TSimWindows);
	void CmEnvironmentPreferences(void);
    void CmSimulationPreferences(void);
    void CmSavePreferences(void) { SavePreferences(); }
};

class TSimulateThread : public TThread
{
private:
	int Run(void) { Solve(); return(0); }
public:
	void Solve(void);
};

class TMacroThread : public TThread
{
private:
	int MacroNumber;
    const char *FileName;

	int Run(void) { Execute(); return(0); }
public:
	TMacroThread(void) : TThread(), MacroNumber(0), FileName(NULL) {}
	void Execute(void);
    void SetMacroNumber(int macroNumber) { MacroNumber=macroNumber; }
    void SetFileName(const char *fileName) { FileName=fileName; }
};


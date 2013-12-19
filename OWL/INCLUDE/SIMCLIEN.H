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

class TSimWindowsMDIClient: public TMDIClient {
private:
	TOpenSaveDialog::TData DeviceFileData;
	TOpenSaveDialog::TData MaterialFileData;
	int number_params;
	char** command_ptr;
	flag device_effects;

public:
	TSimWindowsMDIClient(int argc, char** argv);
	~TSimWindowsMDIClient(void);

#ifndef NDEBUG
public:
	virtual bool IdleAction(long idleCount);
#endif
protected:
	void SetupWindow(void);

private:
	void OpenFile(const char *filename);
	void CreateMultipleEnvironPlot(TValueFlag plot_flags);
	void CreateSingleEnvironPlot(TValueFlag plot_flags, string y_label, string title,
								 logical multi_colored=TRUE);
protected:
	DECLARE_RESPONSE_TABLE(TSimWindowsMDIClient);
	void EvMDIDestroy(HWND hWnd);

// Command Enablers
	void CmEditUndoEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.SetText("&Undo\tCtrl+Z"); commandHandler.Enable(FALSE); }
    void CmEnvironmentResetEnabler(TCommandEnabler& commandHandler)
    	{ commandHandler.Enable(!environment.is_solving() &&
        						(macro_storage.get_solving_macro()==NULL)); }
	void CmEnvironmentLoadMaterialEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(!environment.device()); }
	void CmMacroMenuEnabler(TCommandEnabler& commandHandler);
	void CmCreateMacroPopupEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(TRUE); }
	void CmDeviceMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device()); }
    void CmDeviceSolvingMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() && !environment.is_solving() &&
        						(macro_storage.get_solving_macro()==NULL)); }
	void CmQWMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() &&
								environment.get_number_objects(QUANTUM_WELL)); }
	void CmLaserMenuEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.device() &&
							   ((flag)environment.get_value(DEVICE,EFFECTS) & DEVICE_LASER)); }
	void CmDeviceStartEnabler(TCommandEnabler& commandHandler);
    void CmDeviceStopEnabler(TCommandEnabler& commandHandler);
	void CmDeviceExecuteMacroEnabler(TCommandEnabler& commandHandler);
	void CmExternalSpectraEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.get_number_objects(SPECTRUM)); }
	void CmPlotFreezeEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.SetText("Freeze &Plot\tCtrl+F"); commandHandler.Enable(FALSE); }
	void CmPlotSelectedEnabler(TCommandEnabler& commandHandler)
		{ commandHandler.Enable(environment.get_number_objects(SPECTRUM) ||
								environment.device()); }
	void CmPlotMacroEnabler(TCommandEnabler& commandHandler);

// Command responses
	void CmFileNew(void);
	void CmFileOpen(void)
		{ if (TFileOpenDialog(this, DeviceFileData).Execute()==IDOK) OpenFile(DeviceFileData.FileName); }
	void CmFilePrinterSetup(void);
	void CmEnvironmentReset(void);
	void CmEnvironmentTemperature(void);
	void CmEnvironmentRadius(void);
	void CmEnvironmentOptical(void);
	void CmEnvironmentLoadMaterial(void);
	void CmEnvironmentVoltageMacro(void);
	void CmEnvironmentEditMacro(void);
	void CmEnvironmentDeleteMacro(void);
	void CmDeviceReset(void);
	void CmDeviceInformation(void) { TDialogDeviceInfo(this,DG_DEVICEINFO).Execute(); }
	void CmDeviceContacts(void);
	void CmDeviceSurfaces(void);
	void CmDeviceElectricalModels(void);
	void CmDeviceThermalModels(void);
	void CmDeviceLaserParameters(void);
	void CmDeviceStart(void);
    void CmDeviceStop(void);
	void CmDeviceExecuteMacro(void);
	void CmPlotExternalSpectra(void);
	void CmPlotLatticeTemp(void);
	void CmPlotElectronTemp(void);
	void CmPlotHoleTemp(void);
	void CmPlotDoping(void);
	void CmPlotBoundCarrierConc(void);
	void CmPlotFreeCarrierConc(void);
	void CmPlotTotalCarrierConc(void);
	void CmPlotCharge(void);
	void CmPlotField(void);
	void CmPlotPotential(void);
	void CmPlotBandDiagram(void);
	void CmPlotSHR(void);
	void CmPlotBimolecular(void);
    void CmPlotAuger(void);
	void CmPlotStimulated(void);
	void CmPlotOpticalGeneration(void);
	void CmPlotTotalRecomb(void);
	void CmPlotAllRecomb(void);
	void CmPlotCurrents(void);
	void CmPlotMode(void);
	void CmPlotGain(void);
	void CmPlotPhotonDensity(void);
	void CmPlotSelected(void);
	void CmPlotMacro(void);
	void CmDataRead(void);
	void CmDataWriteDevice(void);
	void CmDataWriteMaterial(void);
	void CmDataWriteSelected(void);
	void CmDataWriteAll(void) { TDialogDataWriteAll(this,DG_WRITEALLPARAMETERS).Execute(); }
	void CmHelpAbout(void) { TDialogAbout(this,DG_ABOUT).Execute(); }
	void CmHelpCredits(void) { TDialog(this,DG_CREDITS).Execute(); }
	void CmLastUpdated(void)
		{ extern char update_string[];
		  MessageBox(update_string,"Last Updated", MB_OK | MB_ICONINFORMATION); }
};


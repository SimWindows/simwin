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

class TElement {
protected:
	TDevice *device_ptr;
	RegionType type;
	flag grid_effects;
	flag device_effects;
	TNode* prev_node;
	TNode* next_node;

public:
	TElement(RegionType region_type, TDevice *device, TNode* node_1, TNode* node_2);
	void get_effects(void);
};

class TElectricalServices {
protected:
	TDevice *device_ptr;
	TNode** grid_ptr;
	TNode* prev_node;
	TNode* next_node;
	prec length;
	prec electron_mobility_length;
	prec hole_mobility_length;
	logical elec_therm_emis_current;
	logical hole_therm_emis_current;
	prec elec_scat_length;
	prec hole_scat_length;
	int elec_transmit_max_node;
	int hole_transmit_max_node;
	ElecDriftDiffParam ec;
	ElecDriftDiffParam ev;
	ThermEmisParam ec_therm;
	ThermEmisParam ev_therm;
public:
	TElectricalServices(TDevice* device, TNode** grid, TNode* node_1, TNode* node_2);

	void comp_length(void);
	void comp_elec_mobil_length(void);
	void comp_hole_mobil_length(void);
	void comp_elec_scat_length(void);
	void comp_hole_scat_length(void);
	void comp_elec_transmit_max_node(void);
	void comp_hole_transmit_max_node(void);
	prec comp_elec_transmit_min_energy(void);
	prec comp_hole_transmit_min_energy(void);
	void comp_conduction_discont(void);
	void comp_conduction_richardson(void);
	prec comp_conduction_transmission(prec energy);
	void comp_valence_discont(void);
	void comp_valence_richardson(void);
	prec comp_valence_transmission(prec energy);
	void comp_cond_drift_diff_param(flag grid_effects);
	void comp_val_drift_diff_param(flag grid_effects);
	void comp_cond_therm_emis_param(flag grid_effects);
	void comp_val_therm_emis_param(flag grid_effects);
};



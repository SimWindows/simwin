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

class TMirror {
private:
	TDevice *device_ptr;
	MirrorType type;
	prec position;
	short node_number;
	prec reflectivity;
	float output_power;
public:
	TMirror(TDevice *ptr);
	void init(void);

	void comp_power(float photon_number, float photon_energy,
					float group_velocity, float cavity_length);

	prec get_value(flag flag_value, ScaleType scale=UNNORMALIZED);
	void put_value(flag flag_value, prec value, ScaleType scale=UNNORMALIZED);

	void read_state_file(FILE *file_ptr);
    void write_state_file(FILE *file_ptr);
};

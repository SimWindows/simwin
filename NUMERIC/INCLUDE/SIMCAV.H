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

class TCavity {
private:
	CavityType type;
	prec area;
	prec length;
	TMode mode;
	TMirror mirror_0;
	TMirror mirror_1;
public:
	TCavity(TDevice *ptr, TNode** grid);
	void init(void);

	error field_iterate(prec& iteration_error, prec initial_error, int iteration_number);
	error photon_iterate(prec& iteration_error);

	prec get_value(FlagType flag_type, flag flag_value, int object,
				   ScaleType scale=UNNORMALIZED);
	void put_value(FlagType flag_type, flag flag_value, int object, prec value,
				   ScaleType scale=UNNORMALIZED);

	void comp_value(FlagType flag_type, flag flag_value,
					int start_object=-1, int end_object=-1);

	void read_state_file(FILE *file_ptr);
	void write_state_file(FILE *file_ptr);
};



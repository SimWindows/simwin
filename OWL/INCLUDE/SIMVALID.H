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

class TScientificRangeValidator: public TFilterValidator {
protected:
	ValidatorType validator_type;
	double min;
	double max;

public:
	TScientificRangeValidator(double mininum, double maximum, ValidatorType type);

	void         Error(TWindow *owner);
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};

class TScientificUpperValidator: public TFilterValidator {
protected:
	ValidatorType validator_type;
	double max;

public:
	TScientificUpperValidator(double maximum, ValidatorType type);

	void         Error(TWindow *owner);
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};

class TScientificLowerValidator: public TFilterValidator {
protected:
	ValidatorType validator_type;
	double min;

public:
	TScientificLowerValidator(double mininum, ValidatorType type);

	void         Error(TWindow *owner);
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};

class TMustEnterValidator: public TValidator {
public:
	TMustEnterValidator(void)
		: TValidator() {}

	void         Error(TWindow *owner);
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};


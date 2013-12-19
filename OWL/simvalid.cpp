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

//************************************ class TScientificValidator ******************************
/*
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
*/
TScientificRangeValidator::TScientificRangeValidator(double minimum, double maximum,
													 ValidatorType type)
	: TFilterValidator(TCharSet("0123456789eE.+-"))
{
	min=minimum;
	max=maximum;
	validator_type=type;
}

void TScientificRangeValidator::Error(TWindow *owner)
{
	string msgTmpl;

	TApplication* app = GetApplicationObject();
	switch(validator_type) {
		case INCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALNOTINSCIINCLRANGE).c_str();
			break;
		case EXCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALNOTINSCIEXCLRANGE).c_str();
			break;
	}

	char* msg = new char[msgTmpl.length() + 10 + 10 + 1];
	sprintf(msg, msgTmpl.c_str(), min, max);
	if (owner)
		owner->MessageBox(msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK);
	else
		::MessageBox(0, msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK|MB_TASKMODAL);
	delete [] msg;
}

bool TScientificRangeValidator::IsValid(const char far* s)
{
	if (TFilterValidator::IsValid(s)) {
		double value = atof(s);
		switch(validator_type) {
			case INCLUSIVE:
				if (value >= min && value <= max) return TRUE;
				break;
			case EXCLUSIVE:
				if (value > min && value < max)	return TRUE;
				break;
		}
	}
	return FALSE;
}

UINT TScientificRangeValidator::Transfer(char far* s, void* buffer, TTransferDirection direction)
{
	if (Options & voTransfer) {
		if (direction == tdGetData) *(double*)buffer = atof(s);
		else {
			if (direction == tdSetData) wsprintf(s, "%lf", *(double*)buffer);
		}
		return sizeof(double);
	}
	else return 0;
}

//****************************** class TScientificUpperValidator *******************************
/*
class TScientificUpperValidator: public TFilterValidator {
protected:
	ValidatorType validator_type;
	double max;

public:
	TScientificUpperValidator(double maximum, ValidatorType type);

	void         Error();
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};
*/
TScientificUpperValidator::TScientificUpperValidator(double maximum, ValidatorType type)
	: TFilterValidator(TCharSet("0123456789eE.+-"))
{
	max=maximum;
	validator_type=type;
}

void TScientificUpperValidator::Error(TWindow *owner)
{
	string msgTmpl;

	TApplication* app = GetApplicationObject();
	switch(validator_type) {
		case INCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALABOVEINCLLIMIT).c_str();
			break;
		case EXCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALABOVEEXCLLIMIT).c_str();
			break;
	}
	char* msg = new char[msgTmpl.length() + 10 + 10 + 1];
	sprintf(msg, msgTmpl.c_str(), max);
	if (owner)
		owner->MessageBox(msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK);
	else
		::MessageBox(0, msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK|MB_TASKMODAL);
	delete [] msg;
}

bool TScientificUpperValidator::IsValid(const char far* s)
{
	if (TFilterValidator::IsValid(s)) {
		double value = atof(s);
		switch(validator_type) {
			case INCLUSIVE:
				if (value <= max) return TRUE;
				break;
			case EXCLUSIVE:
				if (value < max) return TRUE;
				break;
		}
	}
	return FALSE;
}

UINT TScientificUpperValidator::Transfer(char far* s, void* buffer, TTransferDirection direction)
{
	if (Options & voTransfer) {
		if (direction == tdGetData) *(double*)buffer = atof(s);
		else {
			if (direction == tdSetData) wsprintf(s, "%lf", *(double*)buffer);
		}
		return sizeof(double);
	}
	else return 0;
}

//****************************** class TScientificLowerValidator *******************************
/*
class TScientificLowerValidator: public TFilterValidator {
protected:
	ValidatorType validator_type;
	double min;

public:
	TScientificLowerValidator(double minimum, ValidatorType type);

	void         Error();
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};
*/
TScientificLowerValidator::TScientificLowerValidator(double minimum, ValidatorType type)
	: TFilterValidator(TCharSet("0123456789eE.+-"))
{
	min=minimum;
	validator_type=type;
}

void TScientificLowerValidator::Error(TWindow *owner)
{
	string msgTmpl;

	TApplication* app = GetApplicationObject();
	switch(validator_type) {
		case INCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALBELOWINCLLIMIT).c_str();
			break;
		case EXCLUSIVE:
			msgTmpl = app->LoadString(IDS_VALBELOWEXCLLIMIT).c_str();
			break;
	}
	char* msg = new char[msgTmpl.length() + 10 + 10 + 1];
	sprintf(msg, msgTmpl.c_str(), min);
	if (owner)
		owner->MessageBox(msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK);
	else
		::MessageBox(0, msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK|MB_TASKMODAL);
	delete [] msg;
}

bool TScientificLowerValidator::IsValid(const char far* s)
{
	if (TFilterValidator::IsValid(s)) {
		double value = atof(s);
		switch(validator_type) {
			case INCLUSIVE:
				if (value >= min) return TRUE;
				break;
			case EXCLUSIVE:
				if (value > min) return TRUE;
				break;
		}
	}
	return FALSE;
}

UINT TScientificLowerValidator::Transfer(char far* s, void* buffer, TTransferDirection direction)
{
	if (Options & voTransfer) {
		if (direction == tdGetData) *(double*)buffer = atof(s);
		else {
			if (direction == tdSetData) wsprintf(s, "%lf", *(double*)buffer);
		}
		return sizeof(double);
	}
	else return 0;
}

//****************************** class TMustEnterValidator ************************************
/*
class TMustEnterValidator: public TValidator {
public:
	TMustEnterValidator(void)
		: TValidator() {}

	void         Error();
	bool         IsValid(const char far* str);
	UINT         Transfer(char far* str, void* buffer, TTransferDirection direction);
};
*/

void TMustEnterValidator::Error(TWindow *owner)
{
	string msgTmpl;

	TApplication* app = GetApplicationObject();
	msgTmpl = app->LoadString(IDS_VALMUSTENTER).c_str();
	char* msg = new char[msgTmpl.length() + 10 + 10 + 1];
	sprintf(msg, msgTmpl.c_str());
	if (owner)
		owner->MessageBox(msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK);
	else
		::MessageBox(0, msg, app->GetName(), MB_ICONEXCLAMATION|MB_OK|MB_TASKMODAL);
	delete [] msg;
}

bool TMustEnterValidator::IsValid(const char far* s)
{
	string entered_string(s);

	if (TValidator::IsValid(s)) {
		if (entered_string.is_null() || entered_string.find_first_not_of(" ")==NPOS) return FALSE;
		else return(TRUE);
	}
	return FALSE;
}

UINT TMustEnterValidator::Transfer(char far* s, void* buffer, TTransferDirection direction)
{
	if (Options & voTransfer) {
		if (direction == tdGetData) *(double*)buffer = atof(s);
		else {
			if (direction == tdSetData) wsprintf(s, "%lf", *(double*)buffer);
		}
		return sizeof(double);
	}
	else return 0;
}


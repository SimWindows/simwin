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

// Windows colors.
#define RED RGB(255,000,0)
#define BLACK RGB(0,0,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define YELLOW RGB(255,175,0)
#define GRAY RGB(192,192,192)
#define WHITE RGB(255,255,255)

void UpdateValidEnvironPlot(TWindow *window, void*);
void UpdateValidMacroPlot(TWindow *window, void*);
void LoadPreferences(void);
void SavePreferences(void);


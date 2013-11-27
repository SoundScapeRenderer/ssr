/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef _utils_h_
#define _utils_h_

#include <string>
#include <vector>

class Library;

void tokenize(const std::string& str, const char* delimiters, std::vector<std::string>*);
bool fileExists( std::string filename );

void copyFile(std::string from, std::string to);

// executes a command in the native shell and returns output in string
std::string system_get_output(std::string cmd);

// like 'system', runs a command on the system shell, but also prints the command to stdout.
int systemp(std::string& cmd);

#endif
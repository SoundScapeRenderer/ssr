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

#ifndef _settings_
#define _settings_

#include <string>

namespace Settings
{
    
bool isPrefixBundled(std::string prefix);
void ignore_prefix(std::string prefix);
    
bool canOverwriteFiles();
void canOverwriteFiles(bool permission);

bool canOverwriteDir();
void canOverwriteDir(bool permission);

bool canCreateDir();
void canCreateDir(bool permission);

bool bundleLibs();
void bundleLibs(bool on);

std::string destFolder();
void destFolder(std::string path);

void addFileToFix(std::string path);
int fileToFixAmount();
std::string fileToFix(const int n);

std::string inside_lib_path();
void inside_lib_path(std::string p);

}
#endif
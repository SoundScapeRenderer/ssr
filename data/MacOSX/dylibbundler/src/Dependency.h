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


#ifndef _depend_h_
#define _depend_h_

#include <string>
#include <vector>

class Dependency
{
    // origin
    std::string filename;
    std::string prefix;
    std::vector<std::string> symlinks;
    
    // installation
    std::string new_name;
public:
    Dependency(std::string path);

    void print();

    std::string getOriginalFileName() const{ return filename; }
    std::string getOriginalPath() const{ return prefix+filename; }
    std::string getInstallPath();
    std::string getInnerPath();
        
    void addSymlink(std::string s);
    int getSymlinkAmount() const{ return symlinks.size(); }

    std::string getSymlink(const int i) const{ return symlinks[i]; }
    std::string getPrefix() const{ return prefix; }

    void copyYourself();
    void fixFileThatDependsOnMe(std::string file);
    
    // comapres the given Dependency with this one. If both refer to the same file,
    // it returns true and merges both entries into one.
    bool mergeIfSameAs(Dependency& dep2);
};


#endif
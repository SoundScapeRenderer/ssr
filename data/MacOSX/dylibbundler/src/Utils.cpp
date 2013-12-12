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


#include "Utils.h"
#include "Dependency.h"
#include "Settings.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

/*
void setInstallPath(string loc)
{
    path_to_libs_folder = loc;
}*/

void tokenize(const string& str, const char* delim, vector<string>* vectorarg)
{
    vector<string>& tokens = *vectorarg;
    
    string delimiters(delim);
    
    // skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of( delimiters , 0);
    
    // find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);
    
    while (string::npos != pos || string::npos != lastPos)
    {
        // found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        
        // skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        
        // find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    
}



bool fileExists( std::string filename )
{
    if (access( filename.c_str(), F_OK ) != -1)
    {
        return true; // file exists
    }
    else
    {
        //std::cout << "access(filename) returned -1 on filename [" << filename << "] I will try trimming." << std::endl;
        std::string delims = " \f\n\r\t\v";
        std::string rtrimmed = filename.substr(0, filename.find_last_not_of(delims) + 1);
        std::string ftrimmed = rtrimmed.substr(rtrimmed.find_first_not_of(delims));
        if (access( ftrimmed.c_str(), F_OK ) != -1)
        {
            return true;
        }
        else
        {
            //std::cout << "Still failed. Cannot find the specified file." << std::endl;
            return false;// file doesn't exist
        }
    }
}

void fixLibDependency(string old_lib_path, string new_lib_name, string target_file_name)
{

    string command = string("install_name_tool -change ") + old_lib_path + string(" ") + Settings::inside_lib_path() + new_lib_name + string(" ") + target_file_name;
    if( systemp( command ) != 0 )
    {
        cerr << "\n\nError : An error occured while trying to fix depency of " << old_lib_path << " in " << target_file_name << endl;
        exit(1);
    }
}

void copyFile(string from, string to)
{
    bool override = Settings::canOverwriteFiles();
    if(!override)
    {
        if(fileExists( to ))
        {
            cerr << "\n\nError : File " << to.c_str() << " already exists. Remove it or enable overriding." << endl;
            exit(1);
        }
    }
    
    string override_permission = string(override ? "-f " : "-n ");
        
    // copy file to local directory
    string command = string("cp ") + override_permission + from + string(" ") + to;
    if( systemp( command ) != 0 )
    {
        cerr << "\n\nError : An error occured while trying to copy file " << from << " to " << to << endl;
        exit(1);
    }
    
    // give it write permission
    string command2 = string("chmod +w ") + to;
    if( systemp( command2 ) != 0 )
    {
        cerr << "\n\nError : An error occured while trying to set write permissions on file " << to << endl;
        exit(1);
    }
}

std::string system_get_output(std::string cmd)
{
    FILE * command_output;
    char output[128];
    int amount_read = 1;
    
    std::string full_output;
    
    try
    {
        command_output = popen(cmd.c_str(), "r");
        if(command_output == NULL) throw;
        
        while(amount_read > 0)
        {
            amount_read = fread(output, 1, 127, command_output);
            if(amount_read <= 0) break;
            else
            {
                output[amount_read] = '\0';
                full_output += output;
            }
        }
    }
    catch(...)
    {
        std::cerr << "An error occured while executing command " << cmd.c_str() << std::endl;
        pclose(command_output);
        return "";
    }
    
    int return_value = pclose(command_output);
    if(return_value != 0) return "";
    
    return full_output;
}

int systemp(std::string& cmd)
{
    std::cout << "    " << cmd.c_str() << std::endl;
    return system(cmd.c_str());
}

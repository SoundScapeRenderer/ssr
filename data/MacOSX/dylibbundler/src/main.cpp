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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "Settings.h"

#include "Utils.h"
#include "DylibBundler.h"

/*
 TODO
 - what happens if a library is not remembered by full path but only name? (support improved, still not perfect)
 - could get mixed up if symlink and original are not in the same location (won't happen for UNIX prefixes like /usr/, but in random directories?)
 
 FIXME: why does it copy plugins i try to fix to the libs directory?
 
 */

const std::string VERSION = "0.4.1";


// FIXME - no memory management is done at all (anyway the program closes immediately so who cares?)

std::string installPath = "";


void showHelp()
{
    std::cout << "dylibbundler " << VERSION << std::endl;
    std::cout << "dylibbundler is a utility that helps bundle dynamic libraries inside mac OS X app bundles.\n" << std::endl;
    
    std::cout << "-x, --fix-file <file to fix (executable or app plug-in)>" << std::endl;
    std::cout << "-b, --bundle-deps" << std::endl;
    std::cout << "-d, --dest-dir <directory to send bundled libraries (relative to cwd)>" << std::endl;
    std::cout << "-p, --install-path <'inner' path of bundled libraries (usually relative to executable, by default '@executable_path/../libs/')>" << std::endl;
    std::cout << "-of, --overwrite-files (allow overwriting files in output directory)" << std::endl;
    std::cout << "-od, --overwrite-dir (totally overwrite output directory if it already exists. implies --create-dir)" << std::endl;
    std::cout << "-cd, --create-dir (creates output directory if necessary)" << std::endl;
    std::cout << "-i, --ignore <location to ignore> (will ignore libraries in this directory)" << std::endl;
    std::cout << "-h, --help" << std::endl;
}

int main (int argc, char * const argv[])
{
    
    // parse arguments    
    for(int i=0; i<argc; i++)
    {
        if(strcmp(argv[i],"-x")==0 or strcmp(argv[i],"--fix-file")==0)
        {
            i++;
            Settings::addFileToFix(argv[i]);
            continue;
        }
        else if(strcmp(argv[i],"-b")==0 or strcmp(argv[i],"--bundle-deps")==0)
        {
            Settings::bundleLibs(true);
            continue;    
        }
        else if(strcmp(argv[i],"-p")==0 or strcmp(argv[i],"--install-path")==0)
        {
            i++;
            Settings::inside_lib_path(argv[i]);
            continue;
        }
        else if(strcmp(argv[i],"-i")==0 or strcmp(argv[i],"--ignore")==0)
        {
            i++;
            Settings::ignore_prefix(argv[i]);
            continue;
        }
        else if(strcmp(argv[i],"-d")==0 or strcmp(argv[i],"--dest-dir")==0)
        {
            i++;
            Settings::destFolder(argv[i]);
            continue;
        }
        else if(strcmp(argv[i],"-of")==0 or strcmp(argv[i],"--overwrite-files")==0)
        {
            Settings::canOverwriteFiles(true);
            continue;    
        }
        else if(strcmp(argv[i],"-od")==0 or strcmp(argv[i],"--overwrite-dir")==0)
        {
            Settings::canOverwriteDir(true);
            Settings::canCreateDir(true);
            continue;    
        }
        else if(strcmp(argv[i],"-cd")==0 or strcmp(argv[i],"--create-dir")==0)
        {
            Settings::canCreateDir(true);
            continue;    
        }
        else if(strcmp(argv[i],"-h")==0 or strcmp(argv[i],"--help")==0)
        {
            showHelp();
            exit(0);    
        }
        else if(i>0)
        {
            // if we meet an unknown flag, abort
            // ignore first one cause it's usually the path to the executable
            std::cerr << "Unknown flag " << argv[i] << std::endl << std::endl;
            showHelp();
            exit(1);
        }
    }
    
    if(not Settings::bundleLibs() and Settings::fileToFixAmount()<1)
    {
        showHelp();
        exit(0);
    }
    
    std::cout << "* Collecting dependencies"; fflush(stdout);
    
    const int amount = Settings::fileToFixAmount();
    for(int n=0; n<amount; n++)
        collectDependencies(Settings::fileToFix(n));
    
    collectSubDependencies();
    doneWithDeps_go();
    
    return 0;
}

/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <string>

#ifdef _WIN32
  #include <direct.h>
  #define getcwd _getcwd
#else
  #include <unistd.h>
#endif

class FileSystemUtility
{
public:
    static std::string GetCurrentWorkingDir()
    {
        char buffer[1024];
        char* cwd = getcwd(buffer, sizeof(buffer));
        return cwd ? cwd : std::string();
    }
};

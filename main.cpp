/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <cstdlib>
#include <iostream>
#include "Socket.h"
#include "Server.h"
#include <Windows.h>
using namespace std;

int main()
{
    try
    {
	    char cwd[MAX_PATH];
	    GetCurrentDirectoryA(MAX_PATH, cwd);
	    
        auto httpdocs = string(cwd) + "\\httpdocs";

        Server server;
	    server.Start(80, httpdocs);
	    cin.get();
    }
    catch(exception& ex)
    {
        cout << ex.what() << endl;
    }

    return 0;
}

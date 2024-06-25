/**
* @file main.cpp
* @brief Command line compiler for LSL
*
* $LicenseInfo:firstyear=2024&license=viewerlgpl$
* Alchemy Viewer Source Code
* Copyright (C) 2024, Alchemy Viewer Project.
* Copyright (C) 2010, Linden Research, Inc.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation;
* version 2.1 of the License only.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*
* $/LicenseInfo$
*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <filesystem>
#include <chrono>
#include "linden_common.h"
#include "lluuid.h"
#include "llerror.h"
#include "llfile.h"
#include "lltimer.h"
#include "llstring.h"
#include "lscript_execute.h"
#include "lscript_rt_interface.h"
#include "library.h"

int main(int argc, char *argv[])
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("file", boost::program_options::value<std::string>(), "input file")
        ("debug", "Compile in Godmode");

    boost::program_options::positional_options_description pos_desc;
    pos_desc.add("file", 1);

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv)
        .options(desc)
        .positional(pos_desc)
        .run(),
        vm
    );
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if (!vm.count("file"))
    {
        std::cerr << "Error: Input file not specified" << std::endl;
        return 1;
    }

    std::string filename = vm["file"].as<std::string>();

    LLTimer timer;

    const char *error;
    LLScriptExecuteLSL2 *execute = NULL;
    LSLLibrary library(gScriptLibrary);
    library.init();

    if (filename.empty())
    {
        std::cerr << "Error: Input file not found" << std::endl;
        return 1;
    }

    LLFILE* file = LLFile::fopen(filename, "r");  /* Flawfinder: ignore */
    if(!file)
    {
        std::cerr << "Error: failed to read file" << std::endl;
        return 1;
    }
    else
    {
        execute = new LLScriptExecuteLSL2(file);
    }

    if (execute)
    {
        timer.reset();
        F32 time_slice = 3600.0f; // 1 hr.
        U32 events_processed = 0;

        do {
            LLTimer timer2;
            execute->runQuanta(vm.count("debug") > 0, LLUUID::null, &error,
                               time_slice, events_processed, timer2);
        } while (!execute->isFinished());

        F32 time = timer.getElapsedTimeF32();
        F32 ips = execute->mInstructionCount / time;
        LL_INFOS() << execute->mInstructionCount << " instructions in " << time << " seconds" << LL_ENDL;
        LL_INFOS() << ips/1000 << "K instructions per second" << LL_ENDL;
        if(vm.count("debug") > 0){
            printf("ip: 0x%X\n", get_register(execute->mBuffer, LREG_IP));
            printf("sp: 0x%X\n", get_register(execute->mBuffer, LREG_SP));
            printf("bp: 0x%X\n", get_register(execute->mBuffer, LREG_BP));
            printf("hr: 0x%X\n", get_register(execute->mBuffer, LREG_HR));
            printf("hp: 0x%X\n", get_register(execute->mBuffer, LREG_HP));
        }
    }
    else
    {
        std::cerr << "Error: Failed to load LSL" << std::endl;
        return 1;
    }

    return 0;
}

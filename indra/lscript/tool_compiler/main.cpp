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
#include "lscript_rt_interface.h"

int main(int argc, char *argv[])
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("file", boost::program_options::value<std::string>(), "input file")
        ("output", boost::program_options::value<std::string>(), "output file")
        ("error", boost::program_options::value<std::string>(), "error file")
        ("mono", "Compile to mono")
        ("class", boost::program_options::value<std::string>(), "Class ID")
        ("god", "Compile in Godmode");

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
        std::cout << desc << "\n";
        return 1;
    }

    if (!vm.count("file"))
    {
        std::cerr << "Error: Input file not specified\n";
        return 1;
    }

    LLUUID uid = LLUUID::generateNewID();

    std::string dst_filename;
    if (!vm.count("output"))
    {
        std::filesystem::path inputFilePath(vm["file"].as<std::string>());
        dst_filename = inputFilePath.stem().string() + ".lso";
    }
    else
    {
        dst_filename = vm["output"].as<std::string>();
    }

    std::string err_filename;
    if (!vm.count("error"))
    {
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        err_filename = (tempDir / uid.asString().c_str()).string();
    }
    else
    {
        err_filename = vm["error"].as<std::string>();
    }

    std::string classname;
    if (vm.count("class"))
    {
        classname = vm["class"].as<std::string>();
    }
    else
    {
        classname = uid.asString();
    }

    auto start = std::chrono::high_resolution_clock::now();
    if(!lscript_compile(vm["file"].as<std::string>().c_str(),
                        dst_filename.c_str(),
                        err_filename.c_str(),
                        vm.count("mono") > 0,
                        classname.c_str(),
                        vm.count("god") > 0))
    {
        std::cout << "Compile failed:" << std::endl;

        LLFILE* fp = LLFile::fopen(err_filename, "r");
        if(fp)
        {
            char buffer[MAX_STRING];        /*Flawfinder: ignore*/
            std::string line;
            while(!feof(fp)) 
            {
                if (fgets(buffer, MAX_STRING, fp) == NULL)
                {
                    buffer[0] = '\0';
                }

                if(feof(fp))
                {
                    break;
                }
                else
                {
                    line.assign(buffer);
                    LLStringUtil::stripNonprintable(line);
                    std::cerr << line << std::endl;
                }
            }
            fclose(fp);
        }
    }
    else
    {
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Compiled successfully in " << duration.count() << "ms" << std::endl;
    }

    //If we weren't asked to output the error file, delete it when we are done
    if (!vm.count("error"))
    {
        if (LLFile::isfile(err_filename))
        {
            LLFile::remove(err_filename);
        }
    }
    return 0;
}

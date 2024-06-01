#include <stdio.h>
#include <string.h>
#include <iostream>
#include "linden_common.h"
#include "llerror.h"
#include "llfile.h"
#include "lltimer.h"
#include "llstring.h"
#include "lscript_rt_interface.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello, World!" << std::endl;
    //LLUUID id = LLUUID::generateNewID();
    std::string filepath = "./";
    std::string filename = filepath + "fart.lsl";
    std::string dst_filename = llformat("%s.lso", filepath.c_str());
    std::string err_filename = llformat("%s.out", filepath.c_str());

    const bool compile_to_mono = FALSE;
    if(!lscript_compile(filename.c_str(),
                        dst_filename.c_str(),
                        err_filename.c_str(),
                        compile_to_mono,
                        "796b1537-70d8-497d-934e-0abcc2a60050", //id.asString().c_str(),
                        false))
    {
        std::cout << "Compile failed!" << std::endl;
        //char command[256];
        //sprintf(command, "type %s\n", err_filename.c_str());
        //system(command);

        // load the error file into the error scrolllist
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
        std::cout << "Compile worked!" << std::endl;
    }
    return 0;
}

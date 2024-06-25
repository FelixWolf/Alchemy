#ifndef TOOLEXECUTE_LIBRARY_H
#define TOOLEXECUTE_LIBRARY_H

#include "lscript_library.h"

class LSLLibrary {
private:
    LLScriptLibrary& mLibrary;

public:
    LSLLibrary(LLScriptLibrary& library)
        : mLibrary(library)
    {}
    void init();
};

#endif // TOOLEXECUTE_LIBRARY_H

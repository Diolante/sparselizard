#include "hierarchicalformfunction.h"


int hierarchicalformfunction::gettypenumber(std::string fftypename)
{
    int outtypenum = -1;
    for (int i = 0; i < mytypenames.size(); i++)
    {
        if (mytypenames[i] == fftypename)
        {
            outtypenum = i;
            break;
        }
    }
    
    if (outtypenum == -1)
    {
        std::cout << "Error in 'hierarchicalformfunction' object: unknown type name '" << fftypename << "'" << std::endl;
        abort();
    }
    
    return outtypenum;
}

std::string hierarchicalformfunction::gettypename(int fftypenumber)
{
    if (fftypenumber >= mytypenames.size())
    {
        std::cout << "Error in 'hierarchicalformfunction' object: unknown type number " << fftypenumber << std::endl;
        abort();
    }

    return mytypenames[fftypenumber];
}

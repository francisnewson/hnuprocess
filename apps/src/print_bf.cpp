#include "BFCorrection.hh"
#include <iostream>
#include <iomanip>

int main( int argc, char * argv[] )
{
    float  pos[3] = {0, 0, 500 };
    float bfld[3] = {0, 0, 0};

    std::vector<float> zs{ -3000, -2000, -1000, 0, 1000, 2000, 3000, 4000, 5000,
        6000, 7000, 8000, 9000, 10000 };

    user_blueinit_();

        std::cout 
            << std::setw(10) << "z"
            << std::setw(20) << "bx"
            << std::setw(20) << "by"
            << std::endl;

    for ( int z = -3000 ; z < 10001 ; z+=10 )
    {
        pos[2] = float(z);
        print_field_( pos, bfld );
        std::cout 
            << std::setw(10) << pos[2]
            << std::setw(20) << bfld[0] 
            << std::setw(20) << bfld[1] 
            << std::endl;
    }

}

#ifndef XCEPT_HH
#define XCEPT_HH

#include <stdexcept>

#if 0
/*
 * __   __              _   
 * \ \ / /             | |  
 *  \ V / ___ ___ _ __ | |_ 
 *   > < / __/ _ \ '_ \| __|
 *  / . \ (_|  __/ |_) | |_ 
 * /_/ \_\___\___| .__/ \__|
 *               | |        
 *               |_|        
 */
#endif

namespace fn
{
    template <typename T> 
        class Xcept : public std::runtime_error
        {
            public:
            Xcept( std::string e )
                :std::runtime_error( e ){}
        };
}
#endif

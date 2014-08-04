#ifndef XCEPT_HH
#define XCEPT_HH

#include <stdexcept>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

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

#ifndef STL_HELP_HH
#define STL_HELP_HH
#include <iosfwd>
#include <memory>
#if 0
/*
 *      _   _   _          _       
 *  ___| |_| | | |__   ___| |_ __  
 * / __| __| | | '_ \ / _ \ | '_ \ 
 * \__ \ |_| | | | | |  __/ | |_) |
 * |___/\__|_| |_| |_|\___|_| .__/ 
 *                          |_|    
 * 
 */
#endif
namespace fn
{

    /* UNIX SPECIFIC DUMMY FILE */
    //os dev/null
    std::ostream& nullos();


    //Herb Sutter's make_unique
    template<typename T, typename ...Args>
        std::unique_ptr<T> make_unique( Args&& ...args )
        {
            return std::unique_ptr<T>
                ( new T( std::forward<Args>(args)... ) );
        }
}
#endif

#ifndef STL_HELP_HH
#define STL_HELP_HH
#include <iosfwd>
#include <memory>
#include <map>
#include <set>
#include <algorithm>
#include <string>
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

    void echo_launch( int argc, char * argv[], 
            std::ostream& os, char sep = '\n' );

    void write_launch( int argc, char * argv[], 
            std::ostream& os );

    //contains wrappers for stl containers
    //http://stackoverflow.com/a/6197446/1527126

    template <typename Container>
        bool contains(Container const& c, typename Container::const_reference v) {
            return std::find(c.begin(), c.end(), v) != c.end();
        }

    template <typename Key, typename Cmp, typename Alloc>
        bool contains(std::set<Key,Cmp,Alloc> const& s, Key const& k) {
            return s.find(k) != s.end();
        }

    template <typename Key, typename Value, typename Cmp, typename Alloc>
        bool contains(std::map<Key,Value,Cmp,Alloc> const& m, Key const& k) {
            return m.find(k) != m.end();
        }

    bool contains( const std::string& s1, const std::string& s2 );

    //map at with feedback
    template <typename Container>
        typename Container::mapped_type at( Container const& c, const typename Container::key_type& key, const std::string err )
        {
            try
            {
                return c.at( key );
            }
            catch( std::out_of_range& e )
            {
                throw std::out_of_range(  e.what() + ( " " +  err )  );
            }
        }
}
#endif

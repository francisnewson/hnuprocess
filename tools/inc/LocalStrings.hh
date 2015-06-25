#ifndef LOCALSTRINGS_HH
#define LOCALSTRINGS_HH
#include <boost/filesystem/path.hpp>
#include <map>
#include <string>

#if 0
/*
 *  _                    _ ____  _        _                 
 * | |    ___   ___ __ _| / ___|| |_ _ __(_)_ __   __ _ ___ 
 * | |   / _ \ / __/ _` | \___ \| __| '__| | '_ \ / _` / __|
 * | |__| (_) | (_| (_| | |___) | |_| |  | | | | | (_| \__ \
 * |_____\___/ \___\__,_|_|____/ \__|_|  |_|_| |_|\__, |___/
 *                                                |___/     
 * 
 */
#endif

namespace boost
{
    namespace filesystem
    {

        template <> path& path::append<path::iterator>(path::iterator begin, path::iterator end, 
const codecvt_type& cvt);

        // Return path when appended to a_From will resolve to same as a_To
        boost::filesystem::path make_relative( boost::filesystem::path a_From, boost::filesystem::path a_To );

    }
} // namespace boost::filesystem





namespace fn
{
    class LocalStrings
    {
        public:
            LocalStrings( std::string file );
            boost::filesystem::path guess( boost::filesystem::path p );
            boost::filesystem::path convert( boost::filesystem::path p );
            boost::filesystem::path prefix ( boost::filesystem::path p );

        private:
            std::map<std::string, std::string> strings_;
    };

    LocalStrings& g_local_strings();

}
#endif

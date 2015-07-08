#ifndef SHUFFLER_HH
#define SHUFFLER_HH
#include <map>
#include <string>
#if 0
/*
 *  ____  _            __  __ _
 * / ___|| |__  _   _ / _|/ _| | ___ _ __
 * \___ \| '_ \| | | | |_| |_| |/ _ \ '__|
 *  ___) | | | | |_| |  _|  _| |  __/ |
 * |____/|_| |_|\__,_|_| |_| |_|\___|_|
 *
 *
 */
#endif

namespace YAML { class Node; }

namespace boost{ namespace filesystem { class path; } }

class TFile;

namespace fn
{
    class MultiScaling;

    void do_the_shuffle( const YAML::Node& output_node,
            TFile& output_file,
            std::map<std::string, MultiScaling>& scaling_info
            );
}
#endif

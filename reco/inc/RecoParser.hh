#ifndef RECOPARSER_HH
#define RECOPARSER_HH

#include "RecoFactory.hh"
#include <boost/filesystem/path.hpp>

#include "yaml-cpp/yaml.h"
#include "logging.hh"
#include <map>
#include <vector>

#if 0
/*
 *  ____                ____
 * |  _ \ ___  ___ ___ |  _ \ __ _ _ __ ___  ___ _ __
 * | |_) / _ \/ __/ _ \| |_) / _` | '__/ __|/ _ \ '__|
 * |  _ <  __/ (_| (_) |  __/ (_| | |  \__ \  __/ |
 * |_| \_\___|\___\___/|_|   \__,_|_|  |___/\___|_|
 *
 *
 */
#endif
namespace fn
{
    /*****************************************************
     * RECOPARSER
     *
     *  Takes a YAML file and builds the required 
     *  reconstruction using a RecoFactory (stored
     *  as a reference )
     *****************************************************/
    class RecoParser
    {
        public:
            RecoParser( RecoFactory& rf, logger& log );
            void parse( boost::filesystem::path config );
            void parse_output( const YAML::Node& node );

            bool check_ignore_subscriber
                ( const YAML::Node& instruct, std::string channel );

        private:
            RecoFactory& rf_;
            logger& log_;
            std::map<std::string,
                std::vector<std::string>> channel_classes_;
    };
}
#endif

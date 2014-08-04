#ifndef LOGGING_HH
#define LOGGING_HH

#define BOOST_LOG_DYN_LINK
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/expressions/predicates/is_in_range.hpp>

#include <map>
#include <string>

#if 0
/*
 *  _                   _             
 * | |                 (_)            
 * | | ___   __ _  __ _ _ _ __   __ _ 
 * | |/ _ \ / _` |/ _` | | '_ \ / _` |
 * | | (_) | (_| | (_| | | | | | (_| |
 * |_|\___/ \__, |\__, |_|_| |_|\__, |
 *           __/ | __/ |         __/ |
 *          |___/ |___/         |___/ 
 */
#endif

namespace fn
{
    enum severity_level
    {
        per_event,
        debug,
        startup,
        always_print,
        maximum
    };

    BOOST_LOG_ATTRIBUTE_KEYWORD
        (severity, "Severity", severity_level)

        typedef 
        boost::log::sources::severity_logger<severity_level> 
        logger; 

    //Map between strings and severity_level's for runtime config
    std::map<std::string, severity_level> generate_severity_map();
    std::map<std::string, severity_level>& global_severity_map();
}

#endif

#ifndef MUV_BURST_FUNCTIONS_HH
#define MUV_BURST_FUNCTIONS_HH
#if 0
/*
 *
 *  _ __ ___  _   ___   __ | |__  _   _ _ __ ___| |_
 * | '_ ` _ \| | | \ \ / / | '_ \| | | | '__/ __| __|
 * | | | | | | |_| |\ V /  | |_) | |_| | |  \__ \ |_
 * |_| |_| |_|\__,_| \_/___|_.__/ \__,_|_|  |___/\__|
 *   __               |_____|
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *
 *
 */
#endif

#include <memory>
#include "TTree.h"
#include "TList.h"
#include "TFile.h"
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include "BadBurst.hh"

namespace fn
{
    //Combine positive and negative polarities
    class CachedTree
    {
        public:
            CachedTree( TFile& tfin, std::string format_string );
            TTree& get_tree();

        private:
            std::unique_ptr<TFile> cache_file_;
            TTree * cached_tree_;
    };

    //--------------------------------------------------

    //produce one plot per run
    void muv_burst_plots( TTree * tt, boost::filesystem::path output_folder );

    //write stats for each burst to file
    void muv_burst_text( TTree * tt, boost::filesystem::path output_file );

    //write stats for a particular run and bursts to a stream
    void print_bursts( TTree& tt, int run,
            int min_burst, int max_burst, std::ostream& os );

    //find bursts with eff below threshold
    std::vector<fn::BadBurst::BurstId> 
        find_bad_bursts( TTree&  tt,  double threshold );

    //find bursts with eff statistically below threshold
    std::vector<fn::BadBurst::BurstId> 
        find_stat_bad_bursts( TTree&  tt,  double threshold, double level );
}
#endif

#ifndef K2PI_SELECTIONS_HH
#define K2PI_SELECTIONS_HH
#include <memory>
#include "Analysis.hh"
#include "Selection.hh"
#include "DchPlotter.hh"
#include "K2piInterface.hh"
#include "k2pi_fitting.hh"
#include "k2pi_reco_functions.hh"
#include "K2piEventData.hh"
#include "K2piSingleTrack.hh"
#include "logging.hh"

#if 0
/*
 *  _    ____        _              _           _   _
 * | | _|___ \ _ __ (_)    ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * | |/ / __) | '_ \| |   / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * |   < / __/| |_) | |   \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |_|\_\_____| .__/|_|___|___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *            |_|    |_____|
 *
 */
#endif

namespace fn
{
    class K2piRecoBag
    {
        public:
            K2piRecoBag( K2piEventData& event, bool mc, TFile& tfile, 
                    std::string channel, logger& slg);
            
            void add_selection( Selection * sel);
            void add_analysis( Analysis * an );
            void add_dch_selection( DchSelection * sel );
            void add_subscriber( Subscriber * sub );
            void new_event();
            void end_processing();

            std::string get_folder( std::string folder );

            bool is_mc;

            K2piEventData& event;
            K2piLkrData& raw_lkr;
            K2piLkrData& fit_lkr;
            K2piLkrData& fit_lkr_err;
            K2piDchData& raw_dch;
            TFile& tfout;

            typedef std::vector<std::unique_ptr<Analysis>>::iterator anit;
            anit an_begin(){ return an_.begin(); }
            anit an_end(){ return an_.end(); }

        private:
            std::vector<std::unique_ptr<Selection>> sel_;
            std::vector<std::unique_ptr<Analysis>>  an_;
            std::vector<std::unique_ptr<DchSelection>>  dch_slections_;
            std::vector<std::unique_ptr<Subscriber>> sub_;
            int next_id_;
            std::string channel_;
            logger& slg_;
    };

    //--------------------------------------------------

    Selection * get_photon_sep_cut( K2piRecoBag& k2pirb, double sep );
    Selection * get_track_cluster_sep_cut( K2piRecoBag& k2pirb, double sep );
    Selection * get_min_photon_radius_cut( K2piRecoBag& k2pirb, double rad );
    Selection * get_chi2_cut( K2piRecoBag& k2pirb, double chi2 );
    Selection * get_min_chi2_cut( K2piRecoBag& k2pirb, double chi2 );
    Selection * get_bad_prob_cut( K2piRecoBag& k2pirb, double prob );
    Selection * get_muv_cut( K2piRecoBag& k2pirb, bool mc );
    Selection * get_eop_cut( K2piRecoBag& k2pirb, bool mc, double E_thresh, double min, double max );

    K2piSingleTrack * get_k2pi_single_track( K2piRecoBag& k2pirb );

    void add_dch_study( K2piRecoBag & k2pirb);

    void add_pion_study( K2piRecoBag & k2pirb);

    void add_pull_study( K2piRecoBag & k2pirb );


}
#endif

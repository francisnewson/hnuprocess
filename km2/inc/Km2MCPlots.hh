#ifndef KM2MCPLOTS_HH
#define KM2MCPLOTS_HH
#include "Analysis.hh"
#include "FourMomComp.hh"
#include <boost/optional.hpp>

#if 0
/*
 *  _  __          ____  __  __  ____ ____  _       _
 * | |/ /_ __ ___ |___ \|  \/  |/ ___|  _ \| | ___ | |_ ___
 * | ' /| '_ ` _ \  __) | |\/| | |   | |_) | |/ _ \| __/ __|
 * | . \| | | | | |/ __/| |  | | |___|  __/| | (_) | |_\__ \
 * |_|\_\_| |_| |_|_____|_|  |_|\____|_|   |_|\___/ \__|___/
 *
 *
 */
#endif

namespace fne
{
    class Event;
    class Mc;
    class McParticle;
}

namespace fn
{
    class Km2Event;

    class Km2MCPlots : public Analysis
    {
        public:
            Km2MCPlots( 
                    const Selection& sel, 
                    const Km2Event& raw_km2e, 
                    const Km2Event& corr_km2e, 
                    const fne::Event * e,
                    TFile& tfile,
                    std::string folder
                    );

            void end_processing();

        private:
            void process_event();

            const Km2Event& raw_km2e_;
            const Km2Event& corr_km2e_;
            const fne::Event * e_;

            TFile& tfile_;
            boost::filesystem::path folder_;

            FourMomComp raw_mc_;
            FourMomComp corr_mc_;
            FourMomComp corr_raw_;

            FourMomComp muon_raw_mc_;
            FourMomComp muon_corr_mc_;
            FourMomComp muon_corr_raw_;

            TH1D * h_dz_raw_mc_;
            TH1D * h_dz_corr_mc_;
            TH1D * h_dz_corr_raw_;

            TH1D * h_dcda_raw_mc_;
            TH1D * h_dcda_corr_mc_;
            TH1D * h_dcda_corr_raw_;

            TH1D * h_cda_raw_;
            TH1D * h_cda_corr_;
            TH1D * h_cda_mc_;

            HistStore hs_;

            REG_DEC_SUB( Km2MCPlots );
    };

    template<>
        Subscriber * create_subscriber<Km2MCPlots>
        (YAML::Node& instruct, RecoFactory& rf );

    struct mc_km2_event
    {
        const fne::McParticle * kaon;
        const fne::McParticle * muon;
    };

    boost::optional<mc_km2_event> get_mc_km2( const fne::Mc& mc );
}
#endif


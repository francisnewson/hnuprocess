#ifndef CDACOMP_HH
#define CDACOMP_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "KaonTrack.hh"
#include "SingleTrack.hh"
#if 0
/*
 *   ____    _        ____
 *  / ___|__| | __ _ / ___|___  _ __ ___  _ __
 * | |   / _` |/ _` | |   / _ \| '_ ` _ \| '_ \
 * | |__| (_| | (_| | |__| (_) | | | | | | |_) |
 *  \____\__,_|\__,_|\____\___/|_| |_| |_| .__/
 *                                       |_|
 *
 */
#endif
namespace fn
{
    struct CdaPlotter
    {
        std::string name_;
        TH1D * h_cda_;
        TH1D * h_dxdz_;
        TH1D * h_xoff_;
        const KaonTrack* kt_;
    };


    class CdaComp : public Analysis
    {
        public:
            CdaComp( const Selection& sel, const SingleTrack& st,
                    const fne::Event * e, bool mc,
                    std::map<std::string, const KaonTrack*> kaon_tracks,
                    TFile& tf, std::string folder );


            void end_processing();

        private:
            void process_event();

            const SingleTrack& st_;
            const fne::Event * e_;
            bool mc_;
            std::vector<CdaPlotter>  kts_;
            HistStore hs_;
            TH1D * h_mcdxdz_;
            TH1D * h_mccda_;
            TH1D * h_mcxoff_;

            TFile& tf_;
            std::string folder_;

            REG_DEC_SUB( CdaComp );
    };

    template<>
        Subscriber * create_subscriber<CdaComp>
        ( YAML::Node& instruct, RecoFactory& rf );
}
#endif

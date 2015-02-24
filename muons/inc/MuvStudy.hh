#ifndef MUVSTUDY_HH
#define MUVSTUDY_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "TEfficiency.h"
#include <memory>
#if 0
/*
 *  __  __              ____  _             _
 * |  \/  |_   ___   __/ ___|| |_ _   _  __| |_   _
 * | |\/| | | | \ \ / /\___ \| __| | | |/ _` | | | |
 * | |  | | |_| |\ V /  ___) | |_| |_| | (_| | |_| |
 * |_|  |_|\__,_| \_/  |____/ \__|\__,_|\__,_|\__, |
 *                                            |___/
 *
 */
#endif

class TFile;
class TGraph;

namespace fne
{
    class Event;
    class McParticle;
    class Mc;
    class Detector;
    class RecoMuon;
}

namespace fn
{
    class SingleTrack;

    /****************************************************
     * MUV STUDY                                        *
     *                                                  *
     * Analysis class to look at MUV behaviour to help  *
     * simulate efficiency                              *
     ****************************************************/

    class MuvStudy : public Analysis
    {
        public:
            MuvStudy( const Selection& sel, const fne::Event * e,
                    const SingleTrack& st, TFile& tfile, std::string folder);

            void end_processing();

        private:
            void process_event();

            const fne::Event* e_;
            const SingleTrack& st_;

            TFile& tfile_;
            std::string folder_;

            TH1D * h_det_nmuon_;
            TH1D * h_mc_nmuon_;

            TEfficiency early_;
            TEfficiency mid_;
            TEfficiency late_;

            std::vector<std::unique_ptr<TGraph>> multi_muons_;

            HistStore hs_;

            REG_DEC_SUB(MuvStudy);
    };

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvStudy>
        (YAML::Node& instruct, RecoFactory& rf );

    const fne::McParticle * get_mc_muon( const fne::Mc& mc );

    void find_muv12( const fne::Detector& det, 
            std::vector<const fne::RecoMuon*>& res );
}
#endif

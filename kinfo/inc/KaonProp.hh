#ifndef KAONPROP_HH
#define KAONPROP_HH
#include "Analysis.hh"
#include <memory>
#include <boost/optional.hpp>
#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "Event.hh"
#include "PolarityFinder.hh"

#if 0
/*
 *  _  __                 ____
 * | |/ /__ _  ___  _ __ |  _ \ _ __ ___  _ __
 * | ' // _` |/ _ \| '_ \| |_) | '__/ _ \| '_ \
 * | . \ (_| | (_) | | | |  __/| | | (_) | |_) |
 * |_|\_\__,_|\___/|_| |_|_|   |_|  \___/| .__/
 *                                       |_|
 *
 */
#endif
namespace fn
{
    class RecoFactory;

    struct kaon_burst
    {
        Long64_t run;
        Long64_t burst_time;
        Long64_t count;
        Double_t dxdz;
        Double_t dydz;
        Double_t xoff;
        Double_t yoff;
        Double_t pmag;
        Double_t alpha;
        Double_t beta;
        Int_t pol;
    };


    void fill_pos_burst( const fne::Event * e , boost::optional<PolarityFinder>& pf,
            kaon_burst& kb );

    void fill_neg_burst( const fne::Event * e , boost::optional<PolarityFinder>& pf,
            kaon_burst& kb );

    bool operator==( const kaon_burst& lhs, const kaon_burst& rhs );

    struct mag_burst
    {
        Long64_t run;
        Long64_t burst_time;
        Double_t poskick;
        Double_t negkick;
    };

    //--------------------------------------------------

    class KaonProp : public Analysis
    {
        public:
            KaonProp( const Selection& sel, 
                    const fne::Event * e, 
                    TFile& tf,
                    std::string folder );

            void new_burst();
            void end_burst();
            void end_processing();

            void set_pos_sel( Selection* sel );
            void set_neg_sel( Selection* sel );

        private:
            void process_event();
            const fne::Event * e_;

            TFile& tf_;
            std::string folder_;

            kaon_burst kplus_burst_;
            kaon_burst kminus_burst_;
            mag_burst mpn33_burst_;

            kaon_burst test_kplus_burst_;
            kaon_burst test_kminus_burst_;

            TH1D hposkick_;
            TH1D hnegkick_;

            boost::optional<Selection*> selpos_;
            boost::optional<Selection*> selneg_;
            boost::optional<PolarityFinder*> pol_finder_;

            std::unique_ptr<TTree> kplus_tree_;
            std::unique_ptr<TTree> kminus_tree_;
            std::unique_ptr<TTree> mpn33_tree_;

            REG_DEC_SUB( KaonProp );
    };

    template<>
        Subscriber * create_subscriber<KaonProp>
        ( YAML::Node& instruct, RecoFactory& rf );
}
#endif

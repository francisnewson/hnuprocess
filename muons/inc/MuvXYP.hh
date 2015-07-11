#ifndef MUVXYP_HH
#define MUVXYP_HH
#if 0
/*
 *  __  __            __  ____   ______
 * |  \/  |_   ___   _\ \/ /\ \ / /  _ \
 * | |\/| | | | \ \ / /\  /  \ V /| |_) |
 * | |  | | |_| |\ V / /  \   | | |  __/
 * |_|  |_|\__,_| \_/ /_/\_\  |_| |_|
 *
 *
 */
#endif

#include <memory>
#include <vector>
#include "Rtypes.h"
#include "yaml_help.hh"
#include "AreaCut.hh"
#include "TH3D.h"

class TH1D;
class TH2D;
class TH3D;
class TEfficiency;

namespace fn
{
    namespace muveff
    {

        class XYPProcess
        {
            public:
                virtual void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p ){};
        };
        //--------------------------------------------------

        class ReweightXY : public XYPProcess
        {
            public:
                ReweightXY( const TH2D& hw )
                    :hw_( hw ){}

                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );

            private:
                const TH2D& hw_;
        };
        //--------------------------------------------------

        class ReweightP : public XYPProcess
        {
            public:
                ReweightP( const TH1D& hp )
                    :hp_( hp ){}

                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );

            private:
                const TH1D& hp_;
        };
        //--------------------------------------------------

        class CutXY : public XYPProcess
        {
            public:
                CutXY( const YAML::Node& params );
                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );


            private:
                AreaCut cut_;
        };

        //--------------------------------------------------

        class CutP : public XYPProcess
        {
            public:
                CutP( double min_p, double max_p);

                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );


            private:
                double min_p_;
                double max_p_;
        };

        class MuvAcc : public XYPProcess
        {
            public:
                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );

        };

        //--------------------------------------------------

        class CompProcess : public XYPProcess 
        {
            public:
                CompProcess( std::vector<XYPProcess*> procs );

                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );

            private:
                std::vector<XYPProcess*> procs_;
        };

        //--------------------------------------------------

        template <class P>
            void process_hist( TH3D & h, P& process )
            {
                Int_t nBins  = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

                for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin )
                {
                    Int_t xBin, yBin, pBin;
                    h.GetBinXYZ( gBin, xBin, yBin, pBin );

                    double x = h.GetXaxis()->GetBinCenter( xBin );
                    double y = h.GetYaxis()->GetBinCenter( yBin );
                    double p = h.GetZaxis()->GetBinCenter( pBin );

                    process( h, gBin, xBin, yBin, pBin,  x, y, p );
                }
            }

        //--------------------------------------------------

        std::unique_ptr<TH2D> xy_eff( TH3D& hpassed, TH3D& htotal );
        std::unique_ptr<TEfficiency> p_eff( TH3D& hpassed, TH3D& htotal );
        std::unique_ptr<TH1D> get_ratio( TEfficiency& teff );

        //--------------------------------------------------

        class RadialHist : public XYPProcess
        {
            public:
                RadialHist( std::string name, std::string title);

                void operator()( TH3D& h, Int_t gBin,
                        Int_t xBin, Int_t yBin, Int_t pBin,
                        double x, double y, double p );

                TH1D& hist(){ return h_; }

            private:
                TH1D h_;
        };

        std::unique_ptr<TEfficiency> r_eff( TH3D& hpassed, TH3D& htotal );

    }

}

#endif

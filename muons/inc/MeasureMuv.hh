#ifndef MEASUREMUV_HH
#define MEASUREMUV_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  __  __                               __  __
 * |  \/  | ___  __ _ ___ _   _ _ __ ___|  \/  |_   ___   __
 * | |\/| |/ _ \/ _` / __| | | | '__/ _ \ |\/| | | | \ \ / /
 * | |  | |  __/ (_| \__ \ |_| | | |  __/ |  | | |_| |\ V /
 * |_|  |_|\___|\__,_|___/\__,_|_|  \___|_|  |_|\__,_| \_/
 *
 *
 */
#endif
class Tfile;

namespace fn
{
    class SingleTrack;

    class MeasureMuv : public Analysis
    {
        public:
            MeasureMuv( const Selection& base, const Selection& muv, 
                    const SingleTrack& st,
                    TFile& tfile, std::string folder
                    );

            void end_processing();

        private:
            void process_event();
            const Selection& muv_;
            const SingleTrack& st_;

            TH2D * h_xy_passed_;
            TH2D * h_xy_total_;

            HistStore hs_;


            TFile& tfile_;
            std::string folder_;

            REG_DEC_SUB( MeasureMuv);
    };

    template<>
        Subscriber * create_subscriber<MeasureMuv>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif

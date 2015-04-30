#ifndef MUONSCATTERSIM_HH
#define MUONSCATTERSIM_HH
#include "ToyMC.hh"
#include <utility>
#if 0
/*
 *  __  __                   ____            _   _            ____  _
 * |  \/  |_   _  ___  _ __ / ___|  ___ __ _| |_| |_ ___ _ __/ ___|(_)_ __ ___
 * | |\/| | | | |/ _ \| '_ \\___ \ / __/ _` | __| __/ _ \ '__\___ \| | '_ ` _ \
 * | |  | | |_| | (_) | | | |___) | (_| (_| | |_| ||  __/ |   ___) | | | | | | |
 * |_|  |_|\__,_|\___/|_| |_|____/ \___\__,_|\__|\__\___|_|  |____/|_|_| |_| |_|
 *
 *
 */
#endif
namespace fn
{

    class MuonScatterSim 
    {
        public:
            MuonScatterSim();
            typedef toymc::track_params track_params;
            std::pair<double,double> transfer( track_params start_params ) const;
            void set_magnet_polarity( int polarity );

        private:
            void build_simulation();
            toymc::SmartRNG<std::mt19937> gen_;

            toymc::ToyMCLibrary library_;

            std::vector<toymc::ToyMC*> us_toys_;
            std::vector<toymc::ToyMC*> ds_toys_;
            std::vector<toymc::ToyMC*> m2_toys_;

            toymc::ToyMCComposite downstream_sim_;
            toymc::ToyMCComposite muv1_to_muv2_sim_;
            toymc::ToyMCDipoleBend * bend_in_mag_;
    };
}
#endif

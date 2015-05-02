#include "MuonScatterSim.hh"
#include "NA62Constants.hh"
#include "stl_help.hh"
#include <cassert>
#include <iostream>
#include <iomanip>

namespace nc = na62const;

namespace fn
{
    MuonScatterSim::MuonScatterSim()
    {
        build_simulation();
    }

    void MuonScatterSim::build_simulation()
    {
        using namespace toymc;

        //**************************************************
        //nc::zX referes to FRONT face of detector X
        //need to handle detector lengths appropriately
        //
        //   len_X                          len_Y
        //  <------>                       <---->
        //  |++++++|                       |++++|
        //  ^       <--------------------->^
        //  zX         (zY - zX - len_X)   zY 
        //
        //**************************************************

        //materials
        double dch_budget = 4e-3; //rad lengths
        double dch_depth = 5; //cm (guess)
        double dch_rad_length = dch_depth / dch_budget;

        //drift lengths
        double drift_dch1_to_dch2 = nc::zDch2 - nc::zDch1;
        double drift_dch2_to_mag = nc::zMagnet - nc::zDch2;
        double drift_mag_to_dch3 = nc::zDch3 - nc::zMagnet;
        double drift_dch3_to_dch4 = nc::zDch4 - nc::zDch3;
        double drift_dch4_to_al = nc::zAl - nc::zDch4;
        double drift_al_to_lkr = nc::zLkr - nc::zAl;
        double drift_lkr_to_hac = nc::zHac - nc::zLkr;
        double drift_hac_to_muv1 = nc::zMuv1 - nc::zHac;
        double drift_muv1_to_muv2 = nc::zMuv2 - nc::zMuv1;

        double drift_to_dch1 = nc::zDch1;

        //----------------------Upstream detectors (before LKr)----------------------------

        ToyMC * propagate_to_dch1 = library_.add_toy( 
                make_unique<ToyMCPropagate> ( drift_to_dch1 - dch_depth ) );
        us_toys_.push_back(propagate_to_dch1);

        ToyMC *  scatter_in_dch1 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, dch_rad_length, dch_depth, 1 ) );
        us_toys_.push_back(scatter_in_dch1);

        ToyMC * scatter_dch1_to_dch2 = library_.add_toy( 
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_helium, drift_dch1_to_dch2 - dch_depth, 10 ) );
        us_toys_.push_back(scatter_dch1_to_dch2);

        ToyMC *  scatter_in_dch2 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, dch_rad_length, dch_depth, 1 ) );
        us_toys_.push_back(scatter_in_dch2);

        ToyMC *  scatter_dch2_to_mag = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_helium, drift_dch2_to_mag, 1 ) );
        us_toys_.push_back(scatter_dch2_to_mag);

        //save magnet for future retrieval
        bend_in_mag_ = static_cast<ToyMCDipoleBend*>( library_.add_toy(
                    make_unique<ToyMCDipoleBend> ( nc::mpn33_kick, +1 ) ) );
        us_toys_.push_back(bend_in_mag_);

        ToyMC * scatter_mag_to_dch3 = library_.add_toy( 
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_helium, drift_mag_to_dch3 - dch_depth , 10 ) );
        us_toys_.push_back(scatter_mag_to_dch3);

        ToyMC *  scatter_in_dch3 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, dch_rad_length, dch_depth, 1 ) );
        us_toys_.push_back(scatter_in_dch3);

        ToyMC * scatter_dch3_to_dch4 = library_.add_toy( 
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_helium, drift_dch3_to_dch4 - dch_depth, 10 ) );
        us_toys_.push_back(scatter_dch3_to_dch4);

        ToyMC *  scatter_in_dch4 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, dch_rad_length, dch_depth, 1 ) );
        us_toys_.push_back(scatter_in_dch4);

        ToyMC * scatter_to_al_window = library_.add_toy( 
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_helium, drift_dch4_to_al, 10 ) );
        us_toys_.push_back(scatter_to_al_window);

        ToyMC * scatter_al_window_to_lkr = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_air, drift_al_to_lkr - nc::len_lkr, 1 ) );
        us_toys_.push_back(scatter_al_window_to_lkr);

        //----------------------Downstream detector (LKr onwards)----------------------------

        ToyMC * scatter_in_lkr = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_lkr, nc::len_lkr , 10) );
        ds_toys_.push_back(scatter_in_lkr);

        ToyMC * scatter_lkr_to_hac = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_air, drift_lkr_to_hac - nc::len_hac, 1 ) );
        ds_toys_.push_back(scatter_lkr_to_hac);

        ToyMC * scatter_in_hac = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_iron, nc::len_hac , 10) );
        ds_toys_.push_back(scatter_in_hac);

        ToyMC * scatter_hac_to_muv1 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_air, drift_hac_to_muv1 - nc::len_muv_iron, 1 ) );
        ds_toys_.push_back(scatter_hac_to_muv1);

        ToyMC * scatter_in_muv1 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_iron, nc::len_muv_iron , 10) );
        ds_toys_.push_back(scatter_in_muv1);

        downstream_sim_ = ToyMCComposite( ds_toys_ );

        //----------------------Scatter from MUV1 to MUV2----------------------------

        ToyMC * scatter_muv1_to_muv2 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_air, drift_muv1_to_muv2 - nc::len_muv_iron, 10) );
        m2_toys_.push_back(scatter_muv1_to_muv2);

        ToyMC * scatter_in_muv2 = library_.add_toy(
                make_unique<ToyMCThickScatter> ( gen_, nc::X0_iron, nc::len_muv_iron , 10) );
        m2_toys_.push_back(scatter_in_muv2);

        muv1_to_muv2_sim_ = ToyMCComposite( m2_toys_ );
    }

    void MuonScatterSim::set_magnet_polarity( int polarity)
    {
        bend_in_mag_->set_magnet_polarity( polarity );
    }

    std::pair<double,double> MuonScatterSim::transfer( track_params start_params ) const
    {
        using namespace toymc;

        //work out where to start
        double sim_z = 0;
        double start_z = start_params.z;

        auto this_toy = begin(us_toys_);

        //Work through the MC toys
        while( true )
        {
            sim_z += (*this_toy)->get_length();

            if ( sim_z  > start_z ) 
            { 
                //this_toy contains start_z
                break;
            }

            ++this_toy;
        }

        //copy this toy and modify length
        std::unique_ptr<ToyMC> start_element{(*this_toy)->clone()};
        start_element->set_length( sim_z - start_z );

        //move on
        ++this_toy;


        //Create upstream composite
        std::vector<ToyMC*> upstream_toys{ start_element.get() };
        std::copy( this_toy, end(us_toys_), std::back_inserter( upstream_toys ) ) ;

        ToyMCComposite upstream_sim{ upstream_toys };

        track_params lkr_params = upstream_sim.transfer( start_params );
        track_params muv1_params = downstream_sim_.transfer( lkr_params );
        track_params muv2_params = muv1_to_muv2_sim_.transfer( muv1_params );

        //Debugging output if broken
        if ( fabs( muv2_params.z - na62const::zMuv2 ) > 1 )
        {
            double cz = start_z;

            std::cout << "upstream toys: " << std::endl;
            for ( auto& mct : upstream_toys )
            {
                cz += mct->get_length();
                std::cout << std::setw(10) << mct->get_length() << " " << std::setw(10) <<  cz << std::endl;
            }

            cz = 0;

            std::cout << "us toys: " << std::endl;
            for ( auto& mct : us_toys_ )
            {
                cz += mct->get_length();
                std::cout << std::setw(10) << mct->get_length() << " " << std::setw(10) << cz << std::endl;
            }

        track_params test_lkr_params = upstream_sim.noisy_transfer( start_params );
        }

        assert( fabs(muv2_params.z - na62const::zMuv2 ) < 1 );

        return std::make_pair( muv2_params.x, muv1_params.y );
    }
}

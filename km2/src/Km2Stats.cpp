#include "Km2Stats.hh"
#include <iostream>
#include <iomanip>
#include "TF1.h"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( Km2Stats);

    Km2Stats::Km2Stats( const Selection& sel,  std::string channel,
            TFile& tfile, std::string folder,
            std::ostream& os,
            const Km2Event& km2_event)
        :Analysis( sel),channel_( channel),  tfile_(tfile),folder_( folder ),
        os_( os ), km2_event_( km2_event )
    {
        h_m2miss_ = hs_.MakeTH1D( "h_m2miss", "K_{#mu2} missing mass",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                "#events" ); 
    }

    void Km2Stats::process_event()
    {
        //extract mass and weight
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();
        double m2 = km2re.get_m2miss();
        double wgt = get_weight();

        //Fill histogram
        h_m2miss_->Fill( m2, wgt );

        //Add to running stats
        rs_.Push( m2, wgt );
    }

    void Km2Stats::end_processing()
    {
        //Compute fit
        h_m2miss_->Sumw2();
        h_m2miss_->Fit("gaus");
        TF1 * gaussian = h_m2miss_->GetFunction( "gaus" );

        size_t prec = os_.precision( 4 );

        os_
            << std::setw(30) << "#Channel"
            << std::setw(15) << "TotalWeight"
            << std::setw(15) << "SampleMean"
            << std::setw(15) << "SampelSigma"
            << std::setw(15) << "FitMean"
            << std::setw(15) << "FitSigma"
            << std::endl;

        os_
            << std::setw(30) << channel_
            << std::setw(15) << rs_.TotalWeight()
            << std::setw(15) << rs_.Mean()
            << std::setw(15) << rs_.StandardDeviation()
            << std::setw(15) << gaussian->GetParameter(1)
            << std::setw(15) << gaussian->GetParameter(2)
            << std::endl;

        os_.precision( prec );

        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2Stats>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string channel = rf.get_channel();

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            std::string ostream_name =
                get_yaml<std::string>( instruct, "ostream" ) ;
            std::ostream& os = rf.get_ostream( ostream_name);

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new Km2Stats( *sel, channel, tfile, folder, os, *km2_event);
        }
}

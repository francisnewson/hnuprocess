#include "AKLComp.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"
#include "RecoAKLhit.hh"
#include <bitset>
#include <iomanip>


namespace fn
{
    REG_DEF_SUB( AKLComp );

    void AKLStatus::reset()
    {
        for ( auto& pocket : pockets_ )
        {
            pocket.first = false;
            pocket.second = false;
        }
    }

    void AKLStatus::add_hit( int ipocket, int layer )
    {
        auto& pocket = pockets_[ipocket];
        if ( layer == 1 )
        {
            pocket.first = true;
        }
        else if ( layer == 2 )
        {
            pocket.second = true;
        }
        else
        {
            throw std::out_of_range
                ( "Unknown AKLPocket layer: " 
                  + std::to_string( layer ) );
        }
    }

    bool AKLStatus::pocket_or( int ipocket )
    {
        auto& pocket = pockets_[ipocket];
        return ( pocket.first || pocket.second) ;
    }

    bool AKLStatus::pocket_and( int ipocket )
    {
        auto& pocket = pockets_[ipocket];
        return ( pocket.first && pocket.second) ;
    }

    bool AKLStatus::any_pocket_or()
    {
        for ( auto pocket : pockets_ )
        {
            if ( pocket.first || pocket.second )
            { return true;}
        }
        return false;
    }

    bool AKLStatus::any_pocket_and()
    {
        for ( auto pocket : pockets_ )
        {
            if ( pocket.first && pocket.second )
            { return true;}
        }
        return false;
    }

    AKLComp::AKLComp ( const Selection& sel,
            const fne::Event * e,
            TFile& tfile, std::string folder,
            std::ostream& os )
        :Analysis( sel ), e_( e ), os_( os ),
        tfile_( tfile), folder_( folder)
    {

    }


    void AKLComp::process_event()
    {
        //Process AKL Hits
        int naklhits = e_->detector.naklhits;
        auto& aklhits = e_->detector.aklhits;

        akl_status_.reset();

        for ( int iakl = 0 ; iakl != naklhits ; ++iakl )
        {
            fne::RecoAKLhit * rakl =
                static_cast<fne::RecoAKLhit*> ( aklhits[iakl] );

            BOOST_LOG_SEV( get_log(), log_level() )
                << "AKL POCKET LAYER " << rakl->layer;

            akl_status_.add_hit( rakl->pocket , rakl->layer );
        }

        bool my_AKL_OR = akl_status_.any_pocket_or();
        bool my_AKLor_AND = akl_status_.any_pocket_and();

        //Compare with PU bits

        //AKL is pattern unit 2, channel 7 -> pu.channel[15]
        //Bit 7 is AKL-OR, Bit 15 is AKLor-AND
        //Check time slices 5 and 6

        const fne::PatternUnit& pu = e_->pattern_unit;
        const fne::PatternUnitChannel& akl_channel = pu.channel[15];

        for ( unsigned int chan = 0 ; chan != 16 ; ++ chan )
        { os_ << "    CHANNEL: " << std::setw( 6) << chan << " " ; }

        os_ << "\n";

        for ( unsigned int tslice = 0; tslice != 10 ; ++ tslice )
        {
            for ( unsigned int chan = 0 ; chan != 16 ; ++ chan )
            {
                std::bitset<16> bits
                { static_cast< unsigned long long >( 
                        e_->pattern_unit.channel[ chan ].tslice[tslice] )};

                os_ << std::setw( 20 ) << bits.to_string();
            }
        os_ << "\n";

        }

        os_ << "\n";

        const int& akl_word_t5 = akl_channel.tslice[5];
        std::bitset<16> akl_bitset_t5
        { static_cast< unsigned long long >( akl_word_t5 )};

        const int& akl_word_t6 = akl_channel.tslice[6];
        std::bitset<16> akl_bitset_t6
        { static_cast< unsigned long long >( akl_word_t6 )};

        bool pu_AKL_OR  = akl_bitset_t5[ 7] || akl_bitset_t6[ 7];
        bool pu_AKLor_AND = akl_bitset_t5[15] || akl_bitset_t6[15];

        if ( pu_AKL_OR || pu_AKLor_AND )
        {

            os_ 
                << std::setw(4) <<  my_AKL_OR 
                << std::setw(4) << pu_AKL_OR 
                << std::setw(4) << "|";

            os_
                << std::setw(4) <<  my_AKLor_AND 
                << std::setw(4) << pu_AKLor_AND 
                << std::endl;
        }
    }

    void AKLComp::new_run()
    {
        os_ << e_->header.run << std::endl;
    }

    template<>
        Subscriber * create_subscriber<AKLComp>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * event = rf.get_event_ptr();

            std::string ostream_name =
                get_yaml<std::string>( instruct, "ostream" ) ;

            std::ostream& os = rf.get_ostream( ostream_name) ;

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_yaml<std::string>( instruct, "folder") ;

            return new AKLComp( *sel, event, tfile, folder,  os );
        }
}

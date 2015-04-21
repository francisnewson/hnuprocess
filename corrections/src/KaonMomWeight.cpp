#include "KaonMomWeight.hh"
#include <boost/filesystem/fstream.hpp>
#include "StreamFilter.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"
#include "Xcept.hh"

namespace fn
{
    bool in_range( int val, irange range )
    {
        return ( val >= range.first )
            && ( val <= range.second  );
    }

    std::istream& operator>>(std::istream& is, irange& ir)
    { return is >> ir.first >> ir.second ; }

    bool in_range( double val, erange range )
    {
        return ( val > range.first )
            && ( val < range.second  );
    }

    std::istream& operator>>(std::istream& is, erange& er)
    { return is >> er.first >> er.second ; }

    std::istream& operator>>
        (std::istream& is, crude_weight_record& cwr)
        { return is >> cwr.run_range >> cwr.slope; }

    std::istream& operator>>
        (std::istream& is, fine_weight_record& fwr)
        { return is >> fwr.run_range >> fwr.mom_range
            >> fwr.slope >> fwr.central_mom ; }

    //--------------------------------------------------

            REG_DEF_SUB( KaonMomWeight);

    KaonMomWeight::KaonMomWeight( 
            const fne::Event * e,
            boost::filesystem::path crude_weights,
            boost::filesystem::path fine_weights,
            bool mc
            )
        :e_( e ), mc_( mc )
    {
        //load crude weights
        boost::filesystem::ifstream cwifs( crude_weights);
        if ( ! cwifs.good() )
        { throw fn::Xcept<MissingInfo>{ crude_weights.string() }; }
        StreamFilter cwifs_remove_comments( cwifs, '#' );

        crude_weights_.insert( end( crude_weights_),
                std::istream_iterator<crude_weight_record>( cwifs),
                std::istream_iterator<crude_weight_record>() );

        //load fine weights
        boost::filesystem::ifstream fwifs( fine_weights);
        if ( ! fwifs.good() )
        { throw fn::Xcept<MissingInfo>{ fine_weights.string() }; }
        StreamFilter fwifs_remove_comments( fwifs, '#' );

        fine_weights_.insert( end( fine_weights_),
                std::istream_iterator<fine_weight_record>( fwifs),
                std::istream_iterator<fine_weight_record>() );
    }

    double KaonMomWeight::get_weight() const
    {
        if ( ! mc_ ) { return 1.0; }

        double result = 1;
        int nrun = e_->header.run;
        double mom = e_->mc.decay.kaon_momentum.P();

        for ( auto cw : crude_weights_ )
        {
            if ( in_range( nrun, cw.run_range ) )
            {
                //do crude weighting
                result *= ( 1.0 + cw.slope * std::pow( mom - 74.0, 2 ) );
            }
        }

        for ( auto fw : fine_weights_ )
        {
            if ( in_range( nrun, fw.run_range )
                    && in_range( mom, fw.mom_range ) )
            {
                //do fine weighting
                result *= ( 1.0 + fw.slope * ( mom + fw.central_mom ) );
            }
        }
        return result;
    }

    template<>
        Subscriber * create_subscriber<KaonMomWeight>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            using boost::filesystem::path;
            path crude_weights { get_yaml<std::string>( instruct, "crude_weights") };
            path fine_weights { get_yaml<std::string>( instruct, "fine_weights") };
            const fne::Event * e = rf.get_event_ptr();
            bool mc = rf.is_mc();

            return new KaonMomWeight( e, crude_weights, fine_weights, mc );
        }
}

#include "k3pi_selections.hh"
#include "K3piReco.hh"
#include "yaml_help.hh"
namespace fn
{
    REG_DEF_SUB( FoundK3pi );

    FoundK3pi::FoundK3pi ( const K3piReco& k3pi_reco )
        :k3pi_reco_( k3pi_reco )
    {}

    bool FoundK3pi::do_check() const
    {
        return k3pi_reco_.reconstructed_k3pi();
    }

    template<>
        Subscriber * create_subscriber<FoundK3pi>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            return new FoundK3pi( *k3pi_reco );
        }

    //--------------------------------------------------

    REG_DEF_SUB( K3piCharge );

    K3piCharge::K3piCharge ( const K3piReco& k3pi_reco, int charge )
        :k3pi_reco_( k3pi_reco), charge_( charge ){}

    bool K3piCharge::do_check() const
    {
        auto & k3pi_reco_event = k3pi_reco_.get_k3pi_reco_event();
        return k3pi_reco_event.get_charge() == charge_;
    }

    template<>
        Subscriber * create_subscriber<K3piCharge>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            int charge = get_yaml<int>( instruct, "charge" );
            return new K3piCharge( *k3pi_reco, charge );
        }

    //--------------------------------------------------

    REG_DEF_SUB( K3piTrackSep );

    K3piTrackSep::K3piTrackSep ( const K3piReco& k3pi_reco, double min_track_sep )
        :k3pi_reco_( k3pi_reco), min_track_sep_( min_track_sep ){}

    bool K3piTrackSep::do_check() const
    {
        auto & k3pi_reco_event = k3pi_reco_.get_k3pi_reco_event();
        return k3pi_reco_event.get_min_dch1_sep() > min_track_sep_;
    }

    template<>
        Subscriber * create_subscriber<K3piTrackSep>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            double min_track_sep = get_yaml<double>( instruct, "min_track_sep" );
            return new K3piTrackSep( *k3pi_reco, min_track_sep );
        }

    //--------------------------------------------------

    REG_DEF_SUB( K3piDY );

    K3piDY::K3piDY ( const K3piReco& k3pi_reco, double max_dy )
        :k3pi_reco_( k3pi_reco), max_dy_( max_dy ){}

    bool K3piDY::do_check() const
    {
        auto & k3pi_reco_event = k3pi_reco_.get_k3pi_reco_event();
        return k3pi_reco_event.get_max_dy_dch4_sep() < max_dy_;
    }

    template<>
        Subscriber * create_subscriber<K3piDY>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            double max_dy = get_yaml<double>( instruct, "max_dy" );
            return new K3piDY( *k3pi_reco, max_dy );
        }

    //--------------------------------------------------

    REG_DEF_SUB( K3piChi2 );

    K3piChi2::K3piChi2 ( const K3piReco& k3pi_reco, double max_chi2 )
        :k3pi_reco_( k3pi_reco), max_chi2_( max_chi2 ){}

    bool K3piChi2::do_check() const
    {
        auto & k3pi_reco_event = k3pi_reco_.get_k3pi_reco_event();
        return k3pi_reco_event.get_chi2() < max_chi2_;
    }

    template<>
        Subscriber * create_subscriber<K3piChi2>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            double max_chi2 = get_yaml<double>( instruct, "max_chi2" );
            return new K3piChi2( *k3pi_reco, max_chi2 );
        }

    //--------------------------------------------------

    REG_DEF_SUB( K3piPT );

    K3piPT::K3piPT ( const K3piReco& k3pi_reco, double max_pt )
        :k3pi_reco_( k3pi_reco), max_pt_( max_pt ){}

    bool K3piPT::do_check() const
    {
        auto & k3pi_reco_event = k3pi_reco_.get_k3pi_reco_event();
        return k3pi_reco_event.get_pt() < max_pt_;
    }

    template<>
        Subscriber * create_subscriber<K3piPT>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );
            double max_pt = get_yaml<double>( instruct, "max_pt" );
            return new K3piPT( *k3pi_reco, max_pt );
        }
}

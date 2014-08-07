#include "cluster_selections.hh"
#include "Xcept.hh"

namespace fn
{
    K2piClusters * get_k2pi_clusters
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yk2pic = instruct["inputs"]["k2pic"];

            if ( !yk2pic )
            {throw Xcept<MissingNode>( "k2pic" );}

            K2piClusters * k2pic = dynamic_cast<K2piClusters*>
                ( rf.get_subscriber( yk2pic.as<std::string>() ));

            if ( !k2pic )
            { throw Xcept<BadCast>( "K2PIC" ); }

            return k2pic;

        }

    //--------------------------------------------------

    REG_DEF_SUB( FoundK2piClusters);

    FoundK2piClusters::FoundK2piClusters( const K2piClusters& k2pic )
        :k2pic_( k2pic ) {}

    bool FoundK2piClusters::do_check() const
    {
        return k2pic_.found_clusters();
    }

    template<>
        Subscriber * create_subscriber<FoundK2piClusters>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K2piClusters * k2pic = get_k2pi_clusters( instruct, rf );
            return new FoundK2piClusters{ *k2pic };
        }

    //--------------------------------------------------
}

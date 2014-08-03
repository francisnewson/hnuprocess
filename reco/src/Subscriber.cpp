#include "Subscriber.hh"
#include "RecoFactory.hh"

namespace fn
{
    void Subscriber::set_name( std::string name )
    { name_ = name; }

    std::string Subscriber::get_name()
    { return name_; }

    void Subscriber::set_id( int id )
    { id_ = id; }

    int Subscriber::get_id()
    { return id_; }

    //--------------------------------------------------


}

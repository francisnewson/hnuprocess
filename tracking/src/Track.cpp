#include "Track.hh"
#include <stdexcept>

namespace fn
{
    Track::Track( TVector3 point, 
            TVector3 direction)
        :point_( point ), direction_( direction )
    {}

    Track::Track( double px, double py, double pz,
            double vx, double vy, double vz )
        :point_( px, py, pz ),
        direction_( vx, vy, vz ) 
    {}

    TVector3 Track::get_point() const
    { return point_; }

    TVector3 Track::get_direction() const
    { return direction_; }

    TVector3 Track::extrapolate( double z) const
    {
        double lambda = (z - point_.Z()) / direction_.Z();
        return point_ + direction_ * lambda;
    }

    //--------------------------------------------------

    Vertex compute_cda
        ( const Track& a, const Track& b )
        {

            TVector3 r = a.get_point();
            TVector3 v = a.get_direction();

            TVector3 s = b.get_point();
            TVector3 w = b.get_direction();

            //Vector perpendicular to both
            TVector3 n = v.Cross(w);
            TVector3 nhat = n.Unit();

            //Check tracks aren't paralell
            if ( ( n.Mag()  / v.Mag() / w.Mag() ) < 1e-20 )
            {
                throw std::domain_error( "Parallel tracks vertex requested" );
            }

            //Vector joining tracks
            TVector3 j = r - s;

            Vertex result;

            //Distance of closet approach
            result.cda =  nhat * j;

            double lambda = (s - r) * ( nhat.Cross(w) ) / ( v * (nhat.Cross(w)));

            result.point =  r + lambda * v - nhat * (result.cda / 2) ;

            return result;
        }

}

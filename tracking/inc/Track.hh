#ifndef TRACK_HH
#define TRACK_HH
#include "TVector3.h"
#include <utility>
#if 0
/*
 *  _____               _
 * |_   _| __ __ _  ___| | __
 *   | || '__/ _` |/ __| |/ /
 *   | || | | (_| | (__|   <
 *   |_||_|  \__,_|\___|_|\_\
 *
 *
 */
#endif
namespace fn
{
    class Track
    {
        public:
            Track(){}

            Track( TVector3 point, 
                    TVector3 direction);

            Track( double px, double py, double pz,
                    double vx, double vy, double vz );

            TVector3 get_point() const;
            TVector3 get_direction() const;
            TVector3 extrapolate( double z) const;

        private:
            TVector3 point_;
            TVector3 direction_;
    };

    //--------------------------------------------------
    

    struct Vertex
    {
        TVector3 point;
        double cda;
    };

    Vertex compute_cda
        ( const Track& a, const Track& b );

    std::ostream& operator<<( std::ostream& os, const Track& t );
}
#endif

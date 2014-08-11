#include "AreaCut.hh"

namespace fn
{
    AreaCut::AreaCut( std::vector<polygon_type>& areas )
        :allowed_areas_( areas ){}

    void AreaCut::set_allowed_areas
        ( std::vector<polygon_type>& areas )
        {
            allowed_areas_ = areas;
        }

    bool AreaCut::allowed( point_type p )
    {
        for ( auto& area : allowed_areas_ )
        {
            if ( boost::geometry::within( p, area ) )
            { return true; }
        }

        return false;
    }

    //--------------------------------------------------

    RectanglesCut::RectanglesCut( std::vector<rectangle> rectangles )
    {
        std::vector<polygon_type> allowed_areas;

        for ( auto& rec : rectangles )
        {
            //Convert rectangle to a polygon
            polygon_type new_area;

            new_area.outer().push_back
                ( point_type{ rec.minx, rec.miny} );

            new_area.outer().push_back
                ( point_type{ rec.minx, rec.maxy} );

            new_area.outer().push_back
                ( point_type{ rec.maxx, rec.maxy} );

            new_area.outer().push_back
                ( point_type{ rec.maxx, rec.miny} );

            new_area.outer().push_back
                ( point_type{ rec.minx, rec.miny} );

            allowed_areas.push_back( new_area );
        }

        area_cut_.set_allowed_areas( allowed_areas );
    }

    bool RectanglesCut::allowed( point_type p )
    {
        return area_cut_.allowed( p );
    }

    //--------------------------------------------------
}


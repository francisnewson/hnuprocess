#include "AreaCut.hh"

namespace fn
{
}

namespace YAML
{
    Node convert< fn::rectangle>::encode
        ( const fn::rectangle& rhs )
        {
            Node node;

            node.push_back( rhs.minx );
            node.push_back( rhs.maxx );
            node.push_back( rhs.miny );
            node.push_back( rhs.maxy );

            return node;
        }

    bool convert< fn::rectangle>::decode
        ( const Node& node , 
          fn::rectangle & rhs )
        {
            rhs.minx =  node[0].as<double>()  ;
            rhs.maxx =  node[1].as<double>()  ;
            rhs.miny =  node[2].as<double>()  ;
            rhs.maxy =  node[3].as<double>()  ;
            return true;
        }

    //--------------------------------------------------
    Node convert<fn::point_type>::encode 
        ( const fn::point_type& rhs )
        {
            Node node;
            node.push_back( rhs.x() );
            node.push_back( rhs.y() );
            return node;
        }

    bool convert<fn::point_type>::decode ( const Node& node , 
            fn::point_type & rhs )
    {
        rhs.x( node[0].as<double>() );
        rhs.y( node[1].as<double>() );
        return true;
    }

    //--------------------------------------------------

    Node convert<fn::polygon_type>::encode ( const fn::polygon_type& rhs )
    {
        Node node;
        std::vector<fn::point_type> point_list( begin( rhs.outer() ), end( rhs.outer() ) );
        node = point_list;
        return node;
    }

    bool convert<fn::polygon_type>::decode ( const Node& node , 
            fn::polygon_type & rhs )
    {
        std::vector<fn::point_type> points = node.as<std::vector<fn::point_type>>();
        rhs.outer() = 
            fn::polygon_type::ring_type( points.begin(), points.end() );

        return true;
    }
    //--------------------------------------------------
}

namespace fn
{
    polygon_type rectangle2polygon( rectangle rec )
    {
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

        return  new_area;
    }

    //--------------------------------------------------

    AreaCut::AreaCut( std::vector<polygon_type>& areas )
        :allowed_areas_( areas ){}

    AreaCut::AreaCut( std::vector<rectangle> rectangles )
    {
        for ( auto& rec : rectangles )
        {
            //Convert rectangle to a polygon
            polygon_type new_area = rectangle2polygon( rec );
            allowed_areas_.push_back( new_area );
        }
    }

    void AreaCut::set_allowed_areas
        ( std::vector<polygon_type>& areas )
        {
            allowed_areas_ = areas;
        }

    bool AreaCut::allowed( point_type p ) const
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
            polygon_type new_area = rectangle2polygon( rec );
            allowed_areas.push_back( new_area );
        }

        area_cut_.set_allowed_areas( allowed_areas );
    }

    bool RectanglesCut::allowed( point_type p ) const
    {
        return area_cut_.allowed( p );
    }

    //--------------------------------------------------
}


#ifndef AREACUT_HH
#define AREACUT_HH

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <yaml-cpp/yaml.h>

#if 0
/*
 *     _                    ____      _   
 *    / \   _ __ ___  __ _ / ___|   _| |_ 
 *   / _ \ | '__/ _ \/ _` | |  | | | | __|
 *  / ___ \| | |  __/ (_| | |__| |_| | |_ 
 * /_/   \_\_|  \___|\__,_|\____\__,_|\__|
 *                                        
 * 
 */
#endif
namespace fn
{
    typedef boost::geometry::model::d2::point_xy<double> 
        point_type;

    typedef boost::geometry::model::polygon<point_type>
        polygon_type;


    //--------------------------------------------------

    struct rectangle
    {
        double minx;
        double maxx;
        double miny;
        double maxy;
    };
}

namespace YAML
{
    template<>
        struct convert < fn::rectangle >
        {
            static Node encode ( const fn::rectangle& rhs );

            static bool decode ( const Node& node , 
                    fn::rectangle & rhs );
        };

    template<>
        struct convert < fn::point_type >
        {
        static Node encode ( const fn::point_type& rhs );
            static bool decode ( const Node& node , 
                    fn::point_type & rhs );
        };

    template<>
        struct convert < fn::polygon_type >
        {
        static Node encode ( const fn::polygon_type& rhs );
            static bool decode ( const Node& node , 
                    fn::polygon_type & rhs );
        };
}

namespace fn
{

    polygon_type rectangle2polygon( rectangle rec  );

    class AreaCut
    {
        public:
            AreaCut(){};
            AreaCut( std::vector<polygon_type>& areas );
            AreaCut( std::vector<rectangle> rectangles );

            void set_allowed_areas( std::vector<polygon_type>& areas );

            bool allowed( point_type p ) const;
        private:
            std::vector<polygon_type> allowed_areas_;
    };

    //--------------------------------------------------

    class RectanglesCut
    {
        public:
            RectanglesCut( std::vector<rectangle> rectangles );
            bool allowed( point_type p ) const;

        private:
            AreaCut area_cut_;
    };

    //--------------------------------------------------
}
#endif

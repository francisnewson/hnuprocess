#ifndef AREACUT_HH
#define AREACUT_HH

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

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

    class AreaCut
    {
        public:
            AreaCut(){};
            AreaCut( std::vector<polygon_type>& areas );
            void set_allowed_areas( std::vector<polygon_type>& areas );

            bool allowed( point_type p );
        private:
            std::vector<polygon_type> allowed_areas_;
    };

    //--------------------------------------------------

    struct rectangle
    {
        double minx;
        double maxx;
        double miny;
        double maxy;
    };

    class RectanglesCut
    {
        public:
            RectanglesCut( std::vector<rectangle> rectangles );
            bool allowed( point_type p );

        private:
            AreaCut area_cut_;
    };

    //--------------------------------------------------
}
#endif

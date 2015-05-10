#ifndef EFFS2D_HH
#define EFFS2D_HH
#include <vector>
#include <iosfwd>
#include "TH2D.h"
#if 0
/*
 *  _____  __  __     ____  ____
 * | ____|/ _|/ _|___|___ \|  _ \
 * |  _| | |_| |_/ __| __) | | | |
 * | |___|  _|  _\__ \/ __/| |_| |
 * |_____|_| |_| |___/_____|____/
 *
 *
 */
#endif
namespace fn
{
    //**************************************************
    //Eff2D
    //Handle efficiencies on a 2D grid
    //**************************************************

    class Effs2D
    {
        public:
            Effs2D() = default;
            Effs2D( std::vector<double> xedges, 
                    std::vector<double> yedges, 
                    std::vector<double> effs );

            double efficiency( double x, double y ) const;
            void print_eff_map( std::ostream& os ) const;

        private:
            std::size_t get_bin( double val, 
                    const std::vector<double>& edges ) const;

            std::vector<double> xedges_;
            std::vector<double> yedges_;
            std::vector<double> effs_;
    };

    //--------------------------------------------------

    class EffsTH2D
    {
        public:
            EffsTH2D() = default;
            EffsTH2D( const TH2D &h );
            void set_eff_hist( TH2D h );

            double efficiency( double x, double y ) const;


        private:
            TH2D heffs_;
    };
}
#endif

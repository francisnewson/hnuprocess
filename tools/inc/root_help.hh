#ifndef ROOT_HELP_HH
#define ROOT_HELP_HH
#include <boost/filesystem/path.hpp>
#include "TH1D.h"
#include "TH1.h"
#include "TFile.h"
#include <map>
#include "TVector3.h"
#include "TLorentzVector.h"
#include <iosfwd>

namespace fn
{
    //COLORS

    class RootColors
    {
        public:
        RootColors();
        int operator()( std::string name ) const;

        private:
        std::map<std::string, int> colors_;
    };

    int root_color( std::string name );

    //--------------------------------------------------

    //STRING MUNGING

    const char * root_file_string( boost::filesystem::path p );


    void add_file_protocol( std::string& file_string );


    //--------------------------------------------------

    //Histogram extensions
    double integral ( const TH1D& a, double min, double max );

    double flux_ratio( const TH1D& a, const TH1D& b ,
            double centre, double half_width );

    template < class T = TH1>
        std::unique_ptr<T> extract_hist
        (  TFile& tf , boost::filesystem::path p )
        {
            T * h = 0;
            tf.GetObject( root_file_string( p ) , h );
            if ( !h )
            { 
                throw std::runtime_error( "Could not find " + p.string() );
            }
            auto result = std::unique_ptr<T>( 
                    static_cast<T*>( h->Clone() ) );
            result->SetDirectory(0);
            return result;
        }

    void mkdir_p( TFile& f, boost::filesystem::path p );

    //--------------------------------------------------

    //OSTREAM
    std::ostream& operator<<( std::ostream& os, const TVector3& tv );
  
    std::ostream& operator<<
    ( std::ostream& os, const TLorentzVector &tv );
}
#endif

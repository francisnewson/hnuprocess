#include "root_help.hh"
#include "Rtypes.h"
#include <iostream>
#include <iomanip>


namespace fn
{
    //COLORS
    RootColors::RootColors()
        :colors_{ 
            {"kWhite", kWhite}, {"kBlack", kBlack},
                {"kGray", kGray}, {"kRed", kRed},
                {"kGreen", kGreen}, {"kBlue", kBlue},
                {"kYellow", kYellow}, {"kMagenta", kMagenta},
                {"kCyan", kCyan}, {"kOrange", kOrange},
                {"kSpring", kSpring}, {"kTeal", kTeal},
                {"kAzure", kAzure}, {"kViolet", kViolet},
                {"kPink", kPink}
        } {}

    int RootColors::operator()( std::string name ) const
    { 
        int kcolor = 0;
        try {kcolor = colors_.at( name); } 
        catch ( std::out_of_range& e )
        { throw std::out_of_range( "Unkown ROOT color " + name );}
        return kcolor;
    }

    int root_color( std::string name )
    {
        static const RootColors rc;
        return rc( name );
    }

    //--------------------------------------------------

    //STRING MUNGING

    const char * root_file_string( boost::filesystem::path p )
    {
        return p.string().c_str();
    }

    void add_file_protocol( std::string& file_string )
    {
        //Assumes file could be one of castor of eos
        if ( file_string.find( "castor") != std::string::npos )
        {
            file_string =  "root://castorpublic.cern.ch//" + file_string;
        }
        if ( file_string.find( "eos") != std::string::npos )
        {
            file_string = "root://eosna62//" + file_string;
        }
    }

    //--------------------------------------------------

    double integral ( const TH1D& a, double min, double max )
    {
        double bmin = a.GetXaxis()->FindBin( min );
        double bmax = a.GetXaxis()->FindBin( max );
        return a.Integral( bmin, bmax );

    }

    double flux_ratio( const TH1D& a, const TH1D& b ,
            double centre, double half_width )
    {
        double min = centre - half_width;
        double max = centre + half_width;

        return integral( a, min, max ) / integral( b, min , max );
    }

    void mkdir_p( TFile& f, boost::filesystem::path p )
    {
        const char* fname = p.string().c_str();
        auto dir = static_cast<TDirectory*>
            ( f.Get(fname) );

        if ( !dir )
        {( f.mkdir(fname) );}
    }

    //--------------------------------------------------

    std::ostream& operator<<( std::ostream& os, const TVector3 &tv )
    {
        auto prec = os.precision( 5 );
        os << "[" 
            << std::setw(9) << tv.X() << ", " 
            << std::setw(9) << tv.Y()  << ", "  
            << std::setw(9) << tv.Z()
            << "]";

        os.precision( prec );
        return os;
    }


    std::ostream& operator<<
        ( std::ostream& os, const TLorentzVector &tv )
        {
            auto prec = os.precision( 5 );
            os << "[" 
                << std::setw(9) << tv.X() << ", " 
                << std::setw(9) << tv.Y()  << ", "  
                << std::setw(9) << tv.Z() << ", "
                << std::setw(9) << tv.T() 
                << "]";
            os.precision( prec );
            return os;
        }

}

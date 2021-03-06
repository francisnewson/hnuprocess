#include "root_help.hh"
#include "Rtypes.h"
#include <iostream>
#include <iomanip>
#include "stl_help.hh"


namespace fn
{
    void de_zero_hist( TH1& h )
    {
        int nBins = h.GetNbinsX();

        for ( int ibin = 0 ; ibin != nBins +1 ; ++ ibin )
        {
            double value = h.GetBinContent( ibin );
            if ( value < 0 ){ h.SetBinContent( ibin, 0 );
            }
        }
    }

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
        //Assumes file could be one of castor or eos
        if ( file_string.find( "castor") != std::string::npos )
        {
            file_string =  "root://castorpublic.cern.ch/" + file_string+"?svcClass=na62";
        }
        if ( file_string.find( "eos") != std::string::npos )
        {
            file_string = "root://eosna62.cern.ch//" + file_string;
        }
    }

    //--------------------------------------------------

    double integral ( const TH1D& a, double min, double max )
    {
        double bmin = a.GetXaxis()->FindBin( min );
        double bmax = a.GetXaxis()->FindBin( max );

        //have we chosen the right bins?

        //min check
        double min_low = a.GetBinLowEdge( bmin );
        double min_up = a.GetBinLowEdge( bmin + 1 );

        if ( (min_up - min) < (min - min_low ) )
        {
            ++bmin;
        }

        double min_dist_prev_low = fabs( min - a.GetBinLowEdge( bmin - 1 ) );
        double min_dist_low = fabs( min - a.GetBinLowEdge( bmin ) );
        double min_dist_up  = fabs( min - a.GetBinLowEdge( bmin + 1 ) );

        assert( (min_dist_low < min_dist_prev_low) && ( min_dist_low < min_dist_up ) );

        //max check
        double max_low = a.GetBinLowEdge( bmax );
        double max_up  = a.GetBinLowEdge( bmax + 1 );

        if ( ( max - max_low ) < ( max_up - max ) )
        {
            --bmax;
        }

        double max_dist_low = fabs( max - a.GetBinLowEdge( bmax ) );
        double max_dist_up  = fabs( max - a.GetBinLowEdge( bmax + 1 ) );
        double max_next_up  = fabs( max - a.GetBinLowEdge( bmax + 2 ) );

        assert( ( max_dist_up < max_dist_low ) && ( max_dist_up < max_next_up ) );

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

    std::unique_ptr<TFile> get_tfile
        ( boost::filesystem::path filename, Option_t* option )
        {
            auto result = make_unique<TFile>( filename.string().c_str(), option);
            if ( result->IsZombie() )
            {
                throw std::runtime_error( filename.string() + " IsZombie()!" );
            }
            if ( ! result->IsOpen() )
            {
                throw std::runtime_error( filename.string() + "failed IsOpen()!" );
            }
            return result;
        }

    //--------------------------------------------------

    void store_value( std::string name, double value )
    {
        TVectorD val(1);
        val[0] =  value;
        val.Write( name.c_str() );
    }

    double retrieve_value( TFile& tf, boost::filesystem::path p )
    {
        const TVectorD * vd = get_object<TVectorD>( tf, p );
        return (*vd)[0];
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

    //--------------------------------------------------

    double median(const TH1D * h1) { 

        int n = h1->GetXaxis()->GetNbins();  
        std::vector<double>  x(n);
        h1->GetXaxis()->GetCenter( &x[0] );
        const double * y = h1->GetArray(); 
        // exclude underflow/overflows from bin content array y
        return TMath::Median(n, &x[0], &y[1]); 
    }

    //--------------------------------------------------

    SaveDirectory::SaveDirectory()
        :td( gDirectory ), tf( gFile ){
        }

    SaveDirectory::~SaveDirectory()
    {
        gDirectory = td;
        gFile = tf;
    }

}

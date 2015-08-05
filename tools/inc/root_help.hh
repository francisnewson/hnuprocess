#ifndef ROOT_HELP_HH
#define ROOT_HELP_HH
#include <boost/filesystem/path.hpp>
#include "TH1D.h"
#include "TH1.h"
#include "TFile.h"
#include <map>
#include "TVector3.h"
#include "TVectorD.h"
#include "TLorentzVector.h"
#include <iosfwd>
#include <memory>
#include "stl_help.hh"
#include "OwningStack.hh"

namespace fn
{
    template <class T> T * tclone( const T& h ){ return static_cast<T*>( h.Clone() ); }
    template <class T> T * tclone( const T& h, std::string s )
    { return static_cast<T*>( h.Clone(s.c_str()) ); }

    template <class T> std::unique_ptr<T> uclone( const std::unique_ptr<T>& u )
    { return std::unique_ptr<T>( static_cast<T*>( u->Clone() ) ); }

    template <class IT>
        //Assumes container of TH1D*
        std::unique_ptr<TH1D> sum_hists( IT begin, IT end )
        {
            std::unique_ptr<TH1D> result{ static_cast<TH1D*>( (*begin)->Clone() ) };
            result->Reset();
            result->SetDirectory(0);

            for( auto current = begin; current != end  ; ++current )
            {
                result->Add( &*(*current) );
            }

            return result;
        }
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

    //SUM STACKS
    //Assumes container of THStack*
    template <class IT>
    std::unique_ptr<OwningStack> sum_stacks( IT begin, IT end )
    {
        auto res =  make_unique<OwningStack>();

        std::vector<THStack*> stacks;
        std::vector<TList*> lists;

        for ( auto hs = begin ; hs !=end ; ++hs )
        {
            lists.push_back( (*hs)->GetHists() );
        }

        int nchan = lists[0]->GetSize();

        for ( int ichan = 0 ; ichan != nchan ; ++ ichan )
        {
            for( auto& list: lists )
            {
                auto h = std::unique_ptr<TH1>{
                    static_cast<TH1*>( list->At( ichan )->Clone() ) };
                res->Add( std::move(h) );
            }
        }

        return res;
    }

    //--------------------------------------------------

    //STRING MUNGING

    const char * root_file_string( boost::filesystem::path p );


    void add_file_protocol( std::string& file_string );


    //--------------------------------------------------

    //Histogram extensions
    double integral ( const TH1D& a, double min, double max );

    double flux_ratio( const TH1D& a, const TH1D& b ,
            double centre, double half_width );

    template <typename T>
        T * get_object( TFile& tf, boost::filesystem::path name )
        {
            T * h = 0;
            tf.GetObject( name.string().c_str(), h );
            if( !h )
            {
                throw std::runtime_error
                    ( "Can't find " + name.string() + " in " + tf.GetName() );
            }
            return h;
        }

    template < class T = TH1>
        std::unique_ptr<T> extract_hist
        (  TFile& tf , boost::filesystem::path p )
        {
            T * h = 0;
            tf.GetObject( root_file_string( p ) , h );
            if ( !h )
            { 
                throw std::runtime_error
                    ( "Could not find " + p.string() + " in "  + tf.GetName() );
            }
            auto result = std::unique_ptr<T>( 
                    static_cast<T*>( h->Clone() ) );
            result->SetDirectory(0);
            return result;
        }

    void mkdir_p( TFile& f, boost::filesystem::path p );

    //--------------------------------------------------

    std::unique_ptr<TFile> get_tfile
        ( boost::filesystem::path filename, Option_t* option="" );

    //--------------------------------------------------

    template <class T>
    std::vector<std::unique_ptr<T>> extract_object_list
    ( TFile& tf , std::vector<std::string> list )
    {
        std::vector<std::unique_ptr<T>> res;

        for( auto s : list )
        {
            T * o = get_object<T>( tf, s );
            res.push_back( std::unique_ptr<T>( o ) );
        }

        return res;
    }

    template <class T>
    std::vector<std::unique_ptr<T>> extract_hist_list
    ( TFile& tf , std::vector<std::string> list )
    {
        std::vector<std::unique_ptr<T>> res;

        for( auto s : list )
        {
            auto h = extract_hist<T>( tf, s );
            res.push_back( std::move( h ) );
        }

        return res;
    }

    //STORE VALUES
    void store_value( std::string name, double value );
    double retrieve_value( TFile& tf, boost::filesystem::path p );

    //OSTREAM
    std::ostream& operator<<( std::ostream& os, const TVector3& tv );

    std::ostream& operator<<
        ( std::ostream& os, const TLorentzVector &tv );
}
#endif

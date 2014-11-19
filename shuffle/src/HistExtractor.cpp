#include "HistExtractor.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "Rtypes.h"
#include "HistStore.hh"
#include <stdexcept>

namespace fn
{
    RootTFileWrapper::RootTFileWrapper( TFile& fin )
        :fin_ ( fin ) 
    {}

    std::unique_ptr<TH1> RootTFileWrapper::get_hist ( boost::filesystem::path p )
    {

        TH1 * h ;
        fin_.GetObject( p.string().c_str(), h );
        if ( !h )
        {
            throw std::runtime_error( "Can't find " + p.string() );
        }
        h->SetDirectory(0);
        return std::unique_ptr<TH1>( h );
    }

    ChannelHistExtractor::ChannelHistExtractor( FNTFile& fntf )
        :fntf_( fntf ), rebin_( 1 )
    {}

    void ChannelHistExtractor::set_rebin( int rebin )
    { rebin_ = rebin; }

    void ChannelHistExtractor::set_pre_path( boost::filesystem::path p )
    { pre_ = p; }

    void ChannelHistExtractor::set_post_path( boost::filesystem::path p )
    { post_ = p; }

    std::unique_ptr<TH1> ChannelHistExtractor::get_hist ( boost::filesystem::path p )
    {
        auto result =  fntf_.get_hist( pre_/ p / post_ );
        result->Rebin(rebin_);
        return result;
    }

    double integral( TH1& h, double min, double max )
    {
        double bmin = h.GetXaxis()->FindBin( min );
        double bmax = h.GetXaxis()->FindBin( max );
        return h.Integral( bmin, bmax );
    }
}

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

    void ChannelHistExtractor::reset_collapse()
    { col_x_ = false; col_y_ = false; col_range_ = false; }

    void ChannelHistExtractor::set_collapse_x( bool col_x)
    { col_x_ = col_x; }

    void ChannelHistExtractor::set_collapse_y( bool col_y)
    { col_y_ = col_y;}

    void ChannelHistExtractor::set_collapse_x( bool col_x, double min, double max)
    { col_x_ = col_x; col_range_ = true; min_val_ = min ; max_val_ = max; }

    void ChannelHistExtractor::set_collapse_y( bool col_y, double min, double max)
    { col_y_ = col_y; col_range_ = true; min_val_ = min ; max_val_ = max; }

    std::unique_ptr<TH1> ChannelHistExtractor::get_hist ( boost::filesystem::path p )
    {
        std::unique_ptr<TH1> result =  fntf_.get_hist( pre_/ p / post_ );
        TH1 * test_ptr = result.get();

        if ( TH1D * ptr_1d = dynamic_cast<TH1D*> ( test_ptr ) )
        {
            ptr_1d->Rebin(rebin_);
        }
        else if ( TH2D * ptr_2d = dynamic_cast<TH2D*>( test_ptr ) )
        {
            ptr_2d->Rebin2D(rebin_, rebin_);


            if ( col_x_ && col_y_ )
            { throw std::runtime_error(
                    "Attempt to collapse both x and y in " + p.string()  ); }

            if( col_x_ )
            {
                if ( col_range_ )
                {
                    TH1D * y_proj = ptr_2d->ProjectionX(
                            "_",
                            ptr_2d->GetYaxis()->FindBin( min_val_ ), 
                            ptr_2d->GetYaxis()->FindBin( max_val_ )
                            );
                    result.reset( y_proj );
                }
                else
                {
                    TH1D * y_proj = ptr_2d->ProjectionX();
                    result.reset( y_proj );
                }
                result->SetDirectory(0);
            }
            else if ( col_y_ )
            {
                if ( col_range_ )
                {
                    TH1D * x_proj = ptr_2d->ProjectionY(
                            "_",
                            ptr_2d->GetXaxis()->FindBin( min_val_ ), 
                            ptr_2d->GetXaxis()->FindBin( max_val_ )
                            );
                    result.reset( x_proj );
                }
                else
                {
                    TH1D * x_proj = ptr_2d->ProjectionY();
                    result.reset( x_proj );
                }
                result->SetDirectory(0);
            }
        }
        return result;
    }

    double integral( TH1& h, double min, double max )
    {
        double bmin = h.GetXaxis()->FindBin( min );
        double bmax = h.GetXaxis()->FindBin( max );
        //std::cout << "min: " << min << " " << " max " << max << std::endl;
        //std::cout << "bmin: " << bmin << " " << " bmax " << bmax << std::endl;
        return h.Integral( bmin, bmax );
    }
}

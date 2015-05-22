#ifndef HISTEXTRACTOR_HH
#define HISTEXTRACTOR_HH
#include "TFile.h"
#include "TH1.h"
#include <memory>
#include "yaml-cpp/yaml.h"
#include <boost/filesystem/path.hpp>
#if 0
/*
 *  _   _ _     _   _____      _                  _
 * | | | (_)___| |_| ____|_  _| |_ _ __ __ _  ___| |_ ___  _ __
 * | |_| | / __| __|  _| \ \/ / __| '__/ _` |/ __| __/ _ \| '__|
 * |  _  | \__ \ |_| |___ >  <| |_| | | (_| | (__| || (_) | |
 * |_| |_|_|___/\__|_____/_/\_\\__|_|  \__,_|\___|\__\___/|_|
 *
 *
 */
#endif
namespace fn
{
    class FNTFile
    {
        public:
            virtual std::unique_ptr<TH1> get_hist ( boost::filesystem::path p ) = 0;
    };

    class RootTFileWrapper : public FNTFile
    {
        public:
            RootTFileWrapper( TFile& fin );
            virtual std::unique_ptr<TH1> get_hist ( boost::filesystem::path p );

        private:
            TFile& fin_;
    };

    class ChannelHistExtractor : public FNTFile
    {
        public: 
            ChannelHistExtractor( FNTFile& tfw );
            void set_pre_path( boost::filesystem::path p );
            void set_post_path( boost::filesystem::path p );
            void set_rebin( int rebin );
            void reset_collapse();
            void set_collapse_x( bool col_x);
            void set_collapse_y( bool col_y);
            virtual std::unique_ptr<TH1> get_hist ( boost::filesystem::path p );

        private:
            boost::filesystem::path pre_;
            boost::filesystem::path post_;
            FNTFile& fntf_;
            int rebin_;
            bool col_x_;
            bool col_y_;
    };

    template <class IT>
        std::unique_ptr<TH1> get_summed_histogram( FNTFile& tf, IT begin, IT end )
        {
            auto current = begin;
            if ( begin == end )
            { return std::unique_ptr<TH1>{} ;}

            auto result = tf.get_hist( *current );
            result->Reset();

            while( current != end )
            {
                auto current_hist = tf.get_hist( *current );
                result->Add( current_hist.get() );
                ++current;
            }

            return result;
        }

    double integral( TH1& h, double min, double max );

}
#endif

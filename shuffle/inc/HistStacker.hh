#ifndef HISTSTACKER_HH
#define HISTSTACKER_HH
#include "TFile.h"
#include "TH1.h"
#include <memory>
#include "yaml-cpp/yaml.h"
#include <boost/filesystem/path.hpp>
#include "OwningStack.hh"
#include "HistExtractor.hh"
#include "Km2Scaling.hh"
#include "HistStore.hh"
#if 0
/*
 *  _   _ _     _   ____  _             _
 * | | | (_)___| |_/ ___|| |_ __ _  ___| | _____ _ __
 * | |_| | / __| __\___ \| __/ _` |/ __| |/ / _ \ '__|
 * |  _  | \__ \ |_ ___) | || (_| | (__|   <  __/ |
 * |_| |_|_|___/\__|____/ \__\__,_|\___|_|\_\___|_|
 *
 *
 */
#endif
namespace fn
{
    class RootColors
    {
        public:
            RootColors();
            int operator()( std::string name ) const;

        private:
            std::map<std::string, int> colors_;
    };

    int root_color( std::string name );

    class HistFormatter
    {
        public:
            HistFormatter( std::string filename );
            void format( TH1& h , std::string type ) const;

            std::map< std::string, Color_t> type_colors_;
    };

    void format_data_hist( TH1 &h, const YAML::Node& instruct );

    //--------------------------------------------------

    class HistStacker
    {
        public:
            typedef OwningStack::iterator iterator;
            typedef OwningStack::const_iterator const_iterator;
            typedef OwningStack::size_type size_type;

            HistStacker( const YAML::Node& stack_instructions, FNTFile& tfin, 
                    std::map<std::string, Km2Scaling>& scaling_info,
                    const HistFormatter& formatter );

            std::unique_ptr<TH1> load_hist( const YAML::Node& instruct );
            void scale_hist(  TH1& h, const YAML::Node& instruct );
            void color_hist(  TH1& h, const YAML::Node& instruct );

            void create_stack();
            void write( std::string name );
            void write_total( std::string name );

            iterator begin() { return stack_.begin() ; }
            iterator end() { return stack_.end() ; }
            size_type size() { return stack_.size() ;}

            const_iterator begin() const { return stack_.begin() ; }
            const_iterator end() const { return stack_.end() ; }

        private:
            const YAML::Node stack_instructions_;
            FNTFile& tfin_;
            OwningStack stack_;
            std::map<std::string, Km2Scaling>& scaling_info_;
            const HistFormatter& formatter_;
    };

}
#endif

#ifndef EVENTCHAIN_HH
#define EVENTCHAIN_HH

#include "TChain.h"
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "logging.hh"

namespace fne
{
    class Event;
}

namespace fn
{
    class EventChain
    {
        public:
            //Constructors
            EventChain ( logger& log ,
                    std::vector<boost::filesystem::path> filenames,
                    Long64_t nEvents );

            EventChain ( logger& log ,
                    std::vector<boost::filesystem::path> filenames );

            //Build functions
            void init_root_objects();
            void load_root_files(
                    std::vector<boost::filesystem::path> filenames );

            bool load_next_event_header();
            void load_full_event();

            //Tree information
            int get_tree_number() const;
            std::string get_filename() const;

            //Event queries
            const fne::Event * get_event_pointer() const;
            Long64_t get_max_event() const;
            std::string read_info() const;
            int get_event_version() const;
            bool is_mc() const;
            bool is_tree_mc() const;

        private:
            void new_tree();

            logger& log_;
            bool have_max_event_;
            Long64_t max_event_;
            int current_tree_;

            Long64_t next_event_;
            Long64_t local_entry_;

            //ROOT objects
            fne::Event * event_ptr_;
            TBranch * event_branch_;
            std::unique_ptr<fne::Event> event_;
            std::unique_ptr<TChain> tch_;

            int event_version_;
            bool is_mc_;
    };

}

#endif

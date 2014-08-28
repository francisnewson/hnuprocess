#ifndef ECHAIN_HH
#define ECHAIN_HH

#include "TChain.h"
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "logging.hh"
#include "root_help.hh"
#include "TList.h"
#include "TStreamerInfo.h"
#include <sstream>
#include <iomanip>

namespace fn
{

    template <class E>
        class EChain
        {
            private:
                logger& log_;
                bool have_max_event_;
                Long64_t max_event_;
                int current_tree_;

                Long64_t next_event_;
                Long64_t local_entry_;

                //ROOT objects
                E * event_ptr_;
                TBranch * event_branch_;
                std::unique_ptr<E> event_;
                std::unique_ptr<TChain> tch_;
                std::string tree_name_;
                std::string branch_name_;

                int event_version_;


                void new_tree()
                {
                    //A new tree means a new file and potentially 
                    //pointers can get lost

                    BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                        << "New tree:" << get_filename() ;

                    //Reassign event pointer
                    BOOST_LOG_SEV( log_, debug) 
                        << "EBRANCH: " << event_branch_ << "name: " << branch_name_;
                    event_branch_ = tch_->GetBranch( branch_name_.c_str() );
                    BOOST_LOG_SEV( log_, debug) << "EBRANCH: " << event_branch_;
                    event_branch_->SetAddress(&event_ptr_);

                    //Check everything matches
                    current_tree_ = tch_->GetTreeNumber();
                    bool check_event_tree_matches =  
                        ( tch_->GetTree() == event_branch_->GetTree() ) ;

                    BOOST_LOG_SEV( log_, fn::severity_level::debug)
                        << "New tree pointers!"
                        << std::setw(10) << tch_.get() 
                        << std::setw(10) << event_ptr_
                        << std::setw(10) << tch_->GetTreeNumber();

                    assert( check_event_tree_matches);

                    current_tree_ = tch_->GetTreeNumber();
                }

            public:

                //get filename
                std::string get_filename() const
                {
                    auto tree = tch_->GetTree();

                    if ( !tree )
                    { 
                        throw std::runtime_error
                            ( "Attempt to read name of current tree failed"
                              " because we received no tree" ); 
                    }

#if 0
                    auto info =  static_cast<fne::FileInfo*>
                        ( tree->GetUserInfo()->At(0) );

                    return info->filename;
#endif
                    return "get_filename not implemented";
                }

                int get_tree_number() const
                {
                    return current_tree_;
                }

                //--------------------------------------------------

                //Initializers
                void init_root_objects()
                {
                    tch_ = std::unique_ptr<TChain>( new TChain(tree_name_.c_str()) );
                    event_ = std::unique_ptr<E>( new E{} );
                    event_ptr_ = event_.get();
                }

                void load_root_files( std::vector<boost::filesystem::path> filenames )
                {
                    //Check we have some files
                    if ( filenames.size() == 0 )
                    { throw std::runtime_error(
                            "Attempt to load chain with 0 files" ); }

                    for ( auto& filename : filenames )
                    {
                        //prepare filename
                        std::string file_string  = filename.string();
                        add_file_protocol( file_string);

                        //add file: NB we don't count entries
                        int added = tch_->Add( root_file_string( file_string ) );
                        if ( added != 1 )
                        { throw std::runtime_error
                            ("Could not add " + filename.string()  ); }

                        BOOST_LOG_SEV( log_, debug ) << "Added: " << file_string;
                    }


                    if (!have_max_event_)
                    {
                        //If we have no maximum, read the whole lot
                        BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                            << "Counting entries for whole chain ...";
                        max_event_ = tch_->GetEntries();
                    }

                    BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                        << "Going to read " << max_event_ << " entries";

                    //Prepare chain
                    BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                        << "TChain::GetFile() ... ";
                    tch_->GetFile();

                    BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                        << "Setting cache";
                    tch_->SetCacheSize( 100000000 );
                    tch_->AddBranchToCache( "*", kTRUE );

                    //Extract event version
                    //
                    const TList * streamer_infos = tch_->GetFile()->GetStreamerInfoList();
                    TIter next( streamer_infos );
                    for (  TObject * obj = next() ; obj ; obj = next() ) 
                    {
                        //obj->Print();
                        auto streamer_info = static_cast<TStreamerInfo*>( obj );
                        if ( strcmp( streamer_info->GetName(), "fne::Event" ) == 0 )
                        {
                            event_version_ =  streamer_info->GetClassVersion(); 
                        }
                    }

                    if ( event_version_ == 0 )
                    { 
                        std::string message = 
                            "Could not find event version for"
                            " fne::Event ( or it was 0 )" ;

                        BOOST_LOG_SEV( log_, always_print) << message;
                        //throw std::runtime_error ( message );
                    }

                    BOOST_LOG_SEV( log_, fn::severity_level::always_print)
                        << "Using event version " << event_version_ ;

                    new_tree();
                }

                //--------------------------------------------------

                //Constructors
                EChain ( logger& log, std::vector<boost::filesystem::path> filenames,
                        std::string tree_name,std::string branch_name,
                        Long64_t nEs )
                    :log_(log), have_max_event_( true ), max_event_( nEs),
                    current_tree_(-1), next_event_(0), tree_name_( tree_name ),
                    branch_name_( branch_name )
            {
                init_root_objects();
                load_root_files( filenames );
            }

                EChain ( logger& log ,
                        std::vector<boost::filesystem::path> filenames ,
                        std::string tree_name, std::string branch_name
                       )
                    :log_(log), have_max_event_( false ),
                    current_tree_(-1), next_event_(0), tree_name_( tree_name ),
                    branch_name_( branch_name )
                     
            {
                init_root_objects();
                load_root_files( filenames );
            }

                //--------------------------------------------------

                //Events
                bool load_next_event_header()
                {
                    //Does all the work.
                    event_->Clear("C");

                    //Have we finished?
                    if ( next_event_ == max_event_ )
                    { return false; }

                    BOOST_LOG_SEV( log_, fn::severity_level::debug)
                        << "EventChain: about to do LoadTree(event)";

                    local_entry_ = tch_->LoadTree(next_event_);
                    if (tch_->GetTreeNumber() != current_tree_)
                    { new_tree() ;}

                    assert( tch_->GetTree() == event_branch_->GetTree() );

                    BOOST_LOG_SEV( log_, fn::severity_level::debug)
                        << "EventChain: about to do GetEntry(local_entry)";

                    int read = event_branch_->GetEntry( local_entry_ );

                    //Did we read successfully
                    if ( read < 1 )
                    { return false ; }

                    ++next_event_;

                    BOOST_LOG_SEV( log_, fn::severity_level::debug)
                        << "EventChain: Finished load_next_event_header";

                    return true;
                }

                void load_full_event()
                {
                    //Nothing to do atm because we load
                    //everything on header request
                }

                //INFO
                const E* get_event_pointer() const
                { return event_ptr_; }

                Long64_t get_max_event() const
                { return max_event_ ; };

                int get_event_version() const
                { return event_version_; }

                //Print info about data transfer
                std::string read_info() const
                {
                    std::stringstream ss;
                    ss << "Read " << tch_->GetFile()->GetBytesRead()   
                        << " bytes in " << tch_->GetFile()->GetReadCalls()
                        << "  transactions.";
                    return ss.str();
                }
        };
}

#endif

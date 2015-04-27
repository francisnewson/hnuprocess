#include "EventChain.hh"
#include "Event.hh"
#include "root_help.hh"
#include "TList.h"
#include "TStreamerInfo.h"
#include <sstream>
#include <iomanip>

namespace fn
{
    using boost::filesystem::path;
    using std::vector;

    EventChain::EventChain ( logger& log, vector<path> filenames,
            Long64_t nEvents )
        :log_(log), have_max_event_( true ), max_event_( nEvents),
        current_tree_(-1), next_event_(0)
    {
        init_root_objects();
        load_root_files( filenames );
    }

    EventChain::EventChain ( logger& log ,
            std::vector<boost::filesystem::path> filenames )
        :log_(log), have_max_event_( false ),
        current_tree_(-1), next_event_(0)
    {
        init_root_objects();
        load_root_files( filenames );
    }

    //Inititialize pointers
    void EventChain::init_root_objects()
    {
        tch_ = std::unique_ptr<TChain>( new TChain("T") );
        event_ = std::unique_ptr<fne::Event>( new fne::Event{} );
        event_ptr_ = event_.get();
    }

    void EventChain::load_root_files(
            std::vector<boost::filesystem::path> filenames )
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

            if ( !have_max_event_ )
            {
                //Check we can open file
                bool found_file =  false;
                int nb_attempts = 0 ;
                int max_attempts = 100;
                while ( !found_file )
                {
                    ++nb_attempts;
                    std::unique_ptr<TFile> tf{ TFile::Open( root_file_string( file_string ) ) };
                    if ( tf )
                    { 
                        BOOST_LOG_SEV( log_, always_print) << "Checked: " << tf->GetName() ; 
                        found_file = true;
                    }
                    else if ( nb_attempts == max_attempts ){
                        throw std::runtime_error( "Can't open " + file_string + 
                                " after " + std::to_string( max_attempts ) + " attempts"); 
                    }
                }
            }

            //add file: NB we don't count entries
            int added = tch_->Add( root_file_string( file_string ) );
            if ( added != 1 )
            { throw std::runtime_error
                ("Could not add " + filename.string()  
                 +"\ntried: "  + file_string ); }
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
        TFile * floaded  = tch_->GetFile();
        if ( ! floaded )
        {
            throw std::runtime_error( "TChain::GetFile() failed");
        }

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

            if ( event_version_ == 0 )
            { 
                throw std::runtime_error
                    ( "Could not find event version for"
                      " fne::Event ( or it was 0 )" );
            }
        }

        BOOST_LOG_SEV( log_, fn::severity_level::always_print)
            << "Using event version " << event_version_ ;

        new_tree();

        is_mc_ = is_tree_mc();
    }


    //--------------------------------------------------

    //TREE INFORMATION

    int EventChain::get_tree_number() const
    {
        return current_tree_;
    }

    std::string EventChain::get_filename() const
    {
        auto tree = tch_->GetTree();

        if ( !tree )
        { 
            throw std::runtime_error
                ( "Attempt to read name of current tree failed"
                  " because we received no tree" ); 
        }

        auto info =  static_cast<fne::FileInfo*>
            ( tree->GetUserInfo()->At(0) );

        return info->filename;
    }

    bool EventChain::is_mc() const
    {
        return is_mc_;
    }

    bool EventChain::is_tree_mc() const
    {
        auto tree = tch_->GetTree();
        if ( !tree )
        { 
            throw std::runtime_error
                ( "Attempt to read name of current tree failed"
                  " because we received no tree" ); 
        }
        auto info =  static_cast<fne::FileInfo*>
            ( tree->GetUserInfo()->At(0) );

        return info->mc;
    }


    //--------------------------------------------------

    //EVENT QUERIES

    const fne::Event* EventChain::get_event_pointer() const
    { return event_ptr_; }

    Long64_t EventChain::get_max_event() const
    { return max_event_ ; };

    int EventChain::get_event_version() const
    { return event_version_; }

    //Print info about data transfer
    std::string EventChain::read_info() const
    {
        std::stringstream ss;
        ss << "Read " << tch_->GetFile()->GetBytesRead()   
            << " bytes in " << tch_->GetFile()->GetReadCalls()
            << "  transactions.";
        return ss.str();
    }

    //--------------------------------------------------


    void EventChain::new_tree()
    {
        //A new tree means a new file and potentially 
        //pointers can get lost

        BOOST_LOG_SEV( log_, fn::severity_level::always_print)
            << "New tree:" << get_filename() ;

        //Reassign event pointer
        event_branch_ = tch_->GetBranch( "event");
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

        BOOST_LOG_SEV( log_, always_print) 
            << "New tree: " << ( is_tree_mc() ? "MC" : "DATA" );
    }

    bool EventChain::load_next_event_header()
    {
        //Does all the work.
        event_->Clear("C");

        //Have we finished?
        if ( next_event_ == max_event_ )
        { return false; }

        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "EventChain: about to do LoadTree(event)";

        local_entry_ = tch_->LoadTree(next_event_);

        if ( local_entry_ < 0 )
        {
            switch( local_entry_ )
            {
                case -1:
                    throw std::runtime_error( "Empty chain" );
                case -2:
                    throw std::out_of_range( "Entry too big for chain" );
                case -3:
                    throw std::runtime_error( "Cannot open corresponding file" );
                case -4:
                    throw std::runtime_error( "Missing TChainElement or TTree" );
            }
        }


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

    void EventChain::load_full_event()
    {
        //Nothing to do atm because we load
        //everything on header request
    }
}



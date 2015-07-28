#include "OSLoader.hh"
#include "stl_help.hh"
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace fn
{
    std::streambuf * StreamBufLoader::get_buf(boost::filesystem::path& p)
    {
        return get_buf( p.string() );
    }

    std::streambuf * StreamBufLoader::get_buf( std::string target )
    {
        if ( target == "std::cerr" || target == "cerr" )
        {
            return std::cerr.rdbuf(); 
        }
        else if ( target == "std::cout" || target == "cout" )
        {
            return std::cout.rdbuf(); 
        }
        else
        {
            //We need an ofstream
            return get_fbuf( target );
        }
    }

    //--------------------------------------------------

    std::streambuf * StreamBufLoader::get_fbuf( std::string target)
    {
        auto found = fstreams_.find( target );
        if ( found != fstreams_.end() )
        {
            return found->second->rdbuf();
        }
        else
        {
            fstreams_.insert( 
                    make_pair( target, make_unique<std::ofstream>
                        ( target ) ) );
            if ( ! fstreams_.at( target )->good() )
            {
                throw std::runtime_error
                    ( "Couldn't create " + target );

            }
            return get_fbuf( target );
        }
    }
    //--------------------------------------------------

    std::ostream& OSLoader::get_ostream
        ( boost::filesystem::path& p)
        {
            return get_ostream( p.string() );
        }

    std::ostream& OSLoader::get_ostream( std::string target)
    {
        auto found = ostreams_.find( target );
        if ( found != ostreams_.end() )
        {
            return *( found->second.get() );
        }
        else
        {
            ostreams_.insert( 
                    make_pair( target, make_unique<std::ostream>
                        ( sbf_.get_buf( target) ) ) );
            if ( ! ostreams_.at( target )->good() )
            {
                throw std::runtime_error
                    ( "Couldn't create " + target);
            }


            return get_ostream( target );
        }
    }

    //--------------------------------------------------

    TFile& TFileLoader::get_tfile(  boost::filesystem::path target )
    {
        return get_tfile_opt( target, "RECREATE" );

    }

    TFile& TFileLoader::get_tfile_opt
        ( boost::filesystem::path target, std::string opt )
    {
        auto found = tfiles_.find( target.string() );
        if ( found != tfiles_.end() )
        {
            return * ( found->second.get() );
        }
        else
        {
            tfiles_.insert( 
                    make_pair( target.string() , make_unique<TFile> 
                        ( target.string().c_str(), opt.c_str() ) ) );

            if ( tfiles_.at( target.string() )->IsZombie() )
            {
                throw std::runtime_error
                    ( "Couldn't create " + target.string() );
            }

            return get_tfile( target );
        }
    }

}

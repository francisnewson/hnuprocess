#include "text.hh"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <stdexcept>

namespace fn
{
	void splash ( boost::filesystem::path p , std::ostream& os )
	{
		if ( !boost::filesystem::exists( p ) )
		{ throw  std::runtime_error( "Can't find " +  p.string() ); }

		boost::filesystem::ifstream ifs( p );
		os << ifs.rdbuf();
	}
}

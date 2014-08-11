#include "PolarityFinder.hh"
#include <boost/filesystem/fstream.hpp>

namespace fn
{
	PolarityFinder::PolarityFinder(){}

	PolarityFinder::PolarityFinder( boost::filesystem::path path)
	{
		load_data( path );
	}

	void PolarityFinder::load_data( boost::filesystem::path path)
	{
		boost::filesystem::ifstream ifs( path );
		ranger_.load_info(
				std::istream_iterator<OrderedRanger<int, int>::Info>( ifs ),
				std::istream_iterator<OrderedRanger<int, int>::Info>() );
	}

	int PolarityFinder::get_polarity( int run ) const
	{
		try{
			return ranger_.get_value( run );
		}
		catch( std::runtime_error& e )
		{
		ranger_.reset();
		return get_polarity( run );
		}
	}

	//--------------------------------------------------

	std::istream& operator>> ( std::istream& is,
			OrderedRanger<int,int>::Info& inf )
	{
		return is >> inf.min >> inf.max >> inf.val;
	}

}

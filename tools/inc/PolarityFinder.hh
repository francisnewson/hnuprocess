#ifndef POLARITYFINDER_HH
#define POLARITYFINDER_HH

#include <boost/filesystem/path.hpp>
#include "OrderedRanger.hh"
#include <iosfwd>

namespace fn
{

	class PolarityFinder
	{
		public:
			PolarityFinder();
			PolarityFinder( boost::filesystem::path path);
			void load_data(  boost::filesystem::path path);
			int get_polarity( int run ) const;

		private:
			OrderedRanger<int,int> ranger_;
	};

	std::istream& operator>> ( std::istream& is,
			OrderedRanger<int,int>::Info& inf );

}


#endif

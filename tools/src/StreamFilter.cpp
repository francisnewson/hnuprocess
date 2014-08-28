#include "StreamFilter.hh"

namespace fn
{
	int StreamFilter::underflow()
	{
		int const eof = traits_type::eof();
		int results = mySource->sbumpc();
		while ( results == myCommentChar ) 
		{
			while ( results != eof && results != '\n') 
			{
				results = mySource->sbumpc();
			}
			if ( results != eof )
			{
				results = mySource->sbumpc();
			}
		}
		if ( results != eof ) {
			myBuffer = results;
			setg( &myBuffer, &myBuffer, &myBuffer + 1 );
		}
		return results;
	}

	StreamFilter::StreamFilter
		( std::istream& source, char comment )
		: myOwner( source ), 
		mySource( source.rdbuf() ),
		myCommentChar( comment )
	{
		myOwner.rdbuf( this );
	}

	StreamFilter::~StreamFilter()
	{
		myOwner.rdbuf( mySource );
	}


}

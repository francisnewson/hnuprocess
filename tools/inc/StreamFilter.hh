#ifndef STREAMFILTER_HH
#define STREAMFILTER_HH

#include <iostream>
#include <streambuf>
namespace fn
{

	/****************************************************
	 * I stole this from stack overflow:
	 * http://stackoverflow.com/a/13304656/1527126
	 ****************************************************/

	class StreamFilter : public std::streambuf
	{
		private:
			std::istream& myOwner;
			std::streambuf* mySource;
			char myCommentChar;
			char myBuffer;

		protected:
			int underflow();

		public:
			StreamFilter
				( std::istream& source, char comment = '#' );
			~StreamFilter();
	};
}

#endif

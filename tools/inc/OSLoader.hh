#ifndef OSLOADER_HH
#define OSLOADER_HH
#if 0
/*
 *   ___  ____  _                    _           
 *  / _ \/ ___|| |    ___   __ _  __| | ___ _ __ 
 * | | | \___ \| |   / _ \ / _` |/ _` |/ _ \ '__|
 * | |_| |___) | |__| (_) | (_| | (_| |  __/ |   
 *  \___/|____/|_____\___/ \__,_|\__,_|\___|_|   
 *                                               
 * 
*/
#endif

#include <iosfwd>
#include <string>
#include <map>
#include <memory>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include "TFile.h"

namespace fn
{


	/* **************************************************
	 * Class to provide a pointer to
	 * streambuf given a string
	 * **************************************************/
	class StreamBufLoader
	{
		public:
			std::streambuf * get_buf( std::string target);
			std::streambuf * get_fbuf( std::string target);
			std::streambuf * get_buf(boost::filesystem::path& p);


		private:
			std::map
				<std::string, std::unique_ptr<std::ofstream>> 
				fstreams_;

	};

	/* **************************************************
	 * Class to provide reference to
	 * a ostream given a string
	 * **************************************************/
	class OSLoader
	{
		public:
			std::ostream& get_ostream( std::string target);
			std::ostream& get_ostream
				( boost::filesystem::path& p);

		private:
			StreamBufLoader sbf_;
			std::map
				<std::string, std::unique_ptr<std::ostream>> 
				ostreams_;
	};

	/* **************************************************
	 * Class to provide reference to
	 * a TFile given a string
	 * **************************************************/
	class TFileLoader
	{
		public:
			TFile& get_tfile(  boost::filesystem::path& target );

		private:
			std::map<std::string, std::unique_ptr<TFile> > tfiles_;
	};
}
#endif

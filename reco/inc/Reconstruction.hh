#ifndef RECONSTRUCTION_HH
#define RECONSTRUCTION_HH

#include <vector>
#include <boost/optional/optional.hpp>
#include "EventChain.hh"
#include <boost/filesystem/path.hpp>
#include "Notifyer.hh"
#include "Event.hh"
#include "logging.hh"

#if 0
/*
 *
 *
 *   _____                                     
 *   |  __ \                                    
 *   | |__) |___  ___ ___  _ __                 
 *   |  _  // _ \/ __/ _ \| '_ \                
 *   | | \ \  __/ (_| (_) | | | |               
 *   |_|  \_\___|\___\___/|_| |_|  _             
 *        | |                 | | (_)            
 *     ___| |_ _ __ _   _  ___| |_ _  ___  _ __  
 *    / __| __| '__| | | |/ __| __| |/ _ \| '_ \ 
 *    \__ \ |_| |  | |_| | (__| |_| | (_) | | | |
 *    |___/\__|_|   \__,_|\___|\__|_|\___/|_| |_|
 *                                                              
 */
#endif


namespace fn
{
	class Subscriber;
	class Selection;

	/*****************************************************
	 * RECONSTRUCTION CLASS
	 *
	 * Pass events from chain to subscribers
	 *****************************************************/

	class Reconstruction
	{
		public:
			Reconstruction( logger& log);
			void set_filenames
				( std::vector<boost::filesystem::path> filenames );
			void set_required_events( Long64_t n);

			void load_chain();

			//Subscriber management
			void add_header_check( Selection * s );
			void add_raw_header_subscriber( Subscriber * s );
			void add_checked_header_subscriber( Subscriber * s );
			void add_event_subscriber( Subscriber *  s );

			const fne::Event * get_event_ptr() const;

			void status_report( std::ostream& os );

			//Event progressing
			bool auto_next_event();

			bool next_event();
			bool continue_event();
			void process_headers();
			void load_event();
			void process_event();
			void end_processing();

			//Useful information
			Long64_t max_events() const;
			void set_remote_stop( bool& remote_stop );
			boost::filesystem::path get_current_filename();
            bool is_mc() const;

            void set_min_run( int min_run );
            void set_max_run( int max_run );

		private:
			//Subscribers
			std::vector<Selection*> header_checks_;
			Notifyer<Subscriber> notifyer_;

			//Data sources
			std::vector<boost::filesystem::path> filenames_;
			EventChain * chain_;
			const fne::Event* event_ptr_;

			//status
			bool required_events_specified_;
			Long64_t required_events_;
			Long64_t event_count_;
			boost::optional<bool&> remote_stop_;
			bool stop_;
			bool event_loaded_;

            boost::optional<int> min_run_;
            boost::optional<int> max_run_;

			//Debug
			std::ostream& os_;
            logger& log_;
	};
}

#endif

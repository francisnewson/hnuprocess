#ifndef RANDOMSAMPLER_HH
#define RANDOMSAMPLER_HH
#include <utility>
#include <vector>
#include <iterator>
#include <fstream>
#include <iomanip>
#if 0
/*
 *  ____                 _                 ____                        _
 * |  _ \ __ _ _ __   __| | ___  _ __ ___ / ___|  __ _ _ __ ___  _ __ | | ___ _ __
 * | |_) / _` | '_ \ / _` |/ _ \| '_ ` _ \\___ \ / _` | '_ ` _ \| '_ \| |/ _ \ '__|
 * |  _ < (_| | | | | (_| | (_) | | | | | |___) | (_| | | | | | | |_) | |  __/ |
 * |_| \_\__,_|_| |_|\__,_|\___/|_| |_| |_|____/ \__,_|_| |_| |_| .__/|_|\___|_|
 *                                                              |_|
 *
 */
#endif
namespace fn
{
    //A class to pick values from a random distribution
    template <typename Value>
        class RandomSampler
        {
            public:
                typedef std::pair<double, Value> RawPair;
                typedef std::vector<RawPair> RawVector;

                typedef std::pair<int, Value> IntPair;
                typedef std::vector<IntPair> IntVector;

                RandomSampler()
                    :max_key_( 1000 ){}

                //--------------------------------------------------

                //ensure values_ is in ready state
                //load from raw_values
                void update_values()
                {
                    if ( raw_values_.size() == 0 )
                    {
                        return;
                    }

                    //Sort descending keys so the most often used values
                    //are at the start of the vector
                    std::sort( raw_values_.begin(),
                            raw_values_.end() , std::greater<RawPair>() );

                    //Get cumulative values
                    scaled_values_.clear();

                    for ( auto& rawv : raw_values_ )
                    {
                        //First value requires no cumulative addition
                        if ( scaled_values_.size() == 0 )
                        {
                            scaled_values_.push_back( rawv );
                        }
                        //Subsequent probabilites must be added up
                        else
                        {
                            scaled_values_.push_back( std::make_pair
                                    ( rawv.first + scaled_values_.back().first, rawv.second ) );
                        }
                    }

                    //now normalise the cumlative probabilities
                    double total = scaled_values_.back().first;
                    double ratio = double ( max_key_ ) / total;

                    for ( auto& sv : scaled_values_ )
                    {
                        sv.first *= ratio;
                    }

                    //convert to integer keys
                    values_.clear();
                    for ( auto& sv : scaled_values_ )
                    {
                        values_.push_back( std::make_pair
                                ( static_cast<int>( std::ceil( sv.first ) ), sv.second ) );
                    }

                    assert( values_.back().first  == max_key_ );
                }

                //--------------------------------------------------

                //Modify integer granularity
                void set_max_key( int max_key )
                {
                    max_key_ = max_key;
                    update_values() ;
                }

                //--------------------------------------------------

                //Load data
                template< class Iterator>
                    void load_new_data( Iterator start, Iterator finish )
                    {
                        raw_values_ = RawVector( start, finish );
                        update_values();
                    }

                void load_new_data( std::ifstream& is )
                {
                    load_new_data( 
                            std::istream_iterator<RawPair>( is ),
                            std::istream_iterator<RawPair>()
                            );
                }

                //--------------------------------------------------

                //Get a value corresponding to the seed
                const Value& get_value( int seed ) const
                {
                    //Check we have sample
                    if ( values_.size() == 0 )
                    {
                        throw std::runtime_error
                            ( "RandomSampler: request for get_value"
                              "but no data has been specified" );
                    }

                    //Check bounds
                    if ( seed > max_key_ )
                    {
                        throw std::runtime_error
                            ( "RandomSampler: request with key greater than max key"
                              " (which is " + std::to_string( max_key_) + " ) " );
                    }

                    auto pos = values_.begin();

                    while ( seed > pos->first )
                    {
                        ++pos;
                    }

                    return pos->second;
                }

                //--------------------------------------------------

                void print_values( std::ostream& os ) const
                {

                    for ( auto& val : values_ )
                    {
                        os << std::setw(10) << val.first 
                            << " " << val.second << "\n";
                    }

                    for ( auto& val : scaled_values_ )
                    {
                        os << std::setw(10) << val.first 
                            << " " << val.second << "\n";
                    }

                    for ( auto& val : raw_values_ )
                    {
                        os << std::setw(10) << val.first 
                            << " " << val.second << "\n";
                    }
                }

            private:
                int max_key_;
                RawVector raw_values_; //raw ( unsorted, not cumulative )
                RawVector scaled_values_; //cumulative and sorted
                IntVector values_; //cumulative
        };

    //--------------------------------------------------

	//THESE ARE FOUND
	template <typename T>
		bool operator< ( const std::pair<double,T>& lhs, const std::pair<double,T>& rhs )
		{ 
			return lhs.first < rhs.first ;
		}

	template <typename T>
		std::istream& operator>> ( std::istream& is , std::pair<double,T>& rp )
		{
			return is >> rp.first >> rp.second ;
		}

}
#endif


#ifndef ORDEREDRANGER_HH
#define ORDEREDRANGER_HH

#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace fn
{

    //If you are progressing through keys in order,
    //this makes finding the next one easy

    template <class K, class V >
        class OrderedRanger
        {
            public:
                struct Info
                {
                    K min;
                    K max;
                    V val;
                };

            //Load info
            template <class InputIterator>
                void load_info
                ( InputIterator start, InputIterator finish )
                {
                    info_ = std::vector<Info>( start, finish );
                    reset();
                }

            //Rest info
            void reset() const
            {
                current_info_ = info_.begin();
            }

            //Ranges are inclusive
            bool in_range( K key, Info info ) const
            {
                return ! ( key < info.min || key > info.max );
            }

            //Forward lookup K in ranges
            V get_value( const K& key ) const
            {
                while ( !  in_range( key, *current_info_ ) )
                {
                    ++current_info_;
                    if ( current_info_ == info_.end() )
                    {
                        throw std::runtime_error
                            ( "OrderedRanger: Ran past end of info" );
                    }
                }
                return current_info_->val;
            }

            void print_info( std::ostream& os )
            {
                for ( auto& info : info_ )
                {
                    os << info.min << " " << info.max << " " << info.val << std::endl;
                }
            }

            private:
            //Current position is preserved between calls
            mutable typename std::vector<Info>::const_iterator current_info_;
            std::vector<Info> info_;
        };

    //--------------------------------------------------

    template <class K>
        class OrderedRanges
        {
            public:
                template <class InputIterator>
                    void load_info
                    (InputIterator start, InputIterator finish )
                    {
                        ranges_ = std::vector<std::pair<K,K>>( start, finish );
                        reset();
                    }

                void reset() const
                {
                    current_range_ = ranges_.begin();
                }

                //Ranges are inclusive
                bool in_range( const K& key, const std::pair<K,K>& range ) const
                {
                    return ! ( key < range.first || key > range.second );
                }

                bool in_any_range( const K& key ) const
                {
                    //if we are already past all the ranges then return false
                        if ( current_range_ == ranges_.end() ){ return false; }

                    //catch up until upper limit is >= key
                    while ( current_range_->second < key )
                    {
                        if ( current_range_ == ranges_.end() ){ return false; }
                        ++current_range_;
                    }

                    //check if key is in this range
                    return ( in_range( key, *current_range_ ) );
                }

            private:
                //current positions is preserved between calls
                std::vector<std::pair<K,K>> ranges_;
                mutable typename std::vector<std::pair<K,K>>::const_iterator current_range_;
        };

    //--------------------------------------------------

    template <class K>
        class OrderedTracker
        {
            public:
                //Load info
                template <class InputIterator>
                    void load_info
                    ( InputIterator start, InputIterator finish )
                    {
                        info_ = std::vector<K>( start, finish );
                        reset();
                    }

                //Rest info
                void reset()
                {
                    current_info_ = info_.begin();
                }

                //Forward lookup K in ranges
                bool check_value( const K& key ) const
                {
                    if ( current_info_ == info_.end() )
                    { return false; }

                    while (  key >  *current_info_ )
                    {
                        ++current_info_;

                        if ( current_info_ == info_.end() )
                        { return false; }

                    }
                    return ( key == *current_info_ );
                }

            private:
                //Current position is preserved between calls
                mutable typename std::vector<K>::iterator current_info_;
                std::vector<K> info_;
        };
}

#endif

#ifndef BADBURST_HH
#define BADBURST_HH
#include "Selection.hh"
#include "OrderedRanger.hh"
#include "Event.hh"
#include <iosfwd>
#if 0
/*
 *  ____            _ ____                 _
 * | __ )  __ _  __| | __ ) _   _ _ __ ___| |_
 * |  _ \ / _` |/ _` |  _ \| | | | '__/ __| __|
 * | |_) | (_| | (_| | |_) | |_| | |  \__ \ |_
 * |____/ \__,_|\__,_|____/ \__,_|_|  |___/\__|
 *
 *
 */
#endif
namespace fn
{
    /*****************************************************
     * BADBURST
     *
     * Reads a list of BadBurst from a file. For each event,
     * checks we are not in a bad burst using an 
     * OrderedTracker
     *****************************************************/
    class BadBurst : public CachedSelection
    {
        public:
            struct BurstId
            {
                int run;
                int burst_time;
            };

            //Constructor
            template< class InputIterator>
                BadBurst( InputIterator start,
                        InputIterator finish,
                        const fne::Event* e )
                :event_( e )
                {
                    burst_tracker_.load_info( start, finish );
                }


        private:
            bool do_check() const ;
            const fne::Event* event_;
            OrderedTracker<BurstId> burst_tracker_;

            REG_DEC_SUB( BadBurst);
    };

    template<>
        Subscriber * create_subscriber<BadBurst>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    /*****************************************************
     * BADBURSTRANGE
     *
     * Reads a list of BadBurst ranges from a file.
     * For each event, * checks we are not in a bad burst
     * range, using an OrderedRanges object:
     *****************************************************/
    class BadBurstRange : public CachedSelection
    {
        public:
            typedef BadBurst::BurstId BurstId;

            //Constructor
            template< class InputIterator>
                BadBurstRange( InputIterator start,
                        InputIterator finish,
                        const fne::Event* e )
                :event_( e )
                {
                    burst_ranges_.load_info( start, finish );
                }

        private: 
            bool do_check() const;
            const fne::Event* event_;
            OrderedRanges<BurstId> burst_ranges_;


            REG_DEC_SUB( BadBurstRange );
    };

    template<>
        Subscriber * create_subscriber<BadBurstRange>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    std::istream& operator >> 
        ( std::istream& is , BadBurst::BurstId& bi);

    std::istream& operator >> 
        ( std::istream& is , 
          std::pair<BadBurst::BurstId,BadBurst::BurstId>& bi_pair);

    std::ostream& operator << 
        ( std::ostream& os , const BadBurst::BurstId& bi);

    bool operator<  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs );

    bool operator>  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs );

    bool operator!=  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs );

    bool operator==  ( const BadBurst::BurstId& lhs, 
            const BadBurst::BurstId& rhs );

    //--------------------------------------------------

}
#endif

#ifndef KAONTRACK_HH
#define KAONTRACK_HH
#include "Subscriber.hh"
#include "Event.hh"
#if 0
/*
 *  _  __               _____               _    
 * | |/ /__ _  ___  _ _|_   _| __ __ _  ___| | __
 * | ' // _` |/ _ \| '_ \| || '__/ _` |/ __| |/ /
 * | . \ (_| | (_) | | | | || | | (_| | (__|   < 
 * |_|\_\__,_|\___/|_| |_|_||_|  \__,_|\___|_|\_\
 *                                               
 * 
 */
#endif
namespace fn
{

    class KaonTrack : public Subscriber
    {
        public:
            KaonTrack( const fne::Event * e , bool mc);
            void new_event() ;
            double get_kaon_mom() const;
            TVector3 get_kaon_3mom() const;

        private:
            const fne::Event * e_;
            bool mc_;
    };

}
#endif
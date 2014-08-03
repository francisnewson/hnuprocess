#ifndef SELECTION_HH
#define SELECTION_HH
#include "Subscriber.hh"
namespace fn
{

    class Selection : public Subscriber
    {
        public:
            virtual bool check();

    };

}
#endif

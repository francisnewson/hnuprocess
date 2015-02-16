HNURECO
=======

_This document is in Markdown format so it can be read with a Markdown editor._

`hnureco` is the main program used for analysing `.root` files extracted from the NA62 data. 
It uses several core classes which live in the `reco` folder. They are all in the namespace `fn::`

EventChain
----------

The ROOT libraries provide `TTree` and `TChain` libraries to read `.root` files
which contain serialized event data.

`EventChain` is a wrapper round the `TChain` class, specialized for my `Event` class with a few convenience features, such as checking if the file contains real or simulated data, and keeping track of which version fo the `Event` class is being used.
It is constructed with a list of input files and a max number of events to process.

`load_next_event_header()` reads in just the header part of the next event and returns true if the read was successfully and we have not reached the end of the file or max events.

`load_full_event()` should be called afterwards to read in the rest of the event if we want it.

This is all handled automatically by the Reconstruction class.

Reconstruction
--------------

The `Reconstruction` class handles the interaction between the `EventChain` class which loads the data, and the `Subscriber` classes which want to process the data.

Once everything is connected, you can just do ( assuming the `Reconstruction` instance is called `reco`):

    while ( reco.auto_next_event() )
    {
        //Nothing needed here 
    }

and it will loop through all the events, notifying all subscribers of every new event.

You can skip all the automatic processing and  get manual access to the events with:

    fne::Event * e = reco.get_event_ptr()
    while ( reco.auto_next_event() )
    {
        std::cerr << e->detector.ntracks << std::endl;
    }

Subscribers
-----------

Any class which wants to process data should inherit from the `Subscriber` class. 
This provides an interface for handling individual events as well as start-of-burst and end-of-burst etc.
There are no purely virtual functions - an instance of the `Subscriber` will simply ignore everything.

A simple subscriber could just overwrite one function:

    class Speaker : public Subscriber
    {
        public:
            void new_event()
            {
                std::cout << "New event received!" << std::endl;
            }
    };

Normally, a subscriber would require access to some data:

    class KnowledgeableSpeaker : public Subscriber
    {
        private:
            const fne::Event * e_;

        public:
            KnowledgeableSpeaker( const fne::Event * e)
            :e_( e){}

            void new_event()
            {
                std::cout << "New event number "
                << e_->header.compact_number << " received!" << std::endl;
            }
    };

It should obtain the event pointer from the reconstruction class:

    fne::Event * e = reco.get_event_ptr()
    KnowledgeableSpeaker k_speaker( e );
    while ( reco.auto_next_event() )
    {
        k_speaker.new_event();
    }

and we don't need to call new_event() directly, because the `Reconstruction` class will do it for us:

    fne::Event * e = reco.get_event_ptr()
    KnowledgeableSpeaker k_speaker( e );
    reco.add_event_subscriber( &k_speaker);
    while ( reco.auto_next_event() )
    {
        //new_event() is called for all subscribers
    }

This makes life easier when we have lots of subscribers.

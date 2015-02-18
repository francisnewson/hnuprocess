HNURECO
=======

_This document is in Markdown format so it can be read with a Markdown editor._

`hnureco` is the main program used for analysing `.root` files extracted from the NA62 data. 
It uses several core classes which live in the `reco` folder. They are all in the namespace `fn::`

One class `fne::Event` is in a different namespace and the source files are outside of this project.
The `Event` class is compiled separately because it is also used by `COmPACT` so it must be compiled twice with two sets of options.

The model follows the Observer (publish-subscribe) pattern (GoF pg293).
The `Reconstruction` class 'publishes' events which are read by the `Subscriber` classes ( selections, analyses, etc. ).

EventChain
----------

The ROOT libraries provide `TTree` and `TChain` libraries to read `.root` files
which contain serialized event data.

`EventChain` is a wrapper round the `TChain` class, specialized for the `Event` class with a few convenience features, such as checking if the file contains real or simulated data, and keeping track of which version of the `Event` class is being used.
It is constructed with a list of input files and a max number of events to process.

`load_next_event_header()` reads in just the header part of the next event and returns true if the read was successful and did not reach the end of the file or the maximum number of events.

`load_full_event()` should be called afterwards to read in the rest of the event if required.

This is all handled automatically by the `Reconstruction` class.

Reconstruction
--------------

The `Reconstruction` class handles the interaction between the `EventChain` class which loads the data, and the `Subscriber` classes which want to process the data.

Once everything is connected, you can just do ( assuming the `Reconstruction` instance is called `reco`):

    while ( reco.auto_next_event() )
    {
        //Nothing needed here 
    }

and it will loop through all the events, notifying all `Subscriber`s of every new event.

You can skip all the automatic processing and  get manual access to the events with:

    fne::Event * e = reco.get_event_ptr()
    while ( reco.auto_next_event() )
    {
        std::cerr << e->detector.ntracks << std::endl;
    }

Subscribers
-----------

Any class which needs to process data should inherit from the `Subscriber` class. 
This provides an interface for handling individual events as well as start-of-burst and end-of-burst etc.
There are no purely virtual functions - a concrete instance of the `Subscriber` base class will simply ignore everything.

A simple example of a `Subscriber` could just overwrite one function:

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
                << e_->header.compact_number 
                << " received!" << std::endl;
            }
    };

It should obtain the event pointer from the `Reconstruction` class:

    fne::Event * e = reco.get_event_ptr();
    KnowledgeableSpeaker k_speaker( e );

and we don't need to call new_event() directly, because the `Reconstruction` class will do it for us:

    reco.add_event_subscriber( &k_speaker);
    while ( reco.auto_next_event() )
    {
        //new_event() is called for all subscribers automatically
    }

This makes life easier when we have lots of subscribers. For the complete subscriber interface, see `reco/inc/Subscriber.hh`.

Note that Subscribers don't have to store an `Event` pointer; they could instead store a reference to some other class which wraps the event pointer.
In general, it is preferred not to use the `Event` class directly.

See, for example, the `SingleTrack` class in `tracking/inc/SingleTrack.hh`.

There are two interesting subclasses of `Subscriber`: `Selection` and `Analysis`, which are described below.

Selection
---------

Selections are designed to filter the events seen by other subscribers.
They provide two functions, `check()` which returns a `bool` indicating whether the event has been accepted, and `get_weight()` used when filling histograms.
Derived classes should redefine these functions.

Two subsclasses of `Selection` are commonly used. `CachedSelection` implements caching of the selection result so that if it requested several times for the same event it does not have to be completed. `CachedSelection` defines `check()` to return the cached and delegates the actual calculation to `do_check()` which descendents should redefine.

`CompositeSelection` allows a selection to be defined as the logical AND of several other selections. It implements a composite pattern so `CompositeSelections` can be nested.
There is no need to inherit from `CompositeSelection`, the required selections are provided in the constructor as a list of pointers.

See `reco/inc/Selection.hh` for more details.

Analysis
--------

`Analysis` is designed as the base for classes which  produce reults such as histograms or print-outs.
An `Analysis` takes a `Selection` in its constructor  which is used to decide whether to process the event or not. 
Processing is handled by the function `process_event()` which should be redefined in descendent classes.

See `reco/inc/Analysis.hh` for more details.

Example
-------

An example using the library could look like this ( logging detail etc. ignored for clarity, see `apps/src/hnureco.cpp` for a complete program ).

    //Class which only accepts events from even runs.
    class EvenRun : public Selection
    {
    private:
        const fne::Event * e_;

    public:
        EvenRun( const fne::Event* e )
            :e_( e ){}

        bool check(){ return (e_->header.run % 2 == 0); }
        double get_weight(){ return 1.0; }
    };

    //Class which prints number of tracks
    class TrackCounter : public Analysis
    {
    private:
        const fne::Event * e_;

    public:
        TrackCounter( const Selection& s, const fne::Event * e )
        :Analysis( s ), e_( e ){}

        void process_event()
        { std::cout << e_->detector.ntracks << std::endl ; }
    };

    int main()
    {
     ...
     Reconstruction reco( 'datafile.root' );
     fne::Event * e = reco.get_event_ptr()

     EvenRun even_run{ e };
     TrackCounter track_counter{ even_run, e };

     //order matters here: track_counter will call even_run.check(),
     //so even_run must be updated first.

     reco.add_event_subscriber( &even_run );
     reco.add_event_subscriber( &track_counter );

     while ( reco.auto_next_event() )
         {
         //For even runs we will see the number of tracks in each 
         // event printed. Otherwise, we see nothing
         }

     reco.end_processing()
     //calls subscriber.end_processing() for all subscribers

     return 0;
    }

YAML Scripts
------------

It is not practical to recompile every time the selection changes; instead `Subscriber` classes can be created dynamically at runtime, reading their configuration from a file.

For information about reading from a config file, see `RecoParser.md`.

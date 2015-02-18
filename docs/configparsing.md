CONFIG PARSING
==============

_This document is in Markdown format so it can be read with a Markdown editor._

**Important**

For a `Subscriber` to be created via config file, it must specialize the helper function `create_subscriber` and call the macros `REG_DEC_SUB` and `REG_DEF_SUB`.
For details see blow.

---

To avoid having to recompile every time a selection changes, the library is provided with a parsing class to  read instructions from a config file and produce `subscriber` objects dynamically.

The config files are in `yaml` format [yaml.org](http://yaml.org/).

An example config file could look like this (assuming the classes `KaonTrack`, `MuonVeto`, `MuonReqStatus` and `MuonPlotter` have been appropriately defined):

    #a class to provide a corrected kaon track
    - input:
        name: kaon_track
        type: KaonTrack
        kaon_type: auto
        beam_mapping: input/reco/beam/beam_mapping.yaml

    #a class to readout the muon_veto status
    - subscriber:
        name: muon_veto
        type: MuonVeto
        method: Data

    #a selection which only allows muon_veto status 1 or 2
    - selection:
        name: muv_12
        type : MuonReqStatus
        allowed_status: [ 1, 2 ]
        inputs:
            muv: muon_veto

    #a selection which uses the corrected kaon_track to print
    #information about muons. Here it is only applied to events
    #which pass the muv_12 requirement of status 1 or 2
    - analysis:
        name: muon_plotter
        type: MuonPlotter
        selection: muv_12
        inputs:
            kt: kaon_track

Every node in the `.yaml` file should have a `name` and a `type`.
`type` corresponds to the class name of the 'Subscriber' (i.e the real C++ type) and `name` is what would be returned by `Subscriber::get_name()`.
The rest of the entries contain the information needed to construct a particular subscriber.
Reading them is the job of a helper function which should be supplied for all enabled subscribers.

RecoFactory
-----------

`RecoFactory` is a wrapper class for handling the interaction between the `Reconstruction` and `Subscriber` classes.
It automatically connects `Subscriber`s to the `Reconstruction` and handles their lifetime requirements, as well as checking for duplicate names and maintaining unique IDs.
It also provides helper functions for common requirements such as inspecting event file metadata.
See `reco/inc/RecoFactory.hh` for available functions`.

RecoParser
---------

The `RecoParser` class stores a reference to a `RecoFactory`.
It reads a config file in the `yaml` format and create all the requested `Subscriber`s, with calls to the function `RecoFactory::create_subscriber`.
This relies on the helper functions described below.

SubscriberFactory
-----------------

When creating a `Subscriber` object from a `YAML::Node`, the `RecoFactory` needs a mapping between the `std::string`s in the config file and the concrete in the program.
This mapping is handled by the `SubscriberFactory` class.

There is a standard function designed to return a new Subscriber:

    template<typename T> Subscriber * 
        create_subscriber(  YAML::Node& instruct, RecoFactory& rf )
        { return new T ;}

Classes which can be created from a `YAML::Node` should specialize this function to their own type, e.g.:

    class TrackCharge : public Selection
    {
        public:
            TrackCharge( fne::Event * e , int charge );

            /* rest of class implementation *8/
    };

    template<>
    Subscriber * create_subscriber<TrackCharge>
    (YAML::Node& instruct, RecoFactory& rf )
    {
        int charge = instruct["charge"].as<int>();
        fne::Event * e = rf.get_event_ptr();
        return new TrackCharge ( e, charge );
    }

The `SubscriberFactory` class maintains a global mapping between `std::string`s and templated `create_subscriber` functions.
New `Subscriber`s can be registered with the helper template class `SubscriberRegister` but this can be handled automatically by two macros ,`REG_DEC_SUB` and `REG_DEF_SUB` which should be called in the header and source file respectively, e.g.:

    #TrackCharge.hh
    class TrackCharge : public Selection
    {
        public:
            TrackCharge( fne::Event * e , int charge );

            /* rest of class implementation */

        private:
            REG_DEC_SUB( TrackCharge ); //called inside class definition
    };

    #TrackCharge.cpp
    REG_DEF_SUB( TrackCharge ); //called outside any functions;

    /* rest of class functions */ 

If these two macros are in place, then the class can be created from a `YAML::Node`.

The program `print_subscribers` ( source in `apps/src/print_subscribers.cpp` ), prints a list of registered `subscribers`, which can be checked whenever a new class is defined.

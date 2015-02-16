HNU ANALYSIS
============

_This document is in Markdown format so it can be read with a Markdown editor._

A set of libraries, applications and scripts for analysing data from
the NA62 2007 experiment.

The main program is `hnureco` - it reads root files containing recorded 
or simulated data and runs analyses on them to produce plots or new
data.

Other programs perform ancillary tasks.

Source layout
-------------
The general structure is:

    some_folder/src/MyClass.cpp
    some_folder/inc/MyClass.hh
    some_folder/SConscript

Most folders produce a library, `some_folder.so`. The files in `apps`
are compiled to produce executables.

Special folders and files
---------------
- `input`: text files ( mostly .yaml format ) used to instruct programs
- `batch`: scripts used when submitting jobs on the lxplus cpu farm
- `site-scons`: scons build rules
- `tools`: common tools shared by lots of programs
- `output`: synced locally

Building
--------

Building is handled by scons. Type

    scons [-j8]

to build everying.

There are currently 2 build dirs:

* `buildO0` : debug build for finding segfaults etc.
* `buildO3` : production build for running over data

By default both are built, but they can be specified individually:

    scons buildO0

Every subfolder has a SConscript file, describing the rules for building
that folder. The folder site_scons has extra build rules to handle 
linking to boost and ROOT etc.

The SConstruct file at top level sets up the environment used by
everything else.

Running
-------

use:

    source ./scripts/env.sh

to set up the right environment, then run, for example:

    ./buildO3/apps/hnureco -h

to see the options for a particular application.

The folder `docs/` has more info on some of the individual apps.

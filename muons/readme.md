Muon classes and functions
==========================

BasicMuonStats
--------------
Raw info on muons - counts, positions, etc
No reconstruction

SingleMuon
----------
A class to find a single muon in an event
Either raw from muon strucutre
Or generated with scattering etc in MC

MuonShapes
----------
Uses SingleMuon and SingleTrack objects
to look at track muon separation etc

muon_functions
--------------
includes find_muon to extract MC particle
and mu_error_0902SC for cutting

muon_selections
---------------
MuonReq requires a muon with MUV1 and MUV2
MuonXYWeight uses SingleTrack to apply weights

MuonScatterSim
--------------
Does the MC scattering of MC muons to 
decide location of MUV hits 
used by SingleMuon

MuonEffiency
------------
Measure MUV efficiency using a MuonReq
object, as a function of track momentum
and extrapolated position
(using SingleTrack)

MuonMultiplierEfficiency looks at
efficiency vs cut multiplier

MuvBursts
---------
Burst by burst muv eff

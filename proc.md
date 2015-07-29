STEPS TO FINAL LIMITS
---------------------

- ### Generate MUV efficiency files
    - Run over data to measure burst by burst eff
      - run `/b/apps/hnureco -m input/reco/thesis/muv_bursts.yaml`
      - *job*: needs writing!
      
    - Determine bad bursts
      - run `/b/apps/input/muv_bad_bursts.cpp`
      - generate `bad_muv_burst.xx.dat` and `bad_muv_burst_ranges.dat` used by everything else

    - Run over data to measure xyp eff
      - run `/b/apps/hnureco -m input/reco/thesis/muv_3d.yaml`
      - *job*: `muv_3d`

    - Extract MUV efficiency
      - run `/b/apps/extract_muv_eff -i tdata/muv_3d/all.muv_3d.root -o tdata/staging/log/muv_eff_log.root -c output/muv_eff.root`
      - `muv_eff.root` should be placed at `input/reco/conditions/muv/muv_eff_all.root` for use by all other processes

- ### Generate hnu acceptance
    - Run over all hnu samples
      - run `/b/apps/hnureco -m input/reco/thesis/km2_full.yaml`
      - *job*: `km2_acc`
      - acceptance is calculated during limit production

- ### Create main set of plots
    - Run over all data and backgrounds (also includes trigger efficiency plots)
      - run `/b/apps/hnureco -m input/reco/thesis/halo_control/yaml`
      - *job*: `halo_control`

- ### Determine scattering contribution
    - Run over km2 MC
      - run `/b/apps/hnureco -m input/reco/thesis/km2_full.yaml`
      - run `/b/apps/hnureco -m input/reco/thesis/km2_full_scatter.yaml`
      - *job*: km2_noscat
      - *job*: km2_scat

- ### Determine k3pi crosstalk
    - Run over P6 K- data
      - run `/b/apps/hnureco -m input/reco/thesis/k3pi_crosstalk.yaml`
      - *job*: k3pi_crosstalk

- ### Subtract halo k3pi crosstalk
    - run `./b/apps/halo_subtract -m input/halo_sub/hsub.q11t.yaml -o tdata/staging/halo_sub.q11t.root -l tdata/staging/halo_sub_log.q11t.root`

- ### Subtract km2 interference
    - run `./b/apps/km2_subtract -m input/km2_sub/halo_control.haml -o tdata/staging/km2_sub.q11t.root -l tdata/staging/km2_sub_log.q11t.root`

- ### Merge all plots
    - run `hadd tdata/staging/all.plots.root tdata/halo_control/all.halo_control.root tdata/staging/halo_sub.q11t.root tdata/staging/km2_sub.q11t.root`

- ### Generate bacgkround estimate
    - Generate the shuffled file 
      - Data are taken from tdata/staging/all.plots.root
      - MC are scaled to Km2 flux using the RK selection.
      - Halo is scaled using the upper and lower ctrl regions.
      - run `./b/apps/shuffle -m input/shuffle/thesis/halo/halo_signal.sub.q11t.yaml -o tdata/staging/all.mass_plots.root`

- ### Check output
    - Add up all the backgrounds and scale by trigger efficiency to look at by eye
      - run `./b/apps/merge_stack -m input/merge/thesis/full.yaml -o output/test_merge.root`

- ### Get limits
    run `./b/apps/signal_shapes`

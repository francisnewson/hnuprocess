STEPS TO FINAL LIMITS
---------------------

- Generate MUV efficiency files

- Generate hnu acceptance

- Subtract halo k3pi crosstalk
    run `./b/apps/halo_subtract -m input/halo_sub/hsub.q1.yaml -o output/halo_sub.q1.root -l output/halo_sub_log.q1.root`

- Generate bacgkround estimate
    - Generate the shuffled file 
      Data are taken from tdata/staging/all.halo_control.root
      MC are scaled to Km2 flux using the RK selection.
      Halo is scaled using the upper and lower ctrl regions.
      run `./b/apps/shuffle -m input/shuffle/thesis/halo/halo_signal.sub.q11t.yaml -o tdata/staging/all.mass_plots.root`

- Check output
    - Add up all the backgrounds and scale by trigger efficiency to look at by eye
      run `./build_bham/apps/merge_stack -m input/merge/thesis/full.yaml -o output/test_merge.root`

- Generate trigger efficiency


- Get limits
    run ./

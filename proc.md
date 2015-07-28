STEPS TO FINAL LIMITS
---------------------

- Generate MUV efficiency files

- Generate hnu acceptance

- Generate bacgkround estimate
    - Generate the shuffled file with halo subtracted. 
      Data are taken from tdata/staging/all.halo_control.root
      MC are scaled to Km2 flux using the RK selection.
      Halo is scaled using the upper and lower ctrl regions.
      run `./b/apps/shuffle -m input/shuffle/thesis/halo/halo_signal.sub.q11t.yaml -o tdata/staging/all.mass_plots.root`

- Check output
    - Add up all the backgrounds and scale by trigger efficiency to look at by eye


- Generate trigger efficiency


- Get limits
    run ./

import yaml
import os.path as osp

#convenicence yaml dump parameters
def yaml_dump( output_file, yaml_dict ):
    '''Nice defaults for yaml.damp'''
    output_file.write( yaml.dump ( yaml_dict,
        default_flow_style = False,
        explicit_start = True ) )

def scaling_method(  pol, region ):
    return {
            'name' : 'mc_'+pol+'_'+region,
            'halo_channels' : ['p6.data.kless.q11t.v2.' + pol],
            'data_channels' : ['p5.data.q11t.v2.' + pol ],
            'km2_channels' : ['p5.km2.v2.'+ pol],

            'halo' : {
                'strategy': 'm2',
                'input_file': 'data/halo_plots/all.halo_plots.root',
                'plot_path': osp.join( region, 'h_m2m_kmu' ) ,
                'min_mass': -0.3,
                'max_mass': -0.1 if ( region == 'halo_upper') else -0.05,
                },

            'km2' : {
                'strategy': 'm2',
                'input_file': 'data/halo_plots/all.halo_plots.root',
                'plot_path': osp.join( region, 'h_m2m_kmu' ),
                'min_mass': -0.01,
                'max_mass': -0.01,
                }
            }

def stack_chan( chan, pol ):
    return {'item' : {
            'name': '_'.join([ chan, pol]),
            'scaling': '_'.join(['mc', pol, region] ),
            'type':  chan.replace( 'k2pig', 'k2pi'), 
            'fid_weight' : 'p5.'+chan+'.v2.'+pol,
            'channels': [ 'p5.'+chan+'.v2.'+pol ]
            } }

def output_dict( pol, region, plot, rebin ):
    result =  {
    'input_file': 'data/halo_plots/all.halo_plots.root',
    'plot_path' : osp.join( region, plot ),
    'rebin' : rebin,
    'data_plot': {
        'channels': [ 'p5.data.q11t.v2.pos' ],
        'do_blind' : 'false' },
    'stack' : [ stack_chan( chan, pol) for chan in [ 'km3', 'k2pig', 'k3pi', 'halo', 'k3pi0', 'km2'] ]
    }

    halo_stack = next( x for x in result['stack'] if x['item']['name'] == '_'.join([ 'halo', pol] ) )
    halo_stack['item']['channels'] = ['p6.data.kless.q11t.v2.' + pol ]
    return result


result = {}

result['scaling'] = [  
        { 'scaling_method' : scaling_method( 'pos', 'halo_upper' ) },
        { 'scaling_method' : scaling_method( 'pos', 'halo_lower' ) }
        ]

result['weights'] = {'fid_file': 'working/halo_plots/fids.dat'}

for region in [ 'halo_upper', 'halo_lower' ]:
    for  plot in [ 'h_m2m_kmu', 'h_p', 'h_cda', 'h_z' ]:
        result['output'] = output_dict(  'pos', region, plot, 1 )
        file_name = osp.join( 'input/shuffle/halo/','halo_'+plot + '_'+region + '.yaml')
        with open( file_name , 'w'  ) as f :
            yaml_dump( f, result )

        print( './build_O3/apps/shuffle -m {0} -o output/{1}'.format( 
            file_name, osp.basename(file_name).replace('.yaml', '.root' ) ) )
        

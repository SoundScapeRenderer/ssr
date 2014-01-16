% Script for testing the SSR MEX file

infilename = 'input.wav';
outfilename = 'output.wav';

positions = [0; 2];  % one column for each input channel
orientations = -90;  % row vector of angles in degree
models = { 'plane' };  % cell array of model strings

params.block_size = 1024;
params.threads = 2;

% only for loudspeaker renderers:
params.reproduction_setup = '../data/reproduction_setups/circle.asd';

% only for binaural renderer:
params.hrir_file = '../data/impulse_responses/hrirs/hrirs_fabian.wav';

% only for WFS renderer:
params.prefilter_file = ...
    '../data/impulse_responses/wfs_prefilters/wfs_prefilter_120_1500_44100.wav';

if ~exist('ssr', 'var')
    ssr = @ssr_binaural;
end

[sig, params.sample_rate] = wavread(infilename);
sig = single(sig);
sources = size(sig, 2);

ssr('init', sources, params)

ssr('source_position', positions)
ssr('source_orientation', orientations)
ssr('source_model', models{:})

out = ssr_helper(sig, ssr);

assert(ssr('out_channels') == size(out, 2))
assert(ssr('block_size') == params.block_size)
ssr('clear')

wavwrite(out, params.sample_rate, outfilename);

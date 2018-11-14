Building the SSR as MEX-file for GNU Octave and MATLAB
======================================================

Compile for GNU Octave:

    make octave

Compile for Matlab:

    make matlab

By default, all available renderers are compiled, specific renderers can be
selected with the variable MEXFILES, e.g.

    make octave MEXFILES="ssr_binaural ssr_wfs"

Matlab comes with a very old default GCC, so you may have to create
a symbolic link in `$MATLAB_DIR/sys/os/glnxa64/` pointing to
`/usr/lib/x86_64-linux-gnu/libstdc++.so.6` (or similar).

Remove all generated files:

    make clean

Usage example:

``` octave

inputblock = transpose(single([0 0 1 0 0 0 0 0]));
sources = size(inputblock, 2);
params.block_size = size(inputblock, 1);
params.sample_rate = 44100;
params.reproduction_setup = '../data/reproduction_setups/circle.asd';
params.threads = 4;

ssr_dca('init', sources, params)

positions = [0; 2];  % one column for each source

ssr_dca('source_position', positions)

% for more parameters see test_ssr.m

% process (and discard) one block for interpolation:
outputblock = ssr_dca('process', single(zeros(params.block_size, sources)));

% now the source parameters have reached their desired values
outputblock = ssr_dca('process', inputblock);

% do something with 'outputblock' ...
% repeat for each block ...

ssr_dca out_channels
ssr_dca clear
ssr_dca help

```

% Script for testing the MEX file for the NFC-HOA renderer

if ~exist('filename', 'var')
    filename = 'input.wav';
end

blocksize = 1024;
setup = '../data/reproduction_setups/circle.asd';

threads = 1;

positions = [0; 2];  % one column for each input channel

[sig, fs] = wavread(filename);
sig = single(sig);

sources = size(sig, 2);
ssr_nfc_hoa('init', setup, sources, blocksize, fs, threads)
ssr_nfc_hoa('source', 'position', positions)

out = ssr_helper(sig, @ssr_nfc_hoa);

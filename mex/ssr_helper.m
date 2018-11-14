function out = ssr_helper(in, func)
% Run the SSR block-wise on an input signal
% 'func' is the SSR function, e.g. @ssr_dca
% 'func' must be initialized already

block_size = func('block_size');
in_channels = size(in, 2);
in_length = size(in, 1);

% calculate (and discard) one block with empty input
garbage = func('process', single(zeros(block_size, in_channels)));
clear garbage
% now all parameters are up-to-date

% processing does only work if the signal length is a non-zero, integer multiple
% of the block size
signallength = ceil(size(in, 1) / block_size) * block_size;

in((end+1):signallength, :) = 0;  % append zeros, if necessary

out = func('process', in);
out = out(1:in_length, :);

end

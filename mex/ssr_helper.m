function out = ssr_helper(in, func)
% Run the SSR block-wise on an input signal
% 'func' is the SSR function, e.g. @ssr_nfc_hoa
% 'func' must be initialized already

block_size = func('block_size');
out_channels = func('out_channels');
in_channels = size(in, 2);

% calculate (and discard) one block with empty input
garbage = func('process', single(zeros(block_size, in_channels)));
clear garbage
% now all parameters are up-to-date

signallength = size(in, 1);

blocks = ceil(signallength/block_size);

out = zeros(signallength, out_channels);

for idx = 1:blocks-1
    block_start = (idx-1) * block_size + 1;
    block_end = block_start + block_size - 1;
    inputblock = in(block_start:block_end, :);
    out(block_start:block_end, :) = func('process', inputblock);
end

% TODO: handle last block!

end

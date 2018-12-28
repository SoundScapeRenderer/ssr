% This script is part of the SoundScape Renderer (SSR). It exemplarily
% shows how to prepare a set of head-related impulse responses (HRIRs) in
% a format readable by the SSR.
%
% The HRIRs used here are obtained from The CIPIC HRTF Database
% (http://interface.cipic.ucdavis.edu/CIL_html/CIL_HRTF_database.htm)
% Note that the above mentioned material is Copyright (c) 2001 The
% Regents of the University of California. All Rights Reserved.
%
% Download the file
% http://interface.cipic.ucdavis.edu/data/special_kemar_hrir.tar and
% uncompress it in the folder where this Matlab script resides. Finally,
% run the script.
%
% Author of this script: Jens Ahrens, 21.01.2008
% This script is Copyright (c) 2008 Deutsche Telekom Laboratories

clear all;

% load data set from file 'large_pinna_final.mat'
load large_pinna_final.mat;

% this is necessary for interpolation
left  = [left,  left(:,end)];
right = [right, right(:,end)];

% time domain interpolation (note that time domain interpolation is not
% most favourable in this context!)
hrirs_left  = interp2([0:5:360], [1:size(left,1)]', left(:,:), ...
                      [0:1:359], [1:size(left,1)]','spline');

hrirs_right = interp2([0:5:360], [1:size(right,1)]', right(:,:), ...
                      [0:1:359], [1:size(right,1)]','spline');

% prepare data for storage
hrirs = zeros(size(hrirs_right,1), 720);

for n = 1:2:720
    % Something's wrong here. The distribution of left and right
    % channels should be the other way round!?!
    hrirs(:, n+1) = hrirs_left(:,(n+1)/2);
    hrirs(:, n)   = hrirs_right(:,(n+1)/2);
end

% normalize
hrirs = hrirs./max(abs(hrirs(:))) .* .99;

% write data to file
wavwrite(hrirs, 44100, 16, 'hrirs_cipic_large_pinna.wav');

disp('Done.');

% This script generates a sqrt(j k) pre-equalization filter 
% for Wave Field Synthesis
%
% S.Spors
% Deutsche Telekom Laboratories
% 9th Mai 2008

% ===== parameters ========================================================

% sampling frequency of filter
f_s = 48000;

% aliasing frequency of system (= upper frequency limit of equalization filter)
f_al = 1500;

% lower frequency limit of filter (= frequency when subwoofer is active)
f_sw = 120;

% number of coefficients for filter
Nfilt=128;


% ===== variables =========================================================
f = linspace(0,f_s/2,441*10);

idx_al=max(find(f<f_al));
idx_sw=max(find(f<f_sw));

H = ones(1,length(f));


% -------------------------------------------------------------------------
% computation of pre-equalization filter
% -------------------------------------------------------------------------

% desired response
H(1:idx_al) = sqrt(2*pi*f(1:idx_al))./sqrt(2*pi*f_al);
H(1:idx_sw) = H(idx_sw)*ones(1,idx_sw);

% compute filter
h = firls(Nfilt,2*f/f_s,H);

% truncate length to power of 2
h = h(1:end-1);

% -------------------------------------------------------------------------
% plot resulting filter characteristics
% -------------------------------------------------------------------------

Hfilt=fftshift(fft(h));
Hfilt=Hfilt(Nfilt/2+1:end);
f2 = linspace(0,f_s/2,length(Hfilt));

figure
plot(f,20*log10(abs(H)));
hold on
plot(f2,20*log10(abs(Hfilt)),'ro-');
hold off

grid on
xlabel('frequency -> [Hz]');
ylabel('magnitude response -> [dB]');
legend('desired response','filter response','Location','SouthEast');
axis([0 2*f_al -20 2]);


figure
freqz(h,1,[],f_s)

% -------------------------------------------------------------------------
% save filter coefficients
% -------------------------------------------------------------------------
fname = sprintf('wfs_prefilter_%d_%d_%d.wav',f_sw,f_al,f_s);
disp(['Wrote pre-equalization filter into file: ' fname]);
wavwrite(h,f_s,fname);


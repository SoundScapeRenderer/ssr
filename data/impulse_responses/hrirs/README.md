The HRIR/HRTF Sets Shipped With SSR
===================================

SSR ships with two different sets of HRIRs/HRTFs, which are named based on the manikins
on which they were measured (FABIAN and KEMAR).

The reference for the FABIAN measurement is

    Alexander Lindau and Stefan Weinzierl. FABIAN - Schnelle
    Erfassung binauraler Raumimpulsantworten in mehreren Freiheitsgraden. In
    Fortschritte der Akustik, DAGA Stuttgart, 2007.

and the document `hrirs_fabian_documentation.pdf` provides further information.

The reference for the KEMAR measurement is

    Hagen Wierstorf, Matthias Geier, Alexander Raake, and Sascha Spors. A
    Free Database of Head-Related Impulse Response Measurements in the Horizontal Plane
    with Multiple Distances. In 130th Convention of the Audio Engineering Society (AES),
    May 2011.

whereby the low-frequency extension from
`https://github.com/spatialaudio/lf-corrected-kemar-hrtfs` (commit 5b5ec8) has been
applied to the KEMAR HRTFs.

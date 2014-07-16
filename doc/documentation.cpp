// THIS FILE CONTAINS SOME DOXYGEN DOCUMENTATION:

/// @file
/// Some Doxygen documentation

// MAIN PAGE

/** @mainpage

The Audio Processing Framework (APF) is a collection of C++ code which was
written in the context of multichannel audio applications. However, many modules
have a more generic scope.

Website: http://AudioProcessingFramework.github.com

This documentation: http://AudioProcessingFramework.github.com/apf-doc

Development pages: http://github.com/AudioProcessingFramework/apf

Blog: http://spatialaudio.net

Components:

- Multithreaded Multichannel Audio Processing Framework: @subpage MimoProcessor

- C++ wrapper for the JACK Audio Connection Kit (http://jackaudio.org/):
apf::JackClient

- Convolution engine using (uniformly) partitioned convolution: apf::conv

- IIR second order filter (and cascade of filters): apf::BiQuad and apf::Cascade

- Block-based delay line: apf::BlockDelayLine and its slightly awkward cousin
  apf::NonCausalBlockDelayLine

- Several @ref apf_iterators and @ref apf_iterator_macros

- Some simple containers: apf::fixed_vector, apf::fixed_list, apf::fixed_matrix

- Several different methods to prevent denormals: apf::dp

- Some mathematical functions: apf::math

- Functions for string manipulation (and conversion): apf::str

- Parameter map with a few conversion functions: apf::parameter_map

- Some tools for using the FFTW library: fftwtools.h

- Macros and helper functions for creating MEX files: mextools.h

- Simple stopwatch: apf::StopWatch

- Miscellaneous stuff: misc.h


Copyright (c) 2012-2014 Institut für Nachrichtentechnik, Universität Rostock

Copyright (c) 2006-2012 Quality & Usability Lab,
                        Deutsche Telekom Laboratories, TU Berlin
**/

// GROUPS/MODULES

/**
@defgroup apf_policies Policies
Policies for apf::MimoProcessor

New policies can be provided in user code!
**/

// EXAMPLES

/**
@example simpleprocessor.h
@example audiofile_simpleprocessor.cpp
@example flext_simpleprocessor.cpp
@example jack_simpleprocessor.cpp
@example mex_simpleprocessor.cpp
@example jack_dynamic_inputs.cpp
@example jack_dynamic_outputs.cpp
@example jack_minimal.cpp
@example dummy_example.cpp
**/

// APF NAMESPACE

/// @namespace apf
/// Audio Processing Framework

// vim:textwidth=80

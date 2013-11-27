/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

/// @file
/// Helper functions for string conversion.

#ifndef APF_STRINGTOOLS_H
#define APF_STRINGTOOLS_H

#include <string>
#include <sstream>
#include <stdexcept>  // for std::invalid_argument

namespace apf
{
/// Helper functions for string manipulation
namespace str
{

/** Converter <em>"Anything to String"</em>.
 * Converts a numeric value to a @c std::string.
 * A boolean value is converted to either @c "true" or @c "false".
 * @tparam T given input type
 * @param input anything you want to convert to a @c std::string
 * @return If @c std::stringstream could handle it, a @c std::string
 * representation of @a input, if something went wrong, an empty string.
 **/
template<typename T>
std::string A2S(const T& input)
{
  std::ostringstream converter;
  converter << std::boolalpha << input;
  return converter.str();
}

/** Convert a stream to a given type.
 * If the item of interest is followed by anything non-whitespace, the
 * conversion is invalid and @b false is returned.
 * @tparam out_T desired output type
 * @param[in,out] input input stream
 * @param[out] output output
 * @return @b true on success
 * @note If @b false is returned, @a output is unchanged.
 **/
template<typename out_T>
inline bool convert(std::istream& input, out_T& output)
{
  auto result = out_T();
  input >> result >> std::ws;
  if (!input.fail() && input.eof())
  {
    output = result;
    return true;
  }
  return false;
}

/** This is a overloaded function for boolean output.
 * @see convert()
 * @param[in,out] input input stream
 * @param[out] output output (boolean)
 * @return @b true on success
 * @note If @b false is returned, @a output is unchanged.
 **/
inline bool convert(std::istream& input, bool& output)
{
  bool result;
  // first try: if input == "1" or "0":
  input >> result >> std::ws;
  if (input.fail())
  {
    input.clear();  // clear error flags
    input.seekg(0); // go back to the beginning of the stream
    // second try: if input == "true" or "false":
    input >> std::boolalpha >> result >> std::ws;
  }
  if (!input.fail() && input.eof())
  {
    output = result;
    return true;
  }
  return false;
}

/** Converter <em>"String to Anything"</em>.
 * Convert a string (in this case meant as an array of a certain character
 * type) to a given numeric type (or, if you insist on it, to a
 * @c std::string, in which case it would not be very efficient).
 * Also, convert @c "1", @c "true", @c "0" and @c "false" to the respective
 * boolean values.
 * @tparam char_T character type of the input array
 * @tparam out_T desired output type (must have an input stream operator!)
 * @param input pointer to a zero-terminated string of any character type.
 * @param[out] output result converted to the desired type.
 * @return @b true on success
 * @note If @b false is returned, @a output is unchanged.
 **/
template<typename char_T, typename out_T>
bool S2A(const char_T* input, out_T& output)
{
  std::basic_istringstream<char_T> converter(input);
  return convert(converter, output);
}

/** Overloaded function for a STL-like string type.
 * @see S2A()
 * @tparam in_T input string type (e.g. @c std::string)
 * @tparam char_T character type of the input string (e.g. @c char)
 * @tparam traits traits class for the string type @p in_T
 * @tparam Allocator allocator for the string type @p in_T
 * @tparam out_T desired output type (must have an input stream operator!)
 * @param input a string object like @c std::string or @c std::wstring or ...
 * @param[out] output result converted to the desired type.
 * @return @b true on success
 * @note If @b false is returned, @a output is unchanged.
 * @note This uses a template template to allow any input type that has
 * the same structure as @c std::string
 **/
template<template<typename, typename, typename> class in_T,
  typename char_T, typename traits, typename Allocator, typename out_T>
bool S2A(const in_T<char_T, traits, Allocator>& input, out_T& output)
{
  std::basic_istringstream<char_T, traits, Allocator> converter(input);
  return convert(converter, output);
}

/** Converter <em>"String to Return Value"</em>.
 * Convert a string (either an array of a certain character type or an
 * STL-style string class) to a given numeric type and return the result.
 * @tparam int_T string type
 * @tparam out_T desired output type (must have an input stream operator!)
 * @param input string to be converted.
 * @param def default value.
 * @return @p input converted to the desired type. If conversion failed, the
 * default value @p def is returned.
 * @warning If the result is equal to the default value @p def, there is no
 *   way to know if the conversion was successful, sorry! You have to use
 *   S2A() if you want to make sure.
 * @see S2A()
 **/
template<typename out_T, typename in_T>
out_T S2RV(const in_T& input, out_T def)
{
  S2A(input, def); // ignore return value
  return def;
}

/** Throwing version of S2RV().
 * @tparam int_T string type
 * @tparam out_T desired output type (must have an input stream operator!)
 * @param input string to be converted.
 * @return @p input converted to the desired type. If conversion failed, an
 *   exception is thrown.
 * @throw std::invalid_argument if @p input cannot be converted to @p out_T.
 **/
template<typename out_T, typename in_T>
out_T S2RV(const in_T& input)
{
  auto result = out_T();
  if (!S2A(input, result))
  {
    throw std::invalid_argument(
        "S2RV(): Couldn't convert \"" + S2RV(input, std::string()) + "\"!");
  }
  return result;
}

/** Remove a specified number of characters from a stream and convert them to
 * a numeric type.
 * If the stream flag @c skipws is set, leading whitespace is removed first.
 * @tparam digits number of digits to read. The rest of the template
 *   parameters are determined automatically.
 * @tparam char_T character type of the stream
 * @tparam traits traits class for @c std::basic_istream
 * @tparam out_T desired (numerical) output type
 * @param[in,out] input input stream
 * @param[out] output resulting number
 * @return a reference to the modified stream.
 * @attention You have to check the returned stream for
 * @c std::ios_base::failbit to know if the conversion was successful.
 **/
template<int digits, typename char_T, typename traits, typename out_T>
std::basic_istream<char_T, traits>&
convert_chars(std::basic_istream<char_T, traits>& input, out_T& output)
{
  // if an error bit is set on the input, just return without doing anything:
  if (input.fail()) return input;
  // skip whitespace if std::skipws is set
  if (input.flags() & std::ios_base::skipws) input >> std::ws;

  char_T ch[digits];
  if (!input.read(ch, digits)) return input; // error bits are set!

  out_T factor = 1, result = 0;
  for (int i = digits - 1; i >= 0; --i)
  {
    // only numbers are allowed:
    if (ch[i] < input.widen('0') || ch[i] > input.widen('9'))
    {
      input.setstate(std::ios_base::failbit);
      return input; // error bits are set!
    }
    // character type is implicitly cast to out_T
    result += (ch[i] - '0') * factor;
    factor *= 10;
  }
  output = result;
  return input;
}

/** Remove a character from a stream and check if it is the one given as
 * parameter.
 * If the stream flag @c skipws is set, leading whitespace is removed first.
 * @tparam char_T character type of the stream (also used for the output)
 * @tparam traits traits class for @c std::basic_istream
 * @param[in,out] input input stream
 * @param character character to remove
 * @return a reference to the modified stream.
 * @attention You have to check the returned stream for
 * @c std::ios_base::failbit to know if the conversion was successful.
 **/
template<typename char_T, typename traits>
std::basic_istream<char_T, traits>&
remove_char(std::basic_istream<char_T, traits>& input, const char_T character)
{
  // if an error bit is set on the input, just return without doing anything:
  if (input.fail()) return input;
  // skip whitespace if std::skipws is set
  if (input.flags() & std::ios_base::skipws) input >> std::ws;

  char_T ch;
  if (input.get(ch) && (ch != character))
  {
    input.setstate(std::ios_base::failbit);
  }
  return input;
}

/** Remove a colon from an input stream.
 * This function is just a convenient shortcut for
 * <tt>remove_char(stream, ':')</tt>.
 * Contrary to remove_char(), this can be used as a stream modifier like this:
 * @code
 * int i; float f;
 * my_stream >> i >> remove_colon >> f;
 * @endcode
 * If the stream flag @c skipws is set, leading whitespace is removed first.
 * @tparam char_T character type of the stream
 * @tparam traits traits class for @c std::basic_istream
 * @param[in,out] input input stream
 * @return a reference to the modified stream.
 * @attention You have to check the returned stream for
 * @c std::ios_base::failbit to know if there actually was a colon and that
 * is was successfully removed.
 **/
template<typename char_T, typename traits>
std::basic_istream<char_T, traits>&
remove_colon(std::basic_istream<char_T, traits>& input)
{
  remove_char(input, input.widen(':'));
  return input;
}

/** Convert time string to numeric value in seconds.
 * @a input can be in format @c "h:mm:ss.x" or <tt>"xx.x h|min|s|ms"</tt> or
 * just in seconds. Decimals and hours are optional. Time can also be
 * negative. Multiple whitespace is allowed before and after.
 * See http://www.w3.org/TR/SMIL2/smil-timing.html#Timing-ClockValueSyntax
 * for the similar SMIL time syntax.
 * @tparam in_T input string type (e.g. @c std::string)
 * @tparam char_T character type of the input string (e.g. @c char)
 * @tparam traits traits class for the string type @p in_T
 * @tparam Allocator allocator for the string type @p in_T
 * @tparam out_T desired output type
 * @param input time string (similar to SMIL format)
 * @param[out] output numeric result in seconds. This can be either of an
 * integer or a floating point type. Conversion to an integer only works if
 * the resulting value in seconds is a whole number.
 * @return @b true if conversion was successful.
 * @since r404
 **/
template<template<typename, typename, typename> class in_T,
  typename char_T, typename traits, typename Allocator, typename out_T>
bool string2time(const in_T<char_T, traits, Allocator>& input, out_T& output)
{
  // first of all, check if there are 0, 1 or 2 colons:
  char colons = 0;
  for (size_t found = 0
      ; (found = input.find(':', found + 1)) != std::string::npos
      ; ++colons) {}

  std::basic_istringstream<char_T> iss(input);
  out_T seconds = 0; // initialisation is needed for the case (colons == 1)!

  if (colons == 0)
  {
    // no colons, but maybe suffixes like "s", "min", "h" or "ms"
    out_T number;
    if ((iss >> number >> std::ws).fail()) return false;
    if (iss.eof()) // that's everything, no suffixes!
    {
      seconds = number;
    }
    else // still something left ...
    {
      auto the_rest = std::basic_string<char_T>();
      // read the rest and remove following whitespace (see below why)
      if ((iss >> the_rest >> std::ws).fail()) return false;
      // now we check if some garbage is left after the time-string.
      // whitespace was removed in the previous line.
      if (!iss.eof()) return false;

      // now check for possible suffixes:
      if (the_rest == "h")        seconds = number * 60 * 60;
      else if (the_rest == "min") seconds = number * 60;
      else if (the_rest == "s")   seconds = number;
      else if (the_rest == "ms")
      {
        // check if milliseconds can be represented by the type out_T:
        // TODO: hopefully this isn't optimized away!
        if (number / 1000 * 1000 != number) return false;
        else seconds = number / 1000;
      }
      else return false; // no other suffix is allowed
    }
  }
  else if (colons == 1 || colons == 2)
  {
    // check if there is a plus or minus sign
    bool negative = false;
    iss >> std::ws; // remove leading whitespace
    if (iss.peek() == '-')
    {
      iss.ignore();
      negative = true;
    }
    // it doesn't matter if there is a '+' sign.

    long int hours = 0;
    int minutes; // maximum: 59

    if (colons == 1)
    {
      if ((iss >> minutes).fail()) return false;
      // attention: the sign was already removed before!
      if (minutes < 0 || minutes > 59) return false;
    }
    else if (colons == 2)
    {
      iss >> hours
        >> std::noskipws // from now on, no whitespace is allowed
        >> remove_colon; // read one character and check if it's a colon
      convert_chars<2>(iss, minutes); // read minutes as two characters
      if (iss.fail()) return false;
      if (hours < 0) return false; // the sign was already removed before!
      if (minutes > 59) return false;
    }

    out_T whole_seconds;
    out_T fraction_of_second(0);

    iss
      >> std::noskipws // no whitespace is allowed
      >> remove_colon; // read one character and check if it's a colon
    convert_chars<2>(iss, whole_seconds); // read first two digits of seconds
    if (iss.fail()) return false;

    if (whole_seconds > 59) return false;

    if (iss.peek() == '.')
    {
      if ((iss >> fraction_of_second).fail()) return false;
    }

    if (!(iss >> std::ws).eof()) return false; // nothing else is allowed!

    auto the_rest = whole_seconds + fraction_of_second;

    // the seconds part must be smaller than 60
    if (the_rest >= 60) return false;

    if (negative)
    {
      hours    = -hours;
      minutes  = -minutes;
      the_rest = -the_rest;
    }

    seconds  = static_cast<out_T>(hours   * 60 * 60);
    seconds += static_cast<out_T>(minutes * 60);
    seconds += the_rest;
  }
  else return false; // more than three colons are a deal-breaker!

  output = seconds;
  return true;
}

/** Overloaded function for a character array.
 * @see string2time()
 * @tparam char_T character type of the input array
 * @tparam out_T desired output type
 * @param input character array holding time string
 * @param[out] output see above
 * @return @b true if conversion was successful.
 **/
template<typename char_T, typename out_T>
bool string2time(const char_T* input, out_T& output)
{
  return string2time(std::basic_string<char_T>(input), output);
}

}  // namespace str
}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='

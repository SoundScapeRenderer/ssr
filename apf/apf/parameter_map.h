/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

/// @file
/// A "dictionary" for parameters.

#ifndef APF_PARAMETER_MAP_H
#define APF_PARAMETER_MAP_H

#include <map>
#include <stdexcept> // for std::out_of_range
#include "stringtools.h"

namespace apf
{

/** A "dictionary" for parameters.
 * All values are saved as @c std::string's.
 *
 * Usage examples:
 *                                                                         @code
 * apf::parameter_map params;
 * params.set("one", "first value");
 * params.set("two", 2);
 * params.set("three", 3.1415);
 * std::string val1, val5;
 * int val2, val3, val4;
 * val1 = params["one"];
 * val2 = params.get<int>("two");
 * val3 = params.get<int>("one");  // throws std::invalid_argument exception!
 * val4 = params.get("one", 42);  // default value 42 if conversion fails
 *                                // template argument is deduced from 2nd arg
 * if (params.has_key("four"))
 * {
 *   // this is not done because there is no key named "four":
 *   do_something();
 * }
 * params["four"] = "42";  // throws std::out_of_range exception!
 * val5 = params["four"];  // throws std::out_of_range exception!
 *                                                                      @endcode
 **/
struct parameter_map : std::map<std::string, std::string>
{
  /** Constructor.
   * All parameters are forwarded to the @c std::map constructor.
   **/
  template<typename... Args>
  explicit parameter_map(Args&&... args)
    : std::map<std::string, std::string>(std::forward<Args>(args)...)
  {}

  /** "Getter".
   * @param k Name of the parameter which should be retrieved.
   * @return const reference to the value referenced by @p k.
   * @throw std::out_of_range if the key @p k doesn't exist. You should've
   *   checked beforehand with has_key() ...
   * @see has_key(), get()
   **/
  const std::string& operator[](const std::string& k) const
  {
    try
    {
      return this->at(k);
    }
    catch (const std::out_of_range&)
    {
      throw std::out_of_range("Parameter \"" + k + "\" does not exist in map!");
    }
  }

  /** "Setter". Well, not really. It just gives you a reference where you can
   * assign stuff to.
   * @param k Name of the parameter which should be set. The parameter has to be
   * in the map already, if not, an exception is thrown! If you want to add a
   * new value, use set().
   * @return non-const reference to the value referenced by @p k.
   *   You can assign a @c std::string to actually set a new value.
   * @throw std::out_of_range if the key @p k doesn't exist yet.
   * @see has_key(), set()
   **/
  std::string& operator[](const std::string& k)
  {
    try
    {
      return this->at(k);
    }
    catch (const std::out_of_range&)
    {
      throw std::out_of_range("Parameter \"" + k + "\" does not exist in map!");
    }
  }

  /** Get value converted to given type.
   * @tparam T The desired type. Can be omitted if @p def is specified.
   * @param k Name of the parameter which should be retrieved.
   * @param def Default value for cases where conversion fails.
   * @return Value referenced by @p k, converted to type @p T. If the key @p k
   *   isn't available, or if the conversion failed, @p def is returned.
   * @warning If the result is equal to the default value @p def, there is no
   *   way to know ...
   *     - if the key was available
   *     - if the conversion was successful
   *   @par
   *   To check the former, you can use has_key(), for the latter you have to
   *   get the value as string (with operator[]()) and convert it yourself
   *   (e.g. with apf::str::S2A()).
   *   Or you can use the throwing version of get<T>().
   **/
  template<typename T>
  T get(const std::string& k, const T& def) const
  {
    try
    {
      return this->get<T>(k);
    }
    catch (const std::out_of_range&)
    {
      return def;
    }
    catch (const std::invalid_argument&)
    {
      return def;
    }
  }

  /** Overloaded function for character array (aka C-string).
   * This is mainly used to specify a string literal as default value, which
   * wouldn't work with the other get() version, e.g.
   *                                                                       @code
   * apf::parameter_map params;
   * params.set("id", "item42");
   * std::string id1, id2, name1, name2;
   * id1   = params.get("id"  , "no_id_available"); // id1   = "item42";
   * id2   = params.get("id"  , "item42");          // id2   = "item42";
   * name1 = params.get("name", "Default Name");    // name1 = "Default Name";
   * name2 = params.get("name", "");                // name2 = "";
   *                                                                    @endcode
   * @tparam T The given character type. Can be omitted if @p def is specified
   *   (which is normally the case!).
   * @param k Name of the parameter which should be retrieved.
   * @param def Default value for cases where conversion fails.
   * @return Value referenced by @p k, converted to a
   *   @c std::basic_string<char_T>. If the key @p k isn't available, or if the
   *   conversion failed, @p def is returned.
   * @warning Same warning as for the other get() version with default value.
   **/
  template<typename char_T>
  std::basic_string<char_T>
  get(const std::string& k, const char_T* const def) const
  {
    return this->get(k, std::basic_string<char_T>(def));
  }

  /** Throwing getter.
   * @tparam T Desired output type
   * @param k Name of the parameter which should be retrieved.
   * @throw std::out_of_range if the key @p k is not available
   * @throw std::invalid_argument if the content cannot be converted to @p T
   **/
  template<typename T>
  T get(const std::string& k) const
  {
    T temp;
    try
    {
      temp = str::S2RV<T>(this->operator[](k));
    }
    catch (std::invalid_argument& e)
    {
      throw std::invalid_argument(
          "parameter_map key \"" + k + "\": " + e.what());
    }
    return temp;
  }

  /** Set value.
   * @tparam T Input type
   * @param k Name of parameter to be set
   * @param v Value. Will be converted to @c std::string.
   * @return const reference to the @c std::string representation of @p v.
   *  If "" is returned, the conversion failed (or @p v was "" originally).
   **/
  template<typename T>
  const std::string& set(const std::string& k, const T& v)
  {
    return std::map<std::string, std::string>::operator[](k) = str::A2S(v);
  }

  /** Check if a given parameter is available.
   * @param k The parameter name
   * @return @b true if @p k is available
   **/
  bool has_key(const std::string& k) const
  {
    return this->count(k) > 0;
  }
};

}  // namespace apf

#endif

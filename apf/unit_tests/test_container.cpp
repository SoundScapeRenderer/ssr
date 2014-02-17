/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
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

// Tests for fixed_vector, fixed_list, fixed_matrix

#include "apf/container.h"

#include "catch/catch.hpp"

struct NonCopyableButMovable
{
  explicit NonCopyableButMovable(int x_ = 666) : x(x_) {}
  NonCopyableButMovable(NonCopyableButMovable&&) = default;

  NonCopyableButMovable(const NonCopyableButMovable&) = delete;
  NonCopyableButMovable& operator=(const NonCopyableButMovable&) = delete;

  int x;
};

struct mystruct
{
  mystruct(int one, int two) : first(one), second(two) {}
  int first, second;
};

TEST_CASE("fixed_vector", "Test fixed_vector")
{

using fvi = apf::fixed_vector<int>;

SECTION("inherited types", "")
{
  void f01(fvi::value_type);
  void f02(fvi::allocator_type);
  void f03(fvi::reference);
  void f04(fvi::const_reference);
  void f05(fvi::pointer);
  void f06(fvi::const_pointer);
  void f07(fvi::iterator);
  void f08(fvi::const_iterator);
  void f09(fvi::reverse_iterator);
  void f10(fvi::const_reverse_iterator);
  void f11(fvi::difference_type);
  void f12(fvi::size_type);
}

SECTION("inherited functions", "")
{
  fvi fv(1);

  fv.front();
  fv.back();
  fv.begin();
  fv.end();
  fv.rbegin();
  fv.rend();
  fv.cbegin();
  fv.cend();
  fv.crbegin();
  fv.crend();
  fv.size();
  fv.max_size();
  fv.capacity();
  fv.empty();
  fv[0];
  fv.at(0);
  fv.data();
  fv.get_allocator();
}

SECTION("default constructor", "")
{
  fvi fv;
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 0);
}

SECTION("default constructor and allocator", "")
{
  auto a = std::allocator<int>();
  fvi fv(a);
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 0);
}

SECTION("constructor from size", "uses default constructor")
{
  fvi fv(3);
  CHECK(fv[1] == 0);

  fvi fv2(0);
  CHECK(fv2.size() == 0);
}

// TODO: constructor from size and allocator is missing in C++11 (but not C++14)
#if 0
SECTION("constructor from size and allocator", "")
{
  auto a = std::allocator<int>();

  fvi fv(3, a);
  CHECK(fv[2] == 0);

  CHECK(fvi(0, a).size() == 0);

  // rvalue allocator
  CHECK(fvi(0, std::allocator<int>()).size() == 0);
}
#endif

SECTION("constructor from size and default value", "")
{
  fvi fv(3, 99);

  CHECK(fv[2] == 99);
}

SECTION("constructor from size and default value and allocator", "")
{
  auto a = std::allocator<int>();

  fvi fv(3, 99, a);

  CHECK(fv[2] == 99);

  // rvalue allocator
  CHECK(fvi(3, 99, std::allocator<int>())[2] == 99);
}

SECTION("constructor from size and initializer arguments", "")
{
  apf::fixed_vector<mystruct> fv(3, 4, 5);
  CHECK(fv[2].second == 5);
}

SECTION("copy constructor", "")
{
  fvi fv(3, 99);
  fvi fv2(fv);
  CHECK(fv[2] == 99);
  CHECK(fv2[2] == 99);
}

SECTION("move constructor", "")
{
  fvi fv(fvi(3, 99));

  CHECK(fv[2] == 99);
}

SECTION("constructor from initializer list", "")
{
  fvi fv{42};
  CHECK(fv.size() == 1);
  CHECK(fv[0] == 42);

  // Note: extra parentheses because of commas
  CHECK((fvi{42, 43}.size()) == 2);
  CHECK((fvi{42, 43, 44}.size()) == 3);
}

const int size = 4;
int data[size] = { 1, 2, 3, 4 };

SECTION("constructor from range", "")
{
  fvi fv(data, data+size);
  CHECK(fv[1] == 2);
  fv[1] = 100;
  CHECK(fv[1] == 100);

  CHECK(*fv.begin() == 1);
  CHECK(*fv.rbegin() == 4);

  CHECK(fv.size() == 4);
  CHECK_FALSE(fv.empty());

  CHECK(fv.front() == 1);
  CHECK(fv.back() == 4);
}

SECTION("constructor from range (const)", "")
{
  const fvi fv(data, data+4);

  CHECK(*fv.begin() == 1);
  CHECK(fv[2] == 3);

  CHECK(*fv.rbegin() == 4);

  CHECK(fv.size() == 4);
  CHECK_FALSE(fv.empty());

  CHECK(fv.front() == 1);
  CHECK(fv.back() == 4);
}

SECTION("reserve() and emplace_back()", "")
{
  fvi fv;
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 0);

  CHECK_THROWS_AS(fv.emplace_back(666), std::logic_error);

  fv.reserve(1);
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 1);

  fv.emplace_back(1);
  CHECK(fv[0] == 1);

  CHECK_THROWS_AS(fv.emplace_back(666), std::logic_error);

  CHECK_THROWS_AS(fv.reserve(42), std::logic_error);
}

SECTION("fixed_vector of non-copyable type", "")
{
  apf::fixed_vector<NonCopyableButMovable> fv(1000);
  CHECK(fv[999].x == 666);

  apf::fixed_vector<NonCopyableButMovable> fv2(1000, 42);
  CHECK(fv2[999].x == 42);
}

SECTION("fixed_vector of non-copyable type, emplace_back()", "")
{
  apf::fixed_vector<NonCopyableButMovable> fv;
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 0);

  fv.reserve(1);
  CHECK(fv.size() == 0);
  CHECK(fv.capacity() == 1);

  fv.emplace_back(27);
  CHECK(fv.front().x == 27);

  CHECK_THROWS_AS(fv.emplace_back(23), std::logic_error);
}

} // TEST_CASE fixed_vector

TEST_CASE("fixed_list", "Test fixed_list")
{

using fli = apf::fixed_list<int>;

SECTION("inherited types", "")
{
  void f01(fli::value_type);
  void f02(fli::allocator_type);
  void f03(fli::reference);
  void f04(fli::const_reference);
  void f05(fli::pointer);
  void f06(fli::const_pointer);
  void f07(fli::iterator);
  void f08(fli::const_iterator);
  void f09(fli::reverse_iterator);
  void f10(fli::const_reverse_iterator);
  void f11(fli::difference_type);
  void f12(fli::size_type);
}

SECTION("inherited functions", "")
{
  fli fl(1);
  fl.begin();
  fl.end();
  fl.rbegin();
  fl.rend();
  fl.cbegin();
  fl.cend();
  fl.crbegin();
  fl.crend();
  fl.empty();
  fl.size();
  fl.max_size();
  fl.front();
  fl.back();
  fl.get_allocator();
  fl.reverse();
  fl.sort();
}

SECTION("default constructor", "")
{
  fli fl;
  CHECK(fl.size() == 0);
}

SECTION("constructor from size", "")
{
  fli fl(3);
  CHECK(fl.size() == 3);
  CHECK(fl.front() == 0);
}

SECTION("constructor from size and initializer", "")
{
  fli fl(3, 42);
  CHECK(fl.size() == 3);
  CHECK(fl.front() == 42);
}

SECTION("constructor from size and several initializers", "")
{
  apf::fixed_list<mystruct> fl(3, 42, 25);
  CHECK(fl.size() == 3);
  CHECK(fl.front().second == 25);
}

SECTION("constructor from initializer list", "")
{
  fli fl{3, 42};
  CHECK(fl.size() == 2);
  CHECK(fl.front() == 3);
}

SECTION("constructor from sequence and more", "")
{
  int data[] = { 1, 2, 3, 4 };
  fli fl(data, data+4);
  CHECK(*fl.begin() == 1);
  CHECK(*(--fl.end()) == 4);
  CHECK(*fl.rbegin() == 4);
  CHECK(*(--fl.rend()) == 1);

  CHECK(fl.front() == 1);
  CHECK(fl.back() == 4);
  fl.front() = 100;
  CHECK(fl.front() == 100);
  fl.front() = 1;

  CHECK(fl.size() == 4);
  CHECK_FALSE(fl.empty());

  fl.move(fl.begin(), fl.end());
  CHECK(*(fl.begin()) == 2);
  CHECK(*(++fl.begin()) == 3);
  CHECK(*(++++fl.begin()) == 4);
  CHECK(*(++++++fl.begin()) == 1);

  fl.move(++fl.begin(), fl.end());
  CHECK(*(fl.begin()) == 2);
  CHECK(*(++fl.begin()) == 4);
  CHECK(*(++++fl.begin()) == 1);
  CHECK(*(++++++fl.begin()) == 3);

  fl.move(--fl.end(), fl.begin());
  CHECK(*(fl.begin()) == 3);
  CHECK(*(++fl.begin()) == 2);
  CHECK(*(++++fl.begin()) == 4);
  CHECK(*(++++++fl.begin()) == 1);

  fl.move(++fl.begin(), ++++++fl.begin(), fl.begin());
  CHECK(*(fl.begin()) == 2);
  CHECK(*(++fl.begin()) == 4);
  CHECK(*(++++fl.begin()) == 3);
  CHECK(*(++++++fl.begin()) == 1);

  const fli cfl(data, data+4);

  CHECK(cfl.front() == 1);
  CHECK(cfl.back() == 4);

  CHECK(*cfl.begin() == 1);
  CHECK(*(--cfl.end()) == 4);
  CHECK(*cfl.rbegin() == 4);
  CHECK(*(--cfl.rend()) == 1);

  CHECK(cfl.size() == 4);
  CHECK_FALSE(cfl.empty());
}

SECTION("empty()", "not really useful ...")
{
  fli fl(0);
  CHECK(fl.empty());
}

SECTION("fixed_list<NonCopyableButMovable>", "")
{
  apf::fixed_list<NonCopyableButMovable> fl(1000);
  CHECK(fl.back().x == 666);
  apf::fixed_list<NonCopyableButMovable> fl2(1000, 42);
  CHECK(fl2.back().x == 42);
}

} // TEST_CASE fixed_list

using fm = apf::fixed_matrix<int>;

TEST_CASE("fixed_matrix", "Test fixed_matrix")
{

SECTION("default constructor", "... and initialize()")
{
  fm matrix;
  CHECK(matrix.empty());
  CHECK(matrix.channels.begin() == matrix.channels.end());  // not allowed!
  CHECK(matrix.slices.begin() == matrix.slices.end());  // not allowed!

  matrix.initialize(2, 3);
  CHECK_FALSE(matrix.empty());
  CHECK(std::distance(matrix.channels.begin(), matrix.channels.end()) == 2);
  CHECK(std::distance(matrix.slices.begin(), matrix.slices.end()) == 3);
}

SECTION("the normal constructor and more", "")
{
  fm matrix(3, 2);
  CHECK_FALSE(matrix.empty());
  CHECK(std::distance(matrix.channels.begin(), matrix.channels.end()) == 3);
  CHECK(std::distance(matrix.slices.begin(), matrix.slices.end()) == 2);

  matrix.channels[2][0] = 42;

  fm matrix2(2, 3);
  matrix2.set_channels(matrix.slices);

  CHECK(matrix2.channels[0][2] == 42);
  CHECK(matrix2.slices[2][0] == 42);

  CHECK(matrix2.get_channel_ptrs()[0][2] == 42);
}

// TODO: check channels_iterator and slices_iterator

} // TEST_CASE fixed_matrix

#include <list>

struct ClassWithSublist
{
  std::list<int> sublist;
};

TEST_CASE("misc", "the rest")
{

SECTION("append_pointers()", "")
{
  apf::fixed_vector<int> v(3);
  std::list<int*> target;
  apf::append_pointers(v, target);
  CHECK(*target.begin() == &*v.begin());
}

SECTION("const append_pointers()", "")
{
  const apf::fixed_vector<int> v(3);
  std::list<const int*> target;
  apf::append_pointers(v, target);
  CHECK(*target.begin() == &*v.begin());
}

SECTION("distribute_list()", "... and undistribute_list()")
{
  std::list<int> in;
  in.push_back(1);
  in.push_back(2);
  in.push_back(3);
  apf::fixed_vector<ClassWithSublist> out(3);

  distribute_list(in, out, &ClassWithSublist::sublist);

  CHECK(in.empty() == true);
  // lists have different size -> exception:
  CHECK_THROWS_AS(distribute_list(in, out, &ClassWithSublist::sublist)
      , std::logic_error);
  CHECK(out[2].sublist.size() == 1);
  CHECK(out[2].sublist.front() == 3);

  in.clear();
  in.push_back(4);
  in.push_back(5);
  in.push_back(6);

  distribute_list(in, out, &ClassWithSublist::sublist);
  CHECK(out[2].sublist.size() == 2);
  CHECK(out[2].sublist.front() == 3);
  CHECK(out[2].sublist.back() == 6);

  CHECK(in.size() == 0);  // 'in' is empty again ...

  // For undistribute_list(), the first argument can be a different type:
  apf::fixed_vector<int> in2(3);
  in2[0] = 1;
  in2[1] = 2;
  in2[2] = 3;

  std::list<int> garbage;

  undistribute_list(in2, out, &ClassWithSublist::sublist, garbage);

  CHECK(garbage.size() == 3);
  CHECK(in2.size() == 3);
  CHECK(out[2].sublist.size() == 1);
  CHECK(out[2].sublist.front() == 6);

  in.push_back(666);

  // in and out have different size -> exception:
  CHECK_THROWS_AS(undistribute_list(in, out, &ClassWithSublist::sublist
        , garbage), std::logic_error);

  CHECK(in.size() == 1);

  in.push_back(5);
  in.push_back(6);

  // list item is not found -> exception:
  CHECK_THROWS_AS(undistribute_list(in, out, &ClassWithSublist::sublist
        , garbage), std::logic_error);

  CHECK(in.size() == 3);

  in.front() = 4;

  CHECK_NOTHROW(undistribute_list(in, out, &ClassWithSublist::sublist,garbage));
}

} // TEST_CASE misc

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent

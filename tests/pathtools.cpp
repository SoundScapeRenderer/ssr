#include "catch/catch.hpp"

#include "pathtools.h"

using Catch::Matchers::StartsWith;
using Catch::Matchers::EndsWith;

TEST_CASE("make_path_relative_to_file") {

    constexpr auto f = pathtools::make_path_relative_to_file;

    SECTION("basic usage") {
        CHECK(f("a/b", "my/conf.txt") == "../a/b");
    }

    SECTION("empty path is unchanged") {
        CHECK(f("", "my/conf.txt") == "");
        CHECK(f("", "") == "");
    }

    SECTION("absolute path is unchanged") {
        CHECK(f("/a/b", "my/conf.txt") == "/a/b");
        CHECK(f("/a/b", "") == "/a/b");
    }

    SECTION("absolute file") {
        CHECK_THAT(f("a", "/my/conf.txt"), StartsWith("..") && EndsWith("a"));
    }

    SECTION("trailing slash is preserved") {
        CHECK(f("a/b/", "my/conf.txt") == "../a/b/");
    }

    SECTION("directory must have trailing slash") {
        CHECK(f("a/b", "c/d") == "../a/b");
        CHECK(f("a/b", "c/d/") == "../../a/b");
    }

    SECTION("path is simplified") {
        CHECK(f("a//./../b", "my/conf.txt") == "../b");
    }
}

TEST_CASE("make_path_relative_to_current_dir") {

    constexpr auto f = pathtools::make_path_relative_to_current_dir;

    SECTION("basic usage") {
        CHECK(f("a/b", "my/conf.txt") == "my/a/b");
    }

    SECTION("empty path is unchanged") {
        CHECK(f("", "my/conf.txt") == "");
        CHECK(f("", "") == "");
    }

    SECTION("absolute path is unchanged") {
        CHECK(f("/a/b", "my/conf.txt") == "/a/b");
        CHECK(f("/a/b", "") == "/a/b");
    }

    SECTION("absolute file") {
        CHECK_THAT(f("a", "/my/conf.txt"), StartsWith("..") && EndsWith("a"));
    }

    SECTION("trailing slash is preserved") {
        CHECK(f("a/b/", "my/conf.txt") == "my/a/b/");
    }

    SECTION("directory must have trailing slash") {
        CHECK(f("a/b", "c/d") == "c/a/b");
        CHECK(f("a/b", "c/d/") == "c/d/a/b");
    }

    SECTION("path is simplified") {
        CHECK(f("a//./../b", "my/conf.txt") == "my/b");
    }
}

TEST_CASE("get_escaped_filename") {

    constexpr auto f = pathtools::get_escaped_filename;

    SECTION("basic usage") {
        CHECK(f("a b") == "a\\ b");
    }

    SECTION("at beginning and end") {
        CHECK(f(" a ") == "\\ a\\ ");
    }

    SECTION("tab and newline are also escaped") {
        CHECK(f("a\tb") == "a\\\tb");
        CHECK(f("a\nb") == "a\\\nb");
    }

    SECTION("punctuation characters are not escaped") {
        CHECK(f(".!:;,?$\\") == ".!:;,?$\\");
    }
}

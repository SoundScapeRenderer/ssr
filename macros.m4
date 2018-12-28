dnl Some macros for the use in configure.ac.

dnl ENABLE_FEATURE(): macro to enable/disable features
dnl Usage:
dnl ENABLE_FEATURE([auto/explicit/forced], [name], [description], [test code])
dnl
dnl "auto":     if available, activate feature.
dnl             if not available -> warning (but continue).
dnl "explicit": if specified with --enable-bla and available, activate feature.
dnl             if specified with --enable-bla and not available -> error!
dnl             if not specified, don't check.
dnl "forced":   always activate feature except if disabled with --disable-bla.
dnl             if not available -> error!
dnl
dnl If an argument ARG was given to --enable-bla=ARG, it is available as the
dnl value of $enable_bla within the test but it is deleted afterwards.
dnl
dnl The test code must set have_bla=no in case of failure.
dnl
AC_DEFUN([ENABLE_FEATURE],
[
  AC_ARG_ENABLE($2, AS_HELP_STRING(
    ifelse([$1],[explicit], [--enable-$2],            [--disable-$2]),
    ifelse([$1],[explicit], [Enable $3 (default=no)], [Disable $3])))

  dnl define FEATURE "locally", replace - by _
  pushdef([FEATURE], patsubst([$2], -, _))

  AS_IF(ifelse([$1],[explicit],
        [test x$enable_]FEATURE[ != xno -a x$enable_]FEATURE[ != x],
        [test x$enable_]FEATURE[ != xno]),
  [
    # set have_bla to "yes"
    have_]FEATURE[=yes

    # run the actual test (if a test was specified)
    $4

    # if the test passed (= have_bla is still "yes") ...
    AS_IF([test x$have_]FEATURE[ = xyes],
    [
      # ... set the preprocessor variable ENABLE_BLA to 1
      AC_DEFINE(translit([enable_]FEATURE, [a-z], [A-Z]), 1, $3)
    ],
    # else (= the test failed):
    [
      AS_IF([test x$enable_]FEATURE[ = x],
      [
        ifelse([$1], [forced],
        [
          # for failures in forced settings, we generate an error
          AC_MSG_ERROR([$2 ($3) not available! Use --disable-$2 to deactivate.])
        ],
        [
          # for failures in default (auto) settings, we generate a warning
          AC_MSG_WARN( [--enable-$2 ($3) requested but not available!])
        ])
      ],
      [
        # for failures in explicit settings, we generate an error
        AC_MSG_ERROR([--enable-$2 ($3) requested but not available!])
      ])
    ])
  ],
  [
    have_]FEATURE[=no
  ])

  # undefine enable_bla because it only creates confusion, use have_bla instead
  AS_UNSET([enable_]FEATURE)

  # set automake conditional ENABLE_BLA (for Makefile.am)
  AM_CONDITIONAL(translit([enable_]FEATURE, [a-z], [A-Z]),
    [test x$have_]FEATURE[ = xyes])

  popdef([FEATURE])
])

dnl ENABLE_AUTO(): autoconf macro for configure options (default=auto)
dnl Usage: ENABLE_AUTO([name of feature], [description], [test for feature])
dnl
dnl A test for the given feature can be specified as (optional) third parameter
dnl (e.g. some shell code or another macro).
dnl The only thing it must do is "have_<name>=no" if the feature is not possible
dnl
dnl Examples:
dnl   ENABLE_AUTO([bla], [support for bla-feature],
dnl               AC_CHECK_HEADER([bla.h], , [have_bla=no]))
dnl
dnl The macro provides several things:
dnl 1) a configure switch --disable-bla including help text (second parameter)
dnl 2) an autoconf variable $have_bla (yes/no)
dnl 3) a preprocessor define ENABLE_BLA
dnl 4) an automake conditional ENABLE_BLA (use in Makefile.am)
dnl
dnl The autoconf variable $enable_bla is deleted afterwards because it only
dnl causes confusion (former values: yes/no/<empty>/<ARG>).
dnl
dnl Hyphens in "name" will be changed to underscores, e.g.:
dnl all-goodies --> $have_all_goodies, ENABLE_ALL_GOODIES
dnl
dnl If a test fails, a warning message is shown in case of the default actions,
dnl in case of failed explicit choices, an error message is shown.

dnl ENABLE_EXPLICIT(): autoconf macro for configure options (default=no)
dnl Usage: ENABLE_EXPLICIT([name of feature], [description], [test for feature])
dnl
dnl The macro provides several things:
dnl 1) a configure switch --enable-bla including help text (second parameter)
dnl
dnl For the rest of the features see ENABLE_AUTO() above.

dnl ENABLE_FORCED(): autoconf macro for configure options (default=yes)
dnl Same as ENABLE_AUTO, except if the feature is not available, it's an error.

AC_DEFUN([ENABLE_AUTO],     [ENABLE_FEATURE([auto],     [[$1]], [[$2]], [$3])])
AC_DEFUN([ENABLE_EXPLICIT], [ENABLE_FEATURE([explicit], [[$1]], [[$2]], [$3])])
AC_DEFUN([ENABLE_FORCED],   [ENABLE_FEATURE([forced],   [[$1]], [[$2]], [$3])])

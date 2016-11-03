# Makefile to generate externals for Puredata

# see also package.txt

# The flext build system doesn't support compiling multiple externals at once,
# so we work around this with PD_TARGETS and specify NAME here.

EXTERNALS ?= ssr_binaural ssr_nfc_hoa ssr_aap ssr_wfs ssr_vbap

PD_TARGETS := $(EXTERNALS:%=%_pd)
MAX_TARGETS := $(EXTERNALS:%=%_max)

FLEXTPATH ?= /usr/local/src/flext

all: pd

pd: $(PD_TARGETS)

max: $(MAX_TARGETS)

%_pd: %.cpp check_flext_path
	NAME='$(@:%_pd=%~)' MAIN_SOURCE='$<' $(FLEXTPATH)/build.sh pd gcc

%_max: %.cpp check_flext_path
	NAME='$(@:%_max=%~)' MAIN_SOURCE='$<' $(FLEXTPATH)/build.sh max gcc

check_flext_path:
	@test -n '$(FLEXTPATH)' || ( echo \"FLEXTPATH\" is empty! ; false )
	@test -d '$(FLEXTPATH)' || \
		( echo \"$(FLEXTPATH)\" not found! Set FLEXTPATH! ; false )

clean: $(EXTERNALS:%=%_pd_clean) $(EXTERNALS:%=%_max_clean)

%_pd_clean: check_flext_path
	NAME='$(@:%_pd_clean=%~)' $(FLEXTPATH)/build.sh pd gcc clean
	@rmdir pd-linux 2> /dev/null || true

%_max_clean: check_flext_path
	NAME='$(@:%_max_clean=%~)' $(FLEXTPATH)/build.sh max gcc clean
	@rmdir max-darwin 2> /dev/null || true

.PHONY: all pd max clean check_flext_path

# TODO: When adding $(PD_TARGETS) to .PHONY, it ceases to work ... why?


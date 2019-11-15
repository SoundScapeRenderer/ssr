## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

BROWSER_GUI_FILES = package.json yarn.lock webpack.config.js README.md \
	src/index.js \
	src/sceneviewer.js \
	src/style.css

EXTRA_DIST = $(BROWSER_GUI_FILES)

if ENABLE_BROWSER_GUI

export PATH := $(PATH):$(abs_builddir)/node_modules/.bin
export NODE_PATH := $(abs_builddir)/node_modules

DIST_PATH = $(abs_builddir)/webpack-dist

YARN_OPTIONS = --no-progress --non-interactive \
	--cwd $(abs_srcdir) \
	--modules-folder $(NODE_PATH)

all-local: $(DIST_PATH)/index.html

$(DIST_PATH)/index.html: $(BROWSER_GUI_FILES)
	$(YARN) $(YARN_OPTIONS) install
	$(YARN) $(YARN_OPTIONS) run build --output-path $(DIST_PATH)

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/websocket_resources/sourcemaps
	$(INSTALL) -m 644 \
		$(DIST_PATH)/*.html \
		$(DIST_PATH)/*.js \
		$(DESTDIR)$(pkgdatadir)/websocket_resources
	$(INSTALL) -m 644 \
		$(DIST_PATH)/sourcemaps/*.js.map \
		$(DESTDIR)$(pkgdatadir)/websocket_resources/sourcemaps

clean-local:
	$(RM) -r $(DIST_PATH)

distclean-local:
	$(RM) -r $(NODE_PATH)

uninstall-local:
	$(RM) -r $(DESTDIR)$(pkgdatadir)/websocket_resources

endif

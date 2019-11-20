## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

if ENABLE_BROWSER_GUI

BROWSER_GUI_FILES = package.json yarn.lock webpack.config.js README.md \
	src/index.js \
	src/sceneviewer.js \
	src/style.css

dist_noinst_DATA = $(BROWSER_GUI_FILES)

export PATH := $(PATH):$(abs_builddir)/node_modules/.bin
export NODE_PATH := $(abs_builddir)/node_modules

YARN_OPTIONS = --no-progress --non-interactive \
	--cwd $(abs_srcdir) \
	--modules-folder $(NODE_PATH)

WEBPACK_PATH = $(abs_builddir)/../data/websocket_resources
INSTALL_PATH = $(DESTDIR)$(pkgdatadir)/websocket_resources

all-local: $(WEBPACK_PATH)/index.html

$(WEBPACK_PATH)/index.html: $(BROWSER_GUI_FILES)
	$(RM) -r $(WEBPACK_PATH)/chunks
	$(YARN) $(YARN_OPTIONS) install
	$(YARN) $(YARN_OPTIONS) run build --output-path $(WEBPACK_PATH)

install-data-hook:
	$(MKDIR_P) $(INSTALL_PATH)/chunks/sourcemaps
	$(INSTALL) -m 644 $(WEBPACK_PATH)/index.html $(INSTALL_PATH)
	$(INSTALL) -m 644 $(WEBPACK_PATH)/chunks/* $(INSTALL_PATH)/chunks

distclean-local:
	$(RM) -r $(NODE_PATH)
	$(RM) -r $(WEBPACK_PATH)/index.html
	$(RM) -r $(WEBPACK_PATH)/chunks

uninstall-local:
	$(RM) -r $(INSTALL_PATH)

endif

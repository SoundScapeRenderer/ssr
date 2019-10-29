Browser-based 3D GUI for the SSR
================================

To use this, you need to install the [Yarn] package manager.
Note that the official Debian/Ubuntu package is called `yarnpkg` and the
executable is also called `yarnpkg` and not `yarn`!

[Yarn]: https://yarnpkg.com/en/docs/install

If `yarn` (or `yarnpkg`) is available, the GUI resources are automatically
created when running `make` and installed with `make install`.
If you want to make using the GUI explicit, use

    ./configure --enable-browser-gui

Given that the WebSocket interface is enabled, the SSR will automatically serve
the resources for the 3D GUI.
Simply connect with a browser to http://localhost:9422/index.html.

You can change the server port with the option `--websocket-server=9999`
(or whatever port you want to use).

Development Build
-----------------

If you want to make changes to the source files, you can also build the GUI
files locally.

To install all necessary `npm` packages:

    yarn install

To build the GUI:

    yarn run build

During development, it is very handy if any change to the source files is
detected and the generated files are updated automatically.
You can achieve that by running:

    yarn run serve

This will open your favorite web browser and show the freshly built SSR GUI.
And it will automatically update the browser whenever changes are made to any
source file.
If you want to use a different browser, just pass it along, e.g.:

    yarn run serve chromium

Maintenance
-----------

Just for the record, the original `package.json` was:

```json
{
  "private": true,
  "scripts": {
    "serve": "webpack-dev-server --open",
    "build": "webpack"
  }
}
```

We don't intend to publish the SSR GUI as a package on https://www.npmjs.com/,
therefore we have used `"private": true` and we didn't have to specify most of
the fields a normal `npm` package has.
The only thing we needed to get started, are the settings for [webpack].

Speaking of which, [webpack] was installed with:

    yarn add --dev webpack webpack-cli webpack-dev-server
    yarn add --dev imports-loader style-loader css-loader

And we needed [three.js] for all the fancy 3D stuff:

    yarn add three

[webpack]: https://webpack.js.org/
[three.js]: https://threejs.org/

Several other packages are needed, `yarn run build` will mention those.

Whenever needed, packages can be upgraded with `yarn upgrade`,
added with `yarn add` (using `--dev` for packages only needed for development)
and removed with `yarn remove`.

Note: The file `yarn.lock` is supposed to be checked into version control.

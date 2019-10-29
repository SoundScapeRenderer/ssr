const path = require('path');
const webpack = require('webpack');
const CleanWebpackPlugin = require('clean-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const DIST_PATH = 'dist';
const NODE_MODULES = process.env.NODE_PATH || path.join(__dirname, 'node_modules');

module.exports = {
  entry: './src/index.js',
  output: {
    filename: '[name].[contenthash].js',
    // When called from the Makefile, this is ignored:
    path: path.resolve(__dirname, DIST_PATH),
    sourceMapFilename: 'sourcemaps/[file].map',
  },
  target: 'web',
  module: {
    rules: [
      {
        test: /\.css$/,
        use: [
          'style-loader',
          'css-loader'
        ]
      },
      {
        // See https://stackoverflow.com/a/44960831/
        test: /three\/examples\/js/,
        use: 'imports-loader?THREE=three',
      }
    ],
  },
  resolve: {
    modules: [NODE_MODULES],
    alias: {
      // See https://stackoverflow.com/a/44960831/
      'three-examples': path.join(NODE_MODULES, 'three/examples/js')
    },
  },
  resolveLoader: {
    modules: [NODE_MODULES],
  },
  plugins: [
    new CleanWebpackPlugin([DIST_PATH]),
    new HtmlWebpackPlugin({
      title: 'SoundScape Renderer'
    }),
  ],
  devtool: 'source-map',
  devServer: {
    contentBase: DIST_PATH
  },
  mode: 'production',
  optimization: {
    moduleIds: 'hashed',
    runtimeChunk: 'single',  // Separate chunk for webpack runtime
    //runtimeChunk: true,  // Separate chunk for webpack runtime (per entry)
    splitChunks: {
      chunks: 'all',
      //chunks: 'initial',
    }
  },
  performance: {
    hints: false  // three.js is always too large
  }
};

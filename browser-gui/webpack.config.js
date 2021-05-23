const path = require('path');
const webpack = require('webpack');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const NODE_MODULES = process.env.NODE_PATH || path.join(__dirname, 'node_modules');

module.exports = {
  entry: './src/index.js',
  output: {
    filename: 'chunks/[name].[contenthash].js',
    // This is set when called from the Makefile:
    path: undefined,
    sourceMapFilename: '[file].map',
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
      }
    ],
  },
  resolve: {
    modules: [NODE_MODULES],
  },
  resolveLoader: {
    modules: [NODE_MODULES],
  },
  plugins: [
    new HtmlWebpackPlugin({
      title: 'SoundScape Renderer',
      meta: {
        viewport: 'width=device-width, user-scalable=no, initial-scale=1, maximum-scale=1, shrink-to-fit=no',
      }
    }),
  ],
  devtool: 'source-map',
  mode: 'production',
  optimization: {
    runtimeChunk: 'single',  // Separate chunk for webpack runtime
    splitChunks: {
      cacheGroups: {
        vendor: {
          test: /[\\/]node_modules[\\/]/,
          name: 'vendors',
          chunks: 'all',
        },
      }
    }
  },
  performance: {
    hints: false  // three.js is always too large
  }
};

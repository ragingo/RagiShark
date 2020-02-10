const Path = require('path');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const DuplicatePackageCheckerPlugin = require("duplicate-package-checker-webpack-plugin");
const WebpackBar = require('webpackbar');

const rootDir = `${__dirname}/../`
const getFullPath = (path) => Path.resolve(rootDir, path);

module.exports = {
  entry: {
    'app': [
      getFullPath('src/index.tsx')
    ]
  },
  output: {
    path: getFullPath('dist'),
    filename: '[name].js'
  },
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        enforce: 'pre',
        use: [
          {
            loader: 'tslint-loader',
            options: {
              configFile: getFullPath('conf/tslint.jsonc'),
              fix: true
            }
          }
        ]
      },
      {
        test: /\.tsx?$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'babel-loader',
            options: {
              configFile: getFullPath('conf/babel.config.js')
            }
          }
        ]
      },
      { test: /\.html$/, exclude: getFullPath('src/index.html'), loader: 'html-loader' },
      { test: /\.scss$/, use: ['style-loader', 'css-loader', 'sass-loader'], sideEffects: true }
    ]
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.js'],
  },
  plugins: loadPlugins(),
  devServer: {
    contentBase: getFullPath('src'),
    hot: true,
    open: true,
    port: 3001,
    progress: true,
    stats: {
      normal: true
    },
    watchContentBase: true,
  }
}

function loadPlugins() {
  let plugins = [
    new WebpackBar(),
    new CleanWebpackPlugin(),
    new CopyWebpackPlugin([
      { from: getFullPath('./src/index.html'), to: '' },
    ]),
    loadPlugin(process.env.BUILD_ANALYZE, () => new DuplicatePackageCheckerPlugin({ verbose: true, emitError: true })),
  ];
  return plugins.filter(x => x !== null);
}

function loadPlugin(flag, func) {
  if (!func || typeof func !== 'function') {
    return null;
  }
  if (flag !== '1') {
    return null;
  }
  const value = func();
  if (!value || typeof value !== 'object') {
    return null;
  }
  return value;
}

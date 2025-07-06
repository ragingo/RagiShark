const Path = require('path');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const Dotenv = require('dotenv-webpack');
const DuplicatePackageCheckerPlugin = require("duplicate-package-checker-webpack-plugin");

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
      { test: /\.scss$/, use: [
          'style-loader',
          'css-loader',
          {
            loader: 'sass-loader',
            options: {
              implementation: require('sass'),
              api: 'modern', // legacy JS API警告を抑制
            },
          },
        ], sideEffects: true }
    ]
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.js'],
  },
  plugins: loadPlugins(),
  devServer: {
    static: {
      directory: getFullPath('src'),
      watch: true,
    },
    hot: true,
    open: true,
    port: 3001,
    client: {
      logging: 'info'
    }
  }
}

function loadPlugins() {
  let plugins = [
    new Dotenv(),
    new CleanWebpackPlugin(),
    new CopyWebpackPlugin({
      patterns: [
        { from: getFullPath('./src/index.html'), to: '' }
      ]
    }),
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

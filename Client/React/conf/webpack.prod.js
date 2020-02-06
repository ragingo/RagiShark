const merge = require('webpack-merge');
const common = require('./webpack.common.js');
const TerserPlugin = require('terser-webpack-plugin');

module.exports = merge(common, {
  mode: 'production',
  optimization: {
    concatenateModules: true,
    minimizer: [
      new TerserPlugin({
        cache: true,
        extractComments: 'all',
        parallel: true,
        terserOptions: {
          compress: {
            drop_console: true,
          },
        },
      }),
    ],
    nodeEnv: 'production',
    removeAvailableModules: true,
    splitChunks: {
      name: 'vendor',
      chunks: 'initial',
    }
  }
});

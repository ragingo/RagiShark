'use strict';

const presets = [
  ['@babel/preset-env'],
  ['@babel/preset-typescript', {
    isTSX: true,
    allExtensions: true,
  }],
  ['@babel/preset-react', {
    development: false
  }]
];

module.exports = {
  presets
};

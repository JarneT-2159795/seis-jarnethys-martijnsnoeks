const CopyPlugin = require('copy-webpack-plugin')
const path = require('path')
const spawn = require('child_process').spawnSync

module.exports = {
  context: path.resolve(__dirname, '.'),
  devtool: 'nosources-source-map',
  entry: './index.js',
  target: 'webworker',
  plugins: [
    new CopyPlugin([
      { from: './build/test.wasm', to: './worker/test.wasm' },
    ]),
  ],
  module: {
    rules: [
      {
        test: /emscripten\.js$/,
        loader: 'exports-loader',
      },
    ],
  },
}

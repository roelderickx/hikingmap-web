# hikingmap-web

This is the PHP version of hikingmap. It aims at providing a web interface to hikingmap, eliminating the need to setup a database and cartoCSS stylesheet.

## Installation instructions

Configuration needs to be done in configuration.js. The most important setting is the tileserver which is set to localhost by default. Since this requires mod_tile and renderd to be installed on the users local machine it you might choose to change this setting to null, which defaults to OpenStreetMap.
Depending on the use case this might violate [the OpenStreetMap usage policy](https://operations.osmfoundation.org/policies/tiles/), although this will probably not be the case for a typical user.

To install put all the files on a webserver, no additional installation steps are required.

## Limitations

This repository is still in a testing phase, though it may already be useful to you. The python code is fully converted to PHP and the rendering is replaced by downloaded tiles from a server.
However, the scaling is not yet correct. The math works but it is unclear how the boundaries of a map can be set with the OpenLayers library.


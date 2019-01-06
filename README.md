# hikingmap-web

This is the PHP version of hikingmap. It aims at providing a web interface to hikingmap, eliminating the need to setup a database and cartoCSS stylesheet.

## Installation instructions

There are a number of configuration settings which you can find in src/configuration.js. However, hikingmap-web uses the compressed version in hikingmap.min.js, you either have to modify the compressed file or you have to regenerate this file from src/configuration.js and src/hikingmap.js using [UglifyJS](https://github.com/mishoo/UglifyJS2) after each modification.

The most important setting is the tileserver which is set to localhost by default. Since this requires [mod_tile and renderd](https://switch2osm.org/manually-building-a-tile-server-16-04-2-lts/) to be installed on the users local machine you might choose to change this setting to null, which defaults to OpenStreetMap.
Depending on the use case this might violate [the OpenStreetMap usage policy](https://operations.osmfoundation.org/policies/tiles/), although this will almost certainly not be the case for a typical user.

To install copy all php, inc, js and css files from the main folder in this repository to a webserver. No additional installation steps are required.


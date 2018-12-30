# hikingmap-web

This is the PHP version of hikingmap. It aims at providing a web interface to hikingmap, eliminating the need to setup a database and cartoCSS stylesheet.

## Limitations

This repository is still in a testing phase, though it may already be useful to you. The python code is almost fully converted to PHP and the rendering is replaced by OSM map tiles served on openstreetmap.org. However, due to [the usage policy](https://operations.osmfoundation.org/policies/tiles/) only a small preview of each page is rendered.
Also, the scaling is not yet correct. The math works but it is unclear how the boundaries of a map can be set with the OpenLayers library.


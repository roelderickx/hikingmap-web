/*
 hikingmap-web - render maps on paper using data from OpenStreetMap
 Copyright (C) 2019  Roel Derickx <roel.derickx AT gmail>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// OSM tile server
//tileserver = null;
// tile server on localhost
tileserver = "/osm_tiles/${z}/${x}/${y}.png";

page_border_color = "red";
page_border_width = 2;

track_color = "green";
track_width = 5;
track_opacity = 0.5;

waypoint_font = "Arial, Helvetica, sans-serif";
waypoint_font_size = 12;
waypoint_font_color = "black";
waypoint_outline_width = 1;
waypoint_outline_color = "white";


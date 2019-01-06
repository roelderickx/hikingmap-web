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

// functions get_amount_tracks() and get_track(index) are generated in tracks.inc

function add_tracks(map)
{
    for (var i = 0; i < get_amount_tracks(); i++)
    {
        var tracklayer = new OpenLayers.Layer.Vector("track_" + i);
        tracklayer.style = { strokeColor: track_color, strokeWidth: track_width, strokeOpacity: track_opacity };
        tracklayer.projection = new OpenLayers.Projection("EPSG:4326");
        var format = new OpenLayers.Format.EncodedPolyline({geometryType:"linestring"});
        tracklayer.addFeatures(new OpenLayers.Feature.Vector(format.read(get_track(i)).geometry.transform('EPSG:4326', 'EPSG:3857')));
        map.addLayer(tracklayer);
    }
}

// functions get_amount_waypoints() and get_waypoints(index) are generated in tracks.inc

function add_waypoints(map)
{
    for (var i = 0; i < get_amount_waypoints(); i++)
    {
        var wptlayer = new OpenLayers.Layer.Vector("wpt_" + i);
        wptlayer.projection = new OpenLayers.Projection("EPSG:4326");
        var format = new OpenLayers.Format.EncodedPolyline({geometryType:"multipoint"});
        var features = format.read(get_waypoints(i)).geometry.transform('EPSG:4326', 'EPSG:3857').getVertices();
        for (var j = 0; j < features.length; j++)
        {
            var feature = new OpenLayers.Feature.Vector(features[j]);
            feature.style = { label: "" + j, fontFamily: waypoint_font, fontSize: waypoint_font_size, fontColor: waypoint_font_color, labelOutlineWidth: waypoint_outline_width, labelOutlineColor: waypoint_outline_color }
            wptlayer.addFeatures(feature);
        }
        map.addLayer(wptlayer);
    }
}

// function get_page_extents() is generated in trackfinder.inc

var dragstart = null;

function add_pages(map)
{
    var page_extents = get_page_extents();
    var pages = new OpenLayers.Layer.Boxes("Pages");
    for (var i = 0; i < page_extents.length; i++)
    {
        bounds = OpenLayers.Bounds.fromArray(page_extents[i]).transform("EPSG:4326", "EPSG:900913");
        page = new OpenLayers.Marker.Box(bounds);
        page.events.register("mousedown", page, function (e) {
            dragstart = { x: e.screenX, y: e.screenY }
            map.events.handleBrowserEvent(e);
            return true;
        });
        page.events.register("mouseup", page, function (e) {
            if (dragstart &&
                (Math.abs(dragstart.x - e.screenX) < 5 ||
                 Math.abs(dragstart.y - e.screenY) < 5))
            {
                dragstart = null;
                show_page(this.bounds);
            }
            else
            {
                dragstart = null;
            }
            map.events.handleBrowserEvent(e);
            return true;
        });
        page.setBorder(page_border_color, page_border_width);
        pages.addMarker(page);
    }
    map.addLayer(pages);
}

function get_max_extent()
{
    return new OpenLayers.Bounds(-20037508.342789244, -20037508.342789244, 20037508.342789244,  20037508.342789244);
}

function get_max_resolution()
{
    tile_size = 256;
    project_ext = get_max_extent();
    return Math.min(Math.abs(project_ext.right - project_ext.left) / tile_size,
                    Math.abs(project_ext.top - project_ext.bottom) / tile_size);
}

function get_optimal_zoom(mapElementId, minLonLat, maxLonLat)
{
    viewport_width = document.getElementById(mapElementId).clientWidth;
    viewport_height = document.getElementById(mapElementId).clientHeight;
    
    max_res = get_max_resolution();
    zoom = 19;
    do
    {
        zoom -= 1;
        res = max_res / Math.pow(2, zoom);
    }
    while (zoom > 0 &&
           ((maxLonLat.lon - minLonLat.lon) / res > viewport_width ||
            (maxLonLat.lat - minLonLat.lat) / res > viewport_height)
          );
    
    return zoom;
}

function show_overview(minlon, minlat, maxlon, maxlat) {
    var options =
    {
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326"),
        units: "m",
        maxResolution: get_max_resolution(),
        maxExtent: get_max_extent(),
        numZoomLevels: 20,
        controls: [
            new OpenLayers.Control.Navigation(),
            new OpenLayers.Control.PanZoomBar(),
            new OpenLayers.Control.Permalink(),
            new OpenLayers.Control.ScaleLine(),
            new OpenLayers.Control.MousePosition(),
            new OpenLayers.Control.KeyboardDefaults(),
            new OpenLayers.Control.Attribution()
        ]
    };

    map = new OpenLayers.Map("OverviewMap", options);
    var maplayer = new OpenLayers.Layer.OSM("Default", tileserver, {numZoomLevels: 19});
    map.addLayer(maplayer);
    map.zoomIn();

    add_tracks(map);
    add_pages(map);

    var centerLonLat = new OpenLayers.LonLat((minlon + maxlon) / 2, (minlat + maxlat) / 2).transform(new OpenLayers.Projection("EPSG:4326"), map.getProjectionObject());
    var minLonLat = new OpenLayers.LonLat(minlon, minlat).transform(new OpenLayers.Projection("EPSG:4326"), map.getProjectionObject());
    var maxLonLat = new OpenLayers.LonLat(maxlon, maxlat).transform(new OpenLayers.Projection("EPSG:4326"), map.getProjectionObject());
    var zoom = get_optimal_zoom("OverviewMap", minLonLat, maxLonLat);
    map.setCenter(centerLonLat, zoom);
}

function show_page(bounds)
{
    document.getElementById('DetailMap').style.display = 'block';
    document.getElementById('DetailMap').innerHTML = "<div style=\"align=right\"><a href=\"javascript:void(0)\" onclick=\"close_page()\">Close</a>";
    document.getElementById('fade').style.display = 'block'
  
    var options =
    {
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326"),
        units: "m",
        maxResolution: get_max_resolution(),
        maxExtent: get_max_extent(),
        numZoomLevels: 20,
        controls: [
          new OpenLayers.Control.MousePosition(),
          new OpenLayers.Control.KeyboardDefaults()
        ]
    };
  
    map = new OpenLayers.Map("DetailMap", options);
    var maplayer = new OpenLayers.Layer.OSM("Default", tileserver);
    map.addLayer(maplayer);

    add_tracks(map);
    add_waypoints(map);

    // TODO calc center and zoom
    var lonLat = bounds.getCenterLonLat();
    map.setCenter(lonLat, 12);
}

function close_page()
{
    document.getElementById('DetailMap').style.display='none';
    document.getElementById('fade').style.display='none';
}


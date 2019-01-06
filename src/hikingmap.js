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

function add_pages(map, paper_width_cm, paper_height_cm, dpi)
{
    var page_extents = get_page_extents();
    var pages = new OpenLayers.Layer.Boxes("Pages");
    for (var i = 0; i < page_extents.length; i++)
    {
        bounds = OpenLayers.Bounds.fromArray(page_extents[i]).transform("EPSG:4326", "EPSG:900913");
        page = new OpenLayers.Marker.Box(bounds);
        page.events.register("mouseup", page, function (e) {
            if (!map.dragging)
            {
                show_page(this.bounds, paper_width_cm, paper_height_cm, dpi);
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

function get_optimal_zoom(map_bounds, viewport_width, viewport_height)
{
    max_res = get_max_resolution();
    zoom_width = Math.log((viewport_width * max_res) /
                          (map_bounds.right - map_bounds.left)) /
                 Math.log(2);
    zoom_height = Math.log((viewport_height * max_res) /
                           (map_bounds.top - map_bounds.bottom)) /
                  Math.log(2);
    
    return Math.floor(Math.min(zoom_width, zoom_height));
}

function show_overview(minlon, minlat, maxlon, maxlat, paper_width_cm, paper_height_cm, dpi)
{
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
    add_pages(map, paper_width_cm, paper_height_cm, dpi);

    var map_bounds = new OpenLayers.Bounds(minlon, minlat, maxlon, maxlat).transform(new OpenLayers.Projection("EPSG:4326"), map.getProjectionObject());
    
    viewport_width = document.getElementById("OverviewMap").clientWidth;
    viewport_height = document.getElementById("OverviewMap").clientHeight;
    var zoom = get_optimal_zoom(map_bounds, viewport_width, viewport_height);
    
    map.setCenter(map_bounds.getCenterLonLat(), zoom);
}

function show_page(bounds, paper_width_cm, paper_height_cm, dpi)
{
    document.getElementById('DetailMapOverlay').style.display = 'block';
    document.getElementById('DetailMap').innerHTML = "";
    document.getElementById('fade').style.display = 'block';
  
    var options =
    {
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326"),
        units: "m",
        maxResolution: get_max_resolution(),
        maxExtent: get_max_extent(),
        numZoomLevels: 20,
        controls: [
            new OpenLayers.Control.ScaleLine()
        ]
    };
    
    preferred_xsize = Math.floor((paper_width_cm / 2.54) * dpi);
    preferred_ysize = Math.floor((paper_height_cm / 2.54) * dpi);
    // test works only in EPSG:900913 projection
    if ((bounds.right - bounds.left) > (bounds.top - bounds.bottom))
    {
        temp = preferred_xsize;
        preferred_xsize = preferred_ysize;
        preferred_ysize = temp;
    }
    zoom = get_optimal_zoom(bounds, preferred_xsize, preferred_ysize);
    
    // recalculate x and y based on zoom level -- real dpi will be slightly lower
    res = get_max_resolution() / Math.pow(2, zoom);
    xsize = Math.floor((bounds.right - bounds.left) / res);
    ysize = Math.floor((bounds.top - bounds.bottom) / res);
    
    document.getElementById('DetailMap').setAttribute("style", "width:" + xsize + "px;height:" + ysize + "px");
    document.getElementById('DetailMap').style.width = "" + xsize + "px";
    document.getElementById('DetailMap').style.height = "" + ysize + "px";
    
    map = new OpenLayers.Map("DetailMap", options);
    var maplayer = new OpenLayers.Layer.OSM("Default", tileserver);
    map.addLayer(maplayer);

    add_tracks(map);
    add_waypoints(map);

    map.setCenter(bounds.getCenterLonLat(), zoom);
}

function close_page()
{
    document.getElementById('DetailMapOverlay').style.display='none';
    document.getElementById('fade').style.display='none';
}


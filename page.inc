<?php
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

include_once 'area.inc';
include_once 'coordinate.inc';

class Page extends Area
{
    public $orientation_unknown = 0;
    public $orientation_portrait = 1;
    public $orientation_landscape = 2;
    
    private $pageindex;
    private $scale;
    private $pagewidth;
    private $pageheight;
    private $pageoverlap;
    private $orientation;
    private $track_area;
    private $pagesizelon;
    private $pagesizelat;
    private $pagesizelon_full;
    private $pagesizelat_full;
    
    function __construct($parameters, $pageindex)
    {
        parent::__construct(new Coordinate(0.0, 0.0), new Coordinate(0.0, 0.0));
        $this->pageindex = $pageindex;
        $this->scale = $parameters->get_scale();
        $this->pagewidth = $parameters->get_pagewidth();
        $this->pageheight = $parameters->get_pageheight();
        $this->pageoverlap = $parameters->get_pageoverlap();
        $this->set_orientation($this->orientation_unknown);
        $this->track_area = new Area(new Coordinate(0.0, 0.0), new Coordinate(0.0, 0.0));
    }
    
    function get_page_width()
    {
        if ($this->orientation == $this->orientation_landscape)
        {
            return max($this->pagewidth, $this->pageheight);
        }
        else
        {
            return min($this->pagewidth, $this->pageheight);
        }
    }
    
    function get_page_height()
    {
        if ($this->orientation == $this->orientation_landscape)
        {
            return min($this->pagewidth, $this->pageheight);
        }
        else
        {
            return max($this->pagewidth, $this->pageheight);
        }
    }
    
    function get_pageindex()
    {
        return $this->pageindex;
    }
    
    function set_page_area($area)
    {
        $this->minlon = $area->minlon;
        $this->minlat = $area->minlat;
        $this->maxlon = $area->maxlon;
        $this->maxlat = $area->maxlat;
    }
    
    function set_orientation($orientation)
    {
        $this->orientation = $orientation;
        $this->pagesizelon =
            $this->_convert_cm_to_degrees_lon($this->get_page_width() - $this->pageoverlap,
                                              $this->scale, $this->minlat);
        $this->pagesizelat =
            $this->_convert_cm_to_degrees_lat($this->get_page_height() - $this->pageoverlap,
                                              $this->scale);
        $this->pagesizelon_full =
            $this->_convert_cm_to_degrees_lon($this->get_page_width(),
                                              $this->scale, $this->minlat);
        $this->pagesizelat_full =
            $this->_convert_cm_to_degrees_lat($this->get_page_height(), $this->scale);
    }
    
    function initialize_first_point($coord)
    {
        $this->track_area = new Area($coord, $coord);
        $this->set_orientation($this->orientation_unknown);
    }
    
    function __track_area_outside_page()
    {
        $lon_outside = max($this->track_area->sizelon() - $this->pagesizelon, 0);
        $lat_outside = max($this->track_area->sizelat() - $this->pagesizelat, 0);
        
        return $lon_outside * $this->track_area->sizelat() +
               $lat_outside * $this->track_area->sizelon() -
               $lon_outside * $lat_outside;
    }
    
    function add_next_point($prev_coord, $coord)
    {
        if ($coord->get_lon() < $this->track_area->minlon)
        {
            $this->track_area->minlon = $coord->get_lon();
            $this->maxlon = $this->track_area->maxlon;
            $this->minlon = $this->maxlon - $this->pagesizelon;
        }
        else if ($coord->get_lon() > $this->track_area->maxlon)
        {
            $this->track_area->maxlon = $coord->get_lon();
            $this->minlon = $this->track_area->minlon;
            $this->maxlon = $this->minlon + $this->pagesizelon;
        }
        if ($coord->get_lat() < $this->track_area->minlat)
        {
            $this->track_area->minlat = $coord->get_lat();
            $this->maxlat = $this->track_area->maxlat;
            $this->minlat = $this->maxlat - $this->pagesizelat;
        }
        else if ($coord->get_lat() > $this->track_area->maxlat)
        {
            $this->track_area->maxlat = $coord->get_lat();
            $this->minlat = $this->track_area->minlat;
            $this->maxlat = $this->minlat + $this->pagesizelat;
        }
        $outside_page = false;
        if ($this->orientation == $this->orientation_unknown)
        {
            $this->set_orientation($this->orientation_portrait);
            $portrait_excess = $this->__track_area_outside_page();
            $this->set_orientation($this->orientation_landscape);
            $landscape_excess = $this->__track_area_outside_page();
            
            if ($portrait_excess == 0 && $landscape_excess == 0)
            {
                $this->set_orientation($this->orientation_unknown);
            }
            else if ($portrait_excess < $landscape_excess)
            {
                $this->set_orientation($this->orientation_portrait);
                $outside_page = ($portrait_excess * $landscape_excess) > 0;
            }
            else if ($portrait_excess > $landscape_excess)
            {
                $this->set_orientation($this->orientation_landscape);
                $outside_page = ($portrait_excess * $landscape_excess) > 0;
            }
        }
        else
        {
            $outside_page = $this->__track_area_outside_page() > 0;
        }
        return $outside_page;
    }
    
    function __calc_intersection_lon($l_start, $l_end, $lon)
    {
        if ($l_start->get_lon() == $l_end->get_lon())
        {
            return null;
        }
        else
        {
            return new Coordinate($lon,
                                  ($l_end->get_lat() - $l_start->get_lat()) /
                                  ($l_end->get_lon() - $l_start->get_lon()) *
                                  ($lon - $l_start->get_lon()) + $l_start->get_lat());
        }
    }
    
    function __calc_intersection_lat($l_start, $l_end, $lat)
    {
        if ($l_start->get_lat() == $l_end->get_lat())
        {
            return null;
        }
        else
        {
            return new Coordinate(($lat - $l_start->get_lat()) *
                                  ($l_end->get_lon() - $l_start->get_lon()) /
                                  ($l_end->get_lat() - $l_start->get_lat()) +
                                  $l_start->get_lon(),
                                  $lat);
        }
    }

    function calc_border_point($prev_coord, $coord)
    {
        /*
        // assert valid parameters and preconditions
        // prev_coord should be inside page area
        if not (self.minlon <= prev_coord.lon <= self.maxlon and
                self.minlat <= prev_coord.lat <= self.maxlat):
            raise Exception("calc_border_point: prev_coord is not inside page area! " +
                        "report this bug to https://github.com/roelderickx/hikingmap/issues")
        // coord should be outside page area
        if self.minlon <= coord.lon <= self.maxlon and
           self.minlat <= coord.lat <= self.maxlat:
            raise Exception("calc_border_point: coord is not outside page area! " +
                        "report this bug to https://github.com/roelderickx/hikingmap/issues")
        */
        $intersect_coord = null;
        if ($coord->get_lon() <= $this->minlon && $this->minlon <= $prev_coord->get_lon())
        {
            $intersect_coord = $this->__calc_intersection_lon($coord, $prev_coord,
                                                              $this->minlon);
        }
        if ($intersect_coord == null &&
            $prev_coord->get_lon() <= $this->maxlon && $this->maxlon <= $coord->get_lon())
        {
            $intersect_coord = $this->__calc_intersection_lon($prev_coord, $coord,
                                                              $this->maxlon);
        }
        if ($intersect_coord == null &&
            $coord->get_lat() <= $this->minlat && $this->minlat <= $prev_coord->get_lat())
        {
            $intersect_coord = $this->__calc_intersection_lat($coord, $prev_coord,
                                                              $this->minlat);
        }
        if ($intersect_coord == null &&
            $prev_coord->get_lat() <= $this->maxlat && $this->maxlat <= $coord->get_lat())
        {
            $intersect_coord = $this->__calc_intersection_lat($prev_coord, $coord,
                                                              $this->maxlat);
        }
        /*
        if intersect_coord == None:
            raise Exception("calc_border_point: no intersection found! " +
                        "report this bug to https://github.com/roelderickx/hikingmap/issues")
        */
        return $intersect_coord;
    }
    
    function center_map()
    {
        $this->minlat = $this->track_area->minlat;
        $this->set_orientation($this->orientation);
        $this->minlon = $this->track_area->minlon +
                        $this->track_area->sizelon() / 2 -
                        $this->pagesizelon_full / 2;
        $this->maxlon = $this->minlon + $this->pagesizelon_full;
        $this->minlat = $this->track_area->minlat +
                        $this->track_area->sizelat() / 2 -
                        $this->pagesizelat_full / 2;
        $this->maxlat = $this->minlat + $this->pagesizelat_full;
    }
    
    function add_page_to_overview($page)
    {
        if ($page->pageindex == 1 || $page->minlon < $this->track_area->minlon)
        {
            $this->track_area->minlon = $page->minlon;
        }
        if ($page->pageindex == 1 || $page->maxlon > $this->track_area->maxlon)
        {
            $this->track_area->maxlon = $page->maxlon;
        }
        if ($page->pageindex == 1 || $page->minlat < $this->track_area->minlat)
        {
            $this->track_area->minlat = $page->minlat;
        }
        if ($page->pageindex == 1 || $page->maxlat > $this->track_area->maxlat)
        {
            $this->track_area->maxlat = $page->maxlat;
        }
        /* important when rendering to paper, but not when the result is shown on screen
        if ($this->track_area->maxlon - $this->track_area->minlon <
            $this->track_area->maxlat - $this->track_area->minlat)
        {
            $this->set_orientation($this->orientation_portrait);
        }
        else
        {
            $this->set_orientation($this->orientation_landscape);
        }
        
        $delta_lon = $this->track_area->maxlon - $this->track_area->minlon;
        $scale_lon = $this->_convert_degrees_lon_to_cm($delta_lon,
                                                       $this->track_area->minlat) /
                     ($this->get_page_width() - $this->pageoverlap);
        $delta_lat = $this->track_area->maxlat - $this->track_area->minlat;
        $scale_lat = $this->_convert_degrees_lat_to_cm($delta_lat) /
                     ($this->get_page_height() - $this->pageoverlap);
        $this->scale = max($scale_lon, $scale_lat);
        */
    }
    
    function center_map_overview()
    {
        // simplified function for output to screen in stead of paper
        $this->minlon = $this->track_area->minlon;
        $this->maxlon = $this->track_area->maxlon;
        $this->minlat = $this->track_area->minlat;
        $this->maxlat = $this->track_area->maxlat;
    }
    
    function to_string()
    {
        $mapname = 'detail map ' . $this->pageindex;
        if ($this->pageindex == 0)
        {
            $mapname = 'overview map';
        }
        $orientation_string = 'portrait';
        if ($this->orientation == $this->orientation_landscape)
        {
            $orientation_string = 'landscape';
        }
        $retval = $mapname . ' (' . $orientation_string . '): ' . parent::to_string();
        
        return $retval;
    }
}
?>

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

include_once 'coordinate.inc';

define('earthCircumference', 40041.44);
                # km (average, equatorial 40075.017 km / meridional 40007.86 km)
define('cmToKmFactor', 100000.0);

class Area
{
    protected $minlon;
    protected $minlat;
    protected $maxlon;
    protected $maxlat;
    
    function __construct($min_coord, $max_coord)
    {
        $this->minlon = $min_coord->get_lon();
        $this->minlat = $min_coord->get_lat();
        $this->maxlon = $max_coord->get_lon();
        $this->maxlat = $max_coord->get_lat();
    }
    
    function _convert_cm_to_degrees_lon($lengthcm, $scale, $latitude)
    {
        $lengthkm = ($lengthcm / cmToKmFactor) * $scale;
        return $lengthkm / (earthCircumference / 360.0) * cos(deg2rad($latitude));
    }

    function _convert_cm_to_degrees_lat($lengthcm, $scale)
    {
        $lengthkm = ($lengthcm / cmToKmFactor) * $scale;
        return $lengthkm / (earthCircumference / 360.0);
    }

    function _convert_degrees_lon_to_cm($delta_lon, $latitude)
    {
        return $delta_lon * (earthCircumference / 360.0) *
               cos(deg2rad($latitude)) * cmToKmFactor;
    }

    function _convert_degrees_lat_to_cm($delta_lat)
    {
        return $delta_lat * (earthCircumference / 360.0) * cmToKmFactor;
    }
    
    function get_minlon()
    {
        return $this->minlon;
    }

    function get_minlat()
    {
        return $this->minlat;
    }

    function get_maxlon()
    {
        return $this->maxlon;
    }

    function get_maxlat()
    {
        return $this->maxlat;
    }

    function sizelon()
    {
        return $this->maxlon - $this->minlon;
    }

    function sizelat()
    {
        return $this->maxlat - $this->minlat;
    }

    function to_string()
    {
        return (new Coordinate($this->minlon, $this->minlat))->to_string() . ' - ' .
               (new Coordinate($this->maxlon, $this->maxlat))->to_string();
    }
}


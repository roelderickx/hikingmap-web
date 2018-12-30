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

class Coordinate
{
    private $lon = 0.0;
    private $lat = 0.0;
    private $lon_radians = 0.0;
    private $lat_radians = 0.0;
    
    function __construct($lon, $lat, $isDegrees = true)
    {
        if ($isDegrees)
        {
            $this->set_lon($lon);
            $this->set_lat($lat);
        }
        else
        {
            $this->lon = rad2deg($lon);
            $this->lat = rad2deg($lat);
            $this->lon_radians = $lon;
            $this->lat_radians = $lat;
        }
    }
    
    function get_lon()
    {
        return $this->lon;
    }
    
    function set_lon($lon)
    {
        $this->lon = $lon;
        $this->lon_radians = deg2rad($lon);
    }
    
    function get_lat()
    {
        return $this->lat;
    }
    
    function set_lat($lat)
    {
        $this->lat = $lat;
        $this->lat_radians = deg2rad($lat);
    }
    
    function equals($coord)
    {
        return ($this->lon == $coord->lon) && ($this->lat == $coord->lat);
    }
    
    function bearing($coord)
    {
        $dLon = $coord->lon_radians - $this->lon_radians;
        $y = sin($dLon) * cos($coord->lat_radians);
        $x = cos($this->lat_radians) * sin($coord->lat_radians) - 
             sin($this->lat_radians) * cos($coord->lat_radians) * cos($dLon);
        return atan2($y, $x);
    }
    
    function __get_earth_radius($length_unit)
    {
        if ($length_unit == 'mi')
        {
            return 3959;
        }
        else
        {
            return 6371;
        }
    }
    
    function distance_haversine($coord, $length_unit)
    {
        $dLat = $coord->lat_radians - $this->lat_radians;
        $dLon = $coord->lon_radians - $this->lon_radians;
        
        $a = sin($dLat / 2) * sin($dLat / 2) + 
             sin($dLon / 2) * sin($dLon / 2) *
             cos($this->lat_radians) * cos($coord->lat_radians);
        $c = 2 * atan2(sqrt($a), sqrt(1 - $a));
        
        return ($this->__get_earth_radius($length_unit) * $c);
    }
    
    function calc_waypoint_on_line($dest_coord, $distance, $length_unit)
    {
        $b = $this->bearing($dest_coord);
        $earth_radius = $this->__get_earth_radius($length_unit);
        
        return new Coordinate(//lon
                              $this->lon_radians +
                              atan2(sin($b) *
                                    sin($distance / $earth_radius) *
                                    cos($this->lat_radians),
                                    cos($distance / $earth_radius) -
                                    sin($this->lat_radians) *
                                    sin($dest_coord->lat_radians)),
                              //lat
                              asin(sin($this->lat_radians) *
                                   cos($distance / $earth_radius) +
                                   cos($this->lat_radians) *
                                   sin($distance / $earth_radius) *
                                   cos($b)),
                              false);
    }
    
    function to_string() 
    {
        return sprintf("%.6f,%.6f", $this->lon, $this->lat);
    }
}


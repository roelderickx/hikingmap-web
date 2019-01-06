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

define('encodingPrecision', 5);

class Tracks
{
    private $logger;
    private $tracks;
    private $waypoints;
    
    function __construct($logger, $files, $render_waypoints, $waypoint_dist, $length_unit)
    {
        $this->logger = $logger;
        
        $this->tracks = array();
        $this->parse_files($files);
        $this->__add_tracks_as_polyline();

        $this->waypoints = array();
        if ($render_waypoints)
        {
            $this->__calculate_waypoints($waypoint_dist, $length_unit);
            $this->__add_waypoints_as_polyline();
        }
    }
    
    private function parse_files($files)
    {
        foreach ($files as $file)
        {
            $gpx = new SimpleXMLElement($file["tmp_name"], null, true);
            
            foreach ($gpx as $trk)
            {
                if ($trk->getName() == "trk")
                {
                    $this->parse_track($trk);
                }
            }
        }
    }
    
    private function parse_track($trk)
    {
        $trackname = key_exists("name", $trk->children()) ? $trk->name : "[unnamed]";
        $this->logger->log('Found track ' . $trackname);
        
        $track = array();
        foreach ($trk as $trkseg)
        {
            if ($trkseg->getName() == "trkseg")
            {
                foreach ($trkseg as $coord)
                {
                    if ($coord->getName() == "trkpt")
                    {
                        $lon = 0.0;
                        $lat = 0.0;
                        foreach ($coord->attributes() as $attrname => $attrvalue)
                        {
                            if ($attrname == "lon")
                            {
                                $lon = (float)$attrvalue;
                            }
                            else if ($attrname == "lat")
                            {
                                $lat = (float)$attrvalue;
                            }
                        }
                        $track[] = new Coordinate($lon, $lat);
                    }
                }
            }
        }
        
        $foundindex = 0;
        $foundtrack = false;
        foreach ($this->tracks as $foundindex => $existingtrack)
        {
            if ($existingtrack[0]->equals($track[0]))
            {
                $this->logger->log('=> same startpoint as track ' . $foundindex .
                                   ': reversing track');
                $track = array_reverse($track);
            }
            else if ($existingtrack[count($existingtrack)-1]->equals($track[count($track)-1]))
            {
                $this->logger->log('=> same endpoint as track ' . $foundindex .
                                   ': reversing track');
                $track = array_reverse($track);
            }
            
            if ($existingtrack[count($existingtrack)-1]->equals($track[0]))
            {
                $this->logger->log('=> connecting after track ' . $foundindex);
                $newtrack = array_merge($existingtrack, array_slice($track, 1, null));
                $this->tracks[$foundindex] = $newtrack;
                $foundtrack = true;
                break;
            }
            else if ($existingtrack[0]->equals($track[count($track)-1]))
            {
                $this->logger->log('=> connecting before track ' . $foundindex);
                $newtrack = array_merge($track, array_slice($existingtrack, 1, null));
                $this->tracks[$foundindex] = $newtrack;
                $foundtrack = true;
                break;
            }
        }
        
        if (!$foundtrack)
        {
            $this->logger->log('=> new track ' . count($this->tracks));
            $this->tracks[] = $track;
        }
    }
    
    function get_tracks()
    {
        return $this->tracks;
    }
    
    function __add_wpt($trackindex, $coord1, $coord2, $cumul_dist_at_coord1,
                       $waypt_distance, $length_unit)
    {
        if ($coord1->equals($coord2))
        {
            if ($cumul_dist_at_coord1 == 0)
            {
                $this->waypoints[$trackindex] = array();
                $this->waypoints[$trackindex][0] = $coord1;
            }
            return $cumul_dist_at_coord1;
        }
        else
        {
            $cumul_dist_at_coord2 = $cumul_dist_at_coord1 +
                                    $coord1->distance_haversine($coord2, $length_unit);
            
            if ((int)($cumul_dist_at_coord1) + 1 <= (int)$cumul_dist_at_coord2)
            {
                foreach (range((int)($cumul_dist_at_coord1) + 1,
                               (int)($cumul_dist_at_coord2)) as $dist)
                {
                    if ($dist % $waypt_distance == 0)
                    {
                        $d = $dist - $cumul_dist_at_coord1;
                        $waypt = $coord1->calc_waypoint_on_line($coord2, $d, $length_unit);
                        $this->waypoints[$trackindex][$dist] = $waypt;
                    }
                }
            }
            return $cumul_dist_at_coord2;
        }
    }
    
    function __generate_waypoints_track($index, $track, $waypt_distance, $length_unit)
    {
        $cumulDistance = 0;
        $prev_coord = $track[0];
        foreach ($track as $coord)
        {
            $cumulDistance = $this->__add_wpt($index, $prev_coord, $coord, $cumulDistance,
                                              $waypt_distance, $length_unit);
            $prev_coord = $coord;
        }
        $this->logger->log(sprintf('Total track distance: %.2f %s',
                                   $cumulDistance, $length_unit));
    }
    
    function __calculate_waypoints($waypt_distance, $length_unit)
    {
        foreach ($this->tracks as $index => $track)
        {
            $this->logger->log('Generating waypoints for track ' . $index . ': ' .
                               $track[0]->to_string() . ' - ' .
                               $track[count($track)-1]->to_string());
            $this->__generate_waypoints_track($index, $track, $waypt_distance, $length_unit);
        }
    }

    // from https://github.com/emcconville/google-map-polyline-encoding-tool
    // slightly adapted to use with hikingmap
    
    /**
     * Reduce multi-dimensional to single list
     * @param array $array Subject array to flatten.
     * @return array flattened
     */
    private function __flatten($array)
    {
        $flatten = array();
        array_walk(
            $array, // @codeCoverageIgnore
            function ($current) use (&$flatten)
            {
                $flatten[] = $current->get_lat();
                $flatten[] = $current->get_lon();
            }
        );
        return $flatten;
    }

    /**
     * Apply Google Polyline algorithm to list of points.
     * @param array $points List of points to encode. Can be a list of tuples,
     *                      or a flat, one-dimensional array.
     * @return string encoded string
     */
    private function __polyline_encode($points)
    {
        $points = $this->__flatten($points);
        $encodedString = '';
        $index = 0;
        $previous = array(0,0);
        foreach ($points as $number)
        {
            $number = (float)($number);
            $number = (int)round($number * pow(10, encodingPrecision));
            $diff = $number - $previous[$index % 2];
            $previous[$index % 2] = $number;
            $number = $diff;
            $index++;
            $number = ($number < 0) ? ~($number << 1) : ($number << 1);
            $chunk = '';
            while ($number >= 0x20)
            {
                $chunk .= chr((0x20 | ($number & 0x1f)) + 63);
                $number >>= 5;
            }
            $chunk .= chr($number + 63);
            $encodedString .= $chunk;
        }
        return $encodedString;
    }
    
    private function __to_js_string($string)
    {
        return "'" . str_replace("\\", "\\\\", $string) . "'";
    }
    
    private function __add_tracks_as_polyline()
    {
?>
<script type="text/javascript">
function get_amount_tracks() { return <?php print(count($this->tracks)); ?>; }
function get_track(index) {
<?php
    foreach ($this->tracks as $index => $track)
    {
        if ($index > 0)
        {
            print("} else ");
        }
        print("if (index == " . $index . ") {\n");
        print("  return " .
              $this->__to_js_string($this->__polyline_encode($track)) . ";\n");
    }
?>
}
}
</script>
<?php
    }
    
    private function __add_waypoints_as_polyline()
    {
?>
<script type="text/javascript">
function get_amount_waypoints() { return <?php print(count($this->waypoints)); ?>; }
function get_waypoints(index) {
<?php
    foreach ($this->waypoints as $index => $trackwaypoints)
    {
        if ($index > 0)
        {
            print("} else ");
        }
        print("if (index == " . $index . ") {\n");
        print("  return " .
              $this->__to_js_string($this->__polyline_encode($trackwaypoints)) . ";\n");
    }
?>
}
}
</script>
<?php
    }
}
?>

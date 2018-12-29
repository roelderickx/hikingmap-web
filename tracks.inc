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

class Tracks
{
    private $logger;
    private $tracks;
    //private $waypoints;
    
    function __construct($logger, $files)
    {
        $this->logger = $logger;
        
        $this->tracks = array();
        $this->parse_files($files);
        
        // DEBUG: log tracks
        /*
        $this->logger->log("DEBUG");
        $this->logger->log("Amount of tracks: " . count($this->tracks));
        
        foreach ($this->tracks as $index => $track)
        {
            $this->logger->log("Track " . $index);
            foreach ($track as $coord)
            {
                $this->logger->log($coord->to_string());
            }
        }
        */
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
}
/*
    function __del__() {
        if ($this->tempwaypointfile && $os->path->isfile($this->tempwaypointfile)) {
            pyjslib_printnl('Removing temp file ' . $this->tempwaypointfile);
            $os->remove($this->tempwaypointfile);
        }
    }
    
    function __write_wpt($gpxnode,$coord1,$coord2,$cumul_dist_at_coord1,$waypt_distance,$length_unit) {
        if ($coord1->equals($coord2)) {
            if (($cumul_dist_at_coord1 == 0)) {
                $coord1->append_to_xml_node($gpxnode, '0');
            }
            return $cumul_dist_at_coord1;
        }
        else {
            $cumul_dist_at_coord2 = ($cumul_dist_at_coord1 + $coord1->distance_haversine($coord2, $length_unit));
            foreach( pyjslib_list(pyjslib_range((pyjslib_int($cumul_dist_at_coord1) + 1), (pyjslib_int($cumul_dist_at_coord2) + 1))) as $dist ) {
                if ((($dist % $waypt_distance) == 0)) {
                    $d = ($dist - $cumul_dist_at_coord1);
                    $waypt = $coord1->calc_waypoint_on_line($coord2, $d, $length_unit);
                    $waypt->append_to_xml_node($gpxnode, pyjslib_str($dist));
                }
            }
            return $cumul_dist_at_coord2;
        }
    }
    
    function __generate_waypoints_track($gpxnode,$track,$waypt_distance,$length_unit) {
        $cumulDistance = 0;
        $prev_coord = new Coordinate($track[0]->lon, $track[0]->lat);
        foreach( pyjslib_list(array_slice($track, 0, null)) as $trackpoint ) {
            $coord = new Coordinate($trackpoint->lon, $trackpoint->lat);
            $cumulDistance = $this->__write_wpt($gpxnode, $prev_coord, $coord, $cumulDistance, $waypt_distance, $length_unit);
            $prev_coord = $coord;
        }
        pyjslib_printnl(sprintf('Total track distance: %.2f %s', $cumulDistance, $length_unit));
    }
    
    function calculate_waypoints($waypt_distance,$length_unit) {
        $wayptdoc = minidom::Document();
        $gpxnode = $wayptdoc->createElement('gpx');
        $gpxnode->setAttribute('version', '1.0');
        $gpxnode->setAttribute('creator', 'hikingmap');
        $gpxnode->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
        $gpxnode->setAttribute('xmlns', 'http://www.topografix.com/GPX/1/0');
        $gpxnode->setAttribute('xsi:schemaLocation', 'http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd');
        $index = 0;
        foreach( pyjslib_list($this->tracks) as $track ) {
            pyjslib_printnl('Generating waypoints for track ' . pyjslib_str($index) . ': ' . $track[0]->to_string() . ' - ' . $track[-1]->to_string());
            $this->__generate_waypoints_track($gpxnode, $track, $waypt_distance, $length_unit);
            $index += 1;
        }
        $wayptdoc->appendChild($gpxnode);
        list($fd, $this->tempwaypointfile) = py2php_kwargs_method_call($tempfile, null, 'mkstemp', [], ["prefix" => 'hikingmap_temp_waypoints',"suffix" => '.gpx']);
        $f = $os->fdopen($fd, 'w');
        $wayptdoc->writexml($f, '', '  ', '
', 'ISO-8859-1');
        $f->close();
    }
    */


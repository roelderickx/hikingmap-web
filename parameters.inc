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

class Parameters
{
    private $logger;
    private $scale;
    private $pagewidth;
    private $pageheight;
    private $pageoverlap;
    private $render_waypts;
    private $waypt_distance;
    private $length_unit;
    
    function __construct($logger)
    {
        $this->logger = $logger;
        
        $this->scale = htmlentities($_POST['scale'], ENT_COMPAT, 'UTF-8');
        $this->pagewidth = htmlentities($_POST['width'], ENT_COMPAT, 'UTF-8');
        $this->pageheight = htmlentities($_POST['height'], ENT_COMPAT, 'UTF-8');
        $this->pageoverlap = htmlentities($_POST['overlap'], ENT_COMPAT, 'UTF-8');
        $this->render_waypts = key_exists("waypoints", $_POST);
        $this->waypt_distance = htmlentities($_POST['waypointsdist'], ENT_COMPAT, 'UTF-8');
        $this->length_unit = htmlentities($_POST['lengthunit'], ENT_COMPAT, 'UTF-8');
    }
        
    private function all_files_valid()
    {
        foreach ($_FILES as $file)
        {
            if ($file["name"] == null)
            {
                $this->logger->error('Error: No file was uploaded.');
                return false;
            }
            
            $this->logger->log('GPX filename = ' . $file["name"], false);
            
            if ($file["error"] > 0)
            {
                $this->logger->error('Error: ' . $this->gpxfile["error"]);
                return false;
            }
            else
            {
                $this->logger->log(', size = ' . $file["size"] . ' bytes');
                if ($file["size"] == 0)
                {
                    $this->logger->error('Error: File is empty.');
                    return false;
                }
                else if ($file["type"] != 'application/gpx+xml')
                {
                    $this->logger->error('Error: File is not a gpx file.');
                    return false;
                }
            }
        }
        
        return true;
    }

    function verify_parameters()
    {
        $this->logger->log('Parameters:');
        $this->logger->log('scale = ' . $this->scale);
        $this->logger->log('pagewidth = ' . $this->pagewidth);
        $this->logger->log('pageheight = ' . $this->pageheight);
        $this->logger->log('pageoverlap = ' . $this->pageoverlap);
        $this->logger->log('render_waypts = ' . $this->render_waypts);
        $this->logger->log('waypt_distance = ' . $this->waypt_distance);
        $this->logger->log('length_unit = ' . $this->length_unit);
        
        return $this->all_files_valid();
    }
    
    function get_scale()
    {
        return (float)$this->scale;
    }
    
    function get_pagewidth()
    {
        return (float)($this->pagewidth) / 10.0;
    }
    
    function get_pageheight()
    {
        return (float)($this->pageheight) / 10.0;
    }
    
    function get_pageoverlap()
    {
        return (float)($this->pageoverlap) / 10.0;
    }
    
    function render_waypoints()
    {
        return $this->render_waypts;
    }
    
    function get_waypoint_distance()
    {
        return $this->waypt_distance;
    }
    
    function get_length_unit()
    {
        return $this->length_unit;
    }
}
?>

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

include 'logger.inc';
include 'parameters.inc';
include 'tracks.inc';
include 'trackfinder.inc';

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>Hikingmap web</title>
</head>
<body>
    <?php
    $logger = new Logger(true);
    
    $params = new Parameters($logger);
    $params->verify_parameters() or die("Parameters incorrect.");

    $tracks = new Tracks($logger, $_FILES);
    /*if ($params->render_waypts)
    {
        $tracks->calculate_waypoints($params->waypt_distance, $params->length_unit);
    }*/
    
    $trackfinder = new TrackFinder($logger, $params, $tracks);
    $trackfinder->render();
    ?>
</body>
</html>

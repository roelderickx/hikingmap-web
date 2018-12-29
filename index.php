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
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>Hikingmap web</title>
</head>
<body>
    <form action="hikingmap.php" method="post" enctype="multipart/form-data">
        <table>
        <tr>
        <td><label for="gpxfile">GPX Track</label></td>
        <td colspan="2"><input type="file" id="gpxfile" name="gpxfile"/></td>
        </tr>
        <tr>
        <td><label for="scale">Scale</label></td>
        <td colspan="2">
            1:<input type="text" id="scale" name="scale" value="50000" size="7"/>
        </td>
        </tr>
        <tr>
        <td><label for="width">Paper width</label></td>
        <td colspan="2">
            <input type="text" id="width" name="width" value="200" size="4"/> mm
        </td>
        </tr>
        <tr>
        <td><label for="height">Paper height</label></td>
        <td colspan="2">
            <input type="text" id="height" name="height" value="287" size="4"/> mm
        </td>
        </tr>
        <tr>
        <td><label for="overlap">Page overlap</label></td>
        <td colspan="2">
            <input type="text" id="overlap" name="overlap" value="10" size="4"/> mm
        </td>
        </tr>
        <tr>
        <td><label for="waypoints">Add waypoints</label></td>
        <td><input type="checkbox" id="waypoints" name="waypoints" value="true" checked/></td>
        <td>
            every
            <input type="text" name="waypointsdist" value="1" size="2"/>
            <select id="lengthunit" name="lengthunit">
                <option value="km" selected>km</option>
                <option value="mi">mi</option>
            </select>
        </td>
        </tr>
        <tr>
        <td colspan="3"><input type="submit" value="Submit"/></td>
        <!--td colspan="2"><input type="reset" value="Reset" width="100"/></td-->
        </tr>
        </table>
    </form>
</body>
</html>


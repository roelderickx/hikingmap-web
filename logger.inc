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

class Logger
{
    private $verbose = false;
    
    function __construct($verbose = true)
    {
        $this->verbose = $verbose;
    }
    
    function log($text, $newline = true)
    {
        if ($this->verbose)
        {
            print($text);
            
            if ($newline)
            {
                print("<br/>\n");
            }
        }
    }
    
    function error($text)
    {
        print('<span style="color: red;">' . $text . '</span>');
    }
}
?>

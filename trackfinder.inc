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

include_once 'page.inc';

class TrackFinder
{
    private $logger;
    private $parameters;
    
    private $renderedareas;
    private $currentpageindex;
    private $currentpage;
    private $firstpointaccepted;
    private $pages;

    function __construct($logger, $parameters, $tracks)
    {
        $this->logger = $logger;
        $this->parameters = $parameters;
        //$this->tempwaypointfile = $tracks->tempwaypointfile;
        //$this->tempoverviewfile = '';
        $this->pages = array();
        
        $this->logger->log('Calculating track order permutation resulting ' .
                           'in a minimum amount of pages');
        $this->logger->log('This may take a while, checking ' .
                           $this->__factorial(count($tracks->get_tracks())) .
                           ' track permutations');
        $min_amount_pages = -1;
        foreach ($this->__calc_permutations($tracks->get_tracks()) as $trackpermutation)
        {
            $this->renderedareas = array();
            $this->currentpageindex = 1;
            $this->currentpage = null;
            $this->firstpointaccepted = false;
            foreach ($trackpermutation as $track)
            {
                $prev_coord = new Coordinate(0.0, 0.0);
                foreach ($track as $coord)
                {
                    $prev_coord = $this->__add_point($prev_coord, $coord);
                }
                $this->__flush();
            }
            if ($min_amount_pages == -1 || count($this->renderedareas) < $min_amount_pages)
            {
                $min_amount_pages = count($this->renderedareas);
                $this->pages = $this->renderedareas;
                $this->logger->log('Found track permutation with ' .
                                   $min_amount_pages . ' pages');
            }
        }
        /*if ($this->parameters->generate_overview)
        {
            $this->__add_page_overview();
        }*/
        $this->__reorder_pages();
    }
    
    function __factorial($number)
    {
        $factorial = 1;
        for ($i = 1; $i <= $number; $i++)
        {
            $factorial = $factorial * $i;
        }
        return $factorial;
    }
    
    // https://stackoverflow.com/questions/5506888/permutations-all-possible-
    //                                                    sets-of-numbers#5506933
    function __calc_permutations(array $elements)
    {
        if (count($elements) <= 1)
        {
            yield $elements;
        }
        else
        {
            foreach ($this->__calc_permutations(array_slice($elements, 1)) as $permutation)
            {
                foreach (range(0, count($elements) - 1) as $i)
                {
                    yield array_merge(
                        array_slice($permutation, 0, $i),
                        [$elements[0]],
                        array_slice($permutation, $i)
                    );
                }
            }
        }
    }
    
    function __add_point($prev_coord, $coord)
    {
        if (!($this->__is_point_rendered($coord)))
        {
            if (!$this->firstpointaccepted)
            {
                $prev_coord = $this->__add_first_point($coord);
            }
            else
            {
                $prev_coord = $this->__add_next_point($prev_coord, $coord);
            }
        }
        return $prev_coord;
    }
    
    function __flush()
    {
        if ($this->firstpointaccepted)
        {
            $this->currentpage->center_map();
            $this->renderedareas[] = $this->currentpage;
            $this->firstpointaccepted = false;
        }
    }
    
    function __is_point_rendered($coord)
    {
        $found = false;
        foreach ($this->renderedareas as $area)
        {
            if ($area->get_minlon() <= $coord->get_lon() &&
                $coord->get_lon() <= $area->get_maxlon() &&
                $area->get_minlat() <= $coord->get_lat() &&
                $coord->get_lat() <= $area->get_maxlat())
            {
                $found = true;
                break;
            }
        }
        return $found;
    }
    
    function __add_first_point($coord)
    {
        $this->currentpage = new Page($this->parameters, $this->currentpageindex);
        $this->currentpage->initialize_first_point($coord);
        $this->currentpageindex += 1;
        $this->firstpointaccepted = true;
        return $coord;
    }
    
    function __add_next_point($prev_coord, $coord)
    {
        $outside_page = $this->currentpage->add_next_point($prev_coord, $coord);
        if ($outside_page)
        {
            $border_coord = $this->currentpage->calc_border_point($prev_coord, $coord);
            $this->currentpage->center_map();
            $this->renderedareas[] = $this->currentpage;
            $this->__add_first_point($border_coord);
            $this->__add_next_point($border_coord, $coord);
            return $border_coord;
        }
        else
        {
            return $coord;
        }
    }
    
    /*function __add_page_overview() {
        $overviewdoc = minidom::Document();
        $gpxnode = $overviewdoc->createElement('gpx');
        $gpxnode->setAttribute('version', '1.0');
        $gpxnode->setAttribute('creator', 'hikingmap');
        $gpxnode->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
        $gpxnode->setAttribute('xmlns', 'http://www.topografix.com/GPX/1/0');
        $gpxnode->setAttribute('xsi:schemaLocation', 'http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd');
        $overviewpage = new Page($this->parameters, 0);
        foreach( pyjslib_list($this->pages) as $page ) {
            $overviewpage->add_page_to_overview($page);
            $tracknode = $gpxnode->ownerDocument->createElement('trk');
            $tracksegnode = $gpxnode->ownerDocument->createElement('trkseg');
            foreach( pyjslib_list(pyjslib_range(0, 5)) as $i ) {
                if (in_array($i, [0, 3, 4])) {
                    $lon = $page->minlon;
                }
                else {
                    $lon = $page->maxlon;
                }
                if (in_array($i, [0, 1, 4])) {
                    $lat = $page->maxlat;
                }
                else {
                    $lat = $page->minlat;
                }
                $trackptnode = $gpxnode->ownerDocument->createElement('trkpt');
                $trackptnode->setAttribute('lat', pyjslib_str($lat));
                $trackptnode->setAttribute('lon', pyjslib_str($lon));
                $tracksegnode->appendChild($trackptnode);
            }
            $tracknode->appendChild($tracksegnode);
            $gpxnode->appendChild($tracknode);
        }
        $overviewdoc->appendChild($gpxnode);
        list($fd, $this->tempoverviewfile) = py2php_kwargs_method_call($tempfile, null, 'mkstemp', [], ["prefix" => 'hikingmap_temp_overview',"suffix" => '.gpx']);
        $f = $os->fdopen($fd, 'w');
        $overviewdoc->writexml($f, '', '  ', '
', 'ISO-8859-1');
        $f->close();
        $overviewpage->center_map();
        $this->pages->insert(0, $overviewpage);
    }*/
    
    function __reorder_pages()
    {
        /*
        if (($this->parameters->page_order == 'rectoverso')) {
            $oldindex = $math->floor((count($this->pages) / 2));
            $newindex = 1;
            while (($oldindex < count($this->pages))) {
                $this->pages->insert($newindex, $this->pages->pop($oldindex));
                $oldindex += 1;
                $newindex += 2;
            }
            foreach( pyjslib_list($this->pages) as $page ) {
            }
            pyjslib_printnl([]);
        }
        else if (($this->parameters->page_order == 'book')) {
            $amount_empty_pages = ((4 - (count($this->pages) % 4)) % 4);
            foreach( pyjslib_list(pyjslib_range(0, $amount_empty_pages)) as $i ) {
                $this->pages[] = null;
            }
            $oldindex = (count($this->pages) - 1);
            $newindex = 1;
            while (($newindex < $oldindex)) {
                $this->pages->insert($newindex, $this->pages->pop($oldindex));
                $newindex += 2;
            }
            $oldindex = 0;
            $newindex = 1;
            while (($oldindex < count($this->pages))) {
                $this->pages->insert($newindex, $this->pages->pop($oldindex));
                $oldindex += 4;
                $newindex += 4;
            }
            foreach( pyjslib_list($this->pages) as $page ) {
                if (($page != null)) {
                }
                else {
                }
            }
            pyjslib_printnl([]);
            pyjslib_printnl('WARNING: blank pages are not generated!');
        }
        else {
            pyjslib_printnl('Page order is naturalorder');
        }
        */
    }
    
    function render()
    {
        foreach ($this->pages as $page)
        {
            if ($page != null)
            {
                print($page->to_string() . "<br/>\n");
                $page->render($this->parameters);
            }
        }
    }
}
?>

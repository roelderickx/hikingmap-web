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
    private $pointskipped;
    private $pages;

    function __construct($logger, $parameters, $tracks)
    {
        $this->logger = $logger;
        $this->parameters = $parameters;
        $this->pages = array();
        
        $this->renderedareas = array();
        $this->currentpageindex = 1;
        $this->currentpage = null;
        $this->firstpointaccepted = false;
        foreach ($tracks->get_tracks() as $track)
        {
            set_time_limit(30);
            $this->pointskipped = true;
            $prev_coord = new Coordinate(0.0, 0.0);
            foreach ($track as $coord)
            {
                $prev_coord = $this->__add_point($prev_coord, $coord);
            }
            $this->__flush();
        }
        $this->pages = $this->renderedareas;
        
        $this->__add_page_overview();
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
            $this->pointskipped = false;
        }
        else
        {
            $this->pointskipped = true;
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
            //$border_coord = $this->currentpage->calc_border_point($prev_coord, $coord);
            $this->currentpage->remove_last_point();
            if (!$this->pointskipped)
            {
                $border_coord = $this->currentpage->calc_border_point($prev_coord, $coord);
                $this->currentpage->add_next_point($prev_coord, $border_coord);
            }
            $this->currentpage->center_map();
            $this->renderedareas[] = $this->currentpage;
            if (!$this->pointskipped)
            {
                $this->__add_first_point($border_coord);
                $this->__add_next_point($border_coord, $coord);
            }
            else
            {
                $this->__add_first_point($coord);
            }
        }
        return $coord;
    }

    function __add_page_overview()
    {
        $overviewpage = new Page($this->parameters, 0);
        foreach ($this->pages as $index => $page)
        {
            $overviewpage->add_page_to_overview($page);
        }
        $overviewpage->center_map_overview();
        array_splice($this->pages, 0, 0, array($overviewpage));
    }
    
    function render()
    {
        print("<script type=\"text/javascript\">\n");
        print("function get_page_extents() { ");
        print("return [");
        foreach ($this->pages as $index => $page)
        {
            if ($index > 1)
            {
                print(",");
            }
            if ($index > 0)
            {
                print(sprintf("[%.5f,%.5f,%.5f,%.5f]",
                              $page->get_minlon(), $page->get_minlat(),
                              $page->get_maxlon(), $page->get_maxlat()));
            }
        }
        print("];} ");
        
        print(sprintf("show_overview(%.5f,%.5f,%.5f,%.5f,%.2f,%.2f,%d);\n",
                      $this->pages[0]->get_minlon(), $this->pages[0]->get_minlat(),
                      $this->pages[0]->get_maxlon(), $this->pages[0]->get_maxlat(),
                      $this->parameters->get_pagewidth(),
                      $this->parameters->get_pageheight(),
                      $this->parameters->get_dpi()));
        print("</script>\n");
    }
}
?>

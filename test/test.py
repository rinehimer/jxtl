#
# $Id: jxtl.c 146 2010-10-25 00:45:22Z rinehimer $
#
# Description
#   Runs the same tests, but does it by using the Python language bindings.
#   The Python bindings need to be be built and installed to run this.
#
# Copyright 2010 Dan Rinehimer
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import libjxtl;
import glob;
import os.path;
import filecmp;

def format( value, format, context ):
    if ( format == "upper" ):
        return value.upper();
    elif ( format == "lower" ):
        return value.lower();
    else:
        return value;

def compare( file1, file2 ):
    if ( filecmp.cmp( file1, file2 ) == False ):
        print "Failed test in " + os.path.dirname( file1 );
    else:
        os.remove( file2 );

inputs = glob.glob( "/t*/input" );
beers_xml = libjxtl.xml_to_dict( "t.xml" );
beers_json = libjxtl.json_to_dict( "t.json" );
t = libjxtl.Template();

for input in inputs:
    dir = os.path.dirname( input );
    t.load( input );
    t.set_format_callback( format );
    t.expand_to_file( dir + "/test.output", beers_xml );
    compare( dir + "/output", dir + "/test.output" );
    t.expand_to_file( dir + "/test.output", beers_json );
    compare( dir + "/output", dir + "/test.output" );

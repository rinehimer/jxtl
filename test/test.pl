#!/usr/bin/perl

# $Id$
#
# Description
#   Runs the same tests, but does it by using the Perl language bindings.
#   The Perl bindings need to be be built and installed to run this.
#
# Copyright 2011 Dan Rinehimer
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

use LibJXTL;
use File::Glob ':glob';
use File::Basename;
use File::Compare;

sub format_case
{
    my ( $value, $format, $context ) = @_;
    if ( $format eq "upper" ) {
        return uc( $value );        
    }
    elsif ( $format eq "lower" ) {
        return lc( $value );
    }
    else {
        return $value;
    }
}

sub compare_files
{
    my ( $file1, $file2 ) = @_;
    if ( compare( $file1, $file2 ) != 0 ) {
        printf( "Failed test in %s\n", dirname( $file1 ) );
    }
    else {
        unlink( $file2 );
    }
}

@inputs = bsd_glob( "./t*/input" );
$beers_xml = LibJXTL::xml_to_hash( "t.xml" );
$beers_json = LibJXTL::json_to_hash( "t.json" );
$template = LibJXTL::Template->new();

foreach $input (@inputs) {
    $dir = dirname( $input );
    $template->load( $input );
    $template->register_format( "upper", \&format_case );
    $template->register_format( "lower", \&format_case );
    $template->expand_to_file( "$dir/test.output", $beers_xml );
    compare_files( "$dir/output", "$dir/test.output" );
    $template->expand_to_file( "$dir/test.output", $beers_json );
    compare_files( "$dir/output", "$dir/test.output" );
}

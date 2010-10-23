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

inputs = glob.glob( "../../test/t*/input" );
beers_xml = libjxtl.xml_to_dict( "../../test/t.xml" );
beers_json = libjxtl.json_to_dict( "../../test/t.json" );
t = libjxtl.Template();

for input in inputs:
    dir = os.path.dirname( input );
    t.load( input );
    t.set_format_callback( format );
    t.expand_to_file( dir + "/test.output", beers_xml );
    compare( dir + "/output", dir + "/test.output" );
    t.expand_to_file( dir + "/test.output", beers_json );
    compare( dir + "/output", dir + "/test.output" );

# Introduction #

Right now Perl and Python language bindings are available.  All of the functions are part of a "Template" class that is part of LibJXTL.  I am mainly going to use Perl as the examples here.

# Examples #
All you need to do to use a template is create a Perl hash with your data dictionary and then expand the template.  To use the same examples modeling beers, we would have something like this.

```
use LibJXTL;

$beers = { brewery => [
               { name => "Dogfish Head",
                 beer => [
                     { beer_name => "60 Minute IPA" },
                     { beer_name => "90 Minute IPA" },
                     { season => "Fall",
                       beer_name => "Punkin Ale" } ] },
               { name => "Troegs",
                 beer => [
                     { beer_name => "HopBack Amber Ale" },
                     { season => "Summer",
                       beer_name => "Sunshine Pils" },
                     { season => "Winter",
                       beer_name => "Mad Elf" } ] } ] };

## Create, load and expand a template.  A string to parse as a template
## can also be passed to the constructor instead of using the load
## function.
$template = LibJXTL::Template->new();
$template->load( "path/to/template.jxtl" );
$buf = $template->expand_to_buffer( $beers );
print $buf;

## You could also do the same thing using the expand to file function
## ("-" means to use stdout)
## $template->expand_to_file( "-", $beers );
```

### Formatting ###
You can also specify a format function to use.  This allows you to extend the template language almost endlessly.  Here is an example of using a format function.  In this case, if we either uppercase or lowercase the string depending on what format is passed in.

```
sub format
{
   my ( $value, $format, $context ) = @_;
   if ( $format eq "upper" ) {
     return uc( $value );
   }
   elsif ( $format eq "lower" ) {
     return lc( $value );
   }
}

$template = LibJXTL::Template->new();
$template->load( "t.jxtl" );
$template->register_format( "upper", \&format );
$template->register_format( "lower", \&format );
$template->expand_to_buffer( $vars );
```

Using formats in templates works like this:

```
{{breweries/name ; separator="\n", format="upper"}}
```

Every time a template value is to be output, it will call the format function and output what it returns.  The value of each name tag will be passed to the format function and it returns us an upper-cased version of the string.

### Using The Current Context ###
You might have noticed the third parameter in the format callback, $context.  The context is really a pointer to the current node being expanded.  What this means, is that you can use this context to expand another template.  If, for example, you already had a template to expand certain nodes you could reuse it here.  This concept is also used in JSON Template and explained [here](http://json-template.googlecode.com/svn/trunk/doc/On-Design-Minimalism.html).

So, if our template looked something like this:
```
{{breweries ; separator="\n", format="brewery"}}
```

We might do something like this in our format callback.

```
sub brewery
{
   my ( $value, $format, $context ) = @_;
   my $t = LibJXTL::Template->new();
   $t->load( "breweries.jxtl" );
   return $t->expand_to_buffer( $context );
}
```

# Using an XML/JSON File #
If you have an XML or JSON file, you can read it into the corresponding hash reference by doing this:

```
$beers = LibJXTL::xml_to_hash( "file.xml" );
$beers = LibJXTL::json_to_hash( "file.json" );
```
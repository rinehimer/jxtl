# jxtl
A simple, general-purpose template language.

This tool was born out of the desire to use XML or JSON to generate code in any language.  I wanted something I could use without having to write a program to do the template expansion.  The provided jxtl command line executable handles the majority of cases for me.

This templating language intentionally lacks features that other templating languages posess in order to enforce the model-view  separation.  If you have ever worked with PHP* or encountered XSLT being used to create a non-XML document, you probably understand how important this separation is.

This library contains bindings that can be built for both Perl and Python.  See the [Language Bindings](../..//wiki/Language-Bindings) page for more information.

*PHP does not inherently have this problem, but is often used in a manner that violates model-view separation.

## Examples
All examples will use the following simple XML file, which models some beers:
```
<?xml version="1.0"?>
<beers>
  <brewery>
    <name>Dogfish Head</name>
    <beer>
      <beer_name>60 Minute IPA</beer_name>
    </beer>
    <beer>
      <beer_name>90 Minute IPA</beer_name>
    </beer>
    <beer>
      <beer_name>Punkin Ale</beer_name>
      <season>Fall</season>
    </beer>
  </brewery>
  <brewery>
    <name>Troegs</name>
    <beer>
      <beer_name>HopBack Amber Ale</beer_name>
    </beer>
    <beer>
      <beer_name>Sunshine Pils</beer_name>
      <season>Summer</season>
    </beer>
    <beer>
      <beer_name>Mad Elf</beer_name>
      <season>Winter</season>
    </beer>
  </brewery>
</beers>
```

Expansion from the command line can be done using the jxtl command line executable.  If this XML is saved in beers.xml and the template contents are stored in template_file.jxtl, the syntax looks like the following:

```
jxtl -s -x beers.xml -t template_file.jxtl
```

The -s option means to strip the root element from the XML document.

## Example 1 ##
Lets say we wanted to get some really basic information, like printing the name of each brewery in the file.  We could  do it with the following template:
```
{{#section brewery}}
{{name}} Brewery
{{#end}}
```
From this example we see that the template start and end markers are "{{" and "}}".  Template directives start with  "#", and an identifier within the curly braces means to lookup a value and print its string representation.  The template processor will  automatically expand a section for all nodes that have the same name.  In this case we have two brewery nodes, so the section is expanded twice.

Unfortunately the output is not well formatted as there is no spacing between the brewery names.
```
Dogfish Head BreweryTroegs Brewery
```

## Example 2 ##
Our formatting issue can easily be fixed by adding a separator, in this case a newline.  A separator is printed at the end of each section, except for the last iteration.  A separator is a quoted C-style string.
```
{{#section brewery ; separator="\n"}}
{{name}} Brewery
{{#end}}
```

Now the output is much better:
```
Dogfish Head Brewery
Troegs Brewery
```

## Example 3 ##
Now that we have the breweries, lets add the beers.  To do this, all we have to do is add a nested section to select each beer within each brewery, like so:
```
{{#section brewery ; separator="\n"}}
{{name}} Brewery:  {{#section beer ; separator=", "}}{{beer_name}}{{#end}}
{{#end}}
```

The output:
```
Dogfish Head Brewery:  60 Minute IPA, 90 Minute IPA, Punkin Ale
Troegs Brewery:  HopBack Amber Ale, Sunshine Pils, Mad Elf
```

## Example 4 ##
We can slightly simplify the previous example by using a more complex path expression and eliminate the need to have a neseted #section declaration.
```
{{#section brewery ; separator="\n"}}
{{name}} Brewery:  {{beer/beer_name ; separator=", "}}
{{#end}}
```

This template yields the same output as the previous.

## Example 5 ##
If you didn't notice, a few of the beers in the XML file contain a "season" node.  What if we wanted to list yearly beers and season beers separately?  You can use a predicate, just like XPath, as part of the expression to selectively show seasonal beers.

```
{{#section brewery ; separator="\n\n"}}
{{name}} Brewery:
  Year Round Beers:  {{#section beer[!season] ; separator=", "}}{{beer_name}}{{#end}}
  Seasonal Beers:  {{#section beer[season] ; separator=", "}}{{beer_name}} ({{season}}){{#end}}
{{#end}}
```

To get the year rounds beers we test to see if the current "beer" node does not contain a "season" node and do the opposite to get the seasonal beers.

This yields the output:
```
Dogfish Head Brewery:
  Year Round Beers:  60 Minute IPA, 90 Minute IPA
  Seasonal Beers:  Punkin Ale (Fall)

Troegs Brewery:
  Year Round Beers:  HopBack Amber Ale
  Seasonal Beers:  Sunshine Pils (Summer), Mad Elf (Winter)
```

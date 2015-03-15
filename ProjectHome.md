# About #

JXTL is a rapidly developing library aimed at providing a simple, yet powerful, general-purpose template language.  It currently includes a command line template processor which can use either JSON or XML as its data dictionary for the template.  Language bindings are currently being worked on.  Right now there is only support for Perl; but Python bindings should be done in the very near future.  The library is written in C so that it does not have to reimplemented in other languages, only the language bindings are required.  The JXTL library aims to be highly portable by using the [Apache Portable Runtime](http://apr.apache.org/) library.

The original motivation behind this project was to have a simple utility, available at build time, that can be used to generate code (in any language).  I wanted to be able to model my data in either XML or JSON and be able to easily transform that into code.  Originally I had been using XSLT or a Perl script, but this quickly became way too cumbersome.  Unfortunately, I couldn't find any available software that suited my needs.

Over the past few years, I have used and evaluated many different template languages.  I hope to implement the most useful features I have found in these.  I believe strongly in model-view separation and therefore will never allow a template to be a program.  For example, you should not be able to query a database from a template.  That should be done beforehand and the results placed into the data dictionary.  Outputting a template can not be allowed to cause side-effects.   If it can, you will eventually run into some really nasty and hard to debug problems.  Many template languages provide too much functionality and ultimately allow a developer to muddle the model-view separation.

These are some of my personal favorite template languages I discovered along the way:
  * [String Template](http://www.stringtemplate.org/)
  * [Google-CTemplate](http://google-ctemplate.googlecode.com/)
  * [JSON Template](http://code.google.com/p/json-template/)

These all enforce a strict model-view separation.  For different reasons, they are just not quite what I'm looking for.  String Template is the closest to what my view of an ideal template language is.  Terence Parr (the "maniac" behind String Template) is right on with what you should and shouldn't be able to do in a template language.  I share many of the same beliefs as him and hope to implement many of the features he has in String Template.

# Examples #
The easiest way for me to learn a new language is to see some examples.  So, we dive right in to show some of the features currently in the template language.

All examples will use the following simple XML file, which models some beers from two of my favorite breweries:
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

## Example 1 ##
Lets say we wanted to get some really basic information, like printing the name of each brewery in the file.  We could  do it with the following template:
```
{{#section brewery}}
{{name}} Brewery
{{#end}}
```
From this example we see that the template start and end markers are "{{" and "}}".  Template directives start with  "#", and an identifier within the curly braces means to lookup a value and print its string representation.  The template processor will automatically expand a section for all nodes that have the same name.  In this case we have two brewery nodes, so the section is expanded twice.

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
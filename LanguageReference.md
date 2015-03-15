# Template Delimiters #

As of right now, the template interprets everything between two open curly braces and two close curly braces, i.e. {{ ... }}.  It's quite possible that the template delimiters may be configurable in the future.  All other text is saved off verbatim until the template is expanded.

# Comments #

Comments begin with "{{!" and can be ended with either "!}}" or "-}}".  The difference is whether or not a newline directly following the comment is suppressed.  A "-" indicates that the template should consume the newline.
```
{{! This comment will not suppress a newline after it !}}
{{! This comment will suppress a newline after it -}}
```

# Path Expressions #
Path expression are very similar to XPath.  If it starts with an '/', the path expression is considered to be an absolute path.  If not, it will do the selection based off the current node.  Predicates can be used to reduce the number of nodes selected.  Some simple examples:

  * `/brewery/beer` (absolute path of all of the beer nodes that have a brewery parent node)
  * `beer[!season]` (relative path of all of the beer nodes that do not have a season node)
  * `/brewery[beer/season]` (absolute path of all breweries that do have a seasonal beer)


# Sections #

A section directive looks like this:
```
{{#section path_expression}}
section content
{{#end}}
```

path\_expression is evaluated and the content inside of the section is expanded for each node.  Sections can be nested.

## Separator ##
It is possible to specify a separator for a section, which can be a quoted C-style string.  The separator will be printed after each iteration of the section, except for the last iteration.  For example, to print a newline after each iteration you would do something like this:
```
{{#section path_expression ; separator="\n"}}
section content
{{#end}}
```

# Conditionals #
Like almost all programming languages, you can use if ... elseif ... else directives to evaluate a path expression and have conditional parts of a template.  The expression is considered true if any nodes are selected as a result of evaluating the expression.  However, a boolean node that is false actually does evaluate to false.
```
{{#if node[childnode]}}
There is a node with a childnode.
{{#elseif node}}
Just a node.
{{#else}}
No node.
{{#end}}
```

# Data Dictionary #
You can use either XML or JSON file as a data dictionary for your template for the jxtl command line program.  If you are going to use XML, you need to consider whether or not is has a clean conversion to JSON (because that is what is actually done internally).  The basic rule if you have mixed content it is not going to work correctly.  For example, don't do the following:

```
<node>
 some text
 <childnode>more text</childnode>
</node>
<node attr="some value">some text</node>
```

Both nodes in the preceding XML have mixed content.  If you have existing XML that is mixed content, convert it so that it does not contain mixed content, i.e. the above could be represented as.

```
<node>
  <text>some text</text>
  <childnode>more text</childnode>
</node>
<node attr="some value">
  <text>some text</text>
</node>
```

If you have XML and you want to use it in Perl/Python, see the [Language Bindings](http://code.google.com/p/jxtl/wiki/LanguageBindings) for how to read in the XML to a hash reference or dictionary.
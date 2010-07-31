%{
  #include <stdio.h>
%}

%option prefix="jxtl_path_convert_"
%option outfile="jxtl_path_convert.c"
%option noyywrap
%option reentrant
%option case-insensitive

%x directive options str path_expr comment

%%

<INITIAL>{
  "{{" {
    ECHO;
    BEGIN( directive );
  }
  "{{!" {
    ECHO;
    BEGIN( comment );
  }
  "{"|"\n" { ECHO; }
  [^{\n]+ { ECHO; }
}

<directive>{
  "#section" { ECHO; }
  "#end" { ECHO; }
  "#if" { ECHO; }
  "#else" { ECHO; }
  "#elseif" { ECHO; }
  [a-zA-Z0-9_$@^.*\()!]+ {
    int i;
    char c;
    for ( i = 0; i < yyleng; i++ ) {
      c = yytext[i];
      switch ( c ) {
      case '$':
	printf( "/" );
	break;

      case '@':
	printf( "." );
	break;

      case '^':
	printf( ".." );
	break;

      case '(':
	if ( i != 0  && i != yyleng - 1 ) {
	  printf( "[" );
	}
	break;

      case ')':
	printf( "]" );
	break;

      case '.':
	if ( ( i + 1 ) < yyleng && yytext[i + 1] != '(' ) {
	  printf( "/" );
	}
	break;

      default:
	printf( "%c", c );
	break;
      }
    }
  }
  ";" { ECHO; BEGIN( options ); }
  "}}" { ECHO; BEGIN( INITIAL ); }
  [ \t\r]+ { ECHO; }
  "\n" { ECHO; }
  . { ECHO; }

}

<options>{
  "separator" { ECHO; }
  "=" { ECHO; }
  "," { ECHO; }
  "}}" { ECHO; BEGIN( INITIAL ); }
  "\"" { ECHO; BEGIN( str ); }
  [ \t\r]+ { ECHO; }
  "\n" { ECHO; }
  . { ECHO; }
}

<str>{
  "\"" { ECHO; BEGIN( options ); }
  "\\b" { ECHO; }
  "\\f" { ECHO; }
  "\\n" { ECHO; }
  "\\r" { ECHO; }
  "\\t" { ECHO; }
  "\\\"" { ECHO; }
  [^\"\\\n]+ { ECHO; }
  "\n" { ECHO; BEGIN( INITIAL ); }
}

<INITIAL><<EOF>> {
  yyterminate();
}

<comment>{
  "-}}"[\n]? { ECHO; BEGIN( INITIAL ); }
  "!}}" { ECHO; BEGIN( INITIAL ); }
  [^!\-\n]+ { ECHO; }
  "!"|"-" { ECHO; }
  "\n" { ECHO; }
}

%%

int main( int argc, char **argv )
{
  yyscan_t scanner;
  jxtl_path_convert_lex_init( &scanner );
  jxtl_path_convert_lex( scanner );
  jxtl_path_convert_lex_destroy( scanner );
  return;
}

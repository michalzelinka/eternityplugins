////////////////////////////////////////////////////////////////////////////////
//                  ICQOscarJ Plus Mod 0.5.x.x from scratch                   //
////////////////////////////////////////////////////////////////////////////////
// last version: 0.1.5
// last modified: 2008/10/16 by jarvis
////////////////////////////////////////////////////////////////////////////////

! This folder should be strictly managed -- PLEASE be aware you fully understand
! the "rules" set for this folder and its content x)


// << ///////////////////// BASIC SYNTAX MODEL FOR NEW CODE ////////////////////

#include <something>
#include "something"

#define AB 1
#if defined( SOME_DEFINITION_SYMBOL ) // or #ifdef SOME_DEFINITION_SYMBOL
  #define CD 2 // optional indenting -- looks better a bit :)  
#else
  #define CD 3
#endif /* SOME_DEFINITION_SYMBOL */ // with a comment to know what IF clause we are closing

#ifndef SOMETHING // or #if not defined( SOMETHING )
  #error Error report
#endif /* !SOMETHING */

#define MAX( a, b ) ( ( a > b ) ? a : b )

// definitions like to be ordered when they are similar -- also it's very nice
// to easily see values indented (with spaces!) in nice row :)
#define DEFAULT_LONGSTRING_DEFINING                  "abcd"
#define DEFAULT_ANOTHER_LONGSTRING_DEFINING          12983712
#define DEFAULT_JUST_ONE_MORE_LONGSTRING_DEFINING    __TIME__
#define DEFAULT_LAST_LONGSTRING_DEFINING( x )        x

#pragma( )


class ClassName
{
  type* variableName;

  return_type* functionName( type variableName ) // spaces around are OPTIONAL, but use ONE system in a whole code
  {
    // code

    return return_variable;
  }
}


typedef struct
{
  int a;
  char b[ 255 ];
  union
  {
    char szValue[ 255 ];
    TCHAR tszValue[ 255 ];
    wchar_t wszValue[ 255 ];
  }
} struct_name;


return_type* ClassName::functionName( type variableName )
{
  type variable; // var definitions
// EMPTY (no spaces) lines between code sequences that do not belong together
  if ( some_condition ) // simple IF
    do_something( );

  if ( ( a != b ) && ( b != a ) ) // IF with complex body
  {
    do_something( );
  }

  i = some_condition() ? 1 : 2; // ternary operator

  function_call( // calling some function with many parametres
    hwnd,        // it's better to have each argument on a separate 
    IDI_VALUE,   // line in some rare cases when the whole line would
    condition( ) ? // be longer than the screen -- like this comment x)))) ;)
      1 : 2,
    another_attribute(),
    yet_another,
    ( BYTE )last one
  );

  for ( i = N; i < M; i++ )
  {
    // do something
  }

  while ( condition )
  {
    // do something
  }

  do
  {
    // do something
  } while ( i == condition( ) );

  functionCall( );
  i = functionCall( );
  i++;
  i = i / 10;
  i /= 10;

  switch ( i )
  {
  case A :
    do_something( );
    break;
  case B :
    do_something_another( );
  case C :
    yet_another_action( );
    break;
  default:
    {
      int i = 0;
      i++;
    }
    break;
  }

  return variable;
}
// 2 empty lines between definitions

return_type function( )
{
  // another code
  // ..

// << ////////////////// BASIC SYNTAX MODEL FOR NEW CODE -- END ////////////////

// << NAMING //

-  functions, methods, procedures:
  type functionNameItself( ); // default 1
  type functionType_functionNameItself( ); // sometimes function type is obsolete, when it's used for simple or determined operation
  type ComplexiveFunctionNameAllInOne( ); // GNU -- default 2

-  variables:
  type theNameOfTheVariable; // defualt
  type the_name_of_the_variable; // GNU -- not very nice solution, but acceptable x)
  
  GNU Coding Standards say the following system should be used:
  
  class ClassName
  {
    int variable_name;
    int FunctionName(argument_name, another_argument);
  }
  
  Maybe you noticed I'm using the same naming convencion for both functions and
  variables. Reason: variables and functions are "objects" of the same usage
  level -- no matter if you are requesting value from a function or a
  variable -- you'll get the same result. No matter if you call:

  if ( isOnline ) // bool variable
  ..or..
  if ( isOnline( ) ) // function returning bool value
  
  More complexive or important functions may be named with starting capital,
  standard functions are usually named 

// << NAMING -- END //

- when writing NEW code, please:
- USE SPACES! ;) no nasty tabs, when you change the width of the tab (and this
  may happen even when opening file in different editors) the code can look
  very ugly x) BUT use the same one size everywhere :) -- even GNU prefers them.
- follow the included syntax model -- PROPER indenting (!!), the code must be
  nice like a rhyme :]
- try to find the most effective, clean and nice algorhythm, no matter if it
  is for simplier or complexive solution

- when editing EXISTING code:
- try to keep the code as much NICE as possible
- please use the SAME indenting & maybe even formatting method like the
  nearest surrounding of the place you are editing -- when tabs, use tabs,
  when spaces, use spaces.
- try to not modify the existing code if it's not required, if so, do it
  by respecting the above lines :)
  
For information about GNU C coding standards, see:
http://www.gnu.org/prep/standards/standards.html

////////////////////////////////////////////////////////////////////////////////

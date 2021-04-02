# Formatting Files
.c: 
    - Global variables should be before function implementations  
    - Includes should be at the top of the file  
        - std library includes should be first, include directory (libc, libk, etc.) should be next, then 
          includes from the current directory or a specified absolute path
        - includes should be grouped based on file path
.h:  
    - Each .h file must have an include guard  
    - Each section should be denoted by a comment before the section starts and should be in the order below  
    - Sections  
        - includes
        - defines
        - structs
            - enums, unions, etc. should also be defined here  
        - typedefs
        - functions
            - functions should be grouped based on what they do (e.g. functions for modifying mem grouped together)  
assembly:
    - Intel syntax only (unless it is inline assembly)
    - comments should be at the end of a line

# Naming Conventions
structs:
    - names should be in snake case
typedefs:
    - typedefs should be in snake case and end with an _t
defines/macros:
    - all uppercase
    - underscores should seperate words
constants:
    - global constants should be all upercase with underscores seperating words
variables:
    - names should be snake case
    - pointer variables should be written type *name, not type* name
functions:
    - names should be snake case
    - functions with no parameters should be written as foo(), not foo(void), except when defining interfaces/function pointers
    - functions with pointer return types should be written type *foo(), not type* foo()
    - system initializer functions (like the ones that initialize threads and processes) should be named
      init_<system>(), whereas initializer functions for structs/types should be named <struct>_init()
    - function names should start with the file/system they belong to
comments:
    - multiline comments should be marked with /* */
        - the content of the comment should start on the same column over multiple lines, ex.:
            /* this is
             * a comment */
    - every function must be commented with an explanation of the function, then an @param section, then an @return section
        - when the return type of the function is void, the @return section doesn't need to be specified
        - when a function has no parameters, there should be no @param section
        - Ex: /** This function is an example for commenting functions
               *
               * @param foo: first parameter
               *
               * @return a return value 
               */
              int foobar(int foo) {}
    - every file must have a comment at the top of the file explaining the contents of the file
file names:
    - must be snake case
    - file names should be kept to one to two words

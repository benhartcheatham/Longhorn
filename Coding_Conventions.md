<!-- 
Old version text comes through exactly how it looks
# Formatting Files
<pre>
- .c:
    - Global variables should be before function implementations  
    - Includes should be at the top of the file  
        - std library includes should be first, include directory (libc, libk, etc.) should be next, then 
          includes from the current directory or a specified absolute path
        - includes should be grouped based on file path
    - Sections
        - includes
        - defines
        - structs (if applicable)
        - globals
        - prototypes (if applicable)
        - functions  
- .h:
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
- assembly:  
    - Intel syntax only (unless it is inline assembly)  
    - comments should be at the end of a line  
    </pre>  

# Naming Conventions
<pre>
- structs:  
    - names should be in snake case  
- typedefs:  
    - typedefs should be in snake case and end with an _t  
- defines/macros:  
    - all uppercase  
    - underscores should seperate words  
- constants:  
    - global constants should be all upercase with underscores seperating words  
- variables:  
    - names should be snake case  
    - pointer variables should be written type *name, not type* name  
- functions:  
    - names should be snake case  
    - functions with no parameters should be written as foo(), not foo(void), except when defining interfaces/function pointers  
    - functions with pointer return types should be written type *foo(), not type* foo()  
    - system initializer functions (like the ones that initialize threads and processes) should be named  
      init_<system>(), whereas initializer functions for structs/types should be named <struct>_init()  
    - function names should start with the file/system they belong to
- comments:  
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
- file names:  
    - must be snake case  
    - file names should be kept to one to two words  
    </pre>

# Version Numbering
<pre>
For right now, just add the change to the "summary of version changes" comment in kernel/kernel.c in the format \<major\>.\<minor\>.\<patch\>: \<changes\>.  
The comment should be wiped with every minor version change.   -->

<!-- New text formatted with html tags to look pretty in markdown -->
# Formatting Files
- .c:
    - Global variables should be before function implementations  
    - Includes should be at the top of the file  
        - std library includes should be first, include directory (libc, libk, etc.) should be next, then 
          includes from the current directory or a specified absolute path
        - includes should be grouped based on file path
    - Sections
        - includes
        - defines
        - structs (if applicable)
        - globals
        - prototypes (if applicable)
        - functions  
- .h:
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
- assembly:  
    - Intel syntax only (unless it is inline assembly)  
    - comments should be at the end of a line  
# Naming Conventions
- structs:  
    - names should be in snake case  
- typedefs:  
    - typedefs should be in snake case and end with an <code>_t</code>  
- defines/macros:  
    - all uppercase  
    - underscores should seperate words  
- constants:  
    - global constants should be all upercase with underscores seperating words  
- variables:  
    - names should be snake case  
    - pointer variables should be written <code>type \*name</code>, not <code>type\* name</code>  
- functions:  
    - names should be snake case  
    - functions with no parameters should be written as <code>foo()</code>, not <code>foo(void)</code>, except when defining interfaces/function pointers  
    - functions with pointer return types should be written <code>type \*foo()</code>, not <code>type\* foo()</code>  
    - system initializer functions (like the ones that initialize threads and processes) should be named  
      <code>init_\<system\>()</code>, whereas initializer functions for structs/types should be named <code><struct>_init() </code> 
    - function names should start with the file/system they belong to
- comments:  
    - multiline comments should be marked with /\* \*/  
        - the content of the comment should start on the same column over multiple lines, ex.:  
            <pre><code>/* this is  
           * a comment */</code></pre>  
    
    - every function must be commented with an explanation of the function, then an <code>@param</code> section, then an <code>@return</code> section  
        when the return type of the function is void, the <code>@return</code> section doesn't need to be specified  
        - when a function has no parameters, there should be no <code>@param</code> section  
        - Ex: <pre><code>/** This function is an example for commenting functions  
               &nbsp;*  
               &nbsp;* @param foo: first parameter  
               &nbsp;*  
               &nbsp;* @return a return value  
               &nbsp;*/  
               &nbsp;int foobar(int foo) {}  </code></pre>
    - every file must have a comment at the top of the file explaining the contents of the file  
- file names:  
    - must be snake case  
    - file names should be kept to one to two words  

# Version Numbering
For right now, just add the change to the "summary of version changes" comment in kernel/kernel.c in the format <code>\<major\>.\<minor\>.\<patch\>: \<changes\></code>.  
The comment should be wiped with every minor version change.  
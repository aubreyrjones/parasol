The Parasol Language
====================

This document is an introduction to the Parasol shading language. This should be your first stop in learning Parasol.

It's generally assumed in this document that you are familiar with the OpenGL graphics pipeline, and have written
shaders for it using GLSL or some similar technology. While Parasol's compiler is modular enough to accommodate
different runtime environments, such as OpenCL or DirectX, at present only OpenGL GLSL and Vulkan SPIR-V are targeted.


Note on capitalization: While I've tried to be consistent in the project documentation, "Parasol" and "parasol" are
both acceptable capitalizations.


What is Parasol?
----------------

Parasol is a pragmatic functional shading language, designed to make shader code reuse as simple as possible. What does
that mean?

* "pragmatic" - Parasol's purpose is to make work easier, not to embody design purity.

* "functional" - Parasol does not have mutable state, and supports higher-level functional composition at
                 compile-time. Stage outputs are written in terms of expressions and functions of stage inputs. This
                 paradigm was selected because of its excellent match to real-world GPU expectations of shader programs.

* "shading language" - Parasol is for programming GPU shader stages, and does not make concessions to being a
                       general-purpose programming language. It will never be self-hosting.
                       
* "code reuse" - Parasol's **first priority** is to make it effortless to build modular, composable shader elements. You
                 shouldn't ever need to repeat yourself.
                 

What Parasol Isn't
------------------

Mainly, Parasol isn't a general purpose programming language. It's missing many features you would expect in a "real"
programming language. In general, these features are missing either because they're irrelevant for shader code (strings,
file IO, etc.), they're extremely expensive to implement on GPU (jump tables), or they're impossible to implement
generically at runtime on contemporary GPU hardware (recursion and function-typed values).

Also, there is a good argument that Parasol is not the best introduction to shader programming for somebody new to it.
Leaving aside the task of integrating the runtime library, Parasol handles many details implicitly that are quite 
relevant to shader performance and correctness, and which are made explicit by e.g. GLSL.   


Preliminaries
=============

Execution Model
---------------

It's expected that Parasol code will be executed by something resembling the OpenGL shader model. In particular, Parasol
is designed to break computation into several "stages". Typically, these will correspond to the OpenGL shader stages:
vertex, tesselation control, tesselation evaluation, geometry, and fragment. If you're not familiar with the OpenGL
pipeline, this would be a good time to look up some documentation. Don't worry if you aren't sure how to use the
tesselation or geometry stages; you aren't required to.

It's additionally expected that the code for each stage will be executed *many* times, once for each element to be
processed (vertex, patch, fragment, etc.). Much of the versatility of a "real" programming language is actually captured
by the environment enclosing Parasol.


Identifiers
-----------

Pipelines, functions, and variables have identical naming rules. Names must start with a letter or `_`, and may be
followed by any number of letters, numbers, underscores, and question marks (`?`).


Comments
--------

Parasol uses the semicolon (`;`) to introduce a comment. A comment lasts until the end of the line.


Source Files
------------

Parasol source file names end with the `prsl` extension.

The `prslc` compiler expects 8-bit ASCII-encoded text. No unicode support is built in, although the extended ASCII
characters should be fine.


Program Structure
=================

Pipelines
---------

The basic unit of organization in Parasol is the pipeline. Within a Parasol source file pipelines are defined at the top
level by giving a name, followed by a series of declarations and expressions inside curly brackets. The simplest
pipeline is empty and looks like this:

    an_empty_pipeline {
    }

An empty pipeline obviously isn't much good. Within each pipeline, you can declare new inputs, and define new variables
and functions. This pipeline defines some common constants: 
 
    constants {
      pi = 3.14159
      e = 2.718
      c = 299792458
    }
    
The primary goal of pipelines is to permit code reuse. You can bring all of the declarations and definitions from one
pipeline into another pipeline using the `include` keyword. This isn't a textual include (like C/C++), but it does not
introduce any additional namespaces: the definitions in the included pipeline are brought directly into the enclosing
pipeline's namespace.

    circle_functions {
      include constants
      
      def area radius => radius * radius * pi 
    }


Variables
---------

Unlike procedural languages, variables in Parasol are not mutable. This means that their value is computed once, and
then does not change during the current execution. 

Simply mentioning a variable is enough to "declare" it. On its own, this is really only useful for declaring interfaces
or contracts. But we'll see later that when used with stage-scoping, this can be used to declare new vertex attributes. 

    phys_object {
      localCenterOfMass
      inertiaTensor
    }

You define a variable when you set its value:

    g_wing {
      localCenterOfMass = vec3(0, 1.02, 0)
    }

Variable definition and declaration don't need to come in any particular order. And a variable does not need to be
declared before use (or ever, outside of the definition).

    redundant {
      x = 28
      x
      y = 7
    } 

All variables are statically typed. That means any variable can only hold one type of object, and that type is
determined at compile time. The type of a variable can be explicitly stated immediately after the variable introduction,
or you can omit the type and allow the compiler to automatically infer the type based on the value you assign it. If you
do specify the type, it is an error to redeclare the same variable with a different type, or to attempt to assign a
value of any other type to that variable. 

    explicit_and_inferred {
      someVar: vec3 = vec3(1, 2, 3)    ; explicit
      anotherVar = vec3(1, 1, 1)    ; inferred
    }


Primitive Types
---------------

Parasol supports the same primitive types as GLSL, with the same spellings, except that signed and unsigned 32-bit
integers are called `i32` and `u32` respectively.


Structs
-------

New composite datatypes can be defined by the Parasol programmer using structs. A struct is a collection of one or more
named and typed variables. For instance, the typical vertex attributes might be defined like this:

    struct attributes {
      v_inPosition: vec3
      v_inNormal: vec3
      v_texCoord: vec2
    }

Members of structs are accessed with the `.` operator.

    def getDepth attr: attributes  => attr.v_inPosition.z

A struct can appear as the type of any variable or parameter declaration.


Arrays
------

An array of any type can be declared by adding `@` followed by an integer to any type in a variable declaration:

    struct point_light {
      position: vec3
      color: vec4
    }
    
    apipe {
      u[lights: point_light@16]
    }

Arrays are accessed with the same syntax as structs, using the `.` operator:

    apipe {
      u[lights: point_light@16]
      
      def getLight i => lights.(mod(i, length(lights))
    }

All arrays are statically sized at compile time.


Stage Scope
-----------

A GPU shader consists of several stages of computation, each of which is executed after the other, with strict rules
about passing data from one stage to the next. Additionally, input is given to the shader program in the form of uniform
variables and vertex attributes. Parasol attempts to unify all of this down into a concept called "stage scope".

In short, a variable may be given a particular stage scope by wrapping its declaration in square brackets and prefixing
it with the name of the desired stage. The following pipeline declares the usual three uniform matrices, and a vertex
position attribute.

    standard_geom {
      u[projMat: mat4]
      u[viewMat: mat4]
      u[modelMat: mat4]
      
      a[v_inPos: vec3]
    }
  
A variable declared with stage scope is **not defined until its enclosing stage is executed**. For instance, this is the
standard vertex position calculation:

    vert_pos {
      include standard_geom
      
      v[v_pos] = viewMat * modelMat * vec4(v_inPos)    ; stage scope not needed to reference v_inPos
      
      v[gl_Position] = projMat * v_pos    ; here we fulfill the OpenGL vertex shader contract
     }

Variables (scoped or otherwise) can also be declared inline:

    vert_color {
      f[fragColor] = 
        v[v_outColor] = 
          a[v_inColor: vec4]
    }

All together, this makes for some very compact notation for common patterns:

    simple_pipeline {
      v[gl_Position] = 
        u[projMat: mat4] *
        u[viewMat: mat4] *
        u[modelMat: mat4] *
        a[v_inPos: vec4]

      f[outColor] = 
        texture(
          u[tex: sampler2D], 
          v[v_texCoord] = 
            a[v_texCoord1: vec2]
        )
    }


OpenGL Stages
-------------

Since stage scopes are used frequently, Parasol assigns them very short names. For the OpenGL stages, they are as
follows:

* uniform variables - `u` - ex: `u[projMatrix: mat4]`
* vertex attributes - `a` - ex: `a[inPos: vec3]`
* vertex shader - `v` - ex: `v[outNormal] = normMat * inNormal`
* tesselation control - `tc` - not yet implemented
* tesselation evaluation - `te` - not yet implemented
* geometry - `g` - not yet implemented.
* fragment - `f` - ex: `f[outColor] = texture(tex1, texCoords)`

Note that since no Parasol code sets the values of either uniforms or attributes, the programmer *must* specify the
types when declaring these interface variables. Their values will come from calls to `glUniform..()` or from vertex
buffer values. In general, variables whose values are defined by parasol code do not need to have types explicitly
declared.


Let
---

Sometimes it is nice to have a short name for a very long expression, without having to define a new permanent variable
or function. Parasol provides the `let` expression for this purpose, which allows you to define multiple variables with
scope limited to the expression following the `in` clause. A contrived example:

    foo = 
      let 
        a = 1
        b = 2
        c = 3
      in vec(a, b, c)


Psi Expressions (or, why there is no `if`)
------------------------------------------

In general, GPUs do very poorly at branching and non-uniform flow control. In general, when a branch is encountered, and
different executions of a shader in the current execution group take different paths, each execution unit actually
*executes* both branches. The divergent flow control is simulated by disabling memory writes on the execution units where 
the branch is "not taken". As a result, Parasol does not support traditional flow control.
 
Instead, Parasol provides psi expressions. Despite the fancy name (borrowed from the collapsing wave function of quantum
physics), a psi expression is merely a set of conditions and results, enclosed in curly brackets. In its simplest form, 
it looks almost like an `if` or a `switch` statement in a procedural language.

    apipe {
      negative = {
        a < 0    ; first condition 
          => true    ; first result
        _ => false}    ; default condition and result
    }

Within a psi expression, each condition is tested in sequence; the result of the entire expression is the result of the
first condition that tests true. `_` is a special identifier (not symbol or keyword) within psi expressions that always
evaluates `true`; when placed as the condition of the last case, it is used as a "catchall" condition. (The last
condition of the psi expression above could have been written `true => false`, but that's confusing. Thus the `_`
identifier.)


Functions
---------

Parasol supports the grouping of common expressions into functions. A function may be defined at the top level of a
file, in which case it has global scope and may be called anywhere. Or, a function may be defined within a pipeline,
in which case it's only callable from expressions within that pipeline or within another pipeline including the first.  

A named function is defined by using the keyword `def` followed by the function's name, then a comma-separated list of
parameters (with optional types). For example:

    def square a => a * a
    
    def cross a: vec3, b: vec3 => 
      vec3(a.y * b.z - a.z * b.y, 
           a.z * b.x - a.x * b.z,
           a.x * b.y - a.z - b.x)


Functions are called by specifying the function name, then enclosing the arguments in parentheses:

    apipe {
      v[v_outNorm] = 
        normalize(
          cross(v - u, v - w))
    }

Functions may be defined in pipelines with stage scope, in which case they close over the stage inputs, and are only
callable from the stage in which they're defined (and *not* subsequent stages, like stage-scoped variables). 

    apipe {
      a[v_inPos]
    
      def v[calcNorm] u, w => normalize(cross(v_inPos - u, v_inPos - w))
      
      v[outNorm] = calcNorm(a[u: vec3], a[w: vec3])
    }

Of course, functions can also be declared in pipelines *without* stage scope. In this case, the pipeline simply serves
as a namespace.

    apipe {
      def mult a, b => a * b
    }

Functions cannot be recursive, either directly or indirectly. No matter how tempting, it is a compile-time error to
define something like this:

    def recFactorial x => {
      x > 1 
        => x * recFactorial(x - 1)    ; cannot recursively call yourself
      _ => 1}

Lambda
------

Lambda expressions allow you to define a new anonymous function in place. The new function will have access to all
variables defined in that scope, including variables defined with `let` expressions (see below). A lambda is expressed
with the `\` operator, followed by a parameter list just like a function declaration, and the body expression:

    apipe {
      u[lights: point_light@16]
      
      sumLights = 
        reduce(lights,
          \sum, item => sum + item)
    }


Map, Reduce (and why there is no `while`)
-----------------------------------------

I'm sure you're tired of hearing the excuse, but GPUs do poorly with dynamically-bounded iteration. That is, they don't
do well with loops like `while (true) {/*do something*/ if (cond) break;}`. They do just fine with loops like `for (int
= 0; i < 3; i++) do_something(i);`, and they even do okay with changing lengths of iteration so long as all executions
of a shader take the same number of steps in the loop. In the worst case, divergent looping conditions are handled with
the same klugey write-mask as conditional branches. 

As a result, Parasol does not include support for traditional looping. Instead, it provides the functions `map` and
`reduce`. Their signatures look like this, although their actual bodies cannot be implemented in Parasol itself:


A Note on Higher Order Functions
--------------------------------

Parasol includes a handful of higher order functions: `apply`, `map`, `reduce`, etc. These are functions that take a
function as an argument. Additionally, user functions may accept functions as parameters.

You should be aware that due to the GPU's lack of function pointers, using higher-order functions results in the
compiler defining a *new* function body for each combination of higher-order function and parameter function. I'll
restate that for clarity: function composition happens at compile-time, and not at run-time. `apply(x, y, foo)` is a
different "apply" than `apply(x, y, bar)`.




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

Parasol source files end with the `prsl` extension.

A Parasol source file must end in a blank line.

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

All variables are statically typed. That means any variable can only hold one type of object. The type of a variable can
be explicitly stated immediately after the variable introduction, or you can omit the type and allow the compiler to
automatically infer the type based on the value you assign it. If you do specify the type, it is an error to redeclare
the same variable with a different type, or to attempt to assign a value of any other type to that variable. 

   explicit_and_inferred {
     someVar: vec3 = vec3(1, 2, 3)    ; explicit
     anotherVar = vec3(1, 1, 1)    ; inferred
   }


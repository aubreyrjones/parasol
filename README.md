Parasol
-------

This readme represents a fresh start. 

I originally started work on Parasol 6 years ago, got it parsing, and
then couldn't figure out where to start on codegen. I've grown a lot
as a designer and engineer since then, and so has the GPU technology
stack.  When I took this project up again recently, I used the
original Lemon grammar file as test input for my
[lemon-py](https://github.com/aubreyrjones/lemon-py) project.  But now
equipped with years' more experience and vastly more flexible tooling,
it's clear that there are several defects in both the design of the
grammar and the language itself.

Truthfully, I've _forgotten_ a great deal of what I was planning with
the particular syntax of the old language. Some parts are crystal
clear: pure functional with a rich expression language, no explicit
loops, entirely case-based branching, and inline callout of GPU stage
(vertex, fragment, etc.). What I don't think I'd really considered was
how _not_ to use the syntax.  I was just so enamoured with seeing my
ASTs on the screen, I wanted to keep making them more
complicated. That doesn't make for good design.

To get an idea of the goals and design of the language, check the
documentation, grammar, and examples in the `legacy/` directory of
this project. You go ahead and look. I'm trying not to. :)

I'm starting over fresh, with only some design goals in mind:

  * Parasol is a language for writing graphical shaders, with
    first-class support for GPU primitives. It intentionally _lacks_
    constructs that cannot be realized on GPU (such as string data
    types or IO routines).

  * Parasol is for more than games. While I have worked in games, I
    work in medical imaging now. I would like more people to take
    advantage of the GPU, and I think a more integrated, expressive
    environment will help. GLSL is intimidating, and piecing together
    a complete pipeline from fragmented shader text is error-prone and
    confusing.

  * Parasol is _not_ a language for GPGPU programming.  Specifically,
    Parasol is not designed to make the GPU environment more like the
    CPU environment. OpenCL, CUDA, and the proliferating GPU math
    libraries provide excellent GPGPU services already, and Parasol
    doesn't intend to reinvent the wheel.

  * Parasol must be pragmatically "better" than GLSL and HLSL in
    tangible ways. It must be more readable, more understandable, and
    less "magical" than traditional GPU programming. It should express
    simple or ubiquitous operations with simple syntax.  It should
    avoid all boilerplate.

  * Parasol must **both** ease the process of creating shader
    pipelines for new-folder projects, and scale to integration in AAA
    game engines and HPC scientific imaging.

  * The Parasol runtime library must be implemented in self-contained
    C++.
    
  * I reserve the right to implement the compiler in whatever language
    I find convenient.

  * Parasol should be a pure-functional language, as this fits
    perfectly with the GPU paradigm and there are few points of
    interface to procedural systems. A functional language allows the
    compiler to know "everything" about the state of the program at
    any point, allowing Parasol to free the programmer of (hopefully)
    all the boilerplate of GPU programming.
  
  * Parasol must be aware of the staged nature of GPUs which
    effectively introduces both an implicit scope to all calculations,
    and requires interface variables between stages. Parasol should
    not, however, depend on the _behavior_ of any particular
    stage. The stage definitions must be configurable for different
    rendering APIs.

  * Parasol should deduce types wherever possible.

  * Parasol must output SPIR-V code, and probably should output GLSL
    and also whatever DirectX uses.



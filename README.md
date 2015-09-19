Parasol
=======

Parasol is designed to make writing graphical shaders much, much less laborious than it is now. Languages such as GLSL
and HLSL require a lot of boilerplate code redefining interface variables in each shader stage. And they make it
difficult or impossible to create modular components. You end up rewriting the same vertex position multiply over and
over again, in each (very-slightly) different vertex shader.

Parasol is a functional language, designed to permit extreme shader code reuse. Instead of specifying calculations with
stepwise instructions, in Parasol you simply specify the values of outputs as expressions of the inputs. Sets of input
variables, output variables, and functions are grouped into namespaces called 'pipelines'.

For example:

    vertex_position {
        v[v_position] = u[viewMatrix: mat4] *
                        u[modelMatrix: mat4] *
                        vec4(a[v_inPosition: vec3], 1)
        
        v[gl_Position] = u[projMatrix: mat4] * v_position 
    }
    
That small snippet of code declares a vertex output called `v_position`, three uniform matrices, and a vertex attribute.
It also writes to gl_Position (fulfilling the GLSL vertex shader contract).

You can include `vertex_position` into other Parasol pipelines, making the outputs and functions defined within that
pipeline available in the enclosing one. This isn't a textual include, but it's also not really an "import" either. The
namespaces are totally flattened, and all redundant interface definitions are merged. Conflicting interface specs
generate errors, but you can refine previously-declared specs with additional information).

    object_color_complete {
        include vertex_position
    
    
        v[v_color] = a[v_inColor: vec4]
        f[outColor: vec4 0] = v_color    ; that's the output index after the type
        
        a[v_inPosition: 0]    ; refine vertex attribute with index spec
        a[v_inColor: 1]
        
    }


Here's a [longer sample](parasol_examples/simplicity.prsl).
For a more complex example, check out the [Phong-shading example](parasol_examples/phong.prsl).

There's also a [language readme](LANGUAGE_README.md) that provides information on Parasol syntax and semantics. It
should be considered entirely and completely unstable right now.


Sounds neat, will I be able to use it in my project?
----------------------------------------------------

    Don't try yet, Parasol is still in the very early stages. This section is largely speculative. :)
    
**Short Answer**: The goal is to enable shaders written in Parasol and statically compiled to be loaded and used in any
environment with a C++11 compiler, and either OpenGL 3.3+, OpenGL ES 3.0, or Vulkan (when available).

**Long Answer**: We've tried to make parasol fairly self-contained and easy to integrate. C++ standard libraries aside,
everything necessary to build Parasol is contained in this repository. The default build artifacts are
position-independent static libraries, and the [license](LICENSE) permits both FOSS and commerical use with any linking
model.

There are three elements to the Parasol project: `libparasol`, which provides platform-specific runtime routines to
load, link, and inspect compiled Parasol code; `libprsl`, which encapsulates the Parasol compiler logic for use
by a client application; and `prslc`, which is a driver program for `libprsl`, implementing a command-line Parasol
compiler.

In most simple use cases, the client application need only link with `libparasol` (or include the source files into your
existing build). This will allow most applications to easily load fully-compiled Parasol shaders without needing any of
the code necessary to parse or compile Parasol code. The goal is to keep `libparasol` as lightweight as possible,
as its main purpose is simply to load GLSL or SPIR-V code from sqlite files.

Linking with `libprsl` is only necessary when shaders are going to be generated from Parasol source or serialized ASTs
at runtime. This may be the case if integrating Parasol as a dynamic tool in a game engine, or if you are using Parasol
for runtime shader metaprogramming. But in many cases, simply using `libparasol` to load the compiled shaders should be
sufficient.

Additional libraries are included as source code in the Parasol project and built into the Parasol library. These
libraries are:

    * TinyFormat - general text formatting in all components
    * TCLAP - used by `prslc` to parse commandline arguments
    * sqlite - used everywhere [ANTICIPATED, not yet included]

Additionally, the building the compiler components depends on the `lemon` parser generator (included in `./lemon`)
to generate the parser. This step is included automatically in the CMake build, but can be executed offline if you
aren't hacking on Parasol itself (but do build `libprsl` from source). In any case, neither the source code for `lemon`
nor the executable itself is necessary for using any Parasol module; it's necessary only when building the `libprsl`
module.


Anticipated Questions
---------------------

Q. What C++11 elements do you use?

A1. In `libparasol`, C++11 use is quite limited. This means that you should be able to load statically-compiled Parasol
shaders even with partially-compliant C+11 compilers (like MSVS 2013). If you have *no* C++11 support on your platform,
you can probably hack in sufficient support with a few typedefs and some macros.

A2. In `libprsl` and `prslc`, we may use any and all **standard** C++11 features. Writing a compiler is already a pain
in the ass, doubly so in C++. Since nobody in games likes Boost (with arguably good reason), C++11 is the only thing
that helps to ease some of that pain.

*

Q. What's the output look like?

A. While implementation hasn't reached the codegen phase, there are two broad categories of output: serialized ASTs and
compiled (realized) shaders. Both are stored in sqlite files.

Serialized ASTs are designed for dynamic shader generation. They allow source code to be pre-parsed, and for much of the
linking and analysis to be handled offline during asset bake. Client applications can then use pre-processed Parasol
files as a library when generating new shaders at runtime.
 
Shaders are finally compiled down into blocks of GLSL and SPIR-V code. The structure of both is very similar, with the
GLSL code generated in an SSA style like the SPIR-V code requires. While high-quality GLSL output is a priority, we
expect that Vulkan will be the more efficient implementation.


*

Q. Just GLSL and SPIR-V? What about $shader_language output?

A. I don't use $shader_language. Did I just hear you volunteer? 

*

Q. What about shader code for console $foo?

A. I don't have a license or an NDA with that company. If you work for or with them, feel free to contact me. We may be
able to work something out.






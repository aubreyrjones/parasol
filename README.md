parasol
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
        f[outColor: vec4 0] = v_color    // that's the output index after the type
        
        a[v_inPosition: 0]    // refine vertex attribute with index spec
        a[v_inColor: 1]
        
    }

For a more complex example, check out the [Phong-shading example](parasol_examples/phong.prsl).


Sounds neat, will I be able to use it in my project?
----------------------------------------------------

    Don't try yet, it's not complete. This section is speculative. :)
    
I've tried to make parasol fairly self-contained and easy to integrate. No special tools are needed to build Parasol
other than a C++11 compiler and CMake. It should build cleanly on standards-compliant compilers for any platform; if it
doesn't, file a bug. I use UNIX conventions in this documentation and in the CMake file, but don't be fooled--Parasol
works fine on Windows.

There are three elements to the Parasol project: `libparasol`, which provides platform-specific runtime routines to
load, link, inspect, and translate compiled Parasol code; `libprsl`, which encapsulates the Parasol *compiler* for use
by a client application; and `prslc`, which is a standalone Parasol compiler used for offline shader compilation. All
objects are built as position-independent static objects.

In most simple use cases, the client application need only link with `libparasol` (or include the source files into your
existing build). Linking with `libprsl` is only necessary when shaders are going to be generated from Parasol source at
runtime; this is not typically the case.

Additional libraries are included as source code in the Parasol project and built into the Parasol library. These
libraries are:

    * sqlite3

Parasol also uses several elements of the Boost library: __these elements are header-only and do not require any
additional compilation.__ This small subset of Boost is packaged with Parasol in the `include/boost` directory. If
you're already using Boost (and you should be), you can omit this copy from the build; otherwise, you must make sure
that the `parasol/include` directory is on the include path, or that Boost 1.59+ is installed on your system include 
path.

Additionally, the compiler components depend on the `lemon` parser generator (included in `parasol/lemon`) to generate
the parser. This step is included automatically in the CMake build, but can be executed offline if you aren't hacking
on Parasol itself.


Anticipated Questions
---------------------

Q. Why C++11? Why not '98, or C, or Java?

A. I'm doing this in my spare time. Pay me to write Parasol and we can discuss the implementation language.

*

Q. Damn... Boost? Really?

A. It's just a little Boost. I only include what I use and the dependencies thereof. But, really, it's 2015. The
header-only parts of Boost are totally worth the trouble of adding a `-I` to your compile line.

*

Q. What's the output look like?

A. I haven't gotten that far with implementation, but the plan is to generate GLSL and SPIR-V output. GLSL code will
probably be static-single assignment

*

Q. What about HLSL output?

A. I don't use DirectX. Did I just hear you volunteer?

*

Q. What about shader code for console $foo?

A. I don't have a license or an NDA with that company. Feel free to contact me about it.






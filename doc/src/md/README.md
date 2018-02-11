Dynacoe {#mainpage}
=======

Dynacoe is a cross-platform C++11-compliant API designed to simplify the
development process of games and other multimedia projects while still granting 
the high-level performance expected of native code. It contains built-in tools to access audio, rendering capabilities, input, and basic window management through a streamlined interface.

GitHub page: [http://github.com/jcorks/Dynacoe/](http://github.com/jcorks/Dynacoe/)

[Downloads section](@ref downloads)

[Toolkit usage tutorial](@ref toolkit)

[Basic Examples](https://github.com/jcorks/Dynacoe/tree/dev/build/Basic)


Status
------

Version: 0.96


Why use Dynacoe?
----------------

Dynacoe may not be the answer for everything, but it has a feature
set that would be ideal in some projects.

- *Modular Dependency Model* Almost all system-dependent functionality in consolidated in discrete "Backends". These implement a generic set of functionality that Dynacoe itself relies on, allowing you to swap what Dynacoe compiles against.
- *Aggregate + Inheritence-based Feature Agents* Dynacoe's interface utilizes a feature set that is expandable to fit your needs via runtime-oriented component additions
- *Input+Audio+Graphics in one package* Dynacoe has features for all that you would need for a multimedia experience
- *Standard 2D and 3D features* Allows for operations expected of a competent graphics API, including high end ones such as normal mapping, lighting, and more.
- *Cross platform* With the out-of-the-box components, Dynacoe can run on Windows and Linux. With the proper backends written, it could be for any machine.
- *Its FREE* I love doing this stuff, and you get the benefits. WOW!



Dynacoe strives to be a library that is:
- *Intuitive.* Ideally, you should be able to look at a piece of functionality and say "Ah, that makes sense" on some level. This helps make clearer, more concise code, making both development and maintenance far easier. In general, this points favorably to higher-level ideas. For example, in graphics, when you have an image and you want to draw it, ideally, we would prefer syntax akin to "draw(image)" over "AllocateDrawnPixelBuffer(image.w, image.h); flushCommands(wait(100)); etc;"
- *Powerful when you need it, convenient when you don't.* The engine should make convenient things easily available while still providing specialized functionality. 
- *Lightweight.* You should not need to pull in half of libboost to draw a square. In terms of dependencies, Dynacoe is constantly being tuned to be more self-reliant.
- *Not needy.* It's not good to be stuck to any one platform, as we hope to provide more possibilities for other platforms.


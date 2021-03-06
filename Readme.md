# opengl experiments / demos

A repository to put play-around ideas (good, bad, and crazy), semi-finished
projects, and volatile software. Mostly OpenGL but some other graphics-related stuff too.

## contents

| numerus | titulus             | descriptio                                            | status    |
|---------|---------------------|-------------------------------------------------------|-----------|
| 000     | video_modus         | find and list all video modes on troublesome hardware | working   |
| 001     | cube_map_depth      | depth writing to cube maps for omni-direc. shadows    | abandoned |
| 002     | bezier_curve        | demo from splines and curves lecture last year        | working   |
| 003     | cube                | spinning cube from .obj as starting point project     | working   |
| 004     | outlines            | outline rendering prototype                           | working   |
| 005     | outlines_post       | post-processing outline rendering prototype           | working   |
| 006     | raytrace_cs         | path tracer based on compute shaders                  | working   |
| 007     | tiny_font           | minimal-resolution pixel font rendering               | working   |
| 008     | viewports           | making sure I could do one of the assignments         | working   |
| 009     | water_shader        | prototype wave-based vertex animation                 | working   |
| 010     | compressed_textures | demo of various texture compression options           | working   |
| 011     | stencils            | playing around with stencil tests                     | proposed  |
| 012     | gl3w                | alternative to GLEW                                   | working   |
| 013     | sdl2                | SDL2 opengl start-up                                  | osx       |
| 014     | mirror_plane_fb     | simple mirror w/ previous frame's rendering flipped   | working   |
| 015     | hdr_bloom           | high-dynamic range rendering with bloom shader        | proposed  |
| 016     | pbr                 | physically-based rendering                            | proposed  |
| 017     | x11_render_loop     | basic rendering demo with just X11 (not opengl)       | working   |
| 018     | default_texture     | hard-coded fallback texture if img not found          | working   |
| 019     | generic_shader      | abstraction that reverts to default if shader fails   | working   |
| 020     | image_chan_swap     | tool to swap channels in Unreal exported normal maps  | working   |
| 021     | omni_shad_sp        | single-pass omni-directional shadow mapping           | abandoned |
| 022     | glfw_getkeyname     | testing new key name localisation for glfw            | working   |
| 023     | webgl_quats         | webgl demo of quaternion rotation mathematics         | working   |
| 024     | hmap_terrain        | the traditional heightmapped terrain demo             | working   |
| 025     | depth_antioverdraw  | http://fabiensanglard.net/doom3/renderer.php          | working   |
| 026     | x11_cube            | software 3d renderer built on X11 (not opengl)        | working   |
| 027     | omni_shads_cheating | omni-directional shadows with cubemap texture         | unstable  |
| 028     | more_cube           | second pass at shadow mapping with cubemap textures   | working   |
| 029     | more_cube_gl_2_1    | opengl 2.1 port of omni-directional shadows           | working   |
| 030     | clang_vectors       | using clang vector extension data types               | started   |
| 031     | gcc_vectors         | using gcc vector extension data types                 | started   |
| 032     | vulkan_hw           | vulkan skeleton                                       | started   |
| 033     | compute_shader      | compute shader play-around                            | working   |
| 034     | switching_costs     | measuring opengl state switching costs                | working   |
| 035     | vrdemo              | virtual reality framework for Cardboard               | started   |
| 036     | pano2cube           | converts Streetview app panorama to cube map textures | working   |
| 037     | bsp                 | binary space partitioning demo                        | started   |
| 038     | SPEW                | home-made GLEW replacement. not an acronym, just loud | working   |
| 039     | wad_render          | render geometry from a DOOM WAD file in OpenGL        | working   |
| 040     | compute_shader_neural_net | a neural network encoded in a compute shader    | started   |
| 041     | node_terrain        | terrain that subdivides and can do LOD                | working   |
| 042     | dissolve            | a simple dissolving mesh effect in webgl              | working   |
| xxx     | fresnel_prism       | refraction/reflection colour split as in nvidia cg_tutorial_chapter07 | proposed |
| xxx     | wu_line             | wu's line drawing algorithm (pseudo on wiki)          | proposed  |
| xxx     | two-point perspective | matrices for one/two/three point perspective drawing style | proposed |
| xxx     | 3ds parse           | a 3ds binary format loader           | proposed |
| xxx     | glTF                | use latest official tools for glTF file load | proposed |
| xxx     | widgets             | simple slider and text field widgets drop-in for demos | proposed |
| xxx     | stencil buffer      | render outlines like in Lindsay Kay's CAD renderer |

## LICENCE

Some files in this repository may be from third-party libraries. It is the user's responsibility to determine and respect the licences of these files. All other files are licenced under the GNU General Public Licence:

    This is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a copy of the GNU General Public License see <http://www.gnu.org/licenses/>.

## Notes

* find more test 3D models at [Stanford Computer Graphics Laboratory](http://graphics.stanford.edu/data/3Dscanrep/)

## Cool Thing to Try

* http://www.codersnotes.com/notes/untonemapping/

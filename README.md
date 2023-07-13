# AvGraphicsUtilities
A GUI library in written in C using Vulkan and GLFW

## Goals for the project
 - Use the GUI library to make a Visual Editor for UI design
 - Making a simple IDE
 - Migrate project from Visual Studio ( done )
 - Linux and rpi support
 - Add embedded system support (replace vulkan renderer with cpu renderer based on platform)

## Primitive types
- Rect
- Image
- Character
- Viewport (later)
- *Polygon (not certain)*

## Tasks
- [x] Build system migration
- [ ] Renderer
    - [ ] Render rectangles
        - [ ] Basic Transform buffer input
        - [ ] Basic Mesh input
        - [ ] Basic Shaders
        - [ ] Color input
    - [ ] Render images
        - [ ] Image loading
        - [ ] Storing images on gpu
    - [ ] Render text
        - [ ] font loading
        - [ ] font rendering
- [ ] Positioner
    - [ ] Create Layouts 
        - [ ] Make absolute positioner
        - [ ] Make grid positioner
        - [ ] Make split positioner
        - [ ] *Make floating positioner*
    - [ ] Create Positioning Chain
- [ ] Component handler
    - [ ] Component creation
    - [ ] Component destruction
    - [ ] Component pooling
- [ ] .ui parser
    - [x] tokenizer
    - [ ] lexer
    - [ ] preprocessor
    - [ ] scemantic analysis
    - [ ] operation order generation

## Building
clone the repository and run the following commands
### Required dependencies
#### run depenencies
- vulkan drivers (>= 1.0)
- vulkan validation layers (optional: for debugging)
#### build dependencies
- glfw >= 3.3.8 _(might work with older versions)_
- shaderc
- vulkanSDK( aka: vulkan-dev ) (>= 1.0)
### Windows
Create enviroment variables called VULKAN_SDK and GLFW_SDK 
pointing to the respective directories where the SDK's 
are installed

then do:

```shell
gcc build/builder.c -o builder
./builder build AvGraphicsUtilities
```
### linux
```shell
gcc build/builder.c -o builder
./builder build AvGraphicsUtilities
sudo ./builder install AvGraphicsUtilities
```
the test executable should appear in the AvGraphicsUtilities folder. For information about the build system see the [[BUILDSYS_README|build system readme]]

## Documentation
Read the documentation in [[Documentation]]

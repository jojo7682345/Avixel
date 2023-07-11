# ShaderExport
generates header file from shader files. These header files can be used to embed the shaders into the executable
## usage
```shell
bin/shaderExport -MD -O0 -o <output-header-file>.h <input-shader-file>.shader
```
flags:
- -MD : generate dependency file
- -0x : Optimisation possible values are -O0 -O1 (any single digit above 0 will result in optimization)
- -o : specify output file
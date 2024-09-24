# Ocean_OpenGL

Image Synthesis project that modelize ocean waves with C++ and OpenGL graphic pipeline using the model described in **[Jerry Tessendorf's paper](https://people.computing.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf)** "Simulating Ocen Water".

## Technique Description

1. Compute an heightmap (cf. **[Jerry Tessendorf's paper](https://people.computing.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf)** and the raymarcher ocean project)
2. Convert the heightmap to Vertices
3. Compute surfaces normals for the illumination (Phong's model)
4. Compute the shaders (vertex and fragment)
5. Update the heightmap

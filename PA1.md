| CSE |     | 461 Computer |     |     | Graphics |     | –   | Programming |     | Assignment | 1   |
| --- | --- | ------------ | --- | --- | -------- | --- | --- | ----------- | --- | ---------- | --- |
DUE DATE
| November |     | 22, | 2026, | 23:55 |     |     |     |     |     |     |     |
| -------- | --- | --- | ----- | ----- | --- | --- | --- | --- | --- | --- | --- |
1. Description
|     | • This | is an | individual | assignment. |     | Please | do  | not collaborate. |     |     |     |
| --- | ------ | ----- | ---------- | ----------- | --- | ------ | --- | ---------------- | --- | --- | --- |
• Don’t share anything with other students. Don’t even share your models and mesh data. If
detected, using the same mesh data will be considered as “copying someone’s work”. You will
|     | fail | the class. |     |     |     |     |     |     |     |     |     |
| --- | ---- | ---------- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
• If you think that this document does not clearly describe the assignment, ask questions before it is too late.
In this assignment, you are going to implement an efficient, recursive Ray Tracer in C/C++. Your ray tracer
will support: 1. Pure triangle meshes (with vertex, normal, and texture coordinate data). 2. Mesh Instancing
and Transformations (Translation, Rotation, Scaling). 3. Bounding Volume Hierarchy (BVH) acceleration
structure for high-performance rendering. 4. Multithreaded execution across CPU cores. 5. Shading with the
Blinn-Phong reflection model, point lights, and triangular directional lights. 6. Recursive mirror reflections up to a
configurable maximum ray trace depth. 7. Texture mapping with configurable blending factors.
| 2.  | Scene | Description |     |     | File |     |     |     |     |     |     |
| --- | ----- | ----------- | --- | --- | ---- | --- | --- | --- | --- | --- | --- |
The scene is described in an XML file format. You are free to use lightweight third-party XML parsing libraries
| (such | as    | TinyXML-2, | RapidXML, |     | or  | pugixml). |     |     |     |     |     |
| ----- | ----- | ---------- | --------- | --- | --- | --------- | --- | --- | --- | --- | --- |
| 2.1   | Scene | Elements   |           |     |     |           |     |     |     |     |     |
• maxraytracedepth: A non-negative integer defining the maximum recursive depth for reflection rays. Primary
|     | rays | from | the camera | have | a depth | of  | 0.  |     |     |     |     |
| --- | ---- | ---- | ---------- | ---- | ------- | --- | --- | --- | --- | --- | --- |
• background: RGB color values (r g b in the range [0,255]) assigned to a pixel if the ray does not intersect
|     | any           | object       | in the       | scene:           |                 |                |          |           |     |     |     |
| --- | ------------- | ------------ | ------------ | ---------------- | --------------- | -------------- | -------- | --------- | --- | --- | --- |
|     | <background>0 |              |              | 0 0</background> |                 |                |          |           |     |     |     |
|     | • camera:     | Defines      |              | the viewpoint    |                 | and projection |          | settings: |     |     |     |
|     |               | – position:  |              | Camera           | eye coordinates |                | (x,y,z). |           |     |     |     |
|     |               | – gaze:      | Viewing/gaze |                  | direction       | vector         | (x,y,z). |           |     |     |     |
|     |               | – up: Camera |              | up vector        | (x,y,z).        |                |          |           |     |     |     |
– nearplane: Image plane boundaries (left right bottom top distances relative to the gaze axis on the
|     |     | near | plane). |     |     |     |     |     |     |     |     |
| --- | --- | ---- | ------- | --- | --- | --- | --- | --- | --- | --- | --- |
– neardistance: Distance d between the camera position and the near image plane (the gaze vector is
|     |     | perpendicular      |     | to the | near   | plane). |       |            |           |          |     |
| --- | --- | ------------------ | --- | ------ | ------ | ------- | ----- | ---------- | --------- | -------- | --- |
|     |     | – imageresolution: |     |        | Output | image   | width | and height | in pixels | (nx ny). |     |
<camera>
|     |     | <position>0   |          | 0 0</position> |                  |     |     |     |     |     |     |
| --- | --- | ------------- | -------- | -------------- | ---------------- | --- | --- | --- | --- | --- | --- |
|     |     | <gaze>0       | 0        | -1</gaze>      |                  |     |     |     |     |     |     |
|     |     | <up>0         | 1 0</up> |                |                  |     |     |     |     |     |     |
|     |     | <nearplane>-1 |          | 1              | -1 1</nearplane> |     |     |     |     |     |     |
<neardistance>1</neardistance>
|     |     | <imageresolution>800 |     |     |     | 800</imageresolution> |     |     |     |     |     |
| --- | --- | -------------------- | --- | --- | --- | --------------------- | --- | --- | --- | --- | --- |
</camera>
• ambientlight: Ambient RGB light intensity received by all surfaces, including regions in shadow:
|     | <ambientlight>25 |     |     | 25  | 25</ambientlight> |     |     |     |     |     |     |
| --- | ---------------- | --- | --- | --- | ----------------- | --- | --- | --- | --- | --- | --- |
1

• pointlight: An isotropic point light source defined by its 3D position and intensity vector (I ,I ,I ):
r g b
| <pointlight |                 |     | id="1">        |      |                  |     |     |     |     |
| ----------- | --------------- | --- | -------------- | ---- | ---------------- | --- | --- | --- | --- |
|             | <position>0     |     | 5 0</position> |      |                  |     |     |     |     |
|             | <intensity>1000 |     |                | 1000 | 1000</intensity> |     |     |     |     |
</pointlight>
The irradiance at distance d from the point light follows the inverse-square law:
I
E =
d2
• triangularlight: A planar directional light source defined by three vertices of a triangle and an intensity
vector:
| <triangularlight |                |     | id="2">        |                     |     |     |     |     |     |
| ---------------- | -------------- | --- | -------------- | ------------------- | --- | --- | --- | --- | --- |
|                  | <vertex1>0     |     | 10 0</vertex1> |                     |     |     |     |     |     |
|                  | <vertex2>2     |     | 10 0</vertex2> |                     |     |     |     |     |     |
|                  | <vertex3>0     |     | 10 2</vertex3> |                     |     |     |     |     |     |
|                  | <intensity>800 |     |                | 800 800</intensity> |     |     |     |     |     |
</triangularlight>
| The | emission | direction |     | vector | d   | is the | normalized | cross product: |     |
| --- | -------- | --------- | --- | ------ | --- | ------ | ---------- | -------------- | --- |
light
|     |     |     |     |     |     |            |     | (cid:16)   | (cid:17) |
| --- | --- | --- | --- | --- | --- | ---------- | --- | ---------- | -------- |
|     |     |     |     |     | d   | =normalize |     | (v −v )×(v | −v )     |
|     |     |     |     |     |     | light      |     | 1 2        | 1 3      |
Since this is a directional light, light arrives at all surface points along direction d (i.e., shadow rays point
light
| in  | direction | −d  | ) with | no  | distance | attenuation. |     |     |     |
| --- | --------- | --- | ------ | --- | -------- | ------------ | --- | --- | --- |
light
| • material: |     | Defines | surface | optical |     | properties: |     |     |     |
| ----------- | --- | ------- | ------- | ------- | --- | ----------- | --- | --- | --- |
∈[0,1]3).
|     | – ambient: |     | Ambient | reflection |              | coefficients | (k a         |     |     |
| --- | ---------- | --- | ------- | ---------- | ------------ | ------------ | ------------ | --- | --- |
|     | – diffuse: |     | Diffuse | reflection | coefficients |              | (k ∈[0,1]3). |     |     |
d
|     | – specular: |     | Specular | reflection |     | coefficients | (k  | ∈[0,1]3). |     |
| --- | ----------- | --- | -------- | ---------- | --- | ------------ | --- | --------- | --- |
s
|     | – phongexponent: |     |     | Specular | exponent |     | p≥1. |     |     |
| --- | ---------------- | --- | --- | -------- | -------- | --- | ---- | --- | --- |
– mirrorreflectance: Mirror reflection coefficients (k ∈ [0,1]3). If non-zero and current ray depth
m
|     | <maxraytracedepth, |     |     | generate |     | a reflected | ray: |     |     |
| --- | ------------------ | --- | --- | -------- | --- | ----------- | ---- | --- | --- |
r=d−2(d·n)n
– texturefactor: A blend factor t ∈ [0,1]. When t = 0, only the shaded color is used; when t = 1, the
|           | texture     | color   | completely     |     | overrides |       | diffuse shading: |        |         |
| --------- | ----------- | ------- | -------------- | --- | --------- | ----- | ---------------- | ------ | ------- |
|           |             |         |                |     |           | C     | =(1−t)·C         |        | +t·C    |
|           |             |         |                |     |           | final |                  | shaded | texture |
| <material |             | id="1"> |                |     |           |       |                  |        |         |
|           | <ambient>1  |         | 1 1</ambient>  |     |           |       |                  |        |         |
|           | <diffuse>1  |         | 1 1</diffuse>  |     |           |       |                  |        |         |
|           | <specular>1 |         | 1 1</specular> |     |           |       |                  |        |         |
<phongexponent>100</phongexponent>
|     | <mirrorreflectance>0 |     |     |     | 0 0</mirrorreflectance> |     |     |     |     |
| --- | -------------------- | --- | --- | --- | ----------------------- | --- | --- | --- | --- |
<texturefactor>0.5</texturefactor>
</material>
| 2.2 Transformations |     |     | and | Instancing |     |     |     |     |     |
| ------------------- | --- | --- | --- | ---------- | --- | --- | --- | --- | --- |
To avoid duplicating vertex buffers and to support complex scenes, base meshes can be transformed and instantiated
multiple times.
| • transformations: |     |     | Contains |     | reusable | transformations: |     |     |     |
| ------------------ | --- | --- | -------- | --- | -------- | ---------------- | --- | --- | --- |
2

|     | – translation: |     | T(t    | ,t   | ,t ) |     |     |     |     |     |
| --- | -------------- | --- | ------ | ---- | ---- | --- | --- | --- | --- | --- |
|     |                |     |        | x y  | z    |     |     |     |     |     |
|     | – scaling:     |     | S(s ,s | ,s ) |      |     |     |     |     |     |
|     |                |     | x      | y z  |      |     |     |     |     |     |
– rotation: R(θ,r x ,r y ,r z ) where θ is the angle in degrees, and (r x ,r y ,r z ) is the axis of rotation.
<transformations>
|     | <translation |     |             | id="1">2.0 | 0.0               | -5.0</translation> |     |     |     |     |
| --- | ------------ | --- | ----------- | ---------- | ----------------- | ------------------ | --- | --- | --- | --- |
|     | <scaling     |     | id="1">0.5  |            | 0.5 0.5</scaling> |                    |     |     |     |     |
|     | <rotation    |     | id="1">45.0 |            | 0.0 1.0           | 0.0</rotation>     |     |     |     |     |
</transformations>
| • objects: |     | Contains | base | meshes | (<mesh>) | and | instances | (<instance>): |     |     |
| ---------- | --- | -------- | ---- | ------ | -------- | --- | --------- | ------------- | --- | --- |
– A <mesh> defines the base triangle geometry in its local coordinate space. It has a default materialid
|     | and | list | of faces. |     |     |     |     |     |     |     |
| --- | --- | ---- | --------- | --- | --- | --- | --- | --- | --- | --- |
– An <instance> references a base mesh (<basemeshid>), can override the material (<materialid>), and
|     | specifies |     | an ordered | list | of transformations |     | applied | to  | the base | mesh: |
| --- | --------- | --- | ---------- | ---- | ------------------ | --- | ------- | --- | -------- | ----- |
<objects>
<mesh id="1">
<materialid>1</materialid>
<faces>
...
</faces>
</mesh>
|     | <instance |     | id="2"> |     |     |     |     |     |     |     |
| --- | --------- | --- | ------- | --- | --- | --- | --- | --- | --- | --- |
<basemeshid>1</basemeshid>
<materialid>2</materialid>
|     |     | <transformations>t1 |     |     | r1 s1</transformations> |     |     |     |     |     |
| --- | --- | ------------------- | --- | --- | ----------------------- | --- | --- | --- | --- | --- |
</instance>
</objects>
Transformation Convention: The transformations listed in are applied from right to left to
<transformations>
points:
|     |     |     |     |     |     | M=T | ·R  | ·S  |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|     |     |     |     |     |     |     | 1   | 1   | 1   |     |
Ray Transformation Mathematics: Instead of transforming every vertex of the mesh (which is expensive and
duplicates data), transform the ray into the local coordinate frame of the base mesh: 1. Given ray r (t) =
world
o +td , compute its local counterpart using the inverse transformation matrix M−1:
| world | world |     |     |     |        |          |          |       |      |                   |
| ----- | ----- | --- | --- | --- | ------ | -------- | -------- | ----- | ---- | ----------------- |
|       |       |     |     |     |        | (cid:20) | (cid:21) |       |      | (cid:20) (cid:21) |
|       |       |     |     |     |        | o        |          |       |      | d                 |
|       |       |     |     |     | o =M−1 | world    | ,        | d     | =M−1 | world             |
|       |       |     |     |     | local  |          | 1        | local |      | 0                 |
(Donotre-normalized , sothattheintersectionparametertremainsidenticalinbothlocalandworldcoordinates).
local
2. Perform ray-triangle intersection against the base mesh (or its local BVH) using r . 3. For the hit point at
local
| parameter    | t:  |     |         |      |       |            |       |          |       |          |
| ------------ | --- | --- | ------- | ---- | ----- | ---------- | ----- | -------- | ----- | -------- |
|              |     |     |         |      |       | p          | =o    | +td      |       |          |
|              |     |     |         |      |       | world      | world |          | world |          |
|              |     |     |         |      |       |            |       | (cid:16) |       | (cid:17) |
|              |     |     |         |      | n     | =normalize |       | (M−1)Tn  |       |          |
|              |     |     |         |      | world |            |       |          | local |          |
| 2.3 Geometry |     | and | Texture | Data |       |            |       |          |       |          |
• vertexdata: List of x,y,z coordinates of all vertices. Vertices are 1-indexed.
<vertexdata>
|     | 1.0 | -1.0 | -1.0 |     |     |     |     |     |     |     |
| --- | --- | ---- | ---- | --- | --- | --- | --- | --- | --- | --- |
|     | 1.0 | -1.0 | 1.0  |     |     |     |     |     |     |     |
...
</vertexdata>
| • texturedata: |     |     | List of | u,v texture | coordinates |     | (0≤u,v | ≤1). | 1-indexed. |     |
| -------------- | --- | --- | ------- | ----------- | ----------- | --- | ------ | ---- | ---------- | --- |
3

<texturedata>
0.0 0.0
1.0 0.0
...
</texturedata>
| •   | normaldata: | List of | x,y,z | surface normals. | 1-indexed. |     |
| --- | ----------- | ------- | ----- | ---------------- | ---------- | --- |
<normaldata>
|     | 0.0 | 1.0 0.0 |     |     |     |     |
| --- | --- | ------- | --- | --- | --- | --- |
|     | 1.0 | 0.0 0.0 |     |     |     |     |
...
</normaldata>
• textureimage: Path to the texture image file (e.g. PNG or JPG). You may use stb_image or similar libraries
|     | to load | images. |     |     |     |     |
| --- | ------- | ------- | --- | --- | --- | --- |
<textureimage>texture.png</textureimage>
• faces: Each face is a triangle defined by three vertices in counter-clockwise order. Each vertex token follows
the Wavefront OBJ convention: vertex_index/texture_index/normal_index.
<faces>
|     | 1/1/1 | 2/2/1 3/3/1 |     |     |     |     |
| --- | ----- | ----------- | --- | --- | --- | --- |
|     | 4/4/2 | 5/5/2 6/6/2 |     |     |     |     |
</faces>
| 3.  | Mandatory | Acceleration |     | Structure | (BVH) | & Performance |
| --- | --------- | ------------ | --- | --------- | ----- | ------------- |
Rendering complex meshes with tens of thousands of triangles cannot be done with brute-force linear search.
• Bounding Volume Hierarchy (BVH): You must construct an Axis-Aligned Bounding Box (AABB) tree /
|     | BVH for   | triangle meshes. |          |           |     |     |
| --- | --------- | ---------------- | -------- | --------- | --- | --- |
|     | – Compute | AABBs            | for each | triangle. |     |     |
– Recursively partition triangles along the longest bounding box axis (using midpoint, median, or SAH
|     | split) | into a binary | tree. |     |     |     |
| --- | ------ | ------------- | ----- | --- | --- | --- |
– Implementray-AABBintersection(e.g.,Kay-Kajiyaslabmethod)topruneentiresubtreesduringtraversal.
• Multithreading: Ray tracing is embarrassingly parallel. You must utilize multithreading (e.g., C++11
<thread>, OpenMP, or worker pool) to render scanlines/pixels concurrently across all available CPU cores.
| •   | Performance | Evaluation: |     |     |     |     |
| --- | ----------- | ----------- | --- | --- | --- | --- |
– Your ray tracer must be capable of rendering scenes with ≥10,000 triangles efficiently (under a few
|     | seconds | to a couple | minutes | depending | on resolution). |     |
| --- | ------- | ----------- | ------- | --------- | --------------- | --- |
– Performance is a graded criterion. Up to 30 points will be deducted if your program does not
implement a spatial acceleration structure or exhibits unacceptably slow rendering times due to naive
|     | brute-force | checks. |       |     |     |     |
| --- | ----------- | ------- | ----- | --- | --- | --- |
| 4.  | Complete    | Example | Scene | XML |     |     |
<scene>
<maxraytracedepth>6</maxraytracedepth>
|     | <background>0 | 0 0</background> |     |     |     |     |
| --- | ------------- | ---------------- | --- | --- | --- | --- |
<camera>
|     | <position>0   | 0           | 4</position>       |     |     |     |
| --- | ------------- | ----------- | ------------------ | --- | --- | --- |
|     | <gaze>0       | 0 -1</gaze> |                    |     |     |     |
|     | <up>0         | 1 0</up>    |                    |     |     |     |
|     | <nearplane>-1 |             | 1 -1 1</nearplane> |     |     |     |
4

<neardistance>1</neardistance>
| <imageresolution>800 |     | 800</imageresolution> |
| -------------------- | --- | --------------------- |
</camera>
<lights>
| <ambientlight>25 | 25             | 25</ambientlight>     |
| ---------------- | -------------- | --------------------- |
| <pointlight      | id="1">        |                       |
| <position>0      | 3 2</position> |                       |
| <intensity>1200  |                | 1200 1200</intensity> |
</pointlight>
| <triangularlight | id="2">       |                     |
| ---------------- | ------------- | ------------------- |
| <vertex1>0       | 5 0</vertex1> |                     |
| <vertex2>2       | 5 0</vertex2> |                     |
| <vertex3>0       | 5 2</vertex3> |                     |
| <intensity>600   |               | 600 600</intensity> |
</triangularlight>
</lights>
<materials>
| <material     | id="1"> |                |
| ------------- | ------- | -------------- |
| <ambient>0.2  | 0.2     | 0.2</ambient>  |
| <diffuse>0.8  | 0.3     | 0.3</diffuse>  |
| <specular>0.5 | 0.5     | 0.5</specular> |
<phongexponent>32</phongexponent>
| <mirrorreflectance>0.1 |     | 0.1 0.1</mirrorreflectance> |
| ---------------------- | --- | --------------------------- |
<texturefactor>0.4</texturefactor>
</material>
| <material     | id="2"> |                |
| ------------- | ------- | -------------- |
| <ambient>0.2  | 0.2     | 0.2</ambient>  |
| <diffuse>0.2  | 0.7     | 0.9</diffuse>  |
| <specular>0.9 | 0.9     | 0.9</specular> |
<phongexponent>64</phongexponent>
| <mirrorreflectance>0.4 |     | 0.4 0.4</mirrorreflectance> |
| ---------------------- | --- | --------------------------- |
<texturefactor>0.0</texturefactor>
</material>
</materials>
<transformations>
| <translation | id="1">1.5  | 0.0 -1.0</translation> |
| ------------ | ----------- | ---------------------- |
| <scaling     | id="1">0.75 | 0.75 0.75</scaling>    |
| <rotation    | id="1">35.0 | 0.0 1.0 0.0</rotation> |
</transformations>
<vertexdata>
| -1.0 -1.0 | -1.0 |     |
| --------- | ---- | --- |
| 1.0 -1.0  | -1.0 |     |
| 1.0 1.0   | -1.0 |     |
| -1.0 1.0  | -1.0 |     |
| -1.0 -1.0 | 1.0  |     |
| 1.0 -1.0  | 1.0  |     |
| 1.0 1.0   | 1.0  |     |
| -1.0 1.0  | 1.0  |     |
</vertexdata>
<texturedata>
0.0 0.0
5

1.0 0.0
1.0 1.0
0.0 1.0
</texturedata>
<normaldata>
0.0 0.0 -1.0
0.0 0.0 1.0
-1.0 0.0 0.0
1.0 0.0 0.0
0.0 -1.0 0.0
0.0 1.0 0.0
</normaldata>
<textureimage>texture.png</textureimage>
<objects>
<!-- Base Mesh (Cube) -->
<mesh id="1">
<materialid>1</materialid>
<faces>
1/1/1 2/2/1 3/3/1
1/1/1 3/3/1 4/4/1
5/1/2 6/2/2 7/3/2
5/1/2 7/3/2 8/4/2
1/1/3 5/2/3 8/3/3
1/1/3 8/3/3 4/4/3
2/1/4 6/2/4 7/3/4
2/1/4 7/3/4 3/4/4
1/1/5 2/2/5 6/3/5
1/1/5 6/3/5 5/4/5
4/1/6 3/2/6 7/3/6
4/1/6 7/3/6 8/4/6
</faces>
</mesh>
<!-- Transformed Instance of Mesh 1 -->
<instance id="2">
<basemeshid>1</basemeshid>
<materialid>2</materialid>
<transformations>t1 r1 s1</transformations>
</instance>
</objects>
</scene>
5. Remarks & Implementation Rules
• Individual Effort: Don’t share anything with other students. Don’t even share your models and mesh data.
If detected, using the same mesh data will be considered as “copying someone’s work”. You will fail the class.
• Language: You have to use C/C++ (C++11 or higher recommended).
• Libraries: You can use external libraries only for:
– XML parsing (TinyXML-2, RapidXML, pugixml, etc.)
– Image read/write (stb_image, lodepng, CImg, etc.)
– No third-party math/linear algebraor graphicsengines arepermitted(e.g., no GLM,no CGAL,no Embree,
no OptiX). All vector, matrix, ray intersection, and BVH routines must be your own code.
6

• Primitive Restriction: The only object geometry type is triangle mesh. Do not include routines for spheres,
cylinders, cones, planes, etc. If your code includes routines to handle other primitive types, 20
| points will | be deducted. |     |     |
| ----------- | ------------ | --- | --- |
• Numeric Precision (Shadow Acne): When casting shadow or secondary reflection rays, offset the ray
origin slightly along the surface normal (o′ =p+ϵn, where ϵ≈10−4) to prevent self-intersection.
• Model Design: You can use Blender to model scenes and export triangle meshes in OBJ format.
• Explainability & Live Modifications: Don’t submit anything you cannot explain in detail. You will be
asked questions and may be asked to make live modifications to your code during your demonstration to prove
| full ownership | and understanding | of your | submission. |
| -------------- | ----------------- | ------- | ----------- |
| 6. Turn in &   | Demonstration     |         |             |
• You are going to submit your source code and documentation in a .zip file on Microsoft Teams before the
deadline.
| • Documentation: | Include | a /       | explaining: |
| ---------------- | ------- | --------- | ----------- |
|                  |         | README.md | report.pdf  |
– How to build and run your program (provide a working CMakeLists.txt or Makefile).
| – Explanation | of your BVH | structure | and traversal algorithm. |
| ------------- | ----------- | --------- | ------------------------ |
– Runningtimemeasurementsandcomparisons(e.g.,rendertimeswithvs.withoutBVHandmultithreading
| across        | varying triangle | counts).     |     |
| ------------- | ---------------- | ------------ | --- |
| – Screenshots | of your rendered | test scenes. |     |
| • Mandatory   | Demonstration:   |              |     |
– Live Code Execution & Walkthrough: Every student will demonstrate their program running live
(either face-to-face or via a Microsoft Teams meeting). You will execute your ray tracer, demonstrate its
output on different test scenes, and explain all parts and architectural components of your code in detail.
– OralExamination: Youwillbeaskedquestionsregardingyourimplementation,underlyingmathematical
| foundations, | and design | decisions. |     |
| ------------ | ---------- | ---------- | --- |
– Live Code Modification: During the demo, you may be asked to modify your code on the fly (e.g.,
altering a shading parameter, modifying camera or ray behavior, adjusting transformation logic, or
toggling acceleration features) to prove complete authorship and understanding.
– Reminder: As stated in the course syllabus, failing the demonstration or being unable to explain/modify
the code will result in the assignment being graded as 0 and counted as “not submitted” (which affects
| exam | admission eligibility). |     |     |
| ---- | ----------------------- | --- | --- |
7
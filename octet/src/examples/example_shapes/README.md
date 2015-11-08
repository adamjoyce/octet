# Bullet Physics Demo by Adam Joyce

![Physics Demo](https://raw.githubusercontent.com/adamjoyce/octet/working/octet/src/examples/example_shapes/README_images/bridges.PNG "Physics Bridges")

### Introduction
This is a small Bullet Physics demo written in OpenGL and C++.  The demo generates two bridges, one made
using hinges and the other using springs.  A number of ball and slab objects are dropped onto each bridge
to test the constraints.

The demo displays a number of key concepts:
* Settings up objects by reading data from a csv file
* Bullet hinge constraints
* Bullet spring constraints
* Bullet collision callbacks

### Structure
This document is here to highlight the key functions and classes that I have written for this physics
demo project.  It is not a complete list but I have covered the main functions.

### Reading CSV Data - octet\src\resources\csv_parser.h - vec4_file(...)
I chose to read in all bridge and camera data from a csv file.  vec4() takes the file path of the csv file
and assembles an array of vec4's.  Any data rows in the csv file that have less than four values have the rest 
of their array slots padded with zeros.

An important thing to note is that some of the values stored in the csv data file rely on other values in
the same file.  For example in order to increase the width of a plank you need to increase the bridge width
and adjust the plank gaps for the hinge bridge to reposition the hinges.

### Hinge Constraints

#### Hinge Bridge Construction - .\physics_bridges.h - create_hinge_bridge()
Both bridge constructions are based largely on the position of the first platform.  This position information
is read in from a csv file.  I begin by placing the first platform and using its x coordinate information
to place the second platform.  Next I hinge the first bridge plank to the first platform.  From this position
it is relatively simple to hinge the remaining planks to the ones that comes before them, using the hinge plank
gap to calculate the x coordinate for both the new plank and the hinge.  It is then a case of attaching the
final plank to the second platform.

I chose to avoid setting a particular angle limit for the bridge's hinges.  Due to the two solid platforms
acting as anchors at either end of the bridge, I felt that the best 'rope bridge' effect comes when leaving
the hinges unrestricted to fall naturally.

![Hinge Bridge](https://raw.githubusercontent.com/adamjoyce/octet/working/octet/src/examples/example_shapes/README_images/hinge_bridge.PNG "Hinge Bridge")

### Spring Constraints

#### Spring Bridge Construction - .\physics_bridges.h
Similarly to the hinge bridge, the first and second platforms are constructed and placed first.  To 
achieve the spring effect of the planks I create an anchor plank along with each bridge plank to act 
as the fixed attachment point of the spring.

After attaching each spring to their anchor and plank I set linear limits and the angular limits to keep
the planks uniform.  Finally the spring is enabled in the y axis and its stiffness and damping values 
are set.

![Spring Bridge](https://raw.githubusercontent.com/adamjoyce/octet/working/octet/src/examples/example_shapes/README_images/spring_bridge.PNG "Spring Bridge")

### Collision Callback

#### Handle Collisions - .\physics_bridges.h - handle_collisions()
Here I check for collisions by finding the number of contact points in each contact manifold.  The 'bang'
sound is set to play on the first contact point detected.

I have left in a few lines of commented out code that gather useful (but in this case unnecessary) contact
information.

![Collision Objects](https://raw.githubusercontent.com/adamjoyce/octet/working/octet/src/examples/example_shapes/README_images/collision_objects.PNG "Collision Objects")

### Other Functions

#### Dropping the Objects - .\physics_bridges.h - spawn_objects()
This function is used to periodically drop ball and slab objects on the hinge and spring bridges 
respectively.  In the draw_world() function I use the simple integer variable 'frames' to track the
number of frames that have passed since the demo started.  This is then used to determine the wait 
period before dropping more objects on the bridges.  When it is time I simply translate the existing objects
to a (bounded) random position above each bridge.

### Sound

#### FMOD and OpenAL - .\sound_system.h & .\physics_bridges.h - play_sound()
I attempted to use FMOD to play the required collision sound.  Unfortunately, I encountered an error 
regarding a 'bod' identifier in the fmod.h and fmod.hpp files that prevented me from including my 
sound_system.h file within physics_bridges.h.  I am still unsure the exact cause of the problem but
it appears to be related to the bullet 'btMultiBodyDynamicsWorld.cpp' file.  It would be great if 
Andy could shed any light on the issue!

Despite the error, I have left all my FMOD code in the demo commented out. This should give you an idea 
of what I was trying to achieve with the library.  Please note that since I was unable to get it in a
working state the code is untested, and as such likely has a few bugs here and there.

Instead I used OpenAL to play the bang sound on the first collision after each object 'drop-frame'.  
I use a simple boolean variable 'sound_played' to track whether the sound has happened in each 
'drop-frame' period.  You may hear the sound being played prematurely to the objects colliding with
the bridges.  This is because the objects are being randomly placed in a colliding location above
the bridges, thereby counting as the initial collision.


[Demo Youtube Video](https://www.youtube.com/watch?v=CTASXvUjPmk "Demo Youtube Video")
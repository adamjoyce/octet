# Zorkadian Harassment 
Adam Joyce's Invaderer Hack

### Flavour Text
The Zorkadian's are causing havoc up and down the intergalactic highway #42, destroying and pillaging
any ships that cross their path!  Only a direct ram will penetrate their superior missile-invulnerable
shielding technology!  Will the federation ever be free of these pesky invaderers?!

### The Game
* The player controls the yellow spaceship
* He/she must fly into invaderers to destroy them
* Invaderers launch randomly pathing bombs
* If the player comes into contact with a bomb, he/she loses a life
* The player wins if he/she destroys all invaderers before losing all their lives

### Structure
This document is here to highlight the functions and classes that I have written/edited for this
game project.  It is not a complete list but I have covered the majority of the key areas below.

### Gameplay

Brief overview of the gameplay.

#### Ship Movement - .\invaderers_app.h - move_ship() 
An obvious task was to alter the ship movement from it's rather limited 2 degrees of freedom
(left and right) to a full 'compass-like' 8 degrees of freedom.  In doing this I had to be sure to
introduce new collision detection checks between the player ship and the previously untouched upper
and lower borders.

Additionally, I knew that I wanted to have the speed at which the invaderer's bombs move vary
depending on whether the player ship was in motion.  A check is made to see if the player is 
moving and to set the bomb speed accordingly.

#### Bomb Movement - .\invaderers_app.h - move_bombs()
It was relatively trivial to achieve the bomb's 'zigzaggy' motion.  For each bomb I pseudo-randomly 
generate a new x and y coordinate using the current bomb speed.  Each bomb is then translated to 
their new coordinate location.

Since there is a finite number of bombs in play at any one time I recycle the bombs that hit the 
screen borders.  These are then free to be fired again by an invaderer.

![Bombs](https://raw.github.com/adamjoyce/octet/tree/working/octet/src/examples/example_invaderers/README_pictures/bombs.PNG "Bombs")

#### Ship Collisions - .\invaderers_app.h - collide_invaderer()
I needed a function that would check for collisions between the ship and active invaderers.  This is
called on every step of the program and disables any collided invaderers.

![Collision](https://raw.github.com/adamjoyce/octet/tree/working/octet/src/examples/example_invaderers/README_pictures/collision.PNG "Collision")

###Setup

Brief overview of the game's 'setup' process.

#### CSV Parser - octet\src\resources\csv_parser.h - vec4_file(...)
I chose to read in all sprite location and dimension information from a csv file.  To do this I wrote a
simple csv parser.  The parser contains a single function that takes the file path of the csv document
and a reference to a vec4 array where the appropriate data is assigned.

I did originally plan to read in all sprite data from the same csv file.  This proved tricky due to
the way invaderers_app.h relies on it's enum for indicies.  Placing variables such as 'num_borders'
in the csv would mean that they could not be static constants and thus would not be able to be used
to calculate the 'last_border_sprite' index.

This in itself isn't a big issue as you could simulate the latter half of the enum with a struct
and function that calculates the index values based on the one that came before.  However num_sprites
would not be able to be used to declare the sprites array, which would likely mean needing to use
a dynamic array instead.  As a result I decided to only include the sprite location data in my csv
file.

I would be interested to hear if Andy has an idea of an elegant solution to this problem that I may
have missed that wouldn't require using a different type of array or rewriting large portions of
the program's code.

I needed to ignore specific data when parsing the csv file.  Identification words and newline 
characters for example.  Due to the format of my csv file's data, I also needed to place a delimiting
comma at the end of all rows that contained all full four values.

To provide the function with greater flexibility I padded out any row with fewer than four values with
zeros.

#### Sprite Setup - .\invaderers_app.h - sprite_setup()
After reading all the csv data I use an array to initialise all the sprites.

To provide a bit more variety to the game, I setup the invaderer spawns so that after their initial spawn
locations are read from the csv, on each successive replay their locations are pseudo-random.


### Shaders

Brief overview of the game's shaders.

#### Shaders - octet\src\shaders\texture_shader.h & space_shader.h
To begin, to familiarise myself with both vertex and fragment shaders I attempted to alter the existing 
texture_shader class, allowing a colour to be passed to it.  This colour is applied to the texture
on the rendering sprite.

Once familiar, I wrote a simple space_shader that creates a space background theme.  I overloaded
the existing render function in the sprite class to take a space_shader parameter.

![Space](https://raw.github.com/adamjoyce/octet/tree/working/octet/src/examples/example_invaderers/README_pictures/space.PNG "Space")

#### Drawing Sprites - .\invaderers_app.h - draw_sprites()
It was fairly straight forward to render each of the sprites.  All bombs are rendered with either
a light or dark red colour depending on the bomb's current speed.


[Demo Gameplay Youtube Video](https://www.youtube.com/watch?v=CTASXvUjPmk "invaderers_demo Youtube Video")
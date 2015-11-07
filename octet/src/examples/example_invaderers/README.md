# Adam Joyce's Invaderers Hack
### The Game
* The player controls the yellow spaceship
* He/she must fly into invaderers to destroy them
* Invaderers launch randomly 'zigzagging' bombs
* If the player comes into contact with a bomb, he/she loses a life
* The player wins if he/she destroys all invaderers before losing all their lives

### Structure
This document is here to highlight the functions and classes that I have written/edited for this
game project.  It is not a complete list but I have covered the majority of the key areas below.

### Gameplay

Brief overview of the gameplay.

####Ship Movement - .\invaderers_app.h - move_ship()
An obvious task was to alter the ship movement, from it's rather limited 2 degrees of freedom
(left and right) to a full 'compass' 8 degrees of freedom.  I had to make sure to introduce new
collision detection between the ship and the previously untouched upper and lower border.

Additionally, I knew that I wanted to have the speed at which the invaderer's bombs move vary
depending on whether the player ship was in motion.  A check is made in move_ship() to see if
the player is moving and set the bomb speed accordingly.

####Bomb Movement - .\invaderers_app.h - move_bombs()
It was relatively trivial to achieve the 'zigzggy' motion that the invaderer's bombs have.  For
each bomb I pseudo-randomly generate a new x and y coordinate using the current bomb_speed.  Each
bomb is then translated to their new coordinate.

Since there is a finite number of bombs in play at any one time I recycle any bombs that hit the 
screen borders.  These are then free to be fired again by an invaderer.

####Ship Collisions - .\invaderers_app.h - collide_invaderer()
I needed a function that would check for collisions between the ship and active invaderers.  This is
called on every step of the program and disables any collided invaderers.


###Setup

Brief overview of the game's 'setup' process.

####CSV Parser - octet\src\resources\csv_parser.h - vec4_file(...)
I chose to read in all sprite location and dimension information from a csv file.  To do this I wrote a
simple csv parser.  The parser contains a single function that takes the file path of the csv document
and a reference to a vec4 array where the appropriate data is assigned.

I needed to ignore various data when parsing the csv file such as identification words and newline 
characters.  Due to the format of my csv file's data, I also needed to place a delimiting
comma at the end of all rows that contained the full four values.

To provide greater flexibility I padded out any row with fewer than four values with zeros.


####Sprite Setup - .\invaderers_app.h - sprite_setup()
After reading all the csv data I use an array to initialise all the sprites.

To provide a bit more variety to the game, I setup the invaderer spawns so that after their initial spawn
locations are read from the csv, on each successive replay their locations are pseudo-random.


###Shaders

Brief overview of the game's shaders.

####Shaders - octet\src\shaders\texture_shader.h & space_shader.h
To begin, to get familiar with both vertex and fragment shaders I attempted to alter the existing 
texture_shader to allow a colour to be passed to it.  This colour is applied to the texture
on the sprite that is rendering.

Once familiar, I wrote a simple space_shader that creates a space background theme.  I overloaded
the existing render function in the sprite class to take a space_shader parameter.

####Drawing Sprites - .\invaderers_app.h - draw_sprites()
It was fairly straight forward to render each of the sprites.  All bombs are rendered with either
a light or dark red colour depending on the bomb's current speed.


[Demo Gameplay Youtube Video](https://www.youtube.com/watch?v=CTASXvUjPmk "invaderers_demo Youtube Video")
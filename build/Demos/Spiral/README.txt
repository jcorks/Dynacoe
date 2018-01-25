SPIRAL: A simple interactive demo
Johnathan Corkery, 2016
-----------------------


Sprial is a visual demo that uses a few of the utilities
provided by Dynacoe in a clear, conceptual way. It 
is much simpler than the other Time Cube demo.


The Game Itself
---------------

Not much of a game, as there is no structured goal, and 
its rather hard to come up with one yourself, but it is 
interactive.

Little flying "bits" will always swarm towards the mouse position
while changing their colors and velocities every now and then. The 
variation in velocity and target position they target produces a 
near spiral design. 

You can see how the bits react by changing the Camera position and 
changing the bits' maximum velocity.




Controls:
---------

    WASD        - Camera movement 
    
    Up arrow    - Increase bit velocity
    Down arrow  - Decrease bit velocity
    Left arrow  - Set velocity to minimum
    Right arrow - Set velocity to maximum

    Space bar   - Create additional bits
    
    
    
Design
------

This was the first demo that involved multiple concepts,
so the initial its design is pretty minimal. Input mapping is demonstrated,
as well has basic Object2D usage.
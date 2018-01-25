TIMECUBE: An game outline using Dynacoe
Johnathan Corkery, 2016
---------------------------------------

TimeCube is a work in progress, but has enough to demonstrate 
some usage of Dynacoe's utilities to show how little C++ code is needed
to realize a(n almost) full game.




The Game Itself
---------------
When playing the game, there are 2 opposed factions, fighting each other in a constant power struggle.
Both sides are fighting each other over the precious "Time Cube", an artifact 
that seems to express itself in other dimensions. The faction on the left is tyring to defend the Time Cube
to study its effects and composition, while the faction is trying to destroy it, as they find it too powerful for 
any beings to possess.

Each faction has 2 bases that spawn units. A base also has a powerful turret to 
defend itself from nearby adversaries. You yourself spawn as a unit from 
the leftside faction, so your goal is to defend the Time Cube and destroy the 
opposition


With your unit, you can freely fly around, boost, and fire your weapon as you see fit.
When you destroy an enemy unit, you will acquire an "upgrade" of some kind:

    Green - Fully repairs your ship
    Red   - Increases the power of your weapon 
    Blue  - Increases the power of your boost 
    
Keep in mind that ALL units can collect and can drop upgrades, so your 
allies and opponents may become really strong. If you explode, you will 
respawn from own of your allied bases after a few seconds. When in this state,
you can view the battlefield freely using WASD.

If you acquire above 5 total red or blue upgrades, you will start to glow yellow.
If this happens, when you explode, you drop a yellow upgrade. If anyone collects the 
yellow upgrade, they inherit all the upgrades you had up to the point you exploded.


Controls
--------

    WASD   - Change ship direction / free camera control
    Mouse  - fire weapon 
    Shift  - Boost
    Esc    - Pause
    
    


    
    
Design
------

The main intent of this "game" was to utilize the feature set of Dynacoe to maximize 
simplicity of the design, and avoid the use of assets outside audio. 
The base game was developed over the course of a few days, and since
has only had minor cosmetic changes, addition of menus, and bugfixes. 

The choice of putting all the code in headers was both to decrease complexity of the design and 
make referencing the code easier for the user. Circular dependencies for classes are inherently 
much more difficult to pull off in this setup, which also helps keep the complexity trimmed.
Though, for actual development, this isn't recommended, as any time you many a change, 
you need to recompile the entire project. For small projects this is okay, but for anything 
somewhat major, this is a real pain.


Some of the most helpful utilities for simplifying design have been:
    - StateControl components to automatically change states under certain conditions. 
      It is especailly useful to have an arbitrary number of them on the same object.
    - Object2D components to control basic motion and collision detection.
    - The ability to make custom components
    - The use of Entity::IDs over pointers to prevent use of invalid pointers.
    - Using the Wolrd concept to divide responsibilities, control conceptual z-order grouping,
      and simplify group state pausing/resuming.
    
As you read the annoted code, you might note how classical inheritance 
will enhance the design role along its interaction with components. The beggest example of this 
is the unit, as the behavior between the player and the AI is only how the units is controlled, 
and the addition of a HUD for the player.
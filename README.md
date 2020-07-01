# FPS Movement

This is an Unreal Engine 4 project made to experiment with advanced fps movement mechanics.

FPS Movement can be deep, fluid, and fun. Strafe-jumping, bunny-hopping with air control, crouch-sliding, wall-jumps, wall-runs, dodges, grappling hooks and other advanced movement mechanics interest me and this project was made for me to experiment implementing them and testing fun alterations or new movement mechanics.

I have experimented with fps movement before (check out this old prototype of zero-gravity fps movement with strafe-"skating" and boots that stored the energy of collisions that could be released to boost speed in any direction https://youtu.be/iSNfq4pY5Lg). I intend for this project to be more traditional, but put more consideration into network replication. For now that means using the infrastructure UE4 puts into place with `ACharacter` and the `UCharacterMovementComponent` for replicating and smoothing movement over network connections.

------

Inspiration: 

Strafe-jumping physics explained | Matt's Ramblings: https://youtu.be/rTsXO6Zicls

A Loveletter to the Titanfall 2 Movement System | Mokey: https://youtu.be/9lUoA9q0jnM

Quake Champions with its characters having different movement mechanics (vanilla quake 3 strafe-jumping, cpm air-strafing, crouch-sliding, wall-jumps, dodges, grappling hooks etc.)


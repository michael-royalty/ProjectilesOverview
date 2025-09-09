# ProjectilesOverview

Developed with Unreal Engine 5

Spawning methods can be found under Content/Tutorial/

How to test:
Click on BP_Spawner in the outliner
Edit anyhe settings to change how many projectiles will be spawned.
Change the Projectile Manager setting to one of 8 options:
* BP_Turret_1_Destroy
  * Spawns projectiles every time.
  * Impact logic lives in the projectile.
* BP_Turret_2_Pool
  * Spawns projectiles then pools them for reuse.
  * Impact logic lives in the projectile.
  * Pool lives in the Turret
* BP_Turret_3_TraceNiagara
  * Performs a trace for impact checks
  * Spawns a projectile vfx based on the trace
  * VFX is unlinked/desynced from the initial trace
* BP_Turret_3a_TraceNiagara_CPP
  * Same as above, but in C++
  * Code is in TraceProjectile_Niagara.h/cpp
* BP_Turret_4_DataISM
  * Keeps arrays of projectile locations and velocities
  * Every tick, traces the projectile movement for impacts
  * Every tick, writes projectile visuals to Instanced Static Mesh
* BP_Turret_4a_Data_ISM_CPP
  * Same as above, but in C++
  * Code is in DataDrivenProjectile_ISM.h/cpp
* BP_Turret_5_DataNiagara
  * Keeps arrays of projectile locations and velocities
  * Spawns Niagara visuals from the spawn location, in the initial velocity
  * Every tick, traces the projectile movement for impacts
  * On impact, instructs Niagara via user array to destroy that projectile vfx
* BP_Turret_5a_Data_Niagara_CPP
  * Same as above, but in C++
  * Code is in DataDrivenProjectile_ISM.h/cpp
The PCG will refresh and initialize the selected spawner and settings.
Press play and observe results.

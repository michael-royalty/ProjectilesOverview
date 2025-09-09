# ProjectilesOverview

Developed with Unreal Engine 5

Spawning methods can be found under Content/Tutorial/

How to test:
Click on BP_Spawner in the outliner
Edit anyhe settings to change how many projectiles will be spawned.
Change the Projectile Manager setting to one of 8 options:
* BP_Turret_1_Destroy
* BP_Turret_2_Pool
* BP_Turret_3_TraceNiagara
* BP_Turret_3a_TraceNiagara_CPP
* BP_Turret_4_DataISM
* BP_Turret_4a_Data_ISM_CPP
* BP_Turret_5_DataNiagara
* BP_Turret_5a_Data_Niagara_CPP
The PCG will refresh and initialize the selected spawner and settings.
Press play and observe results.

BP_Turret_3a_TraceNiagara_CPP is based on TraceProjectile_Niagara.h/cpp

BP_Turret_4a_Data_ISM_CPP is based on DataDrivenProjectile_ISM.h/cpp

BP_Turret_5a_Data_Niagara_CPP is based on DataDrivenProjectile_Niagara.h/cpp

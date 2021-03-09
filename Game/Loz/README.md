**NOTE:** Compiling CallDelayedFunctions by yourself is not necessary. It should be done only if Project Nightmare gets new functions/scripts that are required for CDF-related systems (i.e. NES walls) to operate.

In order to compile the Wrapper Generator and the DLL itself, you need CodeBlocks and a 32-bit compiler (CodeBlocks pre-bundled with 32-bit MinGW works well).

**1.** Open "CallDelayedFunctions.cbp".

**2.** While the project file is open, change the current project from "Release" to "WrapperGenerator".

**3.** Build WrapperGenerator.

**4.** Set the current project in CodeBlocks back to "Release", then build CallDelayedFunctions.

**5.** In the directory above this one you will have a file called ImportScripts.gml. These are all of the generated Game Maker scripts to be imported. Simply delete the previous versions of the scripts and import this new version of the file to have the new scripts. This step can be skipped if no interfaces have changed between DLL and executable.

After following these steps, you will have built a fresh new "CallDelayedFunctions.dll". Simply replace the current build in ProjectNightmare\datafiles with this one.

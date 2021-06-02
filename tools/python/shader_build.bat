@echo off

for %%f in (%*) do (
python "L:\LightnEngine\tools\python\shader_build.py" %%f
)
pause
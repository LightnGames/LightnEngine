@echo off

for %%f in (%*) do (
python "L:\LightnEngine\tools\python\shader_set_converter.py" %%f
)
pause
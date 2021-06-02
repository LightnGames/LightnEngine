@echo off

for %%f in (%*) do (
python "L:\LightnEngine\tools\python\material_converter.py" %%f
)
pause
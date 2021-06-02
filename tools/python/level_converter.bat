@echo off

for %%f in (%*) do (
python "L:\LightnEngine\tools\python\level_converter.py" %%f
)
pause
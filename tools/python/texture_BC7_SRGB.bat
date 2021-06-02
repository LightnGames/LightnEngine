@echo off

for %%f in (%*) do (
python "L:\LightnEngine\tools\python\texture_converter.py" %%f
)
pause
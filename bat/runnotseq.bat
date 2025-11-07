@echo off
cd "C:\Users\Admin\source\repos\TpFinalProgAvanzada\TpFinalProgAvanzada\out\build\x64-Debug"

echo.
echo === OPENMP (4 threads) ===
set OMP_NUM_THREADS=8
omp_crossfade.exe

echo.
echo === MPI (4 procesos) ===
mpiexec -n 8 mpi_crossfade.exe

echo.
echo Listo! Revisa las carpetas frames_*
pause
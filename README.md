# TpFinalProgAvanzada

## Requisitos

**MSYS2** | [msys2.org](https://www.msys2.org) |
**MS-MPI** | [Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=57469) |
**FFmpeg** | [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) |

## Compilar

mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release

## Ejecutar

cd Release
sequential.exe
set OMP_NUM_THREADS=4 && omp_crossfade.exe
mpiexec -n 4 mpi_crossfade.exe

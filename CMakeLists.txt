cmake_minimum_required(VERSION 3.20)
project(CrossfadeProject LANGUAGES CXX)

# ---- Configuración global ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir stb
include_directories(${CMAKE_SOURCE_DIR}/stb)

# ---- Ejecutable Secuencial ----
add_executable(sequential sequential.cpp)

# ---- Ejecutable OpenMP ----
add_executable(omp_crossfade omp_crossfade.cpp)
target_compile_options(omp_crossfade PRIVATE /openmp)
target_link_options(omp_crossfade PRIVATE /openmp)

# ---- Ejecutable MPI (solo si MS-MPI está instalado) ----
find_package(MPI)
if(MPI_CXX_FOUND)
    add_executable(mpi_crossfade mpi_crossfade.cpp)
    target_link_libraries(mpi_crossfade MPI::MPI_CXX)
else()
    message(WARNING "MPI no encontrado. mpi_crossfade no se compilará.")
endif()

# ---- Optimización en Release ----
foreach(target sequential omp_crossfade mpi_crossfade)
    if(TARGET ${target})
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Debug>:/Od>
        )
    endif()
endforeach()
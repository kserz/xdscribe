cmake_minimum_required(VERSION 3.14)

function(turnoff VAR)
    set(${VAR} OFF CACHE BOOL "" FORCE)
endfunction()

function(turnon VAR)
    set(${VAR} ON CACHE BOOL "" FORCE)
endfunction()

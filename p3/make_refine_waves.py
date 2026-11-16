#!/usr/bin/env python3
"""Refine the existing Fourier reconstruction using known-penalty one-sided probes."""
import make_waves

make_waves.DESIGNS=[('cos',3),('sin',3),('sin',2),('cos',2),('cos',4),('sin',4)]
make_waves.SIGNS=[1]
make_waves.AMPLITUDE=1.5
make_waves.PREFIX='REFINE_'
make_waves.SCRIPT_FILE='refine_console.js'
make_waves.DESIGN_FILE='refine_designs.json'

if __name__=='__main__': make_waves.main()

#!/usr/bin/env python3
"""Four additional measurements refining the existing Fourier reconstruction."""
import make_waves

make_waves.DESIGNS=[('cos',1),('sin',1),('cos',3),('sin',3)]
make_waves.SIGNS=[1]
make_waves.AMPLITUDE=1.0
make_waves.PREFIX='LAST4_'
make_waves.SCRIPT_FILE='last4_console.js'
make_waves.DESIGN_FILE='last4_designs.json'

if __name__=='__main__':make_waves.main()

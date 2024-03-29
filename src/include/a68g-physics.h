//! @file a68g-physics.h
//! @author J. Marcel van der Veer

//! @section Copyright
//!
//! This file is part of Algol68G - an Algol 68 compiler-interpreter.
//! Copyright 2001-2024 J. Marcel van der Veer [algol68g@xs4all.nl].

//! @section License
//!
//! This program is free software; you can redistribute it and/or modify it 
//! under the terms of the GNU General Public License as published by the 
//! Free Software Foundation; either version 3 of the License, or 
//! (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful, but 
//! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
//! or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
//! more details. You should have received a copy of the GNU General Public 
//! License along with this program. If not, see [http://www.gnu.org/licenses/].

//! @section Synopsis
//!
//! Various numerical constants from GSL.

#if !defined (__A68G_PHYSICS_H__)
#define __A68G_PHYSICS_H__

// All kind of constants ex GSL

#define GSL_CONST_NUM_FINE_STRUCTURE (7.297352533e-3)   // -
#define GSL_CONST_NUM_AVOGADRO (6.02214199e23)  // 1 / mol
#define GSL_CONST_NUM_YOTTA (1e24)      // -
#define GSL_CONST_NUM_ZETTA (1e21)      // -
#define GSL_CONST_NUM_EXA (1e18)        // -
#define GSL_CONST_NUM_PETA (1e15)       // -
#define GSL_CONST_NUM_TERA (1e12)       // -
#define GSL_CONST_NUM_GIGA (1e9)        // -
#define GSL_CONST_NUM_MEGA (1e6)        // -
#define GSL_CONST_NUM_KILO (1e3)        // -
#define GSL_CONST_NUM_MILLI (1e-3)      // -
#define GSL_CONST_NUM_MICRO (1e-6)      // -
#define GSL_CONST_NUM_NANO (1e-9)       // -
#define GSL_CONST_NUM_PICO (1e-12)      // -
#define GSL_CONST_NUM_FEMTO (1e-15)     // -
#define GSL_CONST_NUM_ATTO (1e-18)      // -
#define GSL_CONST_NUM_ZEPTO (1e-21)     // -
#define GSL_CONST_NUM_YOCTO (1e-24)     // -
#define GSL_CONST_CGSM_GAUSS (1.0)      // cm / A s^2 
#define GSL_CONST_CGSM_SPEED_OF_LIGHT (2.99792458e10)   // cm / s
#define GSL_CONST_CGSM_GRAVITATIONAL_CONSTANT (6.673e-8)        // cm^3 / g s^2
#define GSL_CONST_CGSM_ASTRONOMICAL_UNIT (1.49597870691e13)     // cm
#define GSL_CONST_CGSM_LIGHT_YEAR (9.46053620707e17)    // cm
#define GSL_CONST_CGSM_PARSEC (3.08567758135e18)        // cm
#define GSL_CONST_CGSM_GRAV_ACCEL (9.80665e2)   // cm / s^2
#define GSL_CONST_CGSM_ELECTRON_VOLT (1.602176487e-12)  // g cm^2 / s^2
#define GSL_CONST_CGSM_MASS_ELECTRON (9.10938188e-28)   // g
#define GSL_CONST_CGSM_MASS_MUON (1.88353109e-25)       // g
#define GSL_CONST_CGSM_MASS_PROTON (1.67262158e-24)     // g
#define GSL_CONST_CGSM_MASS_NEUTRON (1.67492716e-24)    // g
#define GSL_CONST_CGSM_RYDBERG (2.17987196968e-11)      // g cm^2 / s^2
#define GSL_CONST_CGSM_BOLTZMANN (1.3806504e-16)        // g cm^2 / K s^2
#define GSL_CONST_CGSM_MOLAR_GAS (8.314472e7)   // g cm^2 / K mol s^2
#define GSL_CONST_CGSM_STANDARD_GAS_VOLUME (2.2710981e4)        // cm^3 / mol
#define GSL_CONST_CGSM_MINUTE (6e1)     // s
#define GSL_CONST_CGSM_HOUR (3.6e3)     // s
#define GSL_CONST_CGSM_DAY (8.64e4)     // s
#define GSL_CONST_CGSM_WEEK (6.048e5)   // s
#define GSL_CONST_CGSM_INCH (2.54e0)    // cm
#define GSL_CONST_CGSM_FOOT (3.048e1)   // cm
#define GSL_CONST_CGSM_YARD (9.144e1)   // cm
#define GSL_CONST_CGSM_MILE (1.609344e5)        // cm
#define GSL_CONST_CGSM_NAUTICAL_MILE (1.852e5)  // cm
#define GSL_CONST_CGSM_FATHOM (1.8288e2)        // cm
#define GSL_CONST_CGSM_MIL (2.54e-3)    // cm
#define GSL_CONST_CGSM_POINT (3.52777777778e-2) // cm
#define GSL_CONST_CGSM_TEXPOINT (3.51459803515e-2)      // cm
#define GSL_CONST_CGSM_MICRON (1e-4)    // cm
#define GSL_CONST_CGSM_ANGSTROM (1e-8)  // cm
#define GSL_CONST_CGSM_HECTARE (1e8)    // cm^2
#define GSL_CONST_CGSM_ACRE (4.04685642241e7)   // cm^2
#define GSL_CONST_CGSM_BARN (1e-24)     // cm^2
#define GSL_CONST_CGSM_LITER (1e3)      // cm^3
#define GSL_CONST_CGSM_US_GALLON (3.78541178402e3)      // cm^3
#define GSL_CONST_CGSM_QUART (9.46352946004e2)  // cm^3
#define GSL_CONST_CGSM_PINT (4.73176473002e2)   // cm^3
#define GSL_CONST_CGSM_CUP (2.36588236501e2)    // cm^3
#define GSL_CONST_CGSM_FLUID_OUNCE (2.95735295626e1)    // cm^3
#define GSL_CONST_CGSM_TABLESPOON (1.47867647813e1)     // cm^3
#define GSL_CONST_CGSM_TEASPOON (4.92892159375e0)       // cm^3
#define GSL_CONST_CGSM_CANADIAN_GALLON (4.54609e3)      // cm^3
#define GSL_CONST_CGSM_UK_GALLON (4.546092e3)   // cm^3
#define GSL_CONST_CGSM_MILES_PER_HOUR (4.4704e1)        // cm / s
#define GSL_CONST_CGSM_KILOMETERS_PER_HOUR (2.77777777778e1)    // cm / s
#define GSL_CONST_CGSM_KNOT (5.14444444444e1)   // cm / s
#define GSL_CONST_CGSM_POUND_MASS (4.5359237e2) // g
#define GSL_CONST_CGSM_OUNCE_MASS (2.8349523125e1)      // g
#define GSL_CONST_CGSM_TON (9.0718474e5)        // g
#define GSL_CONST_CGSM_METRIC_TON (1e6) // g
#define GSL_CONST_CGSM_UK_TON (1.0160469088e6)  // g
#define GSL_CONST_CGSM_TROY_OUNCE (3.1103475e1) // g
#define GSL_CONST_CGSM_CARAT (2e-1)     // g
#define GSL_CONST_CGSM_UNIFIED_ATOMIC_MASS (1.660538782e-24)    // g
#define GSL_CONST_CGSM_GRAM_FORCE (9.80665e2)   // cm g / s^2
#define GSL_CONST_CGSM_POUND_FORCE (4.44822161526e5)    // cm g / s^2
#define GSL_CONST_CGSM_KILOPOUND_FORCE (4.44822161526e8)        // cm g / s^2
#define GSL_CONST_CGSM_POUNDAL (1.38255e4)      // cm g / s^2
#define GSL_CONST_CGSM_CALORIE (4.1868e7)       // g cm^2 / s^2
#define GSL_CONST_CGSM_BTU (1.05505585262e10)   // g cm^2 / s^2
#define GSL_CONST_CGSM_THERM (1.05506e15)       // g cm^2 / s^2
#define GSL_CONST_CGSM_HORSEPOWER (7.457e9)     // g cm^2 / s^3
#define GSL_CONST_CGSM_BAR (1e6)        // g / cm s^2
#define GSL_CONST_CGSM_STD_ATMOSPHERE (1.01325e6)       // g / cm s^2
#define GSL_CONST_CGSM_TORR (1.33322368421e3)   // g / cm s^2
#define GSL_CONST_CGSM_METER_OF_MERCURY (1.33322368421e6)       // g / cm s^2
#define GSL_CONST_CGSM_INCH_OF_MERCURY (3.38638815789e4)        // g / cm s^2
#define GSL_CONST_CGSM_INCH_OF_WATER (2.490889e3)       // g / cm s^2
#define GSL_CONST_CGSM_PSI (6.89475729317e4)    // g / cm s^2
#define GSL_CONST_CGSM_POISE (1e0)      // g / cm s
#define GSL_CONST_CGSM_STOKES (1e0)     // cm^2 / s
#define GSL_CONST_CGSM_STILB (1e0)      // cd / cm^2
#define GSL_CONST_CGSM_LUMEN (1e0)      // cd sr
#define GSL_CONST_CGSM_LUX (1e-4)       // cd sr / cm^2
#define GSL_CONST_CGSM_PHOT (1e0)       // cd sr / cm^2
#define GSL_CONST_CGSM_FOOTCANDLE (1.076e-3)    // cd sr / cm^2
#define GSL_CONST_CGSM_LAMBERT (1e0)    // cd sr / cm^2
#define GSL_CONST_CGSM_FOOTLAMBERT (1.07639104e-3)      // cd sr / cm^2
#define GSL_CONST_CGSM_CURIE (3.7e10)   // 1 / s
#define GSL_CONST_CGSM_ROENTGEN (2.58e-8)       // abamp s / g
#define GSL_CONST_CGSM_RAD (1e2)        // cm^2 / s^2
#define GSL_CONST_CGSM_SOLAR_MASS (1.98892e33)  // g
#define GSL_CONST_CGSM_BOHR_RADIUS (5.291772083e-9)     // cm
#define GSL_CONST_CGSM_NEWTON (1e5)     // cm g / s^2
#define GSL_CONST_CGSM_DYNE (1e0)       // cm g / s^2
#define GSL_CONST_CGSM_JOULE (1e7)      // g cm^2 / s^2
#define GSL_CONST_CGSM_ERG (1e0)        // g cm^2 / s^2
#define GSL_CONST_CGSM_BOHR_MAGNETON (9.27400899e-21)   // abamp cm^2
#define GSL_CONST_CGSM_NUCLEAR_MAGNETON (5.05078317e-24)        // abamp cm^2
#define GSL_CONST_CGSM_ELECTRON_MAGNETIC_MOMENT (9.28476362e-21)        // abamp cm^2
#define GSL_CONST_CGSM_PROTON_MAGNETIC_MOMENT (1.410606633e-23) // abamp cm^2
#define GSL_CONST_CGSM_FARADAY (9.64853429775e3)        // abamp s / mol
#define GSL_CONST_CGSM_ELECTRON_CHARGE (1.602176487e-20)        // abamp s
#define GSL_CONST_MKS_SPEED_OF_LIGHT (2.99792458e8)     // m / s
#define GSL_CONST_MKS_GRAVITATIONAL_CONSTANT (6.673e-11)        // m^3 / kg s^2
#define GSL_CONST_MKS_ASTRONOMICAL_UNIT (1.49597870691e11)      // m
#define GSL_CONST_MKS_LIGHT_YEAR (9.46053620707e15)     // m
#define GSL_CONST_MKS_PARSEC (3.08567758135e16) // m
#define GSL_CONST_MKS_GRAV_ACCEL (9.80665e0)    // m / s^2
#define GSL_CONST_MKS_ELECTRON_VOLT (1.602176487e-19)   // kg m^2 / s^2
#define GSL_CONST_MKS_MASS_ELECTRON (9.10938188e-31)    // kg
#define GSL_CONST_MKS_MASS_MUON (1.88353109e-28)        // kg
#define GSL_CONST_MKS_MASS_PROTON (1.67262158e-27)      // kg
#define GSL_CONST_MKS_MASS_NEUTRON (1.67492716e-27)     // kg
#define GSL_CONST_MKS_RYDBERG (2.17987196968e-18)       // kg m^2 / s^2
#define GSL_CONST_MKS_BOLTZMANN (1.3806504e-23) // kg m^2 / K s^2
#define GSL_CONST_MKS_MOLAR_GAS (8.314472e0)    // kg m^2 / K mol s^2
#define GSL_CONST_MKS_STANDARD_GAS_VOLUME (2.2710981e-2)        // m^3 / mol
#define GSL_CONST_MKS_MINUTE (6e1)      // s
#define GSL_CONST_MKS_HOUR (3.6e3)      // s
#define GSL_CONST_MKS_DAY (8.64e4)      // s
#define GSL_CONST_MKS_WEEK (6.048e5)    // s
#define GSL_CONST_MKS_INCH (2.54e-2)    // m
#define GSL_CONST_MKS_FOOT (3.048e-1)   // m
#define GSL_CONST_MKS_YARD (9.144e-1)   // m
#define GSL_CONST_MKS_MILE (1.609344e3) // m
#define GSL_CONST_MKS_NAUTICAL_MILE (1.852e3)   // m
#define GSL_CONST_MKS_FATHOM (1.8288e0) // m
#define GSL_CONST_MKS_MIL (2.54e-5)     // m
#define GSL_CONST_MKS_POINT (3.52777777778e-4)  // m
#define GSL_CONST_MKS_TEXPOINT (3.51459803515e-4)       // m
#define GSL_CONST_MKS_MICRON (1e-6)     // m
#define GSL_CONST_MKS_ANGSTROM (1e-10)  // m
#define GSL_CONST_MKS_HECTARE (1e4)     // m^2
#define GSL_CONST_MKS_ACRE (4.04685642241e3)    // m^2
#define GSL_CONST_MKS_BARN (1e-28)      // m^2
#define GSL_CONST_MKS_LITER (1e-3)      // m^3
#define GSL_CONST_MKS_US_GALLON (3.78541178402e-3)      // m^3
#define GSL_CONST_MKS_QUART (9.46352946004e-4)  // m^3
#define GSL_CONST_MKS_PINT (4.73176473002e-4)   // m^3
#define GSL_CONST_MKS_CUP (2.36588236501e-4)    // m^3
#define GSL_CONST_MKS_FLUID_OUNCE (2.95735295626e-5)    // m^3
#define GSL_CONST_MKS_TABLESPOON (1.47867647813e-5)     // m^3
#define GSL_CONST_MKS_TEASPOON (4.92892159375e-6)       // m^3
#define GSL_CONST_MKS_CANADIAN_GALLON (4.54609e-3)      // m^3
#define GSL_CONST_MKS_UK_GALLON (4.546092e-3)   // m^3
#define GSL_CONST_MKS_MILES_PER_HOUR (4.4704e-1)        // m / s
#define GSL_CONST_MKS_KILOMETERS_PER_HOUR (2.77777777778e-1)    // m / s
#define GSL_CONST_MKS_KNOT (5.14444444444e-1)   // m / s
#define GSL_CONST_MKS_POUND_MASS (4.5359237e-1) // kg
#define GSL_CONST_MKS_OUNCE_MASS (2.8349523125e-2)      // kg
#define GSL_CONST_MKS_TON (9.0718474e2) // kg
#define GSL_CONST_MKS_METRIC_TON (1e3)  // kg
#define GSL_CONST_MKS_UK_TON (1.0160469088e3)   // kg
#define GSL_CONST_MKS_TROY_OUNCE (3.1103475e-2) // kg
#define GSL_CONST_MKS_CARAT (2e-4)      // kg
#define GSL_CONST_MKS_UNIFIED_ATOMIC_MASS (1.660538782e-27)     // kg
#define GSL_CONST_MKS_GRAM_FORCE (9.80665e-3)   // kg m / s^2
#define GSL_CONST_MKS_POUND_FORCE (4.44822161526e0)     // kg m / s^2
#define GSL_CONST_MKS_KILOPOUND_FORCE (4.44822161526e3) // kg m / s^2
#define GSL_CONST_MKS_POUNDAL (1.38255e-1)      // kg m / s^2
#define GSL_CONST_MKS_CALORIE (4.1868e0)        // kg m^2 / s^2
#define GSL_CONST_MKS_BTU (1.05505585262e3)     // kg m^2 / s^2
#define GSL_CONST_MKS_THERM (1.05506e8) // kg m^2 / s^2
#define GSL_CONST_MKS_HORSEPOWER (7.457e2)      // kg m^2 / s^3
#define GSL_CONST_MKS_BAR (1e5) // kg / m s^2
#define GSL_CONST_MKS_STD_ATMOSPHERE (1.01325e5)        // kg / m s^2
#define GSL_CONST_MKS_TORR (1.33322368421e2)    // kg / m s^2
#define GSL_CONST_MKS_METER_OF_MERCURY (1.33322368421e5)        // kg / m s^2
#define GSL_CONST_MKS_INCH_OF_MERCURY (3.38638815789e3) // kg / m s^2
#define GSL_CONST_MKS_INCH_OF_WATER (2.490889e2)        // kg / m s^2
#define GSL_CONST_MKS_PSI (6.89475729317e3)     // kg / m s^2
#define GSL_CONST_MKS_POISE (1e-1)      // kg m^-1 s^-1
#define GSL_CONST_MKS_STOKES (1e-4)     // m^2 / s
#define GSL_CONST_MKS_STILB (1e4)       // cd / m^2
#define GSL_CONST_MKS_LUMEN (1e0)       // cd sr
#define GSL_CONST_MKS_LUX (1e0) // cd sr / m^2
#define GSL_CONST_MKS_PHOT (1e4)        // cd sr / m^2
#define GSL_CONST_MKS_FOOTCANDLE (1.076e1)      // cd sr / m^2
#define GSL_CONST_MKS_LAMBERT (1e4)     // cd sr / m^2
#define GSL_CONST_MKS_FOOTLAMBERT (1.07639104e1)        // cd sr / m^2
#define GSL_CONST_MKS_CURIE (3.7e10)    // 1 / s
#define GSL_CONST_MKS_ROENTGEN (2.58e-4)        // A s / kg
#define GSL_CONST_MKS_RAD (1e-2)        // m^2 / s^2
#define GSL_CONST_MKS_SOLAR_MASS (1.98892e30)   // kg
#define GSL_CONST_MKS_BOHR_RADIUS (5.291772083e-11)     // m
#define GSL_CONST_MKS_NEWTON (1e0)      // kg m / s^2
#define GSL_CONST_MKS_DYNE (1e-5)       // kg m / s^2
#define GSL_CONST_MKS_JOULE (1e0)       // kg m^2 / s^2
#define GSL_CONST_MKS_ERG (1e-7)        // kg m^2 / s^2
#define GSL_CONST_MKS_BOHR_MAGNETON (9.27400899e-24)    // A m^2
#define GSL_CONST_MKS_NUCLEAR_MAGNETON (5.05078317e-27) // A m^2
#define GSL_CONST_MKS_ELECTRON_MAGNETIC_MOMENT (9.28476362e-24) // A m^2
#define GSL_CONST_MKS_PROTON_MAGNETIC_MOMENT (1.410606633e-26)  // A m^2
#define GSL_CONST_MKS_FARADAY (9.64853429775e4) // A s / mol
#define GSL_CONST_MKS_ELECTRON_CHARGE (1.602176487e-19) // A s
#define GSL_CONST_MKS_VACUUM_PERMITTIVITY (8.854187817e-12)     // A^2 s^4 / kg m^3
#define GSL_CONST_MKS_VACUUM_PERMEABILITY (1.25663706144e-6)    // kg m / A^2 s^2
#define GSL_CONST_MKS_GAUSS (1e-4)      // kg / A s^2

#endif

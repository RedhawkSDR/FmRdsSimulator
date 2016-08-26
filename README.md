# REDHAWK Basic Devices rh.FmRdsSimulator

## Description

Contains the source and build script for the REDHAWK Basic Devices
rh.FmRdsSimulator. This device is designed to be used in conjunction with the
libRfSimulators library.  Using the simulator library, this Front End Interfaces
compliant REDHAWK device will generate FM modulated mono or stereo audio with
RDS encoded PI (Call Sign), PS (Short Text), and RT (Full Text) data.

Additional documentation is available within the libRfSimulators README.

## Branches and Tags

All REDHAWK core assets use the same branching and tagging policy. Upon release,
the `master` branch is rebased with the specific commit released, and that
commit is tagged with the asset version number. For example, the commit released
as version 1.0.0 is tagged with `1.0.0`.

Development branches (i.e. `develop` or `develop-X.X`, where *X.X* is an asset
version number) contain the latest unreleased development code for the specified
version. If no version is specified (i.e. `develop`), the branch contains the
latest unreleased development code for the latest released version.

## REDHAWK Version Compatibility

| Asset Version | Minimum REDHAWK Version Required |
| ------------- | -------------------------------- |
| 2.x           | 2.0                              |
| 1.x           | 1.10                             |

## Building & Installation

This asset requires the libRfSimulators library. This must be installed in order
to build and run this asset.

    ./reconf
    ./configure
    make -j
    sudo make install

## Notes

The Simulator creates a processing thread for each station within the currently
visible 2.28 Mhz bandwidth (even if bandwidth is set smaller).  Since each of
these threads is resampling a wav file, FM modulating, encoding RDS, and
upsampling to 2.28 Msps a non-trivial amount of CPU is used. 

## FEI Compliance Test Results

See the [FEI Compliance Results](tests/FEI_Compliance_Results.md) document.

## Copyrights

This work is protected by Copyright. Please refer to the
[Copyright File](COPYRIGHT) for updated copyright information.

## License

REDHAWK Basic Devices rh.FmRdsSimulator is licensed under the GNU General Public
License (GPL).

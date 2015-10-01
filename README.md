# FM RDS Simulator Device

This REDHAWK device is designed to be used in conjunction with the RF Simulators Library.  Using the simulator library, this Front End Interfaces compliant REDHAWK device will generate FM modulated mono or stereo audio with RDS encoded PI (Call Sign), PS (Short Text), and RT (Full Text) data.

Additional documentation is available within the RF Simulator Libraries README.

## Building & Installation
This asset requires the libRfSimulators shared library. This must be installed in order to build and run this asset.

    ./reconf
    ./configure
    make -j
    sudo make install

## Notes

The Simulator creates a processing thread for each station within the currently visible 2.28 Mhz bandwidth (even if bandwidth is set smaller).  Since each of these threads is resampling a wav file, FM modulating, encoding RDS, and upsampling to 2.28 Msps a non-trivial amount of CPU is used. 

## Copyrights

This work is protected by Copyright. Please refer to the [Copyright File](COPYRIGHT) for updated copyright information.

## License

The FM RDS Simulator Device is licensed under the GNU General Public License (GPL).
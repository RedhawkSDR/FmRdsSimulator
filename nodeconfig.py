#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of rh.FmRdsSimulator Device.
# 
# rh.FmRdsSimulator Device is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# rh.FmRdsSimulator Device is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import os, sys, commands, logging, platform, shutil, socket
from ossie import parsers
from ossie.utils.model import _uuidgen as uuidgen

class ConfigurationError(StandardError):
    pass

class NodeConfig(object):
    def __init__(self, options, cmdlineProps):
        # Basic setup
        self._log = logging.getLogger('NodeConfig')
        self.localfile_nodeprefix = '/mgr'
        self.options = options
        self.cmdlineProps = cmdlineProps
        self.hostname = socket.gethostname()
        
        # check domainname
        if options.domainname == None:
            raise ConfigurationError("A domainname is required")
        
        # Verify the base rh.FmRdsSimulator profile exists
        self.fmrdssim_templates = {"spd": os.path.join(self.options.sdrroot, "dev", self.options.fmrdssimpath[1:], "FmRdsSimulator.spd.xml"),
                                   "prf": os.path.join(self.options.sdrroot, "dev", self.options.fmrdssimpath[1:], "FmRdsSimulator.prf.xml"),
                                   "scd": os.path.join(self.options.sdrroot, "dev", self.options.fmrdssimpath[1:], "FmRdsSimulator.scd.xml")}

        for template in self.fmrdssim_templates.values():
            if not os.path.exists(template):
                raise ConfigurationError("%s missing" % template)
                
        self.nodedir = os.path.join(self.options.sdrroot, "dev", "nodes", self.options.nodename.replace('.','/'))
        self.path_to_dcd = os.path.join(self.nodedir , "DeviceManager.dcd.xml")
            
        # Figure out where we are going to write the FmRdsSimulator profile
        if self.options.inplace:
            self.fmrdssim_path = os.path.join(self.options.sdrroot, "dev", "devices", "rh", "FmRdsSimulator")
        else:
            self.fmrdssim_path = os.path.join(self.nodedir, "FmRdsSimulator")
            
        # prep uuids
        self.uuids = {}
        self.uuids["softpkg"                ] = 'DCE:' + uuidgen()
        self.uuids["implementation"         ] = 'DCE:' + uuidgen()
        self.uuids["deviceconfiguration"    ] = 'DCE:' + uuidgen()
        self.uuids["componentfile"          ] = 'DCE:' + uuidgen()
        self.uuids["componentinstantiation" ] = 'DCE:' + uuidgen()
        self.uuids["componentimplementation"] = 'DCE:' + uuidgen()
        self.uuids["componentsoftpkg"       ] = 'DCE:' + uuidgen()
        
        self.props = {}

    def register(self):
        if not self.options.silent:
            self._log.debug("Registering...")
        self._gather_fmrdssim_information()
        self._createDeviceManagerProfile()
        self._updateFmRdsSimulatorProfile()
    
    def unregister(self):
        if not self.options.silent:
            self._log.debug("Unregistering...")
        if os.path.isdir(self.nodedir):
            if not self.options.silent:
                self._log.debug("  Removing <" + self.nodedir + ">")
            shutil.rmtree(self.nodedir)
         
    def _ver2rel(self, ver):
        return float(ver[0:1]) + float(ver[2:3])*0.1 + float(ver[4:5])*0.000001

    def _gather_fmrdssim_information(self):
        if not self.options.silent:
            self._log.debug("Checking FmRdsSimulator capacity...")

        self.props["PathToConfiguration"] = self.options.configpath
        #self.props["noiseSigma"] = self.options.noisesigma
        #self.props["addAWGN"] = self.options.addnoise

    def _createDeviceManagerProfile(self):
        #####################
        # Setup environment
        #####################

        # make sure node hasn't already been created
        if os.path.exists(self.path_to_dcd):
            self._log.error("Cannot 'register' new dynamicnode. A previous configuration was found. Please 'unregister' dynamicnode first.")
            sys.exit(1)

        try:
            if not os.path.isdir(self.nodedir):
                os.makedirs(self.nodedir)
            else:
                if not self.options.silent:
                    self._log.debug("Node directory already exists; skipping directory creation")
                pass
        except OSError:
            raise Exception, "Could not create device manager directory"

        FmRdsSimulator_componentfile = 'rh.FmRdsSimulator_' + uuidgen()
        if self.options.inplace:
            compfiles = [{'id':FmRdsSimulator_componentfile, 'localfile':os.path.join('/devices', 'rh', 'FmRdsSimulator', 'FmRdsSimulator.spd.xml')}]
        else:
            compfiles = [{'id':FmRdsSimulator_componentfile, 'localfile':os.path.join('/nodes', self.options.nodename.replace('.','/'), 'FmRdsSimulator', 'FmRdsSimulator.spd.xml')}]
        compplacements = [{'refid':FmRdsSimulator_componentfile, 'instantiations':[{'id':self.uuids["componentinstantiation"], 'usagename':'rh.FmRdsSimulator_' + self.hostname.replace('.', '_')}]}]

        #####################
        # DeviceManager files
        #####################
        if not self.options.silent:
            self._log.debug("Creating DeviceManager profile <" + self.options.nodename + ">")
        
        # set deviceconfiguration info
        _dcd = parsers.DCDParser.deviceconfiguration()
        _dcd.set_id(self.uuids["deviceconfiguration"])
        _dcd.set_name(self.options.nodename)
        _localfile = parsers.DCDParser.localfile(name=os.path.join(self.localfile_nodeprefix, 'DeviceManager.spd.xml'))
        _dcd.devicemanagersoftpkg = parsers.DCDParser.devicemanagersoftpkg(localfile=_localfile)
        
        # add componentfiles and componentfile(s)
        _dcd.componentfiles = parsers.DCDParser.componentfiles()
        for in_cf in compfiles:
            cf = parsers.DCDParser.componentfile(type_='SPD', id_=in_cf['id'], localfile=parsers.DCDParser.localfile(name=in_cf['localfile']))
            _dcd.componentfiles.add_componentfile(cf)

        # add partitioning/componentplacements
        _dcd.partitioning = parsers.DCDParser.partitioning()
        for in_cp in compplacements:
            _comp_fileref = parsers.DCDParser.componentfileref(refid=in_cp['refid'])
            _comp_placement = parsers.DCDParser.componentplacement(componentfileref=_comp_fileref)
            for ci in in_cp['instantiations']:
                comp_inst = parsers.DCDParser.componentinstantiation(id_=ci['id'], usagename=ci['usagename'])
                _comp_placement.add_componentinstantiation(comp_inst)
            _dcd.partitioning.add_componentplacement(_comp_placement)

        # add domainmanager lookup
        if self.options.domainname:
            _tmpdomainname = self.options.domainname + '/' + self.options.domainname
            
        _dcd.domainmanager = parsers.DCDParser.domainmanager(namingservice=parsers.DCDParser.namingservice(name=_tmpdomainname))
        dcd_out = open(self.path_to_dcd, 'w')
        dcd_out.write(parsers.parserconfig.getVersionXML())
        _dcd.export(dcd_out,0)
        dcd_out.close()
        
    def _updateFmRdsSimulatorProfile(self):
        #####################
        # FmRdsSimulator files
        #####################
        
        if not self.options.silent:
            self._log.debug("Creating rh.FmRdsSimulator profile <" + self.fmrdssim_path + ">")
            
        if not self.options.inplace:
            if not os.path.exists(self.fmrdssim_path):
                os.mkdir(self.fmrdssim_path)
            for f in self.fmrdssim_templates.values():
                shutil.copy(f, self.fmrdssim_path)
                
        self._updateFmRdsSimulatorSpd()
        self._updateFmRdsSimulatorPrf()
    
    def _updateFmRdsSimulatorSpd(self):
        # update the spd file
        spdpath = os.path.join(self.fmrdssim_path, 'FmRdsSimulator.spd.xml')
        _spd = parsers.SPDParser.parse(spdpath)
        _spd.set_id(self.uuids["componentsoftpkg"])
        _spd.implementation[0].set_id(self.uuids["componentimplementation"])

        # update the FmRdsSimulator code entry if this wasn't an inplace update
        if not self.options.inplace:
            code = _spd.get_implementation()[0].get_code()
            new_entrypoint = os.path.normpath(os.path.join(self.options.fmrdssimpath, code.get_entrypoint()))
            new_localfile = os.path.normpath(os.path.join(self.options.fmrdssimpath, code.get_localfile().get_name()))
            code.set_entrypoint(new_entrypoint)
            code.get_localfile().set_name(new_localfile)
            
        spd_out = open(spdpath, 'w')
        spd_out.write(parsers.parserconfig.getVersionXML())
        _spd.export(spd_out,0, name_='softpkg')
        spd_out.close()
        
    def _updateFmRdsSimulatorPrf(self):
        # generate the prf file
        prfpath = os.path.join(self.fmrdssim_path, 'FmRdsSimulator.prf.xml')
        _prf = parsers.PRFParser.parse(prfpath)

	    # Set the parameters for the target_device
        for simple in _prf.get_simple():
            if simple.get_name() in self.props:
                simple.set_value(str(self.props[simple.get_name()]))

        prf_out = open(prfpath, 'w')
        prf_out.write(parsers.parserconfig.getVersionXML())
        _prf.export(prf_out,0)
        prf_out.close()
        

        
###########################
# Run from command line
###########################
if __name__ == "__main__":

    ##################
    # setup arg parser
    ##################
    from optparse import OptionParser
    parser = OptionParser()
    #parser.usage = "%s [options] [simple_prop1 simple_value1]..."
    parser.add_option("--domainname", dest="domainname", default=None,
                      help="Must give a domain name")
    # noiseSigma = 0.1 (float)
    # addAWGN = True
    parser.add_option("--configpath", dest="configpath", default="/usr/share/libFmRdsSimulator/examples",
                      help="Path to the libRfSimulator configuration, default '/usr/share/libFmRdsSimulator/examples'")
    parser.add_option("--sdrroot", dest="sdrroot", default=os.path.expandvars("${SDRROOT}"),
                      help="Path to the sdr root; if none is given, ${SDRROOT} is used.")
    parser.add_option("--nodename", dest="nodename", default="rh.DevMgr_FmRdsSimulator_%s" % socket.gethostname(),
                      help="Desired nodename, if none is given rh.DevMgr_FmRdsSimulator_${HOST} is used")
    parser.add_option("--inplace", dest="inplace", default=False, action="store_true",
                      help="Update the rh.FmRdsSimulator profile in-place; default is to create a rh.FmRdsSimulator configuration in the node folder")
    parser.add_option("--fmrdssimpath", dest="fmrdssimpath", default="/devices/rh/FmRdsSimulator",
                      help="The device manager file system absolute path to the rh.FmRdsSimulator, default '/devices/rh/FmRdsSimulator'")
    parser.add_option("--silent", dest="silent", default=False, action="store_true",
                      help="Suppress all logging except errors")
    parser.add_option("--clean", dest="clean", default=False, action="store_true",
                      help="Clean up the previous configuration for this node first (delete entire node)")
    parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true",
                      help="Enable verbose logging")

    (options, args) = parser.parse_args()

    # Configure logging
    logging.basicConfig(format='%(name)-12s:%(levelname)-8s: %(message)s', level=logging.INFO)
    if options.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    # grab tmp logger until class is created
    _log = logging.getLogger('NodeConfig')

    if len(args) % 2 == 1:
        _log.error("Invalid command line arguments - properties must be specified with values")
        sys.exit(1)
    cmdlineProps = {}
    for i in range(len(args)):
        if i % 2 == 0:
            cmdlineProps[args[i]] = args[i + 1]

    # create instance of NodeConfig
    try:
        dn = NodeConfig(options, cmdlineProps)
        if options.clean:
            dn.unregister()
        dn.register()
        if not options.silent:
            _log.info("FmRdsSimulator node registration is complete")
    except ConfigurationError, e:
        _log.error("%s", e)
        sys.exit(1)

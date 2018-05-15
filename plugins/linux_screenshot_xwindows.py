"""
@author:       Fabio Pagani, Hamdi AMMAR, Ahmed MKADEM
@license:      GNU General Public License 2.0
@contact:      pagani@eurecom.fr
@organization: Eurecom (FR)
"""
import volatility.debug as debug
import volatility.plugins.linux.pslist as linux_pslist
import volatility.plugins.linux.dump_map as linux_dump_map
import volatility.plugins.linux.proc_maps as linux_proc_maps
import volatility.plugins.linux_xwindows as linux_xwindows
import volatility.plugins.linux.common as linux_common
import subprocess
import StringIO
import tempfile
import os.path


class linux_screenshot_xwindows(linux_common.AbstractLinuxCommand):
    def __init__(self, config, *args, **kwargs):
        
        linux_common.AbstractLinuxCommand.__init__(self, config, *args, **kwargs)    
        config.add_option('OUT-DIR', short_option='', default=None, help='Output dir', action='store', type='str')
        
        
    def calculate(self):
        if (not self._config.OUT_DIR or 
            not os.path.isdir(self._config.OUT_DIR)):
            debug.error("Please specify an existing output dir (--out-dir)")

        if (not os.path.isfile("./loader")):
            debug.error("Cannot find 'loader' inside current working directory, compile it or `cd` in the right directory")

        # Extract mappings, kind of ugly but i did not find another way to call the modules..
        for p in linux_pslist.linux_pslist(self._config).calculate():
            pname = str(p.comm)
            if pname == 'X' or pname == 'Xorg':
                break
        
        tmp_dir = tempfile.mkdtemp()
        self._config.add_option("PID")
        self._config.add_option("DUMP_DIR")
        self._config.PID = str(p.pid)
        self._config.DUMP_DIR = tmp_dir
        out = StringIO.StringIO()
        data = linux_proc_maps.linux_proc_maps(self._config).calculate()
        print("[+] Dumping Xorg memory mappings in %s" % tmp_dir)
        linux_dump_map.linux_dump_map(self._config).render_text(out, data)
        
        # Extract screenshots
        xw = linux_xwindows.linux_xwindows(self._config)
        for msg, screen_windows, screen_info in xw.calculate():
            for screen_id, win in screen_windows:
                # print('{} on Screen {}\n'.format(str(win), screen_id))
            	if win.drawable.type==0 and win.drawable.width > 150 and win.drawable.height > 150:
                    out = StringIO.StringIO()
                    xw.visit_property(win.optional.dereference().userProps.dereference(),
                                      out)

                    loader = ["./loader",
                              str(win.drawable), str(win.drawable.pScreen.visuals),
                              str(win.borderWidth), str(screen_info.imageByteOrder),
                              str(screen_info.bitmapScanlineUnit), str(screen_info.bitmapScanlinePad),
                              str(screen_info.bitmapBitOrder), str(win.drawable.pScreen.GetImage),
                              str(tmp_dir), str(self._config.OUT_DIR)]
                    print("[+] Calling %s" % ' '.join(loader))
                    subprocess.check_call(loader)

        
    def render_text(self, outfd, data):
        return

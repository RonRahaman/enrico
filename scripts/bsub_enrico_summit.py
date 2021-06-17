#!/usr/bin/env python3

# Based on vendor/nekRS/scripts/nrsqsub_summit
import os
import shutil
import subprocess
import datetime
from configparser import ConfigParser
from xml.etree import ElementTree
from enums import Enum

Driv = Enum('Driv', dict(HEAT='heat_fluids', NEUT='neutronics'})

class JsParams:
    
    def __init__(self, nodes, gpu_per_node=6, cpu_per_node=42, rs_per_node=6):

        self.nodes = nodes
        self.gpu_per_node = gpu_per_node
        self.cpu_per_node = cpu_per_node
        self.rs_per_node = rs_per_node

    @property
    def gpu_per_task(self):
        return 1

    @property
    def rs(self):
        return self.nodes * self.rs_per_node

    @property
    def gpu_per_rs(self):
        return self.gpu_per_node // self.rs_per_node

    @property
    def cpu_per_rs(self):
        return self.cpu_per_node // self.rs_per_node

    @property
    def tasks_per_rs(self):
        return self.gpu_per_rs // self.gpu_per_task

    def __str__(self):
        rs = self.nodes * self.rs_per_node

        tasks_per_rs = gpu_per_rs // gpu_per_task
        cpu_per_rs = cpu_per_node // rs_per_node
        return f"-smpiargs='gpu' -X 1 -n{rs} -r{self.rs_per_node} -a{self.tasks_per_rs} -c{self.cpu_per_rs} -g{self.gpu_per_rs} " \
               f"-b rs -d packed"

class BsubSummitDriver:

    def __init__(self, nodes, time, proj_id=None, ignore_warnings=False):
        self.casename = None
        self.time = time
        self.proj_id = proj_id
        self.ignore_warnings = ignore_warnings
        self.occa_cache_dir = os.getcwd() + "/.cache/occa"
        self.nvme_home = "/mnt/bb/" + os.environ["USER"]
        self.xl_home = "/sw/summit/xl/16.1.1-3/xlC/16.1.1"
        self.jsparams = JsParams(nodes=nodes)

        self.setup_env()
        self.setup_params()

    def setup_env(self):
        os.makedirs(self.occa_cache_dir, exist_ok=True)

        self.nekrs_home = os.environ.get('NEKRS_HOME')
        if not self.nekrs_home:
            raise RuntimeError("NEKRS_HOME was not defined in environment")
        if not os.path.isdir(self.nekrs_home):
            raise NotADirectoryError("NEKRS_HOME was set to {self.nekrs_home}, but that directory doesn't exist")
        nekrs_env = { "OCCA_CACHE_DIR" : self.occa_cache_dir, 
                "NEKRS_HYPRE_NUM_THREADS" : "1", 
                "OGS_MPI_SUPPORT" : "1", 
                "OCCA_CXX" : f"{self.xl_home}/bin/xlc", 
                "OCCA_CXXFLAGS" : "-O3 -qarch=pwr9 -qhot -DUSE_OCCA_MEM_BYTE_ALIGN=64", 
                "OCCA_LDFLAGS" : f"{self.xl_home}/lib/libibmc++.a" }
        os.environ.update(nekrs_env)

    def setup_params(self):

        warns = []

        # ===================
        # enrico.xml options
        # ===================

        enrico_xml = ElementTree.parse("enrico.xml")
        root = enrico_xml.getroot()

        # nekRS options
        # -------------
        self.casename = root.find(Driv.HEAT).find('casename').text

        procs_per_node = root.find(Driv.HEAT).find('procs_per_node').text
        if procs_per_node and int(procs_per_node) != self.jsparams.tasks_per_node:
            warns.append(f'You should set <{Driv.HEAT}><procs_per_node> to ')

        # OpenMC options
        # --------------

        procs_per_node = root.find(Driv.NEUT).find('procs_per_node').text
        if procs_per_node and int(procs_per_node) != self.jsparams.tasks_per_node:
            warns.append(f'You should set <{Driv.HEAT}><procs_per_node> to 6')





        # =====================
        # nekRS parfile options
        # =====================
        
        par = ConfigParser(inline_comment_prefixes=('#',';'))
        par.optionxform = str
        parfile = self.casename + ".par"
        par.read(parfile)

        for name, val in [['backend', 'CUDA'], ['deviceNumber', '0']]:
            if par.get('OCCA', name, fallback='') != val:
                warns.append(f'You should set ["OCCA"]["{name}"] to "{val} in {parfile}')



        if rewrite:
            backup_file = "{}.{}.bk".format(parfile, datetime.datetime.now().isoformat().replace(':', '.'))
            print(f"Rewriting {parfile}.  Original will be moved to {backup_file}")
            shutil.copy2(parfile, backup_file)
            with open(parfile, "w") as f:
                par.write(f)


    def precompile(self):
        cmd = f"mpirun -pami_noib -np 1 {self.nekrs_home}/bin/nekrs --setup {self.casename} --build-only {self.n_tasks} --backend {self.backend}"
        print("Running precompile command:", cmd)
        # Timeout is 2 hours.  This won't be run in bsub, so I'm using an explicit timeout
        subprocess.run(cmd, shell=True, check=True, timeout=7200)

    def bsub(self):
        enrico_bin = f"{self.nekrs_home}/bin/enrico"
        if self.backend == "SERIAL":
            jsrun_cmd = f"jsrun -X 1 -n{self.n_nodes} -r1 -a1 -c1 -g0 -b packed:1 -d packed cp -a {self.occa_cache_dir}/* {self.nvme_home}; export OCCA_CACHE_DIR={self.nvme_home}; jsrun -X 1 -n{self.n_tasks} -a{self.cores_per_socket} -c{self.cores_per_socket} -g0 -b packed:1 -d packed {enrico_bin}" 
        else: # backend == "CUDA"
            jsrun_cmd = f"jsrun -X 1 -n{self.n_nodes} -r1 -a1 -c1 -g0 -b packed:1 -d packed cp -a {self.occa_cache_dir}/* {self.nvme_home}; export OCCA_CACHE_DIR={self.nvme_home}; jsrun --smpiargs='-gpu' -X 1 -n{self.n_tasks} -r{self.gpus_per_node} -a1 -c2 -g1 -b rs -d packed {enrico_bin}" 
        bsub_cmd = f'bsub -nnodes {self.n_nodes} -alloc_flags NVME -W {self.time} -P {self.proj_id} -J nekRS_{self.casename} "{jsrun_cmd}"'

        print("Running bsub command:", bsub_cmd)
        subprocess.run(bsub_cmd, shell=True, check=True)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--nodes", type=int, required=True, help="Specify the number of NODES (not processes) to use")
    parser.add_argument("-t", "--time", type=str, required=True, help="Sets the runtime limit of the job")
    parser.add_argument("-b", "--backend", type=str, help="Sets the OCCA kernel backend [default: CUDA]", choices=['CUDA', 'SERIAL'], default='CUDA')
    parser.add_argument("--no-precompile", action="store_true", help="Skips pre-compile step for NekRS kernels")
    args = parser.parse_args()

    driver = BsubSummitDriver(n_nodes=args.nodes, time=args.time, backend=args.backend)
    if not args.no_precompile:
        driver.precompile()
    driver.bsub()


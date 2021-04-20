import pytest
import shutil
import subprocess
import os
from xml.etree import ElementTree
from collections import namedtuple

CaseParam = namedtuple('CaseParam',
                       ['neutronics_driver', 'heat_fluids_driver', 'casename', 'casedir',
                        'cmake_opts', 'restart'])


def pytest_generate_tests(metafunc):

    # For each case, get its name, directory, and CMake options
    root = ElementTree.parse("ci_pytest/pytest_config.xml").getroot()
    argvalues = []
    ids = []
    for drivers in root.findall("drivers"):

        neutronics_driver = drivers.get("neutronics")
        heat_fluids_driver = drivers.get("heat_fluids")

        for case in drivers.findall("case"):
            casename = case.get("name")
            casedir = os.path.abspath(case.get("dir"))
            restart = case.get("restart")

            cmake_opts = []
            if heat_fluids_driver == "nek5000":
                cmake_opts.extend(("-DNEK_DIST=nek5000", f"-DUSR_LOC={casedir}"))
            elif heat_fluids_driver == "nekrs":
                cmake_opts.append("-DNEK_DIST=nekrs")
            else:
                cmake_opts.append("-DNEK_DIST=none")

            argvalues.append(CaseParam(
                neutronics_driver=neutronics_driver,
                heat_fluids_driver=heat_fluids_driver,
                casename=casename,
                casedir=casedir,
                cmake_opts=cmake_opts,
                restart=restart))
            ids.append(f"{neutronics_driver} + {heat_fluids_driver} : {casename}")

    metafunc.parametrize("case_params", argvalues, ids=ids, indirect=True)


def get_build_dir(par):
    return f"build-pytest-{par.neutronics_driver}-{par.heat_fluids_driver}-{par.casename}"


@pytest.fixture
def case_params(request):
    return request.param


@pytest.fixture(autouse=True)
def unpack_restart(case_params):
    if case_params.restart:
        shutil.unpack_archive(case_params.restart, case_params.casedir)


@pytest.fixture
def run_cmake(case_params):
    build_dir = get_build_dir(case_params)
    # TODO: Allow this to be executed from arbitrary dir
    cmakelists_dir = os.getcwd()
    try:
        os.mkdir(build_dir)
    except FileExistsError:
        pass
    cmd = ["cmake"] + case_params.cmake_opts + [cmakelists_dir]
    print(cmd)
    subprocess.run(cmd, cwd=build_dir, check=True)


@pytest.fixture
def run_make(case_params, run_cmake):
    subprocess.run(["make", "-j8", "install"], cwd=get_build_dir(case_params), check=True)


@pytest.fixture
def run_case(case_params, run_make):
    enrico = os.path.abspath(os.path.join(get_build_dir(case_params), "install", "bin", "enrico"))
    subprocess.run(["mpirun", "-np", "32", enrico], cwd=case_params.casedir, check=True)


def test_verify_case(case_params, run_case):
    assert True

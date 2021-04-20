import pytest
import os
from xml.etree import ElementTree
from collections import namedtuple

CaseParam = namedtuple('CaseParam',
                       ['neutronics_driver', 'heat_fluids_driver', 'casename', 'casedir',
                        'cmake_opts'])


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
            casedir = case.get("dir")
            cmake_opts = ""
            if drivers.get("heat_fluids_driver") == "nek5000":
                cmake_opts = f"{cmake_opts} -DNEK_DIST=nek5000 -DUSR_LOC={casedir}"
            elif drivers.get("heat_fluids_driver") == "nekrs":
                cmake_opts = f"{cmake_opts} -DNEK_DIST=nekrs"
            else:
                cmake_opts = f"{cmake_opts} -DNEK_DIST=none"
            argvalues.append(CaseParam(
                neutronics_driver=neutronics_driver,
                heat_fluids_driver=heat_fluids_driver,
                casename=casename,
                casedir=casedir,
                cmake_opts=cmake_opts))
            ids.append(f"{neutronics_driver} + {heat_fluids_driver} : {casename}")

    metafunc.parametrize("run_cmake", argvalues, ids=ids, indirect=True)


@pytest.fixture(scope="module")
def download_xs():
    print("<<< download xs >>>")


@pytest.fixture(scope="module")
def unzip_nek_statefile():
    print("<<< unzip nek statefile >>>")

@pytest.fixture(scope="module")
def patch_nek_input():
    print("<<< unzip nek input >>>")


@pytest.fixture
def run_cmake(request):
    print(request.param)
    return request.param


@pytest.fixture
def run_make(run_cmake):
    print(run_cmake)
    return run_cmake


def test_run(run_make):
    print(run_make)
    assert True

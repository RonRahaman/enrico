import pytest


@pytest.fixture(scope="module")
def download_xs():
    print("<<< download xs >>>")

@pytest.fixture(scope="module")
def unzip_nek_statefile():
    print("<<< unzip nek statefile >>>")


@pytest.fixture(scope="module")
def patch_nek_input():
    print("<<< unzip nek input >>>")


class TestOpenmcSurrogate:
    @pytest.fixture(scope="class", autouse=True)
    def get_input_data(self, download_xs):
        pass

    @pytest.fixture(scope="class")
    def configure(self):
        print("<<< configure OpenMC + surrogate >>>")

    @pytest.fixture(scope="class", autouse=True)
    def compile(self, configure):
        print("<<< compiling OpenMC + surrogate >>>")

    def test_shortrod(self):
        print("<<< testing OpenMC + surrogate shortrod >>>")
        assert True

    def test_longrod(self):
        print("<<< testing OpenMC + surrogate longrod >>>")
        assert True


class TestOpenmcNek5000:
    @pytest.fixture
    def nek_case_info(self):
        print("<<< ran nek_case_info >>>")
        return {}

    @pytest.fixture
    def compile(self, nek_case_info):
        # Assert that nek_case_info has some usable values?
        print(f"<<< in compile, got nek_case_info = {nek_case_info} >>>")

    @pytest.fixture
    def run(self, nek_case_info, compile):
        print(f"<<< in run, got nek_case_info = {nek_case_info} >>>")

    @pytest.fixture
    def use_short_case(self, nek_case_info):
        nek_case_info["casedir"] = "tests/singlerod/short/nek5000"
        nek_case_info["casename"] = "rodcht"
        print("<<< ran use_short_case >>>")
        return nek_case_info

    @pytest.fixture
    def use_long_case(self, nek_case_info):
        nek_case_info["casedir"] = "tests/singlerod/long/nek5000"
        nek_case_info["casename"] = "rod_l"
        print("<<< ran use_long_case >>>")
        return nek_case_info

    def test_short(self, use_short_case, run):
        assert True

    def test_long(self, use_long_case, run):
        assert True

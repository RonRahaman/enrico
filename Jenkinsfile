/*
node('bigmem') {
  withEnv(["OMP_NUM_THREADS=2",
           "OPENMC_CROSS_SECTIONS=${env.HOME}/endfb71_hdf5/cross_sections.xml",
           "MODE=openmc_nek5000",
           "PATH=/soft/apps/packages/git-2.10.1/bin:/soft/apps/packages/cmake-3.12.4/bin:/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/bin:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/bin:/soft/apps/packages/gcc/gcc-6.2.0/bin:/usr/lib/lightdm/lightdm:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/mcs/bin:/usr/local/bin:/software/common/bin:/soft/apps/bin:/soft/gnu/bin:/soft/com/bin:/soft/adm/bin:/homes/rahaman/bin/linux-Ubuntu_14.04-x86_64:${env.PATH}", 
           "LD_LIBRARY_PATH=/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/lib:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/lib:/soft/apps/packages/gcc/gcc-6.2.0/lib64",
           "LIBRARY_PATH=/usr/lib/x86_64-linux-gnu" ]) {
    stage('clone') {
        checkout([$class: 'GitSCM', branches: [[name: '*//*
jenkins-setup']],
        doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption',
        disableSubmodules: false, parentCredentials: false, recursiveSubmodules: true, reference: '',
        trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: [[url:
        'https://github.com/RonRahaman/enrico.git']]])
    }
    stage('before_install') {
      sh "${WORKSPACE}/ci/patch_singlerod_input.sh"
    }
    stage('install') {
      sh "${WORKSPACE}/ci/cmake_singlerod.sh"
      sh "${WORKSPACE}/ci/build_singlerod.sh"
      sh "${WORKSPACE}/ci/build_unittests.sh"
    }
    stage('before_script') {
      sh "${WORKSPACE}/ci/download_xs.sh"
      sh "${WORKSPACE}/ci/unzip_singlerod_statefile.sh"
    }
    stage('test') {
      sh "${WORKSPACE}/ci/singlerod_matrix.sh"
      sh "${WORKSPACE}/ci/test_unittests.sh"
    }
  }
}
*/

node('bigmem') {
  withEnv(["OMP_NUM_THREADS=2",
           "OPENMC_CROSS_SECTIONS=${env.HOME}/endfb71_hdf5/cross_sections.xml",
           "MODE=openmc_nek5000",
           "CONDA_EXE=/homes/rahaman/miniconda3/bin/conda",
           "CONDA_PYTHON_EXE=/homes/rahaman/miniconda3/bin/python",
           "CONDA_SHLVL=0",
           "PYTHONPATH=",
           "PATH=/soft/apps/packages/Anaconda3-5.2.0/bin/:/soft/apps/packages/git-2.10.1/bin:/soft/apps/packages/cmake-3.12.4/bin:/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/bin:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/bin:/soft/apps/packages/gcc/gcc-6.2.0/bin:/usr/lib/lightdm/lightdm:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/mcs/bin:/usr/local/bin:/software/common/bin:/soft/apps/bin:/soft/gnu/bin:/soft/com/bin:/soft/adm/bin:/homes/rahaman/bin/linux-Ubuntu_14.04-x86_64:${env.PATH}",
           "LD_LIBRARY_PATH=/soft/apps/packages/Anaconda3-5.2.0/lib/:/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/lib:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/lib:/soft/apps/packages/gcc/gcc-6.2.0/lib64",
           "LIBRARY_PATH=/usr/lib/x86_64-linux-gnu" ]) {
    stage('clone') {
        checkout([$class: 'GitSCM', branches: [[name: 'pytest']],
        doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption',
        disableSubmodules: false, parentCredentials: false, recursiveSubmodules: true, reference: '',
        trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: [[url:
        'https://github.com/RonRahaman/enrico.git']]])
    }
    stage('pytest') {
      sh "pytest --junit-xml=enrico-junit.xml ci_pytest/"
    }
    stage('publish') {
        sh "ci_pytest/sanitize_junit_xml.py enrico-junit.xml"
        xunit([JUnit(deleteOutputFiles: false, failIfNotNew: false, pattern: 'enrico-junit.xml', skipNoTestFiles: false, stopProcessingIfError: false)])
    }
  }
}



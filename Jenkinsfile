node {
  withEnv(["OMP_NUM_THREADS=2",
           "OPENMC_CROSS_SECTIONS=${env.WORKSPACE}/endfb71_hdf5/cross_sections.xml",
           "MODE=openmc_nek5000",
           "PATH=/soft/apps/packages/git-2.10.1/bin:/soft/apps/packages/cmake-3.12.4/bin:/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/bin:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/bin:/soft/apps/packages/gcc/gcc-6.2.0/bin:/usr/lib/lightdm/lightdm:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/mcs/bin:/usr/local/bin:/software/common/bin:/soft/apps/bin:/soft/gnu/bin:/soft/com/bin:/soft/adm/bin:/homes/rahaman/bin/linux-Ubuntu_14.04-x86_64:${env.PATH}", 
           "LD_LIBRARY_PATH=/soft/apps/packages/climate/hdf5/1.8.16-parallel/gcc-6.2.0/lib:/soft/apps/packages/climate/mpich/3.2/gcc-6.2.0/lib:/soft/apps/packages/gcc/gcc-6.2.0/lib64",
           "LIBRARY_PATH=/usr/lib/x86_64-linux-gnu" ]) {
    stage('clone') {
        checkout([$class: 'GitSCM', branches: [[name: '*/jenkins-setup']],
        doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption',
        disableSubmodules: false, parentCredentials: false, recursiveSubmodules: true, reference: '',
        trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: [[url:
        'https://github.com/RonRahaman/enrico.git']]])
    }
    stage('before_install') {
      sh "${WORKSPACE}/enrico/ci/patch_singlerod_input.sh"
    }
  }
}

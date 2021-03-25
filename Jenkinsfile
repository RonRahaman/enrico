node {
  withEnv(["OMP_NUM_THREADS=2",
           "OPENMC_CROSS_SECTIONS=${env.WORKSPACE}/endfb71_hdf5/cross_sections.xml",
           "MODE=openmc_nek5000" ]) {
    stage('softenv') {
      sh 'echo "Hi there"'
      sh 'soft add +gcc-6.2.0'
      sh 'soft add +mpich-3.2-gcc-6.2.0'
      sh 'soft add +hdf5-1.8.16-gcc-6.2.0-mpich-3.2-parallel'
      sh 'soft add +cmake-3.12.4'
    }
    stage('before_install') {
      sh "${WORKSPACE}/ci/patch_singlerod_input.sh"
    }
  }
}

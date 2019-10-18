# CMake generated Testfile for 
# Source directory: /home/dlx/Desktop/cgp3-soln/test
# Build directory: /home/dlx/Desktop/cgp3-soln/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(smoketest "/home/dlx/Desktop/cgp3-soln/build/test/tilertest" "-v" "--test=commit")
set_tests_properties(smoketest PROPERTIES  WORKING_DIRECTORY "/home/dlx/Desktop/cgp3-soln/build")

#add_executable(oneconsensus_shortread main.cpp Align.cpp Time.cpp utils.cpp verification.cpp onejoin_dpcpp.cpp onecluster.cpp constants.cpp ../../AlignmentTools/edlib/src/edlib.cpp)
add_executable(oneconsensus_shortread main.cpp ../../../src/AlignmentTools/Aligner.cpp ../../../src/Common/Logger.cpp Time.cpp utils.cpp verification.cpp onejoin_dpcpp.cpp onecluster.cpp constants.cpp ../../../src/AlignmentTools/edlib/src/edlib.cpp)
set_target_properties(oneconsensus_shortread PROPERTIES COMPILE_FLAGS "-DSHORT_READ")
target_link_libraries(oneconsensus_shortread TBB::tbb ${Boost_LIBRARIES})


add_executable(oneconsensus_longread main.cpp ../../../src/AlignmentTools/Aligner.cpp ../../../src/Common/Logger.cpp Time.cpp utils.cpp verification.cpp onejoin_dpcpp.cpp onecluster.cpp constants.cpp ../../../src/AlignmentTools/edlib/src/edlib.cpp)
set_target_properties(oneconsensus_longread PROPERTIES COMPILE_FLAGS "-DLONG_READ")
target_link_libraries(oneconsensus_longread TBB::tbb ${Boost_LIBRARIES})
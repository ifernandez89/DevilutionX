file(REMOVE_RECURSE
  "liblibdevilutionx_so.dll"
  "liblibdevilutionx_so.dll.a"
  "liblibdevilutionx_so.dll.manifest"
  "liblibdevilutionx_so.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/libdevilutionx_so.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()

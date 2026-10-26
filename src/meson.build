sortbench_inc = include_directories('.')

sortbench_sources = files(
  'core/check.cpp',
  'core/dataset.cpp',
  'algorithms/complexity.cpp',
  'algorithms/registry.cpp',
  'algorithms/selection_sort.cpp',
  'algorithms/insertion_sort.cpp',
  'algorithms/bubble_sort.cpp',
  'algorithms/shell_sort.cpp',
  'algorithms/quick_sort.cpp',
  'algorithms/merge_sort.cpp',
  'algorithms/heap_sort.cpp',
  'bench/runner.cpp',
  'io/csv_writer.cpp',
)

sortbench_core = static_library(
  'sortbench_core',
  sortbench_sources,
  include_directories: sortbench_inc,
)

sortbench_dep = declare_dependency(
  link_with: sortbench_core,
  include_directories: sortbench_inc,
)

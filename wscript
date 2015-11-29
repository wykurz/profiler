top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.find_program('clang++-3.6', var='CXX', mandatory=True)
    conf.load('compiler_cxx')
    conf.env.CXXFLAGS = ['-std=c++11', '-O3', '-g', '-Wall']

def build(bld):
    # Separate unit tests, integration tests and benchmarks!
    bld.program(source=bld.path.ant_glob('tests/**/*.cpp'),
                target='tests/testDriver',
                stlib='boost_unit_test_framework',
                includes='.')

    bld.program(source='tests/Instrumentation/Benchmark.cpp',
                target='tests/benchmarks',
                lib='pthread',
                stlib='benchmark')

# TODO:
# clang -E [your -I flags] myfile.cpp > myfile_pp.cpp
# clang -cc1 -fdump-record-layouts myfile.cpp

from waflib import Logs
from waflib.Tools import waf_unit_test

top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx waf_unit_test')

def configure(conf):
    conf.find_program('clang++-3.6', var='CXX', mandatory=True)
    conf.load('compiler_cxx waf_unit_test')
    conf.env.CXXFLAGS = ['-std=c++14', '-O3', '-g', '-Wall']

def test_summary(bld):
    lst = getattr(bld, 'utest_results', [])
    for (f, code, out, err) in lst:
        if f:
            Logs.pprint('BLUE', '    %s' % f)
        if out:
            Logs.pprint('CYAN', '    %s' % out)
        if err:
            Logs.pprint('RED', '    %s' % err)

def build(bld):
    # TODO: build instrumentation lib...
    bld.shlib(source=bld.path.ant_glob('**/*.cpp', excl='tests/*'),
              target='profiler',
              includes='.')

    bld.program(features='test',
                source=bld.path.ant_glob('tests/unit/**/*.cpp'),
                target='unit_tests',
                stlib='boost_unit_test_framework',
                includes='.',
                use='profiler')

    # bld.program(features='test',
    #             source=bld.path.ant_glob('tests/integ/**/*.cpp'),
    #             target='integ_tests',
    #             stlib='boost_unit_test_framework',
    #             includes='.')

    bld.program(features='test',
                source=bld.path.ant_glob('tests/stress/**/*.cpp'),
                target='stress_tests',
                lib='pthread',
                stlib='boost_unit_test_framework',
                includes='.',
                use='profiler')

    bld.program(features='test',
                source=bld.path.ant_glob('tests/perf/**/*.cpp'),
                target='perf_tests',
                lib='pthread',
                stlib='benchmark',
                includes='.',
                use='profiler')

    bld.add_post_fun(test_summary)

# TODO:
# clang -E [your -I flags] myfile.cpp > myfile_pp.cpp
# clang -cc1 -fdump-record-layouts myfile.cpp

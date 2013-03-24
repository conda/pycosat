import sys
from distutils.core import setup, Extension


version = '0.1.1'


ext_kwargs = dict(
    name = "pycosat",
    sources = ["pycosat.c"],
    define_macros = [('PYCOSAT_VERSION', '"%s"' % version)],
)
if '--inplace' in sys.argv:
    ext_kwargs['define_macros'].append(('DONT_INCLUDE_PICOSAT', 1))
    ext_kwargs['library_dirs'] = ['.']
    ext_kwargs['libraries'] = ['picosat']


setup(
    name = "pycosat",
    version = version,
    author = "Ilan Schnell",
    author_email = "ilanschnell@gmail.com",
    url = "https://github.com/ilanschnell/pycosat",
    license = "MIT",
    classifiers = [
        "Development Status :: 2 - Pre-Alpha",
        "Intended Audience :: Developers",
        "Operating System :: OS Independent",
        "Programming Language :: C",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "Topic :: Utilities",
    ],
    ext_modules = [Extension(**ext_kwargs)],
    py_modules = ['test_pycosat'],
    description = "bindings to picosat",
    long_description = open('README.rst').read(),
)

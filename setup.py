import sys
from distutils.core import setup, Extension


ext_kwargs = dict(name = "pycosat",
                  sources = ["pycosat.c"])
if '--inplace' in sys.argv:
    ext_kwargs['define_macros'] = [('DONT_INCLUDE_PICOSAT', 1)]
    ext_kwargs['library_dirs'] = ['.']
    ext_kwargs['libraries'] = ['picosat']


setup(
    name = "pycosat",
    version = "0.1.0",
    author = "Ilan Schenll",
    author_email = "ilanschnell@gmail.com",
    url = "https://github.com/ilanschnell/pycosat",
    license = "MIT",
    classifiers = [
        "Development Status :: 1 - Planning",
        "Intended Audience :: Developers",
        "Operating System :: OS Independent",
        "Programming Language :: C",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "Topic :: Utilities",
    ],
    ext_modules = [Extension(**ext_kwargs)],
    description = "bindings to picosat",
    long_description = open('README.rst').read(),
)

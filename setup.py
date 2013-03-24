import sys
if 'develop' in sys.argv:
    import setuptools

from distutils.core import setup, Extension


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
        #"Programming Language :: Python :: 3",
        "Topic :: Utilities",
    ],
    ext_modules = [Extension(name = "pycosat",
                             sources = ["pycosat.c"])],
    description = "bindings to picosat",
    long_description = open('README.rst').read(),
)

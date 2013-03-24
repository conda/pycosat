try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension


setup(
    name = "pycosat",
    version = "0.0.1",
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
    description = "bindings to picosat",
    long_description = open('README.rst').read(),
)

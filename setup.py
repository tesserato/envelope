import pathlib
from setuptools import setup

# The directory containing this file
HERE = pathlib.Path(__file__).parent

# The text of the README file
README = (HERE / "README.md").read_text()

# This call to setup() does all the work
setup(
    name="signal-envelope",
    version="0.1.1",
    description="Extract the envelope of a digital signal",
    long_description=README,
    long_description_content_type="text/markdown",
    url="https://github.com/tesserato/envelope",
    author="Carlos Tarjano",
    author_email="tesserato@hotmail.com",
    license="MIT", # <| <| <| <|
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
    ],
    packages=["signal_envelope"],
    include_package_data=True,
    package_data={'signal-envelope': ['signal_envelope/*.dll']},
    # package_data={"dlls": ["..\signal_envelope\DLL.dll", "..\signal_envelope\libsndfile-1.dll"]},
    # data_files=[("", "\DLL.dll"), ("","\libsndfile-1.dll")],
    install_requires=["numpy"],
    # entry_points={
    #     "console_scripts": [
    #         "realpython=reader.__main__:main",
    #     ]
    # },
)